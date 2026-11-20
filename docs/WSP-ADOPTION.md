# WSP Adoption Record

**Content type:** Project adoption record

**Project:** wfc-mswinsck (Winsock)

**WSP release:** 1.4.0

**WSP baseline:** Immutable commit `f009399dd1406219571ac978bfee85e85bbdfeac`

**Submodule path:** `wsp/`

**Pinned commit:** `f009399dd1406219571ac978bfee85e85bbdfeac`

**Status:** Adoption in progress; the mechanical steps are done (see the gap
analysis) and the open items are listed below

The library is one of the WFC libraries and follows the WFC adoption record
(`planning/wsp-adoption-record.md` in
[WFC](https://github.com/Thewafflication/wfc)); this record covers what the
library repository itself carries.

## Selected profiles

| Profile | Selected | Scope or rationale |
| --- | --- | --- |
| Security/DFS | Yes | Hardening flags (WSP-SEC-0015) from `wsp/tools/cmake/WspBuild.cmake` in the standalone build |
| C source style | No | The sources are C++; the C test programs follow the same layout |
| CMake style | Yes | `CMakeLists.txt` |
| Windows version resources | Yes | `version.rc.in`, one version resource per DLL |
| Windows code signing and Defender | Deferred | Until the first release |
| Documentation | Yes | `README.md`, `ROADMAP.md`, `COMPATIBILITY.md` |

## Gap analysis (2026-11-19)

The `wsp` submodule already pinned 1.4.0, the release WFC pins; no change was
needed. Status: Done, Partial, Open, or Decision (owner decides).

| Item | WSP source | Status | Action |
| --- | --- | --- | --- |
| WSP pin | `wsp/architecture/adr-0001` | Done | Pins 1.4.0 (`f009399`), same as WFC |
| C++ formatter | `WSP-LANG-0002`, `WSP-STYLE-0004` | Done | `.clang-format` (22.1.3); the sources already matched except `web/wasm_api.cpp`, now formatted |
| CMake formatter | `WSP-LANG-0010` | Done | `.gersemirc` (0.29.2); `CMakeLists.txt` and `cmake/` already clean |
| Editor config | `WSP-STYLE-0002` to `0004`, `templates/source-style.editorconfig` | Done | `.editorconfig`; `.gitattributes` already sets LF |
| 80-column style | `WSP-STYLE-0001` | Done, with one tailoring | 32 files pass `Test-SourceStyle.ps1`; `web/index.html` (a demo page, 13 long markup lines) is left out of the scope |
| Lint stage | `WSP-CHECK-0001`, `0002`, `0003`, `0007` | Done | `scripts/Invoke-Lint.ps1` (style, clang-format, gersemi, PowerShell, YAML, pre-commit validation), pinned tools in `requirements-dev.txt` |
| Build and test stages | `WSP-CHECK-0002`, `0004`, `WSP-STYLE-0007` | Done | `Invoke-Build.ps1`, `Invoke-Tests.ps1` (x64 Debug, zero tests is an error); about 20 s locally |
| Commit gate | `WSP-CHECK-0001`, `templates/pre-commit-config.yaml` | Done | `.pre-commit-config.yaml` runs the gate start, then the three stages; install with `pre-commit install` |
| Single gate and progress window | WFC `AGENTS.md` | Done | `scripts/Start-CommitGate.ps1` takes the machine-wide `wfc-commit-gate.lock` (shared with WFC) and opens `scripts/Show-TestProgress.ps1`, which follows `out/test-progress.log` (lint and build stage lines from `Write-Stage`, then the ctest lines `12/377 Test  #12: name ... Passed` that `Invoke-Tests.ps1` writes). Both scripts are identical to the WFC copies |
| CI backstop | `WSP-CHECK-0008`, `WSP-TEST-0012`, `0013` | Partial | A lint job now runs the same script (first run not yet seen); the x86/x64/ARM64 build and test matrix already exists |
| Project AGENTS.md | WFC `AGENTS.md` | Done | `AGENTS.md` (commit and push only on the owner's request) |
| Commit-checks record | `WSP-CHECK-0006`, `processes/commit-checks.md` | Open | Write `docs/commit-checks.md` as WFC `planning/commit-checks.md` does |
| Requirement IDs | `style/artifact-identifiers.md` | Done | The library has its own namespace: `REQ-0001` (formerly WFC `REQ-0305`) and `REQ-0002`; `docs/id-map.md` maps the old identifiers |
| Requirement structure | `WSP-REQM-0001` to `0010`, `requirements/requirement-template.md` | Partial | Two requirements, status "Proposed", with verification references; no per-statement IDs yet (split into statements with stable IDs) |
| Test cases and traceability | `WSP-TEST-0001` to `0005`, `0011`, `tools/Test-Traceability.ps1` | Done | Five test cases `TC-0001` to `TC-0005`: specification `tests/tc-NNNN-*.tex`, CTest `TC-NNNN-<name>`, runner `tests/run-tc-NNNN.ps1`; the corpus case runs here when CMake is given `WFC_EXECUTABLE`. `Test-Traceability.ps1` runs in the lint stage and passes (2 requirements, 5 specifications, 5 implementations) |
| Test evidence and reports | `WSP-TEST-0006` to `0010`, `tools/Test-TestEvidence.ps1`, `New-TestReport.ps1` | Open | Needs test-case specifications first |
| Static analysis | `WSP-SAST-0001` to `0006`, `tools/static-analysis` | Open | Add clang-tidy with `wsp-clang-tidy.yml`; the build already includes `WspBuild.cmake` hardening |
| Source documentation | `WSP-STYLE-0006`, `style/source-documentation.md` | Open | No `Doxyfile`; headers lack Doxygen contracts |
| Release and versioning | `WSP-REL`, `processes/release-readiness-template.md` | Partial | `VERSION` 0.6.1 and the shared WFC release workflow exist; no release-readiness record, no `CHANGELOG.md` |
| Version resource | `WSP-WINRES-0001` to `0012` | Partial | `version.rc.in` exists; verification is not recorded |
| Signing and Defender | `WSP-SIGN-*` | Deferred | Until the first release, through the WFC workflow |
| Documentation structure | `WSP-DOC-*`, `style/documentation-style.md` | Partial | `README.md`, `ROADMAP.md`, `COMPATIBILITY.md`; no documentation manifest or PDF |
| Security/DFS | `WSP-SEC-*`, `security/dfs-template.md` | Partial | Hardening flags in the build; no DFS record |
| Process records (plan, review, closeout, work log) | `processes/*-template.md` | Deferred | WFC milestone records cover this library; add its own once it has milestones of its own |

## Decisions for the owner

1. **Requirement numbering.** Decided: own sequence from `REQ-0001`; the WFC
   number is recorded in `docs/id-map.md`.
2. **Test case identifiers.** Decided: `TC-NNNN`, CTest `TC-NNNN-<name>`. Open
   consequence: when WFC mounts the library its tests register under these
   names, and the other libraries use the same numbers (`TC-0001-...`); WFC
   needs a rule (a prefix, or registering library tests only in the
   standalone build).
3. **Reformat the rest.** Done in this pass because only five files changed.
   The remaining lint scope (`web/index.html`) is left out; decide whether to
   wrap it.
4. **Gate scope.** The gate runs build and tests (about 20 s). Confirm that
   matches WFC, or reduce it to lint only.

## Open items

| Item | Disposition |
| --- | --- |
| Requirements | Adopted: `requirements/` (moved from WFC); restructure per the gap analysis |
| Traceability check | Done: runs in the lint stage (`-RequirementsPath requirements -TestSpecificationsPath tests -TestImplementationsPath tests`) |
| Signing | Planned for the first release |
