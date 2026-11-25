#pragma once

#include <cstdint>

#include "Role.hpp"

enum class Permission {
    ChangeOwnPassword,
    UpdateOwnStatus,
    BlockUser,
    UnblockUser,
    RevokeAccess
};

// Decides who may do what. Only two roles: User and Admin.
class PermissionService {
public:
    static bool hasPermission(Role role, Permission permission);

    // Own actions only on yourself; admin actions only on other users.
    static bool canActOnUser(Role actorRole,
                             std::int64_t actorIdentifier,
                             Permission permission,
                             std::int64_t targetIdentifier);
};
