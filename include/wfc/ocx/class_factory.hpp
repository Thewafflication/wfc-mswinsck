// The COM side of a WFC control library: DllGetClassObject and a class
// factory whose objects are late-bound (IDispatch) controls described by the
// member table of the library. This file is identical in every WFC control
// library (wfc-mscomctl, wfc-comdlg32, wfc-mswinsck, wfc-mscomctl2); a WFC
// test fails when the copies differ. Windows only; internal to the WFC
// libraries.
//
// An object made here needs no WFC evaluator. It knows its members from the
// table: GetIDsOfNames finds a member by name (any case), a property that has
// a setter stores the Variant it is given and the getter returns it, and a
// method is accepted and returns Empty. The behaviour of the control (events,
// drawing, the work of the methods) is still in the evaluator, and moves into
// the library later (see ROADMAP.md).

#ifndef WFC_CLASS_FACTORY_HPP
#define WFC_CLASS_FACTORY_HPP

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <objbase.h>
#include <oleauto.h>

#include <wfc/ocx/control_class.hpp>
#include <wfc/ocx/registration.hpp>

#include <atomic>
#include <cstddef>
#include <cstring>
#include <map>
#include <new>
#include <string>

namespace wfc::com {

inline std::atomic<long> g_objects{0};
inline std::atomic<long> g_locks{0};

namespace detail {

inline std::string narrow(const wchar_t* text) {
    std::string result;
    for (; text != nullptr && *text != L'\0'; ++text) {
        result.push_back(*text < 128 ? static_cast<char>(*text) : '?');
    }
    return result;
}

inline bool same_name(const char* a, const std::string& b) {
    return _stricmp(a, b.c_str()) == 0;
}

}  // namespace detail

// A control of one class of the library.
class Control final : public IDispatch {
public:
    explicit Control(const wfc::ui::ControlClass* control) : control_(control) {
        ++g_objects;
    }

    Control(const Control&) = delete;
    Control& operator=(const Control&) = delete;

    ~Control() {
        for (auto& entry : values_) {
            VariantClear(&entry.second);
        }
        --g_objects;
    }

    HRESULT __stdcall QueryInterface(REFIID iid, void** object) override {
        if (object == nullptr) {
            return E_POINTER;
        }
        if (IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IDispatch)) {
            *object = static_cast<IDispatch*>(this);
            AddRef();
            return S_OK;
        }
        *object = nullptr;
        return E_NOINTERFACE;
    }

    ULONG __stdcall AddRef() override {
        return static_cast<ULONG>(++references_);
    }

    ULONG __stdcall Release() override {
        const long left = --references_;
        if (left == 0) {
            delete this;
        }
        return static_cast<ULONG>(left);
    }

    HRESULT __stdcall GetTypeInfoCount(UINT* count) override {
        if (count == nullptr) {
            return E_POINTER;
        }
        *count = 0;
        return S_OK;
    }

    HRESULT __stdcall GetTypeInfo(UINT, LCID, ITypeInfo** info) override {
        if (info != nullptr) {
            *info = nullptr;
        }
        return E_NOTIMPL;
    }

    HRESULT __stdcall GetIDsOfNames(REFIID, LPOLESTR* names, UINT count, LCID,
                                    DISPID* ids) override {
        if (names == nullptr || ids == nullptr) {
            return E_POINTER;
        }
        HRESULT result = S_OK;
        for (UINT i = 0; i < count; ++i) {
            ids[i] = DISPID_UNKNOWN;
        }
        for (UINT i = 0; i < count; ++i) {
            // Only the first name is a member; the rest are parameters.
            const std::string wanted = detail::narrow(names[i]);
            const wfc::ui::MemberRow* found = nullptr;
            if (i == 0) {
                for (std::size_t m = 0; m < control_->member_count; ++m) {
                    if (detail::same_name(control_->members[m].name, wanted)) {
                        found = &control_->members[m];
                        break;
                    }
                }
            }
            if (found != nullptr) {
                ids[i] = static_cast<DISPID>(found->id);
            } else {
                result = DISP_E_UNKNOWNNAME;
            }
        }
        return result;
    }

