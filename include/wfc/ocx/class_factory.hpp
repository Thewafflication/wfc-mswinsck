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
//
// A library moves the behaviour of a class by giving get_class_object a
// BehaviourFactory: the Behaviour it makes for the class sees every property
// access first and may keep the property itself (an UpDown keeps Min, Max and
// Value in its model and refuses a Value outside the range). What it does not
// handle is stored and accepted as above.
//
// A control also offers one connection point for its events: a client that
// advises a sink gets the events a Behaviour names, by their dispatch ids
// from the event table of the class.

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
#include <ocidl.h>
#include <oleauto.h>

#include <wfc/ocx/control_class.hpp>
#include <wfc/ocx/registration.hpp>

#include <atomic>
#include <cstddef>
#include <cstring>
#include <map>
#include <memory>
#include <new>
#include <string>
#include <utility>
#include <vector>

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

inline std::string lower(const char* text) {
    std::string result(text);
    for (char& c : result) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return result;
}

// A refused access: the error a Visual Basic client sees is `error`.
inline HRESULT fail(EXCEPINFO* exception, const long error) {
    if (exception != nullptr) {
        *exception = EXCEPINFO{};
        exception->scode = static_cast<SCODE>(
            0x800A0000UL | static_cast<unsigned long>(error));
        exception->bstrSource = SysAllocString(L"WFC");
        exception->bstrDescription = SysAllocString(
            error == 380 ? L"Invalid property value" : L"Control error");
    }
    return DISP_E_EXCEPTION;
}

}  // namespace detail

// The behaviour of one control, made by the library (see the top of the file).
// `member` is the member's name in lower case; `index` holds the arguments of
// a parameterized property (DayBold(date)), the last one first as COM passes
// them. Each function returns S_OK when
// it handled the access, S_FALSE when the control should do what it does for
// any member, and E_FAIL with `*error` set to a Visual Basic error number
// (380 for an invalid property value) to refuse it.
class Behaviour {
public:
    struct Index {
        const VARIANT* arguments = nullptr;
        UINT count = 0;
    };

    virtual ~Behaviour() = default;

    // `events` takes the names of the events to raise once the access is done.
    virtual HRESULT put(const std::string& member, const Index& index,
                        const VARIANT& value, long* error,
                        std::vector<std::string>* events) {
        static_cast<void>(member);
        static_cast<void>(index);
        static_cast<void>(value);
        static_cast<void>(error);
        static_cast<void>(events);
        return S_FALSE;
    }

    virtual HRESULT get(const std::string& member, const Index& index,
                        VARIANT* result, long* error) {
        static_cast<void>(member);
        static_cast<void>(index);
        static_cast<void>(result);
        static_cast<void>(error);
        return S_FALSE;
    }
};

// Makes the behaviour of the class called `class_name`; null when the class has
// none (its objects only store their properties).
using BehaviourFactory = std::unique_ptr<Behaviour> (*)(const char* class_name);

