#!/usr/bin/env python3
"""
Generate 8-ball pool ball textures (512x256 PNG) for sphere mapping.

Requires: pip install pillow
"""

from __future__ import annotations

import argparse
import math
import sys
from pathlib import Path

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    print("Error: Pillow is required. Install with: pip install pillow", file=sys.stderr)
    sys.exit(1)

# --- Image layout ---
IMAGE_HEIGHT = 256
IMAGE_WIDTH = IMAGE_HEIGHT * 2
IMAGE_SIZE = (IMAGE_WIDTH, IMAGE_HEIGHT)

# --- Geometry (ratios relative to IMAGE_HEIGHT) ---
STRIPE_HEIGHT_RATIO = 0.46
CIRCLE_DIAMETER_RATIO = 3 / 8
CIRCLE_CENTER_X = IMAGE_HEIGHT / 2
CIRCLE_CENTER_Y = IMAGE_HEIGHT / 2
FONT_SIZE_RATIO = 0.227
CIRCLE_SUPERSAMPLE = 4

# --- Colors (hex without #) ---
NUMBER_COLOR = "000000"
NUMBER_CIRCLE_COLOR = "FFFFFF"
STRIPE_BALL_BACKGROUND = "FFFFFF"
CUE_BALL_COLOR = "FFFFFF"

SOLID_COLORS: dict[int, str] = {
    1: "FFAE01",
    2: "2B67C2",
    3: "F40010",
    4: "521F92",
    5: "F15F00",
    6: "129026",
    7: "641200",
    8: "000000",
}

# --- Output ---
REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_OUTPUT_DIR = REPO_ROOT / "resources" / "textures" / "8ball"

# --- Font ---
FONT_CANDIDATES = [
    Path("C:/Windows/Fonts/ariblk.ttf"),
    Path("/usr/share/fonts/truetype/msttcorefonts/ariblk.ttf"),
    Path("/usr/share/fonts/truetype/msttcorefonts/Arial_Black.ttf"),
    Path("/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf"),
]


def _hex_to_rgb(hex_color: str) -> tuple[int, int, int]:
    h = hex_color.lstrip("#")
    return (int(h[0:2], 16), int(h[2:4], 16), int(h[4:6], 16))


def _find_font() -> Path:
    for path in FONT_CANDIDATES:
        if path.is_file():
            return path
    searched = "\n  ".join(str(p) for p in FONT_CANDIDATES)
    raise FileNotFoundError(
        f"Arial Black font not found. Searched:\n  {searched}\n"
        "Install arial black (ariblk.ttf) or add a path to FONT_CANDIDATES."
    )


def _stripe_color_for(number: int) -> str:
    if 9 <= number <= 15:
        return SOLID_COLORS[number - 8]
    return SOLID_COLORS[number]


def _background_color_for(number: int) -> str:
    if 9 <= number <= 15:
        return STRIPE_BALL_BACKGROUND
    return SOLID_COLORS[number]


def _draw_aa_filled_circle(
    image: Image.Image,
    center_x: float,
    center_y: float,
    radius: float,
    color: str,
    *,
    supersample: int = CIRCLE_SUPERSAMPLE,
) -> None:
    diameter = radius * 2
    pad = 2
    layer_px = int(math.ceil(diameter)) + pad * 2
    hi = layer_px * supersample

    layer = Image.new("RGBA", (hi, hi), (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)
    cx = hi / 2
    cy = hi / 2
    r = radius * supersample
    draw.ellipse(
        (cx - r, cy - r, cx + r, cy + r),
        fill=_hex_to_rgb(color) + (255,),
    )

    layer = layer.resize((layer_px, layer_px), Image.Resampling.LANCZOS)
    paste_x = int(round(center_x - layer_px / 2))
    paste_y = int(round(center_y - layer_px / 2))
    image.alpha_composite(layer, dest=(paste_x, paste_y))


def render_cue_ball() -> Image.Image:
    image = Image.new("RGBA", IMAGE_SIZE, _hex_to_rgb(CUE_BALL_COLOR) + (255,))
    return image


def render_numbered_ball(number: int, font: ImageFont.FreeTypeFont) -> Image.Image:
    bg_color = _background_color_for(number)
    image = Image.new("RGBA", IMAGE_SIZE, _hex_to_rgb(bg_color) + (255,))
    draw = ImageDraw.Draw(image)

    if 9 <= number <= 15:
        stripe_height = IMAGE_HEIGHT * STRIPE_HEIGHT_RATIO
        stripe_top = (IMAGE_HEIGHT - stripe_height) / 2
        stripe_bottom = (IMAGE_HEIGHT + stripe_height) / 2
        draw.rectangle(
            (0, stripe_top, IMAGE_WIDTH, stripe_bottom),
            fill=_hex_to_rgb(_stripe_color_for(number)),
        )

    circle_diameter = IMAGE_HEIGHT * CIRCLE_DIAMETER_RATIO
    circle_radius = circle_diameter / 2
    _draw_aa_filled_circle(
        image,
        CIRCLE_CENTER_X,
        CIRCLE_CENTER_Y,
        circle_radius,
        NUMBER_CIRCLE_COLOR,
    )

    label = str(number)
    draw.text(
        (CIRCLE_CENTER_X, CIRCLE_CENTER_Y),
        label,
        fill=_hex_to_rgb(NUMBER_COLOR),
        font=font,
        anchor="mm",
    )

    return image


def output_filename(ball_id: int) -> str:
    return f"ball_{ball_id}.png"


def generate_all(output_dir: Path, only: set[int] | None = None) -> list[Path]:
    output_dir.mkdir(parents=True, exist_ok=True)
    font_path = _find_font()
    font_size = int(round(IMAGE_HEIGHT * FONT_SIZE_RATIO))
    font = ImageFont.truetype(str(font_path), font_size)

    targets: list[int] = list(range(0, 16)) if only is None else sorted(only)

    written: list[Path] = []
    for ball_id in targets:
        if ball_id == 0:
            image = render_cue_ball()
        elif 1 <= ball_id <= 15:
            image = render_numbered_ball(ball_id, font)
        else:
            raise ValueError(f"Unknown ball id: {ball_id!r}")

        out_path = output_dir / output_filename(ball_id)
        image.save(out_path, format="PNG")
        written.append(out_path)

    return written


def _parse_only(value: str) -> set[int]:
    result: set[int] = set()
    for part in value.split(","):
        token = part.strip().lower()
        if not token:
            continue
        if token == "cue":
            result.add(0)
            continue
        number = int(token)
        if not 0 <= number <= 15:
            raise argparse.ArgumentTypeError(f"Ball number must be 0-15 or 'cue', got: {token}")
        result.add(number)
    if not result:
        raise argparse.ArgumentTypeError("At least one ball id is required for --only")
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate 8-ball pool ball textures (512x256 PNG).")
    parser.add_argument(
        "--output",
        type=Path,
        default=DEFAULT_OUTPUT_DIR,
        help=f"Output directory (default: {DEFAULT_OUTPUT_DIR.relative_to(REPO_ROOT)})",
    )
    parser.add_argument(
        "--only",
        type=_parse_only,
        default=None,
        help="Comma-separated ball ids to generate, e.g. 0,1,9,15 (cue = 0)",
    )
    args = parser.parse_args()

    try:
        written = generate_all(args.output.resolve(), args.only)
    except (FileNotFoundError, ValueError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1

    print(f"Generated {len(written)} texture(s) in {args.output.resolve()}:")
    for path in written:
        print(f"  {path.name}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
