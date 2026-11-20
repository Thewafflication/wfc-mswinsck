# wfc-mswinsck Repository Instructions

The Winsock control (`MSWinsockLib.Winsock`) of the WFC libraries. This
repository is a library of [WFC](https://github.com/Thewafflication/wfc) and
follows its practices; `wsp/` is the pinned Waughtal Software Process
submodule (see `docs/WSP-ADOPTION.md`).

## Owner Authorization

- Ordinary file edits within this repository and the current task scope do not
  require asking the owner for approval each time.
- Running the local lint, build and test tooling is pre-authorized: `cmake`,
  `ctest`, and the scripts under `scripts/`.
- Commit and push only when the owner asks. Never tag or release on your own;
  the owner decides.
- Run only the x64 configure, build and test locally. Do not run local x86
  builds/tests or ARM64 cross-builds; GitHub Actions validates those targets.
- Destructive actions and actions outside this repository need the owner's
  normal explicit authorization.

## Commit Gate

- `.pre-commit-config.yaml` runs `scripts/Invoke-Lint.ps1`,
  `scripts/Invoke-Build.ps1` and `scripts/Invoke-Tests.ps1` (WSP-CHECK-0001).
  Land every change with a plain `git commit`; never skip the hook with
  `--no-verify` and never run its stages by hand in place of it.
- Setup: `python -m pip install --requirement requirements-dev.txt`, then
  `pre-commit install`. The tools are pinned (clang-format 22.1.3, gersemi
  0.29.2, pre-commit 4.6.2, PSScriptAnalyzer 1.25.0).
- Lint changes no files. To format: `clang-format -i --style=file <file>` and
  `python -m gersemi -i <file>`.

## Source Rules

- Style comes from `wsp/style` (80 columns, LF, UTF-8, final newline) and is
  checked by `wsp/tools/Test-SourceStyle.ps1`; `.clang-format`, `.gersemirc`
  and `.editorconfig` are copies of the WFC ones.
- Never modify `wsp/`; bump it by pinning a newer release tag.
- Requirements are `REQ-NNNN` under `requirements/`, test cases `TC-NNNN`
  (specification `tests/tc-NNNN-*.tex`, CTest name `TC-NNNN-<name>`, runner
  `tests/run-tc-NNNN.ps1`), both numbered from 0001 in this repository's own
  namespace (`docs/id-map.md` has the old WFC identifiers); keep identifiers stable and never reuse one (WSP artifact
  identifiers).
- `VERSION` is the library version; a release tag is the owner's decision.
