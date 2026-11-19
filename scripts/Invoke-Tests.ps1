<#
.SYNOPSIS
Runs the library canonical test stage.
.DESCRIPTION
Runs every registered CTest test of the build directory. Zero registered
tests is an error (WSP-STYLE-0007), so a broken registration cannot pass.
.PARAMETER BuildDirectory
CMake binary directory, relative to the repository root.
.EXAMPLE
pwsh -NoProfile -File scripts/Invoke-Tests.ps1
#>
[CmdletBinding()]
param(
    [string]$BuildDirectory = 'out/build'
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
Import-Module (Join-Path $PSScriptRoot 'LibChecks.psm1') -Force

$root = Split-Path -Parent $PSScriptRoot
Push-Location $root
try {
    Write-Stage '[test] ctest Debug'
    $listed = (& ctest --test-dir $BuildDirectory -C Debug -N |
            Out-String)
    if ($listed -notmatch 'Total Tests: ([1-9]\d*)') {
        throw 'No tests are registered; the test stage must run tests.'
    }
    Invoke-CheckedCommand ctest @(
        '--test-dir', $BuildDirectory, '-C', 'Debug',
        '--output-on-failure')
    Write-Output '[PASS] tests'
}
finally {
    Pop-Location
}
