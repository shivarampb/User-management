# User Management System (RBAC) — Qt 5.14.2 + MariaDB

A secure Role-Based Access Control user-management module built with Qt Widgets.

## Roles & hierarchy

| Role          | Privilege | Can manage            |
|---------------|-----------|-----------------------|
| Developer     | 0         | Administrators + Users |
| Administrator | 1         | Users                  |
| User          | 2         | Self (password) only   |

A role can **never** manage its own tier or a higher tier (e.g. a Developer
cannot edit another Developer).

## Feature → permission summary

| Feature          | Developer (0)            | Administrator (1) | User (2)        |
|------------------|--------------------------|-------------------|-----------------|
| Add User         | Admins + Users           | Users             | hidden          |
| Update User      | Admins + Users           | Users             | hidden          |
| Delete User      | Admins + Users           | Users             | hidden          |
| Display list     | Admins + Users           | Users             | nothing         |
| Change Password  | own + Admin + User       | own + User        | own only        |

For Users, the Add/Update/Delete menu items and buttons are **completely
hidden** (not just disabled), as required.

### Change Password — "Windows-style" verification
Changing **any** password requires the *currently logged-in* user to re-enter
their **own** active password to authorise the change, regardless of whose
password is being changed. See `changepassworddialog.cpp`.

## Files

| File | Purpose |
|------|---------|
| `rbac.h/.cpp` | Central access-control rules (the security "middleware") |
| `databasemanager.h/.cpp` | MariaDB connection + parameterised CRUD queries |
| `logindialog.*` | Login screen |
| `mainwindow.*` | Main window: user table, menu, buttons, permission gating |
| `userdialog.*` | Add / Update user dialog |
| `changepassworddialog.*` | Verified password change |
| `main.cpp` | Entry point (login → main window → logout loop) |
| `schema.sql` | Table + seed Developer account |

## Build

1. Open `UserManagement.pro` in Qt Creator (Qt 5.14.2, 64-bit kit), or:
   ```
   qmake
   make            # (nmake / jom on Windows MSVC)
   ```
2. Run `schema.sql` against your MariaDB instance.
3. Edit the connection settings at the top of `main.cpp`
   (host / database / user / password / port).
4. Build and run. Log in with `dev` / `dev123` (then change that password).

## MariaDB driver note (important)

Qt talks to MariaDB through the **`QMYSQL`** SQL driver. If you see
*"QMYSQL driver not loaded"*, the plugin is missing from your Qt build.
On Qt 5.14.2 you typically need to:

- Install the MariaDB Connector/C (provides `libmariadb`/`libmysql`), and
- Ensure the `qsqlmysql` plugin DLL/.so is present in
  `<Qt>/plugins/sqldrivers/`, with the connector library on your PATH
  (Windows) or `LD_LIBRARY_PATH` (Linux).

If your Qt install lacks the plugin, build it from
`<Qt>/Src/qtbase/src/plugins/sqldrivers/mysql` against the connector headers.

## Security notes

- All queries are **parameterised** (`prepare` + `bindValue`) to prevent SQL
  injection.
- Authority is re-checked server-side in each dialog before any write
  (defence-in-depth), not only by hiding UI.
- **Passwords are stored in plain text** to match the given `userdetails`
  schema. For production, hash them (Argon2id or bcrypt) and compare hashes in
  `authenticate()` / `verifyPassword()`; the column already allows 255 chars
  for a hash. This is the single most important hardening step before any real
  deployment.
