#ifndef PERMISSION_MANAGER_H
#define PERMISSION_MANAGER_H

#include <SQLiteCpp/SQLiteCpp.h>
#include <sdbus-c++/IConnection.h>
#include <sdbus-c++/IObject.h>
#include <sdbus-c++/Types.h>

#include <memory>

namespace services {

inline constexpr std::string_view PM_DEFAULT_DB_NAME = "permissions.db3";
inline constexpr std::string_view PM_DEFAULT_SERVICE_NAME = "com.system.permissions";
inline constexpr std::string_view PM_DEFAULT_OBJECT_NAME = "/";

class PermissionManager {
public:
    static PermissionManager& Locate() {
        static PermissionManager instance;
        return instance;
    }

    PermissionManager(const PermissionManager& rhs) = delete;
    void operator=(const PermissionManager& rhs) = delete;

private:
    PermissionManager();
    // We need sender data. I don't know how to get it in convenience mode. Used basic call instead.
    void RequestPermission(sdbus::MethodCall call);
    bool CheckApplicationHasPermission(const std::string& exePath, unsigned int permissionEnumCode) const;
    void CreateTableAppsPermissions();

private:
    std::unique_ptr<sdbus::IConnection> m_connection;
    std::unique_ptr<sdbus::IObject> m_object;
    sdbus::ServiceName m_serviceName{std::string{PM_DEFAULT_SERVICE_NAME}};
    SQLite::Database m_dbPermissions{PM_DEFAULT_DB_NAME, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE};
};

};  // namespace services

#endif  // PERMISSION_MANAGER_H