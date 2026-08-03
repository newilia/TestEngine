"""Length-prefixed protobuf frame helpers."""

from __future__ import annotations

import struct
from typing import Protocol


class _Reader(Protocol):
    async def readexactly(self, n: int) -> bytes: ...


class _Writer(Protocol):
    def write(self, data: bytes) -> None: ...

    async def drain(self) -> None: ...


async def read_frame(reader: _Reader) -> bytes:
    length_bytes = await reader.readexactly(4)
    (length,) = struct.unpack("<I", length_bytes)
    return await reader.readexactly(length)


async def write_frame(writer: _Writer, payload: bytes) -> None:
    writer.write(struct.pack("<I", len(payload)) + payload)
    await writer.drain()
