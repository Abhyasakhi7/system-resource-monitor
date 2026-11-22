#include "process/ProcessInfo.h"

#include <sstream>

namespace srm {

ProcessStat parseProcessStat(const std::string& text) {
    ProcessStat result;

    const std::string::size_type open = text.find('(');
    const std::string::size_type close = text.rfind(')');
    if (open == std::string::npos || close == std::string::npos || close < open) {
        return result;
    }

    std::istringstream pidStream(text.substr(0, open));
    if (!(pidStream >> result.pid)) {
        return result;
    }

    result.name = text.substr(open + 1, close - open - 1);

    std::istringstream fields(text.substr(close + 2));
    std::string state;
    int parentPid = 0;
    int pgrp = 0;
    int session = 0;
    int ttyNr = 0;
    int tpgid = 0;
    unsigned long long flags = 0;
    unsigned long long minflt = 0;
    unsigned long long cminflt = 0;
    unsigned long long majflt = 0;
    unsigned long long cmajflt = 0;
    unsigned long long utime = 0;
    unsigned long long stime = 0;
    unsigned long long cutime = 0;
    unsigned long long cstime = 0;
    long priority = 0;
    long nice = 0;
    long numThreads = 0;
    unsigned long long itrealvalue = 0;
    unsigned long long starttime = 0;

    if (!(fields >> state >> parentPid >> pgrp >> session >> ttyNr >> tpgid >> flags >>
          minflt >> cminflt >> majflt >> cmajflt >> utime >> stime >> cutime >> cstime >>
          priority >> nice >> numThreads >> itrealvalue >> starttime)) {
        return result;
    }

    result.state = state;
    result.parentPid = parentPid;
    result.utime = utime;
    result.stime = stime;
    result.startTicks = starttime;
    result.threadCount = static_cast<int>(numThreads);
    result.valid = true;
    return result;
}

} // namespace srm
