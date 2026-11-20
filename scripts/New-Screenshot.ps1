<#
.SYNOPSIS
Regenerates the screenshot of the Winsock echo example.
.DESCRIPTION
Runs examples/winsock-echo with WFC in a real window, lets the exchange finish,
and copies the final screenshot of the form to docs/images/winsock-echo.png
(the image the README shows). The program's own window is captured, not the
screen.
.PARAMETER Wfc
Path of wfc.exe (default: wfc on PATH).
.EXAMPLE
pwsh -NoProfile -File scripts/New-Screenshot.ps1
#>
[CmdletBinding()]
param(
    [string]$Wfc = 'wfc'
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
Import-Module (Join-Path $PSScriptRoot 'LibChecks.psm1') -Force

$root = Split-Path -Parent $PSScriptRoot
$shots = Join-Path $root 'out/screenshots'
Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $shots
New-Item -ItemType Directory -Force -Path $shots | Out-Null
Invoke-CheckedCommand $Wfc @(
    'run', (Join-Path $root 'examples/winsock-echo/echo.vbp'), '--gui',
    '--exit-after', '4000', '--screenshots', $shots)
$image = Join-Path $shots 'frmEcho-closing.png'
if (-not (Test-Path -LiteralPath $image)) {
    throw "wfc wrote no screenshot of the form ($image)."
}
Copy-Item -LiteralPath $image `
    -Destination (Join-Path $root 'docs/images/winsock-echo.png') -Force
Write-Output '[PASS] docs/images/winsock-echo.png'
