from __future__ import annotations

import asyncio
from dataclasses import dataclass, field

FIXED_SESSION_ID = 1
MAX_PLAYERS = 2


@dataclass
class ConnectedClient:
    client_id: int
    player_name: str
    writer: asyncio.StreamWriter


@dataclass
class Session:
    session_id: int
    next_client_id: int = 1
    clients: dict[int, ConnectedClient] = field(default_factory=dict)


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
        if len(session.clients) >= MAX_PLAYERS:
            return None, "Session is full"
        if len(session.clients) > 0:
            return None, "Session already has a host; use JoinSession"

        client_id = session.next_client_id
        session.next_client_id += 1
        session.clients[client_id] = ConnectedClient(client_id, player_name, writer)
        return (FIXED_SESSION_ID, client_id)

    def join_session(
        self, session_id: int, player_name: str, writer: asyncio.StreamWriter
    ) -> tuple[int, int] | tuple[None, str]:
        if session_id != FIXED_SESSION_ID:
            return None, f"Session {session_id} not found"
        session = self._sessions.get(session_id)
        if session is None:
            return None, f"Session {session_id} not found"
        if len(session.clients) >= MAX_PLAYERS:
            return None, "Session is full"
        if len(session.clients) == 0:
            return None, "Session has no host; create session first"

        client_id = session.next_client_id
        session.next_client_id += 1
        session.clients[client_id] = ConnectedClient(client_id, player_name, writer)
        return (FIXED_SESSION_ID, client_id)

    def is_session_ready(self, session_id: int) -> bool:
        session = self._sessions.get(session_id)
        return session is not None and len(session.clients) >= MAX_PLAYERS

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

    def remove_client(self, writer: asyncio.StreamWriter) -> None:
        for session in self._sessions.values():
            to_remove = [
                client_id
                for client_id, client in session.clients.items()
                if client.writer is writer
            ]
            for client_id in to_remove:
                del session.clients[client_id]
