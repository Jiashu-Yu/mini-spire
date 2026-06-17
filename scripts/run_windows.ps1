param(
    [string]$SfmlRoot = "",
    [string]$BuildDir = "build\mini-spire-windows",
    [switch]$NoRun
)

$ErrorActionPreference = "Stop"

function Write-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host "==> $Message" -ForegroundColor Cyan
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
$buildPath = [System.IO.Path]::GetFullPath($buildPath)

Write-Step "Checking Visual Studio and SFML"
$vsDevCmd = Find-VsDevCmd
$sfmlCmakeDir = Find-SfmlCmakeDir -RequestedRoot $SfmlRoot -RepoRoot $repoRoot
$sfmlRootPath = (Resolve-Path -LiteralPath (Join-Path $sfmlCmakeDir "..\..\..")).Path
$sfmlBin = Join-Path $sfmlRootPath "bin"

Write-Host "Visual Studio: $vsDevCmd"
Write-Host "SFML CMake:    $sfmlCmakeDir"
Write-Host "Build folder:  $buildPath"

Write-Step "Configuring and building Mini Spire"
$cmd = "`"$vsDevCmd`" -arch=x64 -host_arch=x64 && " +
    "cmake -S `"$repoRoot`" -B `"$buildPath`" -G `"NMake Makefiles`" " +
    "-DSFML_DIR=`"$sfmlCmakeDir`" -DMINISPIRE_BUILD_APP=ON -DMINISPIRE_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release && " +
    "cmake --build `"$buildPath`" && " +
    "ctest --test-dir `"$buildPath`" --output-on-failure"

& cmd.exe /d /c $cmd
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Write-Step "Copying SFML runtime DLLs"
Copy-Item -LiteralPath (Join-Path $sfmlBin "openal32.dll") -Destination $buildPath -Force -ErrorAction SilentlyContinue
Copy-Item -Path (Join-Path $sfmlBin "*.dll") -Destination $buildPath -Force

if ($NoRun) {
    Write-Step "Build complete"
    Write-Host "Game executable: $(Join-Path $buildPath "mini_spire.exe")"
} else {
    Write-Step "Starting Mini Spire"
    Push-Location $buildPath
    try {
        & ".\mini_spire.exe"
    } finally {
        Pop-Location
    }
}
