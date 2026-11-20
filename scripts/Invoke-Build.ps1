<#
.SYNOPSIS
Runs the library canonical build stage.
.DESCRIPTION
Configures the standalone library build and compiles it for one platform.
The local commit gate builds x64 Debug only; CI covers the other targets.
.PARAMETER Platform
Visual Studio platform: x64 (default), Win32, or ARM64.
.PARAMETER BuildDirectory
CMake binary directory, relative to the repository root.
.PARAMETER Static
Builds the static library instead of the DLL. The default builds
wfcmswinsck.dll (WFC_MSWINSCK_SHARED=ON), which test case TC-0004 needs.
.PARAMETER WfcExecutable
wfc.exe that runs test case TC-0005 (the corpus program). Defaults to the
WFC_EXECUTABLE environment variable, then to wfc on PATH; without one TC-0005
is skipped.
.EXAMPLE
pwsh -NoProfile -File scripts/Invoke-Build.ps1
#>
[CmdletBinding()]
param(
    [ValidateSet('x64', 'Win32', 'ARM64')][string]$Platform = 'x64',
    [string]$BuildDirectory = 'out/build',
    [switch]$Static,
    [string]$WfcExecutable = $env:WFC_EXECUTABLE
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
Import-Module (Join-Path $PSScriptRoot 'LibChecks.psm1') -Force

$root = Split-Path -Parent $PSScriptRoot
Push-Location $root
try {
    if (-not $WfcExecutable) {
        $wfc = Get-Command wfc -CommandType Application `
            -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($wfc) { $WfcExecutable = $wfc.Source }
    }
    $shared = if ($Static) { 'OFF' } else { 'ON' }
    Write-Stage "[build] configure $Platform (shared $shared)"
    Invoke-CheckedCommand cmake @(
        '-S', '.', '-B', $BuildDirectory, '-A', $Platform,
        "-DWFC_MSWINSCK_SHARED=$shared",
        "-DWFC_EXECUTABLE=$WfcExecutable")
    Write-Stage "[build] compile $Platform Debug"
    Invoke-CheckedCommand cmake @(
        '--build', $BuildDirectory, '--config', 'Debug')
    Write-Output "[PASS] build $Platform"
}
finally {
    Pop-Location
}
