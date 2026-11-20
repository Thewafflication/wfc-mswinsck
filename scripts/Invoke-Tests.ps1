<#
.SYNOPSIS
Runs the library canonical test stage.
.DESCRIPTION
Runs every registered CTest test of the build directory. Zero registered
tests is an error (WSP-STYLE-0007), so a broken registration cannot pass.
The ctest output also goes, line by line, to out/test-progress.log, which the
test progress window (scripts/Show-TestProgress.ps1, opened by the first step
of the commit gate) follows.
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
    # The log is followed live by scripts/Show-TestProgress.ps1; it holds
    # lines such as `12/377 Test  #12: name ... Passed`.
    $progressLog = Join-Path $root 'out/test-progress.log'
    New-Item -ItemType Directory -Force -Path (Split-Path $progressLog) |
        Out-Null
    $arguments = @(
        '--test-dir', $BuildDirectory, '-C', 'Debug',
        '--output-on-failure', '--no-tests=error')
    # Each line goes to the log as soon as ctest prints it.
    [IO.File]::WriteAllText($progressLog, '')
    & ctest @arguments 2>&1 | ForEach-Object {
        $line = "$_"
        [IO.File]::AppendAllText($progressLog, $line + "`n")
        $line
    }
    if ($LASTEXITCODE -ne 0) {
        throw "ctest $($arguments -join ' ') failed ($LASTEXITCODE)."
    }
    Write-Output '[PASS] tests'
}
finally {
    Pop-Location
}
