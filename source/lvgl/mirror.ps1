param([Parameter(Mandatory)][string]$Assets)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
foreach ($folder in 'recline', 'fx') {
    $target = New-Item -ItemType Directory -Force (Join-Path $Assets "right/$folder")
    foreach ($file in Get-ChildItem (Join-Path $Assets $folder) -File) {
        $image = [System.Drawing.Image]::FromFile($file.FullName)
        try {
            $image.RotateFlip([System.Drawing.RotateFlipType]::RotateNoneFlipX)
            $format = if ($file.Extension -eq '.jpg') { [System.Drawing.Imaging.ImageFormat]::Jpeg } else { [System.Drawing.Imaging.ImageFormat]::Png }
            $image.Save((Join-Path $target.FullName $file.Name), $format)
        } finally { $image.Dispose() }
    }
}
