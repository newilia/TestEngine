from __future__ import annotations

import asyncio
import logging

from billiards_server.framing import read_frame, write_frame
from billiards_server.generated import BilliardSession_pb2
from billiards_server.sessions import FIXED_SESSION_ID, SessionManager

LOG = logging.getLogger(__name__)

RELAY_PAYLOAD_TYPES = frozenset(
    {
        "cue_aim_update",
        "table_state_update",
        "turn_result",
        "turn_started",
        "ball_in_hand_drag_started",
        "ball_in_hand_drag_ended",
        "cue_released",
    }
)


class BilliardSessionServer:
    def __init__(self, host: str, port: int) -> None:
        self._host = host
        self._port = port
        self._sessions = SessionManager()
        self._server: asyncio.Server | None = None

    async def start(self) -> None:
        self._server = await asyncio.start_server(self._handle_client, self._host, self._port)
        sockets = ", ".join(str(sock.getsockname()) for sock in self._server.sockets or ())
        LOG.info("BilliardSession server listening on %s", sockets)

    async def serve_forever(self) -> None:
        if self._server is None:
            raise RuntimeError("BilliardSessionServer.start() must be called first")
        async with self._server:
            await self._server.serve_forever()

    async def _handle_client(
        self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter
    ) -> None:
        peer = writer.get_extra_info("peername")
        LOG.info("Client connected: %s", peer)
        session_id: int | None = None
        client_id: int | None = None
        try:
            while True:
                payload = await read_frame(reader)
                envelope = BilliardSession_pb2.Envelope()
                envelope.ParseFromString(payload)
                response, registered, should_broadcast = await self._dispatch(
                    envelope, writer, session_id, client_id
                )
                if registered is not None:
                    session_id, client_id = registered
                if response is not None:
                    await write_frame(writer, response.SerializeToString())
                if should_broadcast and session_id is not None:
                    await self._broadcast_game_started(session_id)
        except asyncio.IncompleteReadError:
            LOG.info("Client disconnected: %s", peer)
        except Exception:
            LOG.exception("Client handler error: %s", peer)
        finally:
            removal = self._sessions.remove_client(writer)
            if removal.removed_client_id is not None:
                LOG.info(
                    "Client removed session_id=%s client_id=%s host=%s session_destroyed=%s",
                    removal.session_id,
                    removal.removed_client_id,
                    removal.was_host,
                    removal.session_destroyed,
                )
            if removal.peer_writer is not None and not removal.peer_writer.is_closing():
                removal.peer_writer.close()
            writer.close()
            try:
                await writer.wait_closed()
            except Exception:
                pass

    async def _broadcast_game_started(self, session_id: int) -> None:
        for client_id in self._sessions.get_client_ids(session_id):
            client_writer = self._sessions.get_client_writer(session_id, client_id)
            if client_writer is None:
                continue

            envelope = BilliardSession_pb2.Envelope()
            body = envelope.game_started
            body.session_id = session_id
            body.your_player_index = client_id - 1
            await write_frame(client_writer, envelope.SerializeToString())
            LOG.info(
                "GameStarted pushed session_id=%s client_id=%s player_index=%s",
                session_id,
                client_id,
                client_id - 1,
            )

    async def _relay_to_peer(
        self, envelope: BilliardSession_pb2.Envelope, session_id: int, sender_client_id: int
    ) -> None:
        peer_writer = self._sessions.get_peer_writer(session_id, sender_client_id)
        if peer_writer is None:
            return
        await write_frame(peer_writer, envelope.SerializeToString())

    async def _dispatch(
        self,
        envelope: BilliardSession_pb2.Envelope,
        writer: asyncio.StreamWriter,
        session_id: int | None,
        client_id: int | None,
    ) -> tuple[BilliardSession_pb2.Envelope | None, tuple[int, int] | None, bool]:
        payload_type = envelope.WhichOneof("payload")

        if payload_type == "create_session_request":
            request = envelope.create_session_request
            result = self._sessions.create_session(request.player_name, writer)
            response = BilliardSession_pb2.Envelope()
            body = response.create_session_response
            if isinstance(result[1], str):
                body.success = False
                body.error_message = result[1]
                LOG.warning("CreateSession failed peer=%s reason=%s", writer, result[1])
            else:
                sid, cid = result
                body.session_id = sid
                body.client_id = cid
                body.success = True
                LOG.info(
                    "CreateSession peer=%s player=%r session_id=%s client_id=%s",
                    writer.get_extra_info("peername"),
                    request.player_name,
                    sid,
                    cid,
                )
                return response, (sid, cid), False
            return response, None, False

        if payload_type == "join_session_request":
            request = envelope.join_session_request
            result = self._sessions.join_session(request.session_id, request.player_name, writer)
            response = BilliardSession_pb2.Envelope()
            body = response.join_session_response
            body.session_id = request.session_id
            if isinstance(result[1], str):
                body.success = False
                body.error_message = result[1]
                LOG.warning(
                    "JoinSession failed peer=%s player=%r session_id=%s reason=%s",
                    writer.get_extra_info("peername"),
                    request.player_name,
                    request.session_id,
                    result[1],
                )
            else:
                sid, cid = result
                body.session_id = sid
                body.client_id = cid
                body.success = True
                LOG.info(
                    "JoinSession peer=%s player=%r session_id=%s client_id=%s",
                    writer.get_extra_info("peername"),
                    request.player_name,
                    sid,
                    cid,
                )
                should_broadcast = self._sessions.is_session_ready(sid)
                return response, (sid, cid), should_broadcast
            return response, None, False

        if payload_type in RELAY_PAYLOAD_TYPES:
            if session_id is None or client_id is None:
                LOG.warning("Relay rejected: client not registered payload=%s", payload_type)
                return None, None, False
            await self._relay_to_peer(envelope, session_id, client_id)
            return None, None, False

        LOG.warning("Unhandled payload type=%r", payload_type)
        return None, None, False
