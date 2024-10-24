#include <sdbus-c++/IConnection.h>
#include <sdbus-c++/Types.h>
#include <sdbus-c++/sdbus-c++.h>

#include <cstdint>
#include <ctime>
#include <iostream>
#include <string>

#include "common/Permissions.h"

const std::string PM_SERVICE_NAME = "com.system.permissions";
const std::string TS_SERVICE_NAME = "com.system.time";
const std::string TS_GET_TIME = "GetSystemTime";
const std::string PM_REQUEST_PERMISSION = "RequestPermission";

int main() {
    uint64_t timestampNow = 0;
    std::unique_ptr<sdbus::IConnection> connection{};
    try {
        connection = sdbus::createSessionBusConnection();
        auto dbusProxyTS = sdbus::createProxy(*connection, sdbus::ServiceName(TS_SERVICE_NAME), sdbus::ObjectPath("/"));
        dbusProxyTS->callMethod(TS_GET_TIME)
            .onInterface(sdbus::InterfaceName(TS_SERVICE_NAME))
            .storeResultsTo(timestampNow);
    } catch (const sdbus::Error& e) {
        if (e.getName() == TS_SERVICE_NAME + ".UnauthorizedAccess") {
            std::cout << "Can not get time due to a lack of permission. Trying to get permission from "
                      << PM_SERVICE_NAME << "..." << std::endl;
            try {
                auto dbusProxyPM =
                    sdbus::createProxy(*connection, sdbus::ServiceName(PM_SERVICE_NAME), sdbus::ObjectPath("/"));
                dbusProxyPM->callMethod(PM_REQUEST_PERMISSION)
                    .onInterface(sdbus::InterfaceName(PM_SERVICE_NAME))
                    .withArguments(common::Permissions::SystemTime);
            } catch (const sdbus::Error& anotherErr) {
                std::cout << "Can not request permission due to sdbus error with name: " << anotherErr.getName()
                          << " and message: " << anotherErr.getMessage() << std::endl;
                return 0;
            }
            try {
                auto dbusProxyTS =
                    sdbus::createProxy(*connection, sdbus::ServiceName(TS_SERVICE_NAME), sdbus::ObjectPath("/"));
                dbusProxyTS->callMethod(TS_GET_TIME)
                    .onInterface(sdbus::InterfaceName(TS_SERVICE_NAME))
                    .storeResultsTo(timestampNow);
            } catch (const sdbus::Error& anotherErr) {
                std::cout << "Can not get system time due to sdbus error with name: " << anotherErr.getName()
                          << " and message: " << anotherErr.getMessage() << std::endl;
                return 0;
            }
        } else {
            std::cout << "Can not get system time due to sdbus error with name: " << e.getName()
                      << " and message: " << e.getMessage() << std::endl;
            return 0;
        }
    }

    std::time_t timeNow{static_cast<time_t>(timestampNow)};
    std::tm* localTimeNow = std::localtime(&timeNow);

    char buf[80];
    strftime(buf, sizeof(buf), "%d-%m-%Y %X", localTimeNow);
    std::string strTimeNow{buf};

    std::cout << "Success! Current system time: " << strTimeNow << std::endl;

    return 0;
}