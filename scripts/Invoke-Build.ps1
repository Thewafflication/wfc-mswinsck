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
.EXAMPLE
pwsh -NoProfile -File scripts/Invoke-Build.ps1
#>
[CmdletBinding()]
param(
    [ValidateSet('x64', 'Win32', 'ARM64')][string]$Platform = 'x64',
    [string]$BuildDirectory = 'out/build'
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
Import-Module (Join-Path $PSScriptRoot 'LibChecks.psm1') -Force

$root = Split-Path -Parent $PSScriptRoot
Push-Location $root
try {
    Write-Stage "[build] configure $Platform"
    Invoke-CheckedCommand cmake @(
        '-S', '.', '-B', $BuildDirectory, '-A', $Platform)
    Write-Stage "[build] compile $Platform Debug"
    Invoke-CheckedCommand cmake @(
        '--build', $BuildDirectory, '--config', 'Debug')
    Write-Output "[PASS] build $Platform"
}
finally {
    Pop-Location
}
