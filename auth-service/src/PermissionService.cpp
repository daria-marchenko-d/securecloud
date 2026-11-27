#include "PermissionService.hpp"

namespace {

bool isOwnAction(Permission permission)
{
    return permission == Permission::ChangeOwnPassword
        || permission == Permission::UpdateOwnStatus;
}

}

bool PermissionService::hasPermission(Role role, Permission permission)
{
    if (role == Role::Admin) {
        return true;
    }
    return isOwnAction(permission);
}

bool PermissionService::canActOnUser(Role actorRole,
                                     std::int64_t actorIdentifier,
                                     Permission permission,
                                     std::int64_t targetIdentifier)
{
    if (!hasPermission(actorRole, permission)) {
        return false;
    }
    if (isOwnAction(permission)) {
        return actorIdentifier == targetIdentifier;
    }
    return actorIdentifier != targetIdentifier;
}
