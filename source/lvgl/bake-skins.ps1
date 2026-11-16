param([Parameter(Mandatory)][string]$ReferenceAssets)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
$dest = New-Item -ItemType Directory -Force (Join-Path $PSScriptRoot 'assets/skins')
$families = @{
    button=@('218x36','70x44','38x44','142x44','144x44','142x32','144x32','52x26','80x44','88x44','444x38','56x40','44x44','52x38','144x36','142x36')
    selected=@('218x36','70x44','52x26','142x32','142x44','80x44','88x44','56x40','44x44','52x38','144x36','142x36')
    'selected-heat'=@('38x44')
    'selected-cool'=@('38x44')
    panel=@('444x51','444x60','142x80','144x80')
    dock=@('480x66')
}
foreach ($name in $families.Keys) {
    $source = [Drawing.Bitmap]::new((Join-Path $ReferenceAssets "$name.png"))
    try {
        foreach ($size in $families[$name]) {
            $width, $height = ($size -split 'x') | ForEach-Object {[int]$_}
            $bitmap = [Drawing.Bitmap]::new($width, $height)
            $graphics = [Drawing.Graphics]::FromImage($bitmap)
            try {
                $graphics.CompositingMode = [Drawing.Drawing2D.CompositingMode]::SourceCopy
                $graphics.InterpolationMode = [Drawing.Drawing2D.InterpolationMode]::HighQualityBilinear
                $sx = @(0,26,($source.Width-26),$source.Width)
                $sy = @(0,26,($source.Height-26),$source.Height)
                $dx = @(0,12,($width-12),$width)
                $dy = @(0,12,($height-12),$height)
                for ($y=0; $y -lt 3; $y++) {
                    for ($x=0; $x -lt 3; $x++) {
                        $from = [Drawing.Rectangle]::FromLTRB($sx[$x],$sy[$y],$sx[$x+1],$sy[$y+1])
                        $to = [Drawing.Rectangle]::FromLTRB($dx[$x],$dy[$y],$dx[$x+1],$dy[$y+1])
                        $graphics.DrawImage($source,$to,$from,[Drawing.GraphicsUnit]::Pixel)
                    }
                }
                $bitmap.Save((Join-Path $dest.FullName "$name-$size.png"),[Drawing.Imaging.ImageFormat]::Png)
            } finally { $graphics.Dispose(); $bitmap.Dispose() }
        }
    } finally { $source.Dispose() }
}
