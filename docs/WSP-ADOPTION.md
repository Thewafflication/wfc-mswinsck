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
| Commit gate | `WSP-CHECK-0001`, `templates/pre-commit-config.yaml` | Done | `.pre-commit-config.yaml` runs the three stages; install with `pre-commit install` |
| CI backstop | `WSP-CHECK-0008`, `WSP-TEST-0012`, `0013` | Partial | A lint job now runs the same script (first run not yet seen); the x86/x64/ARM64 build and test matrix already exists |
| Project AGENTS.md | WFC `AGENTS.md` | Done | `AGENTS.md` (commit and push only on the owner's request) |
| Commit-checks record | `WSP-CHECK-0006`, `processes/commit-checks.md` | Open | Write `docs/commit-checks.md` as WFC `planning/commit-checks.md` does |
| Requirement IDs | `style/artifact-identifiers.md` | Decision | Only `REQ-0305` exists, inherited from WFC numbering; see the decisions below |
| Requirement structure | `WSP-REQM-0001` to `0010`, `requirements/requirement-template.md` | Partial | One requirement, status "Proposed", no per-statement IDs or verification table; split into statements with stable IDs |
| Test cases and traceability | `WSP-TEST-0001` to `0005`, `0011`, `tools/Test-Traceability.ps1` | Open | CTest names are `TC-MP0010-mswinsck-*` (a WFC milestone prefix, not `TC-NNNN`); there are no test-case specifications, and the corpus case `TC-*` named in `REQ-0305` runs only in WFC |
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

1. **Requirement numbering.** Keep the WFC `REQ-NNNN` numbers (REQ-0305 in a
   library-owned namespace, shared with WFC) or give this library its own
   sequence (for example `REQ-0001`) and record the WFC number as a
   cross-reference. A shared namespace needs a rule for who assigns numbers.
2. **Test case identifiers.** Rename `TC-MP0010-*` to `TC-NNNN` (a stable name
   must not carry a milestone) or keep them and document the format.
3. **Reformat the rest.** Done in this pass because only five files changed.
   The remaining lint scope (`web/index.html`) is left out; decide whether to
   wrap it.
4. **Gate scope.** The gate runs build and tests (about 20 s). Confirm that
   matches WFC, or reduce it to lint only.

## Open items

| Item | Disposition |
| --- | --- |
| Requirements | Adopted: `requirements/` (moved from WFC); restructure per the gap analysis |
| Traceability check | Open: needs test-case specifications and the identifier decision |
| Signing | Planned for the first release |
