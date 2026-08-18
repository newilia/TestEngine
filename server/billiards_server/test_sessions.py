from __future__ import annotations

import asyncio
import unittest

from billiards_server.sessions import (
    FIXED_SESSION_ID,
    HOST_CLIENT_ID,
    JOINER_CLIENT_ID,
    SessionManager,
)


class _FakeWriter:
    pass


class SessionManagerTests(unittest.TestCase):
    def setUp(self) -> None:
        self._manager = SessionManager()
        self._host_writer = _FakeWriter()
        self._joiner_writer = _FakeWriter()

    def test_host_always_gets_client_id_one(self) -> None:
        sid, cid = self._manager.create_session("host", self._host_writer)
        self.assertEqual((sid, cid), (FIXED_SESSION_ID, HOST_CLIENT_ID))

        removal = self._manager.remove_client(self._host_writer)
        self.assertTrue(removal.session_destroyed)

        sid, cid = self._manager.create_session("host-again", self._host_writer)
        self.assertEqual((sid, cid), (FIXED_SESSION_ID, HOST_CLIENT_ID))

    def test_joiner_always_gets_client_id_two(self) -> None:
        self._manager.create_session("host", self._host_writer)
        sid, cid = self._manager.join_session(FIXED_SESSION_ID, "joiner", self._joiner_writer)
        self.assertEqual((sid, cid), (FIXED_SESSION_ID, JOINER_CLIENT_ID))

    def test_host_disconnect_closes_session_and_allows_recreate(self) -> None:
        self._manager.create_session("host", self._host_writer)
        self._manager.join_session(FIXED_SESSION_ID, "joiner", self._joiner_writer)

        removal = self._manager.remove_client(self._host_writer)
        self.assertTrue(removal.was_host)
        self.assertTrue(removal.session_destroyed)
        self.assertIs(removal.peer_writer, self._joiner_writer)
        self.assertFalse(self._manager.is_session_ready(FIXED_SESSION_ID))

        sid, cid = self._manager.create_session("new-host", self._host_writer)
        self.assertEqual((sid, cid), (FIXED_SESSION_ID, HOST_CLIENT_ID))

    def test_joiner_disconnect_keeps_session_open(self) -> None:
        self._manager.create_session("host", self._host_writer)
        self._manager.join_session(FIXED_SESSION_ID, "joiner", self._joiner_writer)

        removal = self._manager.remove_client(self._joiner_writer)
        self.assertFalse(removal.was_host)
        self.assertFalse(removal.session_destroyed)
        self.assertIsNone(removal.peer_writer)

        sid, cid = self._manager.join_session(FIXED_SESSION_ID, "joiner-again", self._joiner_writer)
        self.assertEqual((sid, cid), (FIXED_SESSION_ID, JOINER_CLIENT_ID))


if __name__ == "__main__":
    unittest.main()
