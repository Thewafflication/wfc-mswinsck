# Identifier map

**Content type:** Project record

Before 2026-11-19 the requirement and the test cases of this library carried
identifiers from the global sequence of the
[WFC](https://github.com/Thewafflication/wfc) repository (`REQ-0305`, the
milestone-prefixed `TC-MP0010-*` CTest names). The library is its own
repository and now numbers its artifacts from 0001 in its own namespace
(`wsp/style/artifact-identifiers.md`). WSP asks that references to historical
identifiers stay resolvable; this table is that record. A new identifier is
never reused.

## Requirements

| Old identifier | Old title | New identifier | File |
| --- | --- | --- | --- |
| `REQ-0305` (WFC) | Winsock control (`MSWinsockLib.Winsock`) | `REQ-0001` | `requirements/req-0001-winsock-control.md` |
| none | (written from the README: the library DLL and its registration) | `REQ-0002` | `requirements/req-0002-library-dll-registration.md` |

## Test cases

| Old identifier | Old title | New identifier | CTest name (in WFC: `wfc-mswinsck-` before it) | Specification |
| --- | --- | --- | --- | --- |
| `TC-MP0010-mswinsck-tables` | Member table of the Winsock library | `TC-0001` | `TC-0001-tables` | `tests/tc-0001-member-table.tex` |
| `TC-MP0010-mswinsck-states` | State rules of the Winsock control | `TC-0002` | `TC-0002-states` | `tests/tc-0002-state-rules.tex` |
| `TC-MP0010-mswinsck-io` | Socket layer over loopback | `TC-0003` | `TC-0003-io` | `tests/tc-0003-socket-layer.tex` |
| `TC-MP0010-mswinsck-dll` | Library DLL, version resource and registration | `TC-0004` | `TC-0004-dll` | `tests/tc-0004-library-dll.tex` |
| `TC-MP0002-corpus-196-ocx-winsock-dialog` | Corpus program 196, Winsock and Common Dialog | `TC-0005` | `TC-0005-winsock-corpus` | `tests/tc-0005-winsock-corpus.tex` |

The old titles are the descriptions the tests carried in their source comments;
the old names had no specification document.

## What stays under the old names

The WFC repository keeps running the corpus program as
`TC-MP0002-corpus-196-ocx-winsock-dialog` (its corpus registration, which
globs `lib/*/tests/corpus/*.expected`), and still names `REQ-0305` and the
`TC-MP0010-mswinsck-*` tests in its planning records (milestone closeouts and
work logs) and its test selection. Those are history and are not edited; this
table resolves them. When WFC mounts a newer version of this library, the
`TC-0001` to `TC-0004` tests register in WFC's CTest under the new names with
the library name in front (`wfc-mswinsck-TC-0001-tables`, and so on; the
`WFC_MSWINSCK_TEST_PREFIX` variable of `CMakeLists.txt`). `TC-0005` is not
registered there: WFC runs the program under its own name.