    HRESULT __stdcall Invoke(DISPID id, REFIID, LCID, WORD flags,
                             DISPPARAMS* parameters, VARIANT* result,
                             EXCEPINFO*, UINT*) override {
        bool getter = false;
        bool setter = false;
        bool method = false;
        for (std::size_t m = 0; m < control_->member_count; ++m) {
            const wfc::ui::MemberRow& row = control_->members[m];
            if (static_cast<DISPID>(row.id) != id) {
                continue;
            }
            getter = getter || row.kind == 'g';
            setter = setter || row.kind == 'l' || row.kind == 's';
            method = method || row.kind == 'm';
        }
        if (!getter && !setter && !method) {
            return DISP_E_MEMBERNOTFOUND;
        }
        if ((flags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF)) != 0) {
            if (!setter || parameters == nullptr || parameters->cArgs < 1) {
                return DISP_E_MEMBERNOTFOUND;
            }
            // The value is the last argument (the first of rgvarg).
            VARIANT& slot = values_[id];
            VariantClear(&slot);
            VariantInit(&slot);
            return VariantCopyInd(&slot, &parameters->rgvarg[0]);
        }
        if ((flags & DISPATCH_PROPERTYGET) != 0 && getter) {
            if (result != nullptr) {
                VariantInit(result);
                const auto found = values_.find(id);
                if (found != values_.end()) {
                    return VariantCopy(result, &found->second);
                }
            }
            return S_OK;
        }
        if ((flags & DISPATCH_METHOD) != 0 && method) {
            if (result != nullptr) {
                VariantInit(result);
            }
            return S_OK;
        }
        return DISP_E_MEMBERNOTFOUND;
    }

private:
    std::atomic<long> references_{1};
    const wfc::ui::ControlClass* control_;
    std::map<DISPID, VARIANT> values_;
};

// The class factory of one class.
class Factory final : public IClassFactory {
public:
    explicit Factory(const wfc::ui::ControlClass* control) : control_(control) {
        ++g_objects;
    }

    Factory(const Factory&) = delete;
    Factory& operator=(const Factory&) = delete;

    ~Factory() { --g_objects; }

    HRESULT __stdcall QueryInterface(REFIID iid, void** object) override {
        if (object == nullptr) {
            return E_POINTER;
        }
        if (IsEqualIID(iid, IID_IUnknown) ||
            IsEqualIID(iid, IID_IClassFactory)) {
            *object = static_cast<IClassFactory*>(this);
            AddRef();
            return S_OK;
        }
        *object = nullptr;
        return E_NOINTERFACE;
    }

    ULONG __stdcall AddRef() override {
        return static_cast<ULONG>(++references_);
    }

    ULONG __stdcall Release() override {
        const long left = --references_;
        if (left == 0) {
            delete this;
        }
        return static_cast<ULONG>(left);
    }

    HRESULT __stdcall CreateInstance(IUnknown* outer, REFIID iid,
                                     void** object) override {
        if (object == nullptr) {
            return E_POINTER;
        }
        *object = nullptr;
        if (outer != nullptr) {
            return CLASS_E_NOAGGREGATION;
        }
        Control* control = new (std::nothrow) Control(control_);
        if (control == nullptr) {
            return E_OUTOFMEMORY;
        }
        const HRESULT result = control->QueryInterface(iid, object);
        control->Release();
        return result;
    }

    HRESULT __stdcall LockServer(BOOL lock) override {
        if (lock != FALSE) {
            ++g_locks;
        } else {
            --g_locks;
        }
        return S_OK;
    }

private:
    std::atomic<long> references_{1};
    const wfc::ui::ControlClass* control_;
};

// DllGetClassObject: the factory of the class whose WFC or Microsoft class id
// is `clsid`. `classes` is the registration table of the library and `tables`
// the member tables, which name the same classes.
inline HRESULT get_class_object(REFCLSID clsid, REFIID iid, void** object,
                                const wfc::registration::ComClass* classes,
                                std::size_t class_count,
                                const wfc::ui::ControlClass* tables,
                                std::size_t table_count) {
    if (object == nullptr) {
        return E_POINTER;
    }
    *object = nullptr;
    for (std::size_t c = 0; c < class_count; ++c) {
        CLSID wfc_id{};
        CLSID ms_id{};
        const bool wfc_match =
            CLSIDFromString(classes[c].wfc_clsid, &wfc_id) == S_OK &&
            IsEqualCLSID(clsid, wfc_id);
        const bool ms_match =
            classes[c].ms_clsid != nullptr &&
            CLSIDFromString(classes[c].ms_clsid, &ms_id) == S_OK &&
            IsEqualCLSID(clsid, ms_id);
        if (!wfc_match && !ms_match) {
            continue;
        }
        const std::string name = detail::narrow(classes[c].name);
        for (std::size_t t = 0; t < table_count; ++t) {
            if (!detail::same_name(tables[t].name, name)) {
                continue;
            }
            Factory* factory = new (std::nothrow) Factory(&tables[t]);
            if (factory == nullptr) {
                return E_OUTOFMEMORY;
            }
            const HRESULT result = factory->QueryInterface(iid, object);
            factory->Release();
            return result;
        }
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}

// DllCanUnloadNow.
inline HRESULT can_unload() {
    return g_objects.load() == 0 && g_locks.load() == 0 ? S_OK : S_FALSE;
}

}  // namespace wfc::com

#endif  // WFC_CLASS_FACTORY_HPP
