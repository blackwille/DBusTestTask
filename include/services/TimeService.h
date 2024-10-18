#ifndef TIME_SERVICE_H
#define TIME_SERVICE_H

#include <sdbus-c++/IConnection.h>
#include <sdbus-c++/IObject.h>
#include <sdbus-c++/Message.h>
#include <sdbus-c++/Types.h>

#include <memory>

namespace services {

inline constexpr std::string_view TS_DEFAULT_SERVICE_NAME = "com.system.time";
inline constexpr std::string_view TS_DEFAULT_OBJECT_NAME = "/";

class TimeService {
public:
    static TimeService& Locate() {
        static TimeService instance;
        return instance;
    }

    TimeService(const TimeService& rhs) = delete;
    void operator=(const TimeService& rhs) = delete;

private:
    TimeService();
    // We need sender data. I don't know how to get it in convenience mode. Used basic call instead.
    void GetSystemTime(sdbus::MethodCall call) const;

private:
    std::unique_ptr<sdbus::IConnection> m_connection;
    std::unique_ptr<sdbus::IObject> m_object;
    sdbus::ServiceName m_serviceName{std::string{TS_DEFAULT_SERVICE_NAME}};
};

};  // namespace services

#endif  // TIME_SERVICE_H