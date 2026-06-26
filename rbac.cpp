#include "rbac.h"

namespace Rbac {

QString roleName(int privilege)
{
    switch (privilege) {
    case Developer:     return QStringLiteral("Developer");
    case Administrator: return QStringLiteral("Administrator");
    case User:          return QStringLiteral("User");
    default:            return QStringLiteral("Unknown");
    }
}

QList<int> manageableLevels(int actorPrivilege)
{
    switch (actorPrivilege) {
    case Developer:     return { Administrator, User }; // 1, 2
    case Administrator: return { User };                // 2
    case User:          return {};                      // none
    default:            return {};
    }
}

QList<int> assignableLevels(int actorPrivilege)
{
    // You may only create/assign roles strictly below your own tier.
    return manageableLevels(actorPrivilege);
}

bool canManage(int actorPrivilege, int targetPrivilege)
{
    return manageableLevels(actorPrivilege).contains(targetPrivilege);
}

bool canAdd(int actorPrivilege, int targetPrivilege)
{
    return assignableLevels(actorPrivilege).contains(targetPrivilege);
}

bool canChangePassword(int actorPrivilege, int targetPrivilege, bool isSelf)
{
    if (isSelf)
        return true; // everyone may change their own password
    return canManage(actorPrivilege, targetPrivilege);
}

bool canUseAddUser(int actorPrivilege)
{
    return !assignableLevels(actorPrivilege).isEmpty();
}

bool canUseUpdateUser(int actorPrivilege)
{
    return !manageableLevels(actorPrivilege).isEmpty();
}

bool canUseDeleteUser(int actorPrivilege)
{
    return !manageableLevels(actorPrivilege).isEmpty();
}

} // namespace Rbac
