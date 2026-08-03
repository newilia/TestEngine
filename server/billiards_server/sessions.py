from __future__ import annotations

from dataclasses import dataclass, field


@dataclass
class Session:
    session_id: int
    next_client_id: int = 1
    client_names: dict[int, str] = field(default_factory=dict)


class SessionManager:
    def __init__(self) -> None:
        self._sessions: dict[int, Session] = {}
        self._next_session_id = 1

    def create_session(self, player_name: str) -> tuple[int, int]:
        session_id = self._next_session_id
        self._next_session_id += 1
        session = Session(session_id=session_id)
        client_id = session.next_client_id
        session.next_client_id += 1
        session.client_names[client_id] = player_name
        self._sessions[session_id] = session
        return session_id, client_id

    def join_session(self, session_id: int, player_name: str) -> tuple[int, int] | None:
        session = self._sessions.get(session_id)
        if session is None:
            return None
        client_id = session.next_client_id
        session.next_client_id += 1
        session.client_names[client_id] = player_name
        return session_id, client_id

    def has_session(self, session_id: int) -> bool:
        return session_id in self._sessions
