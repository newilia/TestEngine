from __future__ import annotations

import asyncio
import logging
from typing import Any

from billiards_server.framing import read_frame, write_frame
from billiards_server.generated import BilliardSession_pb2
from billiards_server.sessions import SessionManager

LOG = logging.getLogger(__name__)


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
        try:
            while True:
                payload = await read_frame(reader)
                envelope = BilliardSession_pb2.Envelope()
                envelope.ParseFromString(payload)
                response = self._dispatch(envelope, peer)
                if response is not None:
                    await write_frame(writer, response.SerializeToString())
        except asyncio.IncompleteReadError:
            LOG.info("Client disconnected: %s", peer)
        except Exception:
            LOG.exception("Client handler error: %s", peer)
        finally:
            writer.close()
            try:
                await writer.wait_closed()
            except Exception:
                pass

    def _dispatch(
        self, envelope: BilliardSession_pb2.Envelope, peer: Any
    ) -> BilliardSession_pb2.Envelope | None:
        payload_type = envelope.WhichOneof("payload")
        if payload_type == "create_session_request":
            request = envelope.create_session_request
            session_id, client_id = self._sessions.create_session(request.player_name)
            LOG.info(
                "CreateSession peer=%s player=%r session_id=%s client_id=%s",
                peer,
                request.player_name,
                session_id,
                client_id,
            )
            response = BilliardSession_pb2.Envelope()
            body = response.create_session_response
            body.session_id = session_id
            body.client_id = client_id
            body.success = True
            return response

        if payload_type == "join_session_request":
            request = envelope.join_session_request
            joined = self._sessions.join_session(request.session_id, request.player_name)
            response = BilliardSession_pb2.Envelope()
            body = response.join_session_response
            body.session_id = request.session_id
            if joined is None:
                body.success = False
                body.error_message = f"Session {request.session_id} not found"
                LOG.warning(
                    "JoinSession failed peer=%s player=%r session_id=%s reason=%s",
                    peer,
                    request.player_name,
                    request.session_id,
                    body.error_message,
                )
            else:
                session_id, client_id = joined
                body.session_id = session_id
                body.client_id = client_id
                body.success = True
                LOG.info(
                    "JoinSession peer=%s player=%r session_id=%s client_id=%s",
                    peer,
                    request.player_name,
                    session_id,
                    client_id,
                )
            return response

        LOG.warning("Unhandled payload type=%r from peer=%s", payload_type, peer)
        return None
