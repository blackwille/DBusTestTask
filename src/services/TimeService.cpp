#include "services/TimeService.h"

#include <sdbus-c++/Types.h>
#include <sdbus-c++/sdbus-c++.h>

#include <chrono>
#include <cstdint>
#include <string>

#include "common/Permissions.h"
#include "common/Utils.h"

namespace services {

TimeService::TimeService() {
    m_connection = sdbus::createBusConnection(m_serviceName);

    sdbus::ObjectPath objectPath{std::string{TS_DEFAULT_OBJECT_NAME}};
    m_object = sdbus::createObject(*m_connection, objectPath);

    auto GetSystemTime = [this](sdbus::MethodCall call) -> void { return this->GetSystemTime(std::move(call)); };
    m_object
        ->addVTable(sdbus::MethodVTableItem{sdbus::MethodName{"GetSystemTime"},
                                            {},
                                            {},
                                            sdbus::Signature{"t"},
                                            {"systemTimestamp"},
                                            {std::move(GetSystemTime)},
                                            {}})
        .forInterface(sdbus::InterfaceName(std::string{TS_DEFAULT_SERVICE_NAME}));

    m_connection->enterEventLoopAsync();
}

void TimeService::GetSystemTime(sdbus::MethodCall call) const {
    sdbus::InterfaceName interfaceName{std::string{TS_DEFAULT_SERVICE_NAME}};
    std::string senderAddr = call.getSender();
    bool isSenderHasPermission =
        common::IsSenderHasPermission(interfaceName, senderAddr, common::Permissions::SystemTime);

    if (not isSenderHasPermission) {
        sdbus::Error::Name errorName{std::string(TS_DEFAULT_SERVICE_NAME) + ".UnauthorizedAccess"};
        throw sdbus::Error{errorName,
                           "Application has not access to system time. Please, get it via com.system.permissions"};
    }

    auto durationNow = std::chrono::system_clock::now().time_since_epoch();
    uint64_t timestampNow = std::chrono::duration_cast<std::chrono::seconds>(durationNow).count();

    auto reply = call.createReply();
    reply << timestampNow;
    reply.send();
}

};  // namespace services