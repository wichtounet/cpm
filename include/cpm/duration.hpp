//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_DURATION_HPP
#define CPM_DURATION_HPP

#include <chrono>
#include <ctime>

namespace cpm {

struct measure_result {
    double mean;
    double stddev;
    double min;
    double max;
};

using timer_clock = std::chrono::steady_clock;
using wall_clock = std::chrono::system_clock;
using wall_time_point = wall_clock::time_point;
using seconds = std::chrono::seconds;
using millseconds = std::chrono::milliseconds;
using microseconds = std::chrono::microseconds;

inline std::string us_duration_str(double duration_us){
    double duration = duration_us;

    if(duration > 1000 * 1000){
        return std::to_string(duration / 1000.0 / 1000.0) + "s";
    } else if(duration > 1000){
        return std::to_string(duration / 1000.0) + "ms";
    } else {
        return std::to_string(duration_us) + "us";
    }
}

} //end of namespace cpm

#endif //CPM_DURATION_HPP
