# Test cases of wfc-mswinsck

The corpus programs here are run by [WFC](https://github.com/Thewafflication/wfc)
(`wfc` is the interpreter and compiler these controls belong to): each
`NN-name.vbp` or `.bas` must print exactly the matching `.expected` text (and
the `.debug` text, when there is one). WFC registers each as the CTest case
`TC-MP0002-corpus-NN-name` (its own namespace). This repository runs
`196-ocx-winsock-dialog` as `TC-0005-winsock-corpus` when CMake is given
`-DWFC_EXECUTABLE=<wfc.exe>` (`run_corpus_case.cmake` is the runner); the
case is specified in `tests/tc-0005-winsock-corpus.tex` and verifies
`REQ-0001` (see `requirements/`).

The C and C++ tests of the library itself are in `tests/` and run in this
repository's CI.
