# wfc-mswinsck requirements

**Content type:** Project requirement index

Identifiers are `REQ-NNNN`, numbered from 0001 in this repository's own
namespace (the numbers the requirements had in the WFC repository are in
[`docs/id-map.md`](../docs/id-map.md)).

| Requirement | Subject | Verified by |
| --- | --- | --- |
| [REQ-0001](req-0001-winsock-control.md) | Winsock control (`MSWinsockLib.Winsock`) | `TC-0001`, `TC-0002`, `TC-0003`, `TC-0005` |
| [REQ-0002](req-0002-library-dll-registration.md) | Library DLL and registration with Windows | `TC-0004` |

The test case specifications are `tests/tc-NNNN-<title>.tex`; the CTest names
are `TC-NNNN-<name>`; `tests/run-tc-NNNN.ps1` runs one test case (checked by
`wsp/tools/Test-Traceability.ps1` in the lint stage).

> The `evidence/` paths named in these requirements refer to
> [WFC](https://github.com/Thewafflication/wfc), which keeps the reference
> evidence; the corpus programs are in `tests/corpus` of this repository.
