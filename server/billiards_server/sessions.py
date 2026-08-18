from __future__ import annotations

import asyncio
from dataclasses import dataclass, field

FIXED_SESSION_ID = 1
HOST_CLIENT_ID = 1
JOINER_CLIENT_ID = 2


@dataclass
class ConnectedClient:
    client_id: int
    player_name: str
    writer: asyncio.StreamWriter


@dataclass
class Session:
    session_id: int
    clients: dict[int, ConnectedClient] = field(default_factory=dict)


@dataclass
class RemoveClientResult:
    session_id: int | None = None
    removed_client_id: int | None = None
    was_host: bool = False
    session_destroyed: bool = False
    peer_writer: asyncio.StreamWriter | None = None


class SessionManager:
    def __init__(self) -> None:
        self._sessions: dict[int, Session] = {}

    def _get_or_create_session(self) -> Session:
        session = self._sessions.get(FIXED_SESSION_ID)
        if session is None:
            session = Session(session_id=FIXED_SESSION_ID)
            self._sessions[FIXED_SESSION_ID] = session
        return session

    def create_session(
        self, player_name: str, writer: asyncio.StreamWriter
    ) -> tuple[int, int] | tuple[None, str]:
        session = self._get_or_create_session()
        if HOST_CLIENT_ID in session.clients:
            return None, "Session already has a host; use JoinSession"

        session.clients[HOST_CLIENT_ID] = ConnectedClient(HOST_CLIENT_ID, player_name, writer)
        return (FIXED_SESSION_ID, HOST_CLIENT_ID)

    def join_session(
        self, session_id: int, player_name: str, writer: asyncio.StreamWriter
    ) -> tuple[int, int] | tuple[None, str]:
        if session_id != FIXED_SESSION_ID:
            return None, f"Session {session_id} not found"
        session = self._sessions.get(session_id)
        if session is None:
            return None, f"Session {session_id} not found"
        if HOST_CLIENT_ID not in session.clients:
            return None, "Session has no host; create session first"
        if JOINER_CLIENT_ID in session.clients:
            return None, "Session is full"

        session.clients[JOINER_CLIENT_ID] = ConnectedClient(JOINER_CLIENT_ID, player_name, writer)
        return (FIXED_SESSION_ID, JOINER_CLIENT_ID)

    def is_session_ready(self, session_id: int) -> bool:
        session = self._sessions.get(session_id)
        if session is None:
            return False
        return HOST_CLIENT_ID in session.clients and JOINER_CLIENT_ID in session.clients

    def get_peer_writer(
        self, session_id: int, sender_client_id: int
    ) -> asyncio.StreamWriter | None:
        session = self._sessions.get(session_id)
        if session is None:
            return None
        for client_id, client in session.clients.items():
            if client_id != sender_client_id:
                return client.writer
        return None

    def get_client_writer(
        self, session_id: int, client_id: int
    ) -> asyncio.StreamWriter | None:
        session = self._sessions.get(session_id)
        if session is None:
            return None
        client = session.clients.get(client_id)
        return client.writer if client is not None else None

    def get_client_ids(self, session_id: int) -> list[int]:
        session = self._sessions.get(session_id)
        if session is None:
            return []
        return list(session.clients.keys())

    def remove_client(self, writer: asyncio.StreamWriter) -> RemoveClientResult:
        result = RemoveClientResult()
        for session_id, session in list(self._sessions.items()):
            for client_id, client in list(session.clients.items()):
                if client.writer is not writer:
                    continue

                result.session_id = session_id
                result.removed_client_id = client_id
                result.was_host = client_id == HOST_CLIENT_ID

                peer_writer: asyncio.StreamWriter | None = None
                for other_id, other in session.clients.items():
                    if other_id != client_id:
                        peer_writer = other.writer

                if result.was_host:
                    del self._sessions[session_id]
                    result.session_destroyed = True
                    result.peer_writer = peer_writer
                else:
                    del session.clients[client_id]
                    if not session.clients:
                        del self._sessions[session_id]
                        result.session_destroyed = True

                return result

        return result
