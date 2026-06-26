#ifndef RBAC_H
#define RBAC_H

#include <QString>
#include <QList>

// ---------------------------------------------------------------------------
//  Role-Based Access Control logic.
//
//  Privilege hierarchy (lower number = higher power):
//      Developer     = 0  -> may manage Administrators and Users
//      Administrator = 1  -> may manage Users only
//      User          = 2  -> may manage nobody (self password only)
//
//  Strict rule: a role may NEVER manage its own tier or a higher tier.
//  All UI and database actions are validated against these functions.
// ---------------------------------------------------------------------------
namespace Rbac {

enum Privilege {
    Developer     = 0,
    Administrator = 1,
    User          = 2
};

// Human-readable role name.
QString roleName(int privilege);

// Privilege levels the actor may view / update / delete.
QList<int> manageableLevels(int actorPrivilege);

// Privilege levels the actor may assign when adding/editing a user.
QList<int> assignableLevels(int actorPrivilege);

// Can the actor view/update/delete a target of the given privilege?
bool canManage(int actorPrivilege, int targetPrivilege);

// Can the actor create a user of the given privilege?
bool canAdd(int actorPrivilege, int targetPrivilege);

// Can the actor change the password of the target?
// isSelf == true means the target is the currently logged-in user.
bool canChangePassword(int actorPrivilege, int targetPrivilege, bool isSelf);

// Menu / button visibility helpers.
bool canUseAddUser(int actorPrivilege);
bool canUseUpdateUser(int actorPrivilege);
bool canUseDeleteUser(int actorPrivilege);

} // namespace Rbac

#endif // RBAC_H
