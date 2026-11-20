# REQ-0002 — Library DLL and registration with Windows

**Content type:** Project requirement

**Status:** Proposed (written from the library README; the registration
behaviour already exists and is tested)

## Statement

- **Entry points.** With `WFC_MSWINSCK_SHARED` the library is built as
  `wfcmswinsck.dll`, which exports the four COM entry points
  (`DllRegisterServer`, `DllUnregisterServer`, `DllGetClassObject` and
  `DllCanUnloadNow`); `DllCanUnloadNow` answers `S_OK` when no object is alive.
- **Version resource.** The DLL carries a version resource whose file version
  is the version in `VERSION` and whose `Comments` string names the Microsoft
  component in `TARGET`.
- **Registration.** `DllRegisterServer` writes `Software\WFC\Libraries\mswinsck`
  (the version, the target and the path of the DLL as `Server`) and, for the
  class, a ProgID and a class id of WFC's own (`WFC.MSWinsockLib.Winsock`) with
  the DLL as in-process server. `DllUnregisterServer` removes everything it
  wrote.
- **Microsoft identifiers.** With `WFC_REGISTER_MICROSOFT_IDS=1` the class is
  also served under the Microsoft ProgID and class id; what was registered
  there before is saved and restored by the unregistration.
- **Scope.** `WFC_REGISTER_SCOPE=user|machine` picks the registry.
- **Class factory.** `DllGetClassObject` answers `CLASS_E_CLASSNOTAVAILABLE`
  for a class id it does not serve; for the class id of `Winsock` it returns a
  class factory whose `CreateInstance` makes a late-bound (`IDispatch`)
  control built from the member table. The control keeps the value written to
  a property, accepts a method call, does not know a name that is not in the
  table, and refuses aggregation (`CLASS_E_NOAGGREGATION`). After the control
  and the factory are released, `DllCanUnloadNow` answers `S_OK`.

## Known limits

The controls the class factory makes have none of the behaviour of the Winsock
control: no sockets and no events. That behaviour (`REQ-0001`) stays with the
program that includes the library (WFC).

## Verification

**Method:** Test

**References:** `TC-0004`

`TC-0004` loads the DLL from a C program, creates a control through the class
factory and runs the registration round trip in the registry of the current
user, under WFC's own identifiers and the Microsoft ones.

## Relationships

- **Related:** `REQ-0001` (the control the DLL describes).
