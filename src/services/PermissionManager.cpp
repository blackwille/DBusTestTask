#include "services/PermissionManager.h"

#include <SQLiteCpp/Statement.h>
#include <sdbus-c++/sdbus-c++.h>

#include <string>

#include "common/Permissions.h"
#include "common/Utils.h"

namespace services {

PermissionManager::PermissionManager() {
    m_connection = sdbus::createSessionBusConnection(m_serviceName);

    sdbus::ObjectPath objectPath{std::string{PM_DEFAULT_OBJECT_NAME}};
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
        .forInterface(sdbus::InterfaceName(std::string{PM_DEFAULT_SERVICE_NAME}));

    m_connection->enterEventLoopAsync();
}

void PermissionManager::RequestPermission(sdbus::MethodCall call) {
    unsigned int permissionEnumCode;
    call >> permissionEnumCode;
    if (not(permissionEnumCode < common::Permissions::Offset)) {
        sdbus::Error::Name errorName{std::string(PM_DEFAULT_SERVICE_NAME) + ".UnknownPermission"};
        throw sdbus::Error{errorName, "Unknown permission in params"};
    }

    std::string senderAddr = call.getSender();
    std::string exePath{common::GetSenderExecPath(std::string(PM_DEFAULT_SERVICE_NAME), senderAddr)};

    if (not CheckApplicationHasPermission(exePath, permissionEnumCode)) {
        try {
            SQLite::Statement insertQuery(m_dbPermissions,
                                          "INSERT INTO apps_permissions (app_path, permission_enum_code) "
                                          "VALUES (?, ?);");
            insertQuery.bind(1, exePath);
            insertQuery.bind(2, permissionEnumCode);
            insertQuery.exec();
        } catch (const std::exception& e) {
            sdbus::Error::Name errorName{std::string(PM_DEFAULT_SERVICE_NAME) + ".DbInsert"};
            throw sdbus::Error{errorName, std::string("Can't insert into apps_permissions. Reason: ") + e.what()};
        }
    }

    auto reply = call.createReply();
    reply.send();
}

bool PermissionManager::CheckApplicationHasPermission(const std::string& exePath,
                                                      unsigned int permissionEnumCode) const {
    if (not(permissionEnumCode < common::Permissions::Offset)) {
        sdbus::Error::Name errorName{std::string(PM_DEFAULT_SERVICE_NAME) + ".UnknownPermission"};
        throw sdbus::Error{errorName, "Unknown permission in params"};
    }
    bool hasPermission = false;
    try {
        SQLite::Statement selectQuery(
            m_dbPermissions, "SELECT * FROM apps_permissions WHERE app_path = ? AND permission_enum_code = ?;");
        selectQuery.bind(1, exePath);
        selectQuery.bind(2, permissionEnumCode);
        if (selectQuery.executeStep()) {
            hasPermission = true;
        }
    } catch (const std::exception& e) {
        sdbus::Error::Name errorName{std::string(PM_DEFAULT_SERVICE_NAME) + ".DbSelect"};
        throw sdbus::Error{errorName, std::string("Can't select in apps_permissions table. Reason: ") + e.what()};
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
        sdbus::Error::Name errorName{std::string(PM_DEFAULT_SERVICE_NAME) + ".DbCreate"};
        throw sdbus::Error{errorName, std::string("Can't create an apps_permissions table. Reason: ") + e.what()};
    }
}

};  // namespace services