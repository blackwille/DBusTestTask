#ifndef UTILS_H
#define UTILS_H

#include <string>

#include "common/Permissions.h"

namespace common {

std::string GetSenderExecPath(const std::string& callerInterfaceName, const std::string& senderAddr);
bool IsSenderHasPermission(const std::string& callerInterfaceName, const std::string& senderAddr,
                           Permissions permission);
};  // namespace common

#endif  // UTILS_H