// Registration of the Winsock library with Windows: the entry points
// regsvr32 calls. The work is in wfc/ocx/registration.hpp. Internal to the WFC
// libraries.
//
// The class table lists the controls of the library: WFC's own ProgIDs and
// class ids, and the Microsoft ones used when WFC_REGISTER_MICROSOFT_IDS=1.
// DllGetClassObject gives the class factory of a class
// (wfc/ocx/class_factory.hpp): its objects are late-bound controls built from
// the member table, without the WFC evaluator.

#include <wfc/mswinsck/mswinsck.hpp>
#include <wfc/ocx/class_factory.hpp>
#include <wfc/ocx/registration.hpp>
#include "registration_info.hpp"

#include <iterator>

namespace {

const wfc::registration::Library kLibrary{L"mswinsck", L"" WFC_LIB_VERSION,
                                          L"" WFC_LIB_TARGET};

const wfc::registration::ComClass kClasses[] = {
    {L"Winsock", L"{321E9CDB-8E15-5447-9E85-37085D23CD1E}",
     L"WFC.MSWinsockLib.Winsock", L"{248DD896-BB45-11CF-9ABC-0080C7E7B78D}",
     L"MSWinsockLib.Winsock.1"},
};

HMODULE this_module() {
    HMODULE module = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                           GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       reinterpret_cast<LPCWSTR>(&this_module), &module);
    return module;
}

}  // namespace

extern "C" {

HRESULT __stdcall DllRegisterServer() {
    return wfc::registration::register_library(this_module(), kLibrary,
                                               kClasses, std::size(kClasses));
}

HRESULT __stdcall DllUnregisterServer() {
    return wfc::registration::unregister_library(
        kLibrary, kClasses, std::size(kClasses), this_module());
}

HRESULT __stdcall DllCanUnloadNow() {
    return wfc::com::can_unload();
}

HRESULT __stdcall DllGetClassObject(REFCLSID clsid, REFIID iid, void** object) {
    const auto tables = wfc::ui::mswinsck_classes();
    return wfc::com::get_class_object(clsid, iid, object, kClasses,
                                      std::size(kClasses), tables.data(),
                                      tables.size());
}

}  // extern "C"
