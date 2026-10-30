# wfc-mswinsck

Winsock for WFC: the native replacement for `mswinsck.ocx` (library MSWinsockLib).

- `src/mswinsck_tables.cpp` the member table of the control.
- `src/winsock_io.cpp` the non-blocking socket layer; it stays in the program
  (target `wfc_mswinsck_io`), not in the DLL.
- `include/wfc/mswinsck/mswinsck.hpp` the accessor `wfc::ui::mswinsck_classes()`.
- `include/wfc/mswinsck/mswinsck.inc` the behaviour of the control: the
  properties, methods and events, compiled into the WFC evaluator, which
  includes it (`src/interpreter/forms.cpp`). It uses the evaluator's internal
  types, so it builds only inside WFC.
- `include/wfc/ocx/control_class.hpp` the table types, identical in every WFC
  control library (a WFC test checks the copies).
- `VERSION` the library version, stamped into `wfcmswinsck.dll`.

Target: `wfc_mswinsck` (static library, or `wfcmswinsck.dll` with `WFC_MSWINSCK_SHARED=ON`;
the parent default is `WFC_OCX_SHARED`).

## Version and target

The version is in `VERSION` and the Microsoft component it stands in for in
`TARGET`: **mswinsck** targets MSWINSCK.OCX 6.1.98 (Visual Basic 6.0 SP6). The version stays
`0.<target major>.<target minor>` until the library matches the Microsoft one
functionally; then it takes the Microsoft version. Both are stamped into the
DLL's version resource (`FileVersion`, `Comments`).

## Registration with Windows

`regsvr32 wfcmswinsck.dll` (`/u` to remove) writes `Software\WFC\Libraries\mswinsck`
(version, target, path) and, for every control, a class under WFC's own ProgID and class id (`WFC.<Library>.<Class>`) with the DLL as in-process server. `WFC_REGISTER_SCOPE=user|machine` picks the registry
(default: the machine, the user when that is not writable). With `WFC_REGISTER_MICROSOFT_IDS=1` the controls are also served under the Microsoft ProgIDs and class ids; what was registered there before is saved and restored by `regsvr32 /u`. The library does not create the controls itself yet: `DllGetClassObject` reports that the class is not available; the evaluator of WFC implements the controls.

Each library is also a WPM package of its own (`wfc-mswinsck`), whose install script
copies the DLL to `lib\wfcmswinsck\<version>\<arch>` of the WFC installation and
registers it, so dynamically linked programs get an update by installing that
package alone.

Part of [WFC](https://github.com/Thewafflication/wfc), where it is mounted at
`lib/wfc-mswinsck`. Licence: GPL-3.0-or-later (see the WFC `LICENSE`).
