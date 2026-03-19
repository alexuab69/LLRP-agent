param(
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$cppDir = Join-Path $repoRoot "cpp"
$buildDir = Join-Path $cppDir "build"
$cmakeExe = "C:\msys64\mingw64\bin\cmake.exe"
$mingwBin = "C:\msys64\mingw64\bin"

if (-not (Test-Path $cmakeExe)) {
    throw "No se encontro CMake en $cmakeExe. Instala msys2/mingw64 o ajusta la ruta en build_mingw.ps1"
}

if ($Clean -and (Test-Path $buildDir)) {
    Remove-Item -Recurse -Force $buildDir
}

if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

Write-Host "[1/3] Configurando CMake..."
& $cmakeExe -G Ninja -B $buildDir -S $cppDir
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "[2/3] Compilando..."
& $cmakeExe --build $buildDir
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$libxmlDll = Join-Path $mingwBin "libxml2-16.dll"
if (Test-Path $libxmlDll) {
    Copy-Item -Force $libxmlDll $buildDir
}

Write-Host "[3/3] Binarios generados:"
Get-ChildItem $buildDir -Filter *.exe | Select-Object Name, Length, LastWriteTime | Format-Table -AutoSize

Write-Host "Build completada."
