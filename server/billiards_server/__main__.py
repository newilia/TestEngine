from __future__ import annotations

import argparse
import asyncio
import logging

from billiards_server.billiard_session_server import BilliardSessionServer


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="BilliardSession server (MVP)")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=7777)
    return parser.parse_args()


def main() -> None:
    args = _parse_args()
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s %(levelname)s %(name)s: %(message)s",
    )
    server = BilliardSessionServer(host=args.host, port=args.port)
    asyncio.run(_run(server))


async def _run(server: BilliardSessionServer) -> None:
    await server.start()
    await server.serve_forever()


if __name__ == "__main__":
    main()
