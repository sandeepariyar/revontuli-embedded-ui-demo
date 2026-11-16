"""Export existing climate artwork at its LVGL display size using Pillow."""
from pathlib import Path
from PIL import Image

project = Path(__file__).resolve().parent
source = project.parents[1] / "revontuli-html" / "assets"
out = project / "assets" / "skins" / "climate"
out.mkdir(parents=True, exist_ok=True)

def export(path, name):
    with Image.open(path) as frame:
        # The approved web scene keeps the bottom 75% so the floor vents stay visible.
        image = frame.convert("RGB").crop((0, frame.height // 4, frame.width, frame.height))
        image = image.resize((480, 240), Image.Resampling.LANCZOS)
        image.save(out / (name + ".jpg"), quality=84, subsampling=0, progressive=False)

export(source / "climate.webp", "static")
for tone in ("warm", "cool", "vent"):
    for direction in ("face", "body", "feet"):
        sequence = f"airflow-{direction}-vent" if tone == "vent" else f"climate-{tone}-{direction}"
        for index, frame in enumerate((0, 2, 4, 6)):
            export(source / "sequences" / sequence / f"{frame:02}.webp", f"{tone}-{direction}-{index}")
