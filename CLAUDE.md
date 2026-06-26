# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Qt 5.14.2 + MariaDB desktop application implementing Role-Based Access Control (RBAC) user management. Built with Qt Widgets and the `QMYSQL` SQL driver (which talks to MariaDB).

## Build

```bash
qmake
make          # Linux/macOS
# nmake or jom on Windows MSVC
```

Open `UserManagement.pro` in Qt Creator as an alternative. The `.pro` file requires Qt modules: `core gui sql widgets`.

There is no test suite in this project.

## Database setup

Run `schema.sql` against a MariaDB instance before first launch. This creates the `user_mgmt` database and a seed `dev`/`dev123` Developer account.

Edit the connection parameters at the top of `main.cpp` (`openDatabase` call) — host, database name, user, password, port are all hardcoded there.

**QMYSQL driver note**: If you see *"QMYSQL driver not loaded"*, the `qsqlmysql` plugin is missing from your Qt build. Install the MariaDB Connector/C and ensure the plugin `.so`/`.dll` is in `<Qt>/plugins/sqldrivers/`.

## Architecture

### Application flow

`main.cpp` runs a `while(true)` login loop: show `LoginDialog` → on success, construct and show `MainWindow` → on logout (`w.property("logout").toBool() == true`), loop back to login; on window-close (not logout), exit.

### Layers

| Layer | Files | Responsibility |
|---|---|---|
| Access control | `rbac.h/.cpp` | Pure stateless functions — no DB, no Qt signals. All permission decisions go here first. |
| Data access | `databasemanager.h/.cpp` | Singleton (`DatabaseManager::instance()`). All SQL lives here. Every query uses `prepare` + `bindValue`. |
| UI | `mainwindow.*`, `logindialog.*`, `userdialog.*`, `changepassworddialog.*` | Qt Widgets dialogs and main window. |

### RBAC privilege model

Privilege is an integer stored in the DB (`Privilege` column) and passed through the app:

- `0` = Developer — can manage Administrators and Users
- `1` = Administrator — can manage Users only
- `2` = User — can manage nobody (own password only)

Lower number = higher authority. A role **never** manages its own tier or above. The `Rbac` namespace (`rbac.h`) is the single source of truth for all permission logic — `canManage`, `canAdd`, `canChangePassword`, `canUseAddUser/UpdateUser/DeleteUser`.

### Defence-in-depth pattern

Permission is enforced at **two levels** for every write:
1. **UI** — `MainWindow::applyPermissions()` hides (not just disables) menu items and buttons that the logged-in role cannot use.
2. **Dialog** — `UserDialog::apply()` and `ChangePasswordDialog::apply()` each re-check `Rbac::can*()` before calling `DatabaseManager`, regardless of what the UI shows.

When adding new write operations, always follow this pattern.

### UserDialog (dual-mode)

`UserDialog` handles both Add and Edit. The password field is **only shown in Add mode** — existing passwords are changed exclusively through `ChangePasswordDialog`. Mode is set at construction via `UserDialog::Mode` enum.

### ChangePasswordDialog — Windows-style auth

Regardless of whose password is being changed, the dialog always requires the **currently logged-in user** to enter their own current password first (`DatabaseManager::verifyPassword`). Then it checks `Rbac::canChangePassword` with `isSelf` to decide whether the target is allowed.

### DatabaseManager

Singleton, wraps a single `QSqlDatabase` (default connection). All public methods return `bool` on failure and set `m_lastError` — callers check `lastError()` to get the SQL error text for display.

The only table is `userdetails (Username VARCHAR PK, Password VARCHAR, Privilege INT)`.

## Password hashing

Passwords are hashed with **SHA3-256 (NIST FIPS 202)** via `QCryptographicHash::RealSha3_256` and stored as a 64-character lowercase hex string. The private helper `DatabaseManager::hashPassword()` is the single call site — it is applied in `addUser`, `changePassword`, `authenticate`, and `verifyPassword`. Never pass a plaintext password directly to a SQL bind value.

The seed `dev` account in `schema.sql` already stores the pre-computed hash of `dev123`. If you need to seed additional accounts manually, hash the password first:
```python
import hashlib; print(hashlib.sha3_256(b"yourpassword").hexdigest())
```

## UI theming

A global QSS stylesheet is applied in `main.cpp` (`kAppStyle`). Widget-level `styleSheet` properties in `.ui` files are used only for elements that cannot be targeted by class/id selectors in the global sheet (e.g., the blue header frames and white-on-blue labels inside them). Edit `kAppStyle` in `main.cpp` to change colours, fonts, or spacing globally.
