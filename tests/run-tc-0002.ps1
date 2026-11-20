<#
.SYNOPSIS
Runs test case TC-0002.
.DESCRIPTION
Runs the CTest test TC-0002-* (see tests/tc-0002-*.tex for the
specification).
.PARAMETER BuildDirectory
CMake binary directory, relative to the repository root.
#>
[CmdletBinding()]
param([string]$BuildDirectory = 'out/build')

& (Join-Path $PSScriptRoot 'Invoke-TestCase.ps1') -Id 'TC-0002' `
    -BuildDirectory $BuildDirectory
