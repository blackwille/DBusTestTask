#include "PermissionManager.h"

#include <SQLiteCpp/Statement.h>
#include <sdbus-c++/Error.h>
#include <sdbus-c++/IObject.h>
#include <sdbus-c++/Types.h>
#include <sdbus-c++/VTableItems.h>
#include <sdbus-c++/sdbus-c++.h>
#include <unistd.h>

#include <climits>
#include <string>

#include "common/Permissions.h"

PermissionManager::PermissionManager() {
    m_connection = sdbus::createBusConnection(m_serviceName);

    sdbus::ObjectPath objectPath{std::string{DEFAULT_OBJECT_NAME}};
    m_object = sdbus::createObject(*m_connection, objectPath);

    CreateTableAppsPermissions();

    auto RequestPermissionCb = [this](sdbus::MethodCall call) -> void {
        return this->RequestPermission(std::move(call));
    };
    auto CheckApplicationHasPermissionCb = [this](const std::string& exePath, unsigned int permissionEnumCode) -> bool {
        return this->CheckApplicationHasPermission(exePath, permissionEnumCode);
    };
    m_object
        ->addVTable(sdbus::MethodVTableItem{sdbus::MethodName{"RequestPermission"},
                                            sdbus::Signature{"u"},
                                            {"permissionEnumCode"},
                                            {},
                                            {},
                                            {std::move(RequestPermissionCb)},
                                            {}},
                    sdbus::registerMethod("CheckApplicationHasPermission")
                        .withInputParamNames("applicationExecPath", "permissionEnumCode")
                        .withOutputParamNames("isAppHasPermission")
                        .implementedAs(std::move(CheckApplicationHasPermissionCb)))
        .forInterface(sdbus::InterfaceName(std::string{DEFAULT_SERVICE_NAME}));

    m_connection->enterEventLoopAsync();
}

void PermissionManager::RequestPermission(sdbus::MethodCall call) {
    unsigned int permissionEnumCode;
    call >> permissionEnumCode;
    if (not(permissionEnumCode < Permissions::Offset)) {
        sdbus::Error::Name errorName{std::string(DEFAULT_SERVICE_NAME) + ".PermissionError"};
        throw sdbus::Error{errorName, "Unknown permission"};
    }

    std::string senderAddr = call.getSender();
    std::string exePath{GetSenderExecPath(senderAddr)};

    if (not CheckApplicationHasPermission(exePath, permissionEnumCode)) {
        try {
            SQLite::Statement insertQuery(m_dbPermissions,
                                          "INSERT INTO apps_permissions (app_path, permission_enum_code) "
                                          "VALUES (?, ?);");
            insertQuery.bind(1, exePath);
            insertQuery.bind(2, permissionEnumCode);
            insertQuery.exec();
        } catch (const std::exception& e) {
            sdbus::Error::Name errorName{std::string(DEFAULT_SERVICE_NAME) + ".DbError"};
            throw sdbus::Error{errorName, std::string("Can't insert into apps_permissions. Reason: ") + e.what()};
        }
    }

    auto reply = call.createReply();
    reply.send();
}

bool PermissionManager::CheckApplicationHasPermission(const std::string& exePath,
                                                      unsigned int permissionEnumCode) const {
    bool hasPermission = false;
    try {
        SQLite::Statement insertQuery(
            m_dbPermissions, "SELECT * FROM apps_permissions WHERE app_path = ? AND permission_enum_code = ?;");
        insertQuery.bind(1, exePath);
        insertQuery.bind(2, permissionEnumCode);
        if (insertQuery.executeStep()) {
            hasPermission = true;
        }
    } catch (const std::exception& e) {
        sdbus::Error::Name errorName{std::string(DEFAULT_SERVICE_NAME) + ".DbError"};
        throw sdbus::Error{errorName, std::string("Can't create an apps_permissions table. Reason: ") + e.what()};
    }
    return hasPermission;
}

void PermissionManager::CreateTableAppsPermissions() {
    std::string createQuery{
        "CREATE TABLE IF NOT EXISTS apps_permissions ( "
        "   app_path TEXT NOT NULL,"
        "   permission_enum_code INT NOT NULL,"
        "   PRIMARY KEY(app_path, permission_enum_code)"
        ");"};
    try {
        m_dbPermissions.exec(createQuery);
    } catch (const std::exception& e) {
        sdbus::Error::Name errorName{std::string(DEFAULT_SERVICE_NAME) + ".DbError"};
        throw sdbus::Error{errorName, std::string("Can't create an apps_permissions table. Reason: ") + e.what()};
    }
}

std::string PermissionManager::GetSenderExecPath(const std::string& senderAddr) {
    auto dbusProxy =
        sdbus::createProxy(sdbus::ServiceName("org.freedesktop.DBus"), sdbus::ObjectPath("/org/freedesktop/DBus"));
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
        sdbus::Error::Name errorName{std::string(DEFAULT_SERVICE_NAME) + ".PathError"};
        throw sdbus::Error{errorName, std::string("Can't find exe path of pid: ") + std::to_string(pid)};
    }
}