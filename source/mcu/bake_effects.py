"""Bake seat effects into small RGBA PNGs using only the Python standard library."""
from pathlib import Path
import math
import struct
import zlib

WIDTH, HEIGHT = 90, 128
destination = Path(__file__).parent / "assets" / "fx"
destination.mkdir(parents=True, exist_ok=True)

def chunk(kind, data):
    return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))

def bake(kind, level, frame):
    pixels = bytearray(WIDTH * HEIGHT * 4)
    rgb = {"heat": (255, 185, 85), "cool": (129, 222, 255), "massage": (159, 235, 195)}[kind]
    radius = 2 if kind == "cool" else 4
    def dot(x, y):
        for dy in range(-radius, radius + 1):
            for dx in range(-radius, radius + 1):
                px, py = round(x) + dx, round(y) + dy
                if 0 <= px < WIDTH and 0 <= py < HEIGHT:
                    offset = (py * WIDTH + px) * 4
                    alpha = int(max(0, 1 - math.hypot(dx, dy) / (radius + 0.5)) * (155 + frame * 20))
                    if alpha > pixels[offset + 3]:
                        pixels[offset:offset + 4] = bytes((*rgb, alpha))
    for base_x, base_y in [(47, 47), (23, 107)]:
        for lane in range(level):
            x = base_x + (lane - (level - 1) / 2) * 13
            for step in range(31):
                t = step / 30
                if kind == "heat":
                    dot(x + 3 * math.sin(t * math.tau), base_y - 22 * t - 3 * frame)
                elif kind == "cool":
                    # Match the web arrow: a short curved shaft and two straight head strokes.
                    ax, ay = x - 2 * frame, base_y + frame
                    dot(ax + 6 - 15 * t, ay - 8 + 5 * t + 6 * t * t)
                    dot(ax - 9, ay + 3 - 5 * t)
                    dot(ax - 9 + 5 * t, ay + 3 - t)
                else:
                    angle = t * math.tau
                    dot(x + (6 + frame * 2) * math.cos(angle), base_y + (4 + frame) * math.sin(angle))
    stride = WIDTH * 4
    rows = b"".join(b"\0" + pixels[y:y + stride] for y in range(0, len(pixels), stride))
    header = struct.pack(">IIBBBBB", WIDTH, HEIGHT, 8, 6, 0, 0, 0)
    return b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", header) + chunk(b"IDAT", zlib.compress(rows)) + chunk(b"IEND", b"")

for effect, levels in (("heat", 3), ("cool", 3), ("massage", 1)):
    for level in range(1, levels + 1):
        for frame in range(4):
            (destination / f"{effect}-{level}-{frame}.png").write_bytes(bake(effect, level, frame))
