"""Export the existing photos and animation frames for the LVGL screen sizes."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageOps

project = Path(__file__).resolve().parent
source = project.parents[1] / "revontuli-html" / "assets"
out = project / "assets" / "skins" / "cabin"
out.mkdir(parents=True, exist_ok=True)

def export(source_path, name, size):
    with Image.open(source_path) as image:
        ImageOps.fit(image.convert("RGB"), size, Image.Resampling.LANCZOS).save(
            out / (name + ".jpg"), quality=78, progressive=False)

export(source / "cabin.webp", "cabin", (480, 208))
export(source / "album.webp", "album", (190, 190))
export(source / "album.webp", "album-mini", (36, 36))
for name in ("aurora", "lounge", "relax", "club", "cinema"):
    with Image.open(source / "presets" / f"{name}.webp") as master:
        for width in (88, 142):
            size = (width, 54)
            image = ImageOps.fit(master.convert("RGBA"), size, Image.Resampling.LANCZOS)
            shade = Image.new("RGBA", size)
            draw = ImageDraw.Draw(shade)
            for y in range(27, 54):
                draw.line((0, y, width, y), fill=(0, 0, 0, (y - 27) * 6))
            image = Image.alpha_composite(image, shade)
            mask = Image.new("L", size)
            ImageDraw.Draw(mask).rounded_rectangle((0, 0, width - 1, 53), 12, fill=255)
            image.putalpha(mask)
            image.save(out / f"{name}-{width}.png", optimize=True)
    for index in range(10):
        export(source / "sequences" / f"light-{name}" / f"{index:02}.webp",
               f"light-{name}-{index}", (480, 208))
for name in ("window", "shade", "roof"):
    for index in range(10):
        export(source / "sequences" / name / f"{index:02}.webp", f"{name}-{index}", (480, 208))
