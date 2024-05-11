#include <ctime>
#include <string>

namespace Scorpion {

class TimeHelper {
public:
    static std::string GetUTCDateTime(time_t ts);

    static std::string GetUTCDate(time_t ts);

    static std::string GetUTCTime(time_t ts);

    static std::string GetLocalDateTime(time_t ts);

    static std::string GetLocalDate(time_t ts);

    static std::string GetLocalTime(time_t ts);

    static time_t ParseUTCDateTime(const std::string &dt, const std::string &fmt = "%Y-%m-%d %H:%M:%S");

    static time_t ParseLocalDateTime(const std::string &dt, const std::string &fmt = "%Y-%m-%d %H:%M:%S");
};

} // namespace Scorpion