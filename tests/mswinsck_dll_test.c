/* Black-box check of wfcmswinsck.dll: exports, version resource and the
 * registration round trip (see dll_check.h). Run by CTest with the DLL,
 * the version and the target as arguments. */
#include "dll_check.h"

static const struct wfc_check_class kClasses[] = {
    {L"WFC.MSWinsockLib.Winsock",
     L"{321E9CDB-8E15-5447-9E85-37085D23CD1E}",
     L"MSWinsockLib.Winsock.1",
     L"{248DD896-BB45-11CF-9ABC-0080C7E7B78D}"},
};

int main(int argc, char** argv) {
    return wfc_check_dll(argc, argv, L"mswinsck", kClasses,
                         sizeof(kClasses) / sizeof(kClasses[0]));
}
