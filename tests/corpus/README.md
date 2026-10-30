# Test cases of wfc-mswinsck

The corpus programs here are run by [WFC](https://github.com/Thewafflication/wfc)
(`wfc` is the interpreter and compiler these controls belong to): each
`NN-name.vbp` or `.bas` must print exactly the matching `.expected` text (and
the `.debug` text, when there is one). WFC registers each as the CTest case
`TC-MP0002-corpus-NN-name`, so the cases are kept here with the requirements
they verify (see `requirements/`).

The C and C++ tests of the library itself are in `tests/` and run in this
repository's CI.
