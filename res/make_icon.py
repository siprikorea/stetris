#!/usr/bin/env python3
"""Generate the application icon.

Draws the S block from cpp/stetris/stblocks.cpp on a rounded dark tile and writes
a multi-size .ico plus a 256px .png. Pure standard library, no Pillow.

    python3 res/make_icon.py
"""

import math
import struct
import zlib
from pathlib import Path

OUT_DIR = Path(__file__).resolve().parent

# Icon sizes to place in the .ico
ICO_SIZES = [16, 32, 48, 64, 128, 256]

# Sizes stored as PNG rather than as a DIB. Windows Vista and later read
# these, and they keep the file from growing to a third of a megabyte.
PNG_SIZES = {128, 256}

# Supersampling factor used for anti-aliasing
SAMPLES = 4

# Colours, RGB
BG_OUTER = (0x0E, 0x14, 0x24)
BG_INNER = (0x1B, 0x26, 0x40)
CELL_FILL = (0x3D, 0xD5, 0x68)
CELL_LIGHT = (0x7B, 0xF0, 0x9A)
CELL_DARK = (0x1F, 0x8C, 0x40)

# The S block, as it appears in cpp/stetris/stblocks.cpp
SHAPE = [
    (1, 0), (2, 0),
    (0, 1), (1, 1),
]


def rounded_rect_coverage(px, py, x, y, w, h, r):
    """Return True when the sample point falls inside the rounded rect."""
    cx = min(max(px, x + r), x + w - r)
    cy = min(max(py, y + r), y + h - r)
    dx = px - cx
    dy = py - cy
    return dx * dx + dy * dy <= r * r


def blend(dst, src, alpha):
    return tuple(int(round(d + (s - d) * alpha)) for d, s in zip(dst, src))


def render(size):
    """Render one square icon, returning a list of RGBA rows."""
    ss = size * SAMPLES

    # Accumulate colour and coverage per output pixel
    acc = [[[0.0, 0.0, 0.0, 0.0] for _ in range(size)] for _ in range(size)]

    # Geometry in supersampled units
    margin = ss * 0.045
    tile_r = ss * 0.22

    # The block grid is 4 x 4 cells, the shape uses 3 x 2 of them
    grid = ss * 0.66
    cell = grid / 3.0
    grid_x = (ss - cell * 3) / 2.0
    grid_y = (ss - cell * 2) / 2.0

    gap = cell * 0.08
    cell_r = cell * 0.16
    bevel = cell * 0.18

    for sy in range(ss):
        py = sy + 0.5
        row = acc[sy // SAMPLES]

        for sx in range(ss):
            px = sx + 0.5

            # Background tile
            if not rounded_rect_coverage(px, py, margin, margin,
                                         ss - 2 * margin, ss - 2 * margin, tile_r):
                continue

            # Vertical gradient so the tile does not look flat
            t = py / ss
            colour = blend(BG_OUTER, BG_INNER, 1.0 - t)

            # Block cells
            for gx, gy in SHAPE:
                cx = grid_x + gx * cell + gap / 2
                cy = grid_y + gy * cell + gap / 2
                cw = cell - gap

                if not rounded_rect_coverage(px, py, cx, cy, cw, cw, cell_r):
                    continue

                # Light on the top left edge, shadow on the bottom right
                inset = rounded_rect_coverage(px, py, cx + bevel, cy + bevel,
                                              cw - 2 * bevel, cw - 2 * bevel,
                                              cell_r * 0.5)
                if inset:
                    colour = CELL_FILL
                elif (px - cx) + (py - cy) < cw:
                    colour = CELL_LIGHT
                else:
                    colour = CELL_DARK
                break

            pixel = row[sx // SAMPLES]
            pixel[0] += colour[0]
            pixel[1] += colour[1]
            pixel[2] += colour[2]
            pixel[3] += 1.0

    # Resolve the accumulated samples into RGBA rows
    total = float(SAMPLES * SAMPLES)
    rows = []
    for y in range(size):
        row = bytearray()
        for x in range(size):
            r, g, b, n = acc[y][x]
            if n == 0:
                row += bytes((0, 0, 0, 0))
            else:
                row += bytes((int(round(r / n)), int(round(g / n)),
                              int(round(b / n)), int(round(255 * n / total))))
        rows.append(bytes(row))
    return rows


def encode_png(rows, size):
    raw = b"".join(b"\x00" + row for row in rows)

    def chunk(tag, data):
        body = tag + data
        return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body))

    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", size, size, 8, 6, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(raw, 9))
    png += chunk(b"IEND", b"")
    return png


def write_png(path, rows, size):
    path.write_bytes(encode_png(rows, size))


def bmp_entry(rows, size):
    """Encode one icon image as a DIB, which every .ico reader understands."""
    header = struct.pack("<IiiHHIIiiII", 40, size, size * 2, 1, 32, 0,
                         size * size * 4, 0, 0, 0, 0)

    # The colour data is stored bottom up
    xor = bytearray()
    for row in reversed(rows):
        for x in range(size):
            r, g, b, a = row[x * 4:x * 4 + 4]
            xor += bytes((b, g, r, a))

    # A fully transparent AND mask, padded to four byte rows
    stride = ((size + 31) // 32) * 4
    mask = bytes(stride * size)

    return header + bytes(xor) + mask


def write_ico(path, images):
    count = len(images)
    header = struct.pack("<HHH", 0, 1, count)
    offset = 6 + 16 * count

    entries = b""
    body = b""
    for size, data in images:
        entries += struct.pack("<BBBBHHII", size if size < 256 else 0,
                               size if size < 256 else 0, 0, 0, 1, 32,
                               len(data), offset)
        body += data
        offset += len(data)

    path.write_bytes(header + entries + body)


def main():
    images = []
    for size in ICO_SIZES:
        rows = render(size)
        if size in PNG_SIZES:
            images.append((size, encode_png(rows, size)))
        else:
            images.append((size, bmp_entry(rows, size)))
        if size == 256:
            write_png(OUT_DIR / "stetris_icon.png", rows, size)
        print(f"  rendered {size}x{size}")

    write_ico(OUT_DIR / "stetris.ico", images)
    print(f"  wrote {OUT_DIR / 'stetris.ico'}")
    print(f"  wrote {OUT_DIR / 'stetris_icon.png'}")


if __name__ == "__main__":
    main()
