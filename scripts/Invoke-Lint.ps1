<#
.SYNOPSIS
Runs the library canonical lint stage.
.DESCRIPTION
Checks the complete owned source and configuration scope:
- WSP physical style (80 columns, UTF-8, whitespace, final newline);
- clang-format check mode for C++ (WSP-LANG-0002);
- gersemi check mode for CMake (WSP-LANG-0010);
- PowerShell syntax (WSP-LANG-0007);
- YAML syntax (WSP-LANG-0005);
- pre-commit configuration validity (WSP-CHECK-0001).
Stops at the first failing check and exits nonzero. Changes no files.
.EXAMPLE
pwsh -NoProfile -File scripts/Invoke-Lint.ps1
#>
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
Import-Module (Join-Path $PSScriptRoot 'LibChecks.psm1') -Force

$root = Split-Path -Parent $PSScriptRoot
Push-Location $root
try {
    Write-Stage '[lint] WSP physical source style' -Reset
    # web/index.html is a demo page with long markup lines; tailored out.
    $styleScope = @(
        'src', 'include', 'cmake', 'scripts', 'tests', 'web/wasm_api.cpp',
        '.github',
        'CMakeLists.txt', '.clang-format', '.editorconfig', '.gersemirc',
        '.gitattributes', '.gitignore', '.pre-commit-config.yaml',
        'PSScriptAnalyzerSettings.psd1'
    )
    $quoted = ($styleScope | ForEach-Object { "'$_'" }) -join ','
    Invoke-CheckedCommand pwsh @('-NoProfile', '-Command',
        "& ./wsp/tools/Test-SourceStyle.ps1 -RepositoryRoot '$root' " +
        "-SourcePath @($quoted)")

    Write-Stage '[lint] clang-format (C++)'
    $clangFormat = Resolve-PinnedTool 'clang-format' '22.1.3'
    $cxx = Get-OwnedFile @('src', 'include', 'tests', 'web') `
        @('.cpp', '.hpp', '.h')
    Invoke-CheckedCommand $clangFormat (
        @('--dry-run', '--Werror', '--style=file') + $cxx)

    Write-Stage '[lint] gersemi (CMake)'
    $gersemi = Resolve-PinnedTool 'gersemi' '0.29.2'
    $cmake = @('CMakeLists.txt') + (Get-OwnedFile @('cmake') @('.cmake'))
    Invoke-CheckedCommand $gersemi (@(
            '--check', '--warnings-as-errors',
            '--definitions', 'cmake', 'wsp/tools/cmake',
            '--') + $cmake)

    Write-Stage '[lint] PowerShell syntax and analysis'
    # Test-RepositorySyntax.ps1 scans the WSP checkout itself, so the
    # library's own scripts are parsed here.
    $ps = Get-OwnedFile @('scripts') @('.ps1', '.psm1')
    foreach ($file in $ps) {
        $errors = $null
        [Management.Automation.Language.Parser]::ParseFile(
            (Join-Path $root $file), [ref]$null, [ref]$errors) | Out-Null
        if ($errors) {
            throw "${file}: $($errors[0].Message)"
        }
    }
    Import-Module PSScriptAnalyzer -RequiredVersion 1.25.0
    $findings = @(Invoke-ScriptAnalyzer -Path scripts -Recurse `
            -Settings PSScriptAnalyzerSettings.psd1)
    if ($findings.Count -gt 0) {
        $findings | Format-Table ScriptName, Line, RuleName, Message -Wrap |
            Out-String | Write-Output
        throw 'PowerShell analysis failed; see findings above.'
    }

    Write-Stage '[lint] YAML syntax'
    $yaml = Get-OwnedFile @('.github', '.pre-commit-config.yaml') `
        @('.yml', '.yaml')
    $parseYaml = 'import sys, yaml; [yaml.safe_load(open(p, ' +
        "encoding='utf-8')) for p in sys.argv[1:]]"
    Invoke-CheckedCommand python (@('-c', $parseYaml) + $yaml)

    Write-Stage '[lint] pre-commit configuration'
    $preCommit = Resolve-PinnedTool 'pre-commit' '4.6.2'
    Invoke-CheckedCommand $preCommit @('validate-config')

    Write-Output '[PASS] lint'
}
finally {
    Pop-Location
}
