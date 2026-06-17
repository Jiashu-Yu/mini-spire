param(
    [string]$SfmlRoot = "",
    [string]$BuildDir = "build\mini-spire-package",
    [string]$DistDir = "dist",
    [string]$PackageName = "MiniSpire-Windows-x64"
)

$ErrorActionPreference = "Stop"

function Write-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host "==> $Message" -ForegroundColor Cyan
}

function Assert-InRepo {
    param([string]$Path, [string]$RepoRoot)

    $fullPath = [System.IO.Path]::GetFullPath($Path)
    if (-not $fullPath.StartsWith($RepoRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to modify a path outside the repository: $fullPath"
    }
    return $fullPath
}

function Find-VsDevCmd {
    $candidates = @(
        "$env:ProgramFiles\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat",
        "$env:ProgramFiles\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat",
        "$env:ProgramFiles\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
    )

    foreach ($path in $candidates) {
        if (Test-Path -LiteralPath $path) {
            return $path
        }
    }

    throw "Visual Studio 2022 C++ build tools were not found. Install Visual Studio 2022 with the 'Desktop development with C++' workload."
}

function Resolve-SfmlCmakeDir {
    param([string]$InputPath)

    if ([string]::IsNullOrWhiteSpace($InputPath)) {
        return $null
    }

    $expanded = [Environment]::ExpandEnvironmentVariables($InputPath)
    if (-not (Test-Path -LiteralPath $expanded)) {
        return $null
    }

    $resolved = (Resolve-Path -LiteralPath $expanded).Path
    if (Test-Path -LiteralPath (Join-Path $resolved "SFMLConfig.cmake")) {
        return $resolved
    }

    $cmakeDir = Join-Path $resolved "lib\cmake\SFML"
    if (Test-Path -LiteralPath (Join-Path $cmakeDir "SFMLConfig.cmake")) {
        return $cmakeDir
    }

    return $null
}

function Find-SfmlCmakeDir {
    param([string]$RequestedRoot, [string]$RepoRoot)

    $candidates = @(
        $RequestedRoot,
        $env:SFML_DIR,
        $env:SFML_ROOT,
        "D:\Libraries\SFML-2.6.2",
        "C:\Libraries\SFML-2.6.2",
        "$env:USERPROFILE\Libraries\SFML-2.6.2",
        (Join-Path $RepoRoot "SFML-2.6.2")
    )

    foreach ($candidate in $candidates) {
        $cmakeDir = Resolve-SfmlCmakeDir $candidate
        if ($cmakeDir) {
            return $cmakeDir
        }
    }

    throw "SFML 2.6.2 was not found. Download 'Visual C++ 17 (2022) - 64-bit' from https://www.sfml-dev.org/download/sfml/2.6.2/ and unzip it to D:\Libraries\SFML-2.6.2, or run this script with -SfmlRoot <your SFML folder>."
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = (Resolve-Path -LiteralPath (Join-Path $scriptDir "..")).Path
$buildPath = if ([System.IO.Path]::IsPathRooted($BuildDir)) { $BuildDir } else { Join-Path $repoRoot $BuildDir }
$distPath = if ([System.IO.Path]::IsPathRooted($DistDir)) { $DistDir } else { Join-Path $repoRoot $DistDir }
$buildPath = Assert-InRepo -Path $buildPath -RepoRoot $repoRoot
$distPath = Assert-InRepo -Path $distPath -RepoRoot $repoRoot
$packagePath = Assert-InRepo -Path (Join-Path $distPath $PackageName) -RepoRoot $repoRoot
$zipPath = Assert-InRepo -Path (Join-Path $distPath "$PackageName.zip") -RepoRoot $repoRoot

Write-Step "Checking Visual Studio and SFML"
$vsDevCmd = Find-VsDevCmd
$sfmlCmakeDir = Find-SfmlCmakeDir -RequestedRoot $SfmlRoot -RepoRoot $repoRoot
$sfmlRootPath = (Resolve-Path -LiteralPath (Join-Path $sfmlCmakeDir "..\..\..")).Path
$sfmlBin = Join-Path $sfmlRootPath "bin"

Write-Host "Visual Studio: $vsDevCmd"
Write-Host "SFML CMake:    $sfmlCmakeDir"
Write-Host "Build folder:  $buildPath"
Write-Host "Package zip:   $zipPath"

Write-Step "Building Release package binaries"
$cmd = "`"$vsDevCmd`" -arch=x64 -host_arch=x64 && " +
    "cmake -S `"$repoRoot`" -B `"$buildPath`" -G `"NMake Makefiles`" " +
    "-DSFML_DIR=`"$sfmlCmakeDir`" -DMINISPIRE_BUILD_APP=ON -DMINISPIRE_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release && " +
    "cmake --build `"$buildPath`" && " +
    "ctest --test-dir `"$buildPath`" --output-on-failure"

& cmd.exe /d /c $cmd
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Write-Step "Creating clean package folder"
New-Item -ItemType Directory -Force -Path $distPath | Out-Null
if (Test-Path -LiteralPath $packagePath) {
    Remove-Item -LiteralPath $packagePath -Recurse -Force
}
if (Test-Path -LiteralPath $zipPath) {
    Remove-Item -LiteralPath $zipPath -Force
}
New-Item -ItemType Directory -Force -Path $packagePath | Out-Null

Write-Step "Copying game executable, assets, and runtime DLLs"
Copy-Item -LiteralPath (Join-Path $buildPath "mini_spire.exe") -Destination $packagePath -Force
Copy-Item -LiteralPath (Join-Path $repoRoot "assets") -Destination $packagePath -Recurse -Force

$runtimeDlls = Get-ChildItem -LiteralPath $sfmlBin -Filter "*.dll" |
    Where-Object { $_.Name -notmatch "-d-2\.dll$" }

foreach ($dll in $runtimeDlls) {
    Copy-Item -LiteralPath $dll.FullName -Destination $packagePath -Force
}

$runNotes = @(
    "Mini Spire Windows x64 portable package",
    "",
    "How to play:",
    "1. Extract the whole folder.",
    "2. Double-click mini_spire.exe.",
    "",
    "Do not move mini_spire.exe out of this folder. It needs the assets folder and DLL files beside it.",
    "",
    "If Windows SmartScreen shows an unknown publisher warning, choose More info, then Run anyway."
)

Set-Content -Path (Join-Path $packagePath "README_PLAY.txt") -Value $runNotes -Encoding UTF8

Write-Step "Checking package contents"
$requiredFiles = @(
    "mini_spire.exe",
    "assets\images\backgrounds\title_spire.png",
    "assets\images\sprites\player_rift_traveler.png",
    "sfml-graphics-2.dll",
    "sfml-window-2.dll",
    "sfml-system-2.dll",
    "sfml-audio-2.dll",
    "openal32.dll",
    "README_PLAY.txt"
)

foreach ($relativePath in $requiredFiles) {
    $fullPath = Join-Path $packagePath $relativePath
    if (-not (Test-Path -LiteralPath $fullPath)) {
        throw "Package is missing required file: $relativePath"
    }
}

Write-Step "Creating zip archive"
Compress-Archive -Path $packagePath -DestinationPath $zipPath -Force

$zip = Get-Item -LiteralPath $zipPath
Write-Host ""
Write-Host "Package created: $($zip.FullName)" -ForegroundColor Green
Write-Host ("Size: {0:N1} MB" -f ($zip.Length / 1MB))