// A control of one class of the library.
class Control final : public IDispatch,
                      public IConnectionPointContainer,
                      public IConnectionPoint {
public:
    Control(const wfc::ui::ControlClass* control,
            std::unique_ptr<Behaviour> behaviour)
        : control_(control), behaviour_(std::move(behaviour)) {
        ++g_objects;
    }

    Control(const Control&) = delete;
    Control& operator=(const Control&) = delete;

    ~Control() {
        for (auto& entry : values_) {
            VariantClear(&entry.second);
        }
        for (auto& sink : sinks_) {
            sink.second->Release();
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
        if (IsEqualIID(iid, IID_IConnectionPointContainer)) {
            *object = static_cast<IConnectionPointContainer*>(this);
            AddRef();
            return S_OK;
        }
        if (IsEqualIID(iid, IID_IConnectionPoint)) {
            *object = static_cast<IConnectionPoint*>(this);
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

    // IConnectionPointContainer: one connection point, for the events of the
    // class, whatever source interface the client asks for.
    HRESULT __stdcall EnumConnectionPoints(IEnumConnectionPoints**) override {
        return E_NOTIMPL;
    }

    HRESULT __stdcall FindConnectionPoint(REFIID,
                                          IConnectionPoint** point) override {
        if (point == nullptr) {
            return E_POINTER;
        }
        *point = static_cast<IConnectionPoint*>(this);
        AddRef();
        return S_OK;
    }

    // IConnectionPoint.
    HRESULT __stdcall GetConnectionInterface(IID* iid) override {
        if (iid == nullptr) {
            return E_POINTER;
        }
        *iid = IID_IDispatch;
        return S_OK;
    }

    HRESULT __stdcall GetConnectionPointContainer(
        IConnectionPointContainer** container) override {
        if (container == nullptr) {
            return E_POINTER;
        }
        *container = static_cast<IConnectionPointContainer*>(this);
        AddRef();
        return S_OK;
    }

    HRESULT __stdcall Advise(IUnknown* sink, DWORD* cookie) override {
        if (sink == nullptr || cookie == nullptr) {
            return E_POINTER;
        }
        IDispatch* dispatch = nullptr;
        if (sink->QueryInterface(IID_IDispatch,
                                 reinterpret_cast<void**>(&dispatch)) != S_OK) {
            return CONNECT_E_CANNOTCONNECT;
        }
        *cookie = ++last_cookie_;
        sinks_[*cookie] = dispatch;
        return S_OK;
    }

    HRESULT __stdcall Unadvise(DWORD cookie) override {
        const auto found = sinks_.find(cookie);
        if (found == sinks_.end()) {
            return CONNECT_E_NOCONNECTION;
        }
        found->second->Release();
        sinks_.erase(found);
        return S_OK;
    }

    HRESULT __stdcall EnumConnections(IEnumConnections**) override {
        return E_NOTIMPL;
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

    // Raises the event called `name` on every sink (any letter case); it has
    // no arguments here.
    void raise(const std::string& name) {
        for (std::size_t e = 0; e < control_->event_count; ++e) {
            if (!detail::same_name(control_->events[e].name, name)) {
                continue;
            }
            const DISPID id = static_cast<DISPID>(control_->events[e].id);
            DISPPARAMS none = {nullptr, nullptr, 0, 0};
            // A sink may unadvise while it is called: walk a copy.
            const auto copy = sinks_;
            for (const auto& sink : copy) {
                sink.second->Invoke(id, IID_NULL, 0, DISPATCH_METHOD, &none,
                                    nullptr, nullptr, nullptr);
            }
            return;
        }
    }

    HRESULT __stdcall Invoke(DISPID id, REFIID, LCID, WORD flags,
                             DISPPARAMS* parameters, VARIANT* result,
                             EXCEPINFO* exception, UINT*) override {
        std::string member;
        bool getter = false;
        bool setter = false;
        bool method = false;
        for (std::size_t m = 0; m < control_->member_count; ++m) {
            const wfc::ui::MemberRow& row = control_->members[m];
            if (static_cast<DISPID>(row.id) != id) {
                continue;
            }
            member = detail::lower(row.name);
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
            if (behaviour_ != nullptr) {
                long error = 0;
                std::vector<std::string> events;
                const Behaviour::Index index{parameters->rgvarg + 1,
                                             parameters->cArgs - 1};
                const HRESULT handled = behaviour_->put(
                    member, index, parameters->rgvarg[0], &error, &events);
                if (handled == E_FAIL) {
                    return detail::fail(exception, error);
                }
                if (handled == S_OK) {
                    for (const std::string& name : events) {
                        raise(name);
                    }
                    return S_OK;
                }
            }
            VARIANT& slot = values_[id];
            VariantClear(&slot);
            VariantInit(&slot);
            return VariantCopyInd(&slot, &parameters->rgvarg[0]);
        }
        if ((flags & DISPATCH_PROPERTYGET) != 0 && getter) {
            if (result != nullptr) {
                VariantInit(result);
                if (behaviour_ != nullptr) {
                    long error = 0;
                    const Behaviour::Index index{
                        parameters != nullptr ? parameters->rgvarg : nullptr,
                        parameters != nullptr ? parameters->cArgs : 0};
                    const HRESULT handled =
                        behaviour_->get(member, index, result, &error);
                    if (handled == E_FAIL) {
                        return detail::fail(exception, error);
                    }
                    if (handled == S_OK) {
                        return S_OK;
                    }
                }
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
    std::unique_ptr<Behaviour> behaviour_;
    std::map<DISPID, VARIANT> values_;
    std::map<DWORD, IDispatch*> sinks_;
    DWORD last_cookie_ = 0;
};

// The class factory of one class.
class Factory final : public IClassFactory {
public:
    Factory(const wfc::ui::ControlClass* control, BehaviourFactory behaviour)
        : control_(control), behaviour_(behaviour) {
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
        Control* control = new (std::nothrow)
            Control(control_, behaviour_ != nullptr ? behaviour_(control_->name)
                                                    : nullptr);
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
    BehaviourFactory behaviour_;
};

// DllGetClassObject: the factory of the class whose WFC or Microsoft class id
// is `clsid`. `classes` is the registration table of the library and `tables`
// the member tables, which name the same classes.
inline HRESULT get_class_object(REFCLSID clsid, REFIID iid, void** object,
                                const wfc::registration::ComClass* classes,
                                std::size_t class_count,
                                const wfc::ui::ControlClass* tables,
                                std::size_t table_count,
                                BehaviourFactory behaviour = nullptr) {
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
            Factory* factory =
                new (std::nothrow) Factory(&tables[t], behaviour);
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
