<#
.SYNOPSIS
Runs one test case of the library through CTest.
.DESCRIPTION
The test cases are registered as CTest tests named TC-NNNN-<name>
(CMakeLists.txt); a project that mounts the library sees them as
wfc-mswinsck-TC-NNNN-<name> (WFC_MSWINSCK_TEST_PREFIX), which is matched too.
This helper runs the one whose identifier is given and fails when none is
registered, so tests/run-tc-NNNN.ps1 can name its test case
(WSP-TEST-0005) without a registration that is silently missing.
.PARAMETER Id
Test case identifier, for example TC-0001.
.PARAMETER BuildDirectory
CMake binary directory, relative to the repository root. TC-0004 exists only
in a build configured with WFC_MSWINSCK_SHARED=ON, which is what
scripts/Invoke-Build.ps1 configures.
.EXAMPLE
pwsh -NoProfile -File tests/run-tc-0001.ps1
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)][ValidatePattern('^TC-\d{4}$')][string]$Id,
    [string]$BuildDirectory = 'out/build'
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
Import-Module (Join-Path $PSScriptRoot '../scripts/LibChecks.psm1') -Force

$root = Split-Path -Parent $PSScriptRoot
Invoke-CheckedCommand ctest @(
    '--test-dir', (Join-Path $root $BuildDirectory), '-C', 'Debug',
    '-R', "^(wfc-mswinsck-)?$Id-", '--no-tests=error', '--output-on-failure')
