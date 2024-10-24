#include "common/Utils.h"

#include <sdbus-c++/IConnection.h>
#include <sdbus-c++/IProxy.h>
#include <sdbus-c++/Types.h>
#include <sdbus-c++/sdbus-c++.h>
#include <unistd.h>

#include <climits>
#include <string>

namespace common {

std::string GetSenderExecPath(const std::string& callerInterfaceName, const std::string& senderAddr) {
    auto connection = sdbus::createSessionBusConnection();
    auto dbusProxy = sdbus::createProxy(*connection, sdbus::ServiceName("org.freedesktop.DBus"),
                                        sdbus::ObjectPath("/org/freedesktop/DBus"));
    uint32_t pid;
    dbusProxy->callMethod("GetConnectionUnixProcessID")
        .onInterface("org.freedesktop.DBus")
        .withArguments(senderAddr)
        .storeResultsTo(pid);

    std::string procExePath = "/proc/" + std::to_string(pid) + "/exe";
    char exePath[PATH_MAX];
    ssize_t len = readlink(procExePath.c_str(), exePath, sizeof(exePath) - 1);
    if (len != -1) {
        exePath[len] = '\0';
        return std::string(exePath);
    } else {
        sdbus::Error::Name errorName{callerInterfaceName + ".InvalidPath"};
        throw sdbus::Error{errorName, std::string("Can't find exe path of pid: ") + std::to_string(pid)};
    }
}

bool IsSenderHasPermission(const std::string& callerInterfaceName, const std::string& senderAddr,
                           Permissions permission) {
    std::string exePath = GetSenderExecPath(callerInterfaceName, senderAddr);

    auto connection = sdbus::createSessionBusConnection();
    auto dbusProxy =
        sdbus::createProxy(*connection, sdbus::ServiceName("com.system.permissions"), sdbus::ObjectPath("/"));
    bool isSenderHasPermission;
    dbusProxy->callMethod("CheckApplicationHasPermission")
        .onInterface("com.system.permissions")
        .withArguments(exePath, permission)
        .storeResultsTo(isSenderHasPermission);

    return isSenderHasPermission;
}

};  // namespace common