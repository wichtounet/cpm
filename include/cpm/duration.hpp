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
    double mean_lb;
    double mean_ub;
    double stddev;
    double min;
    double max;
    double throughput;

    constexpr void update(std::size_t size_eff){
        throughput = mean == 0.0 ? 0.0 : size_eff / (mean / 1000.0 / 1000.0);
    }
};

struct measure_full {
    std::size_t size_eff;
    std::string size;
    measure_result result;
};

using timer_clock = std::chrono::steady_clock;
using wall_clock = std::chrono::system_clock;
using wall_time_point = wall_clock::time_point;
using seconds = std::chrono::seconds;
using millseconds = std::chrono::milliseconds;
using microseconds = std::chrono::microseconds;

inline std::string to_string_precision(double duration, int precision = 6){
    std::ostringstream out;
    out << std::setprecision(precision) << duration;
    return out.str();
}

inline std::string us_duration_str(double duration_us, int precision = 6){
    double duration = duration_us;

    if(duration > 1000 * 1000){
        return to_string_precision(duration / 1000.0 / 1000.0, precision) + "s";
    } else if(duration > 1000){
        return to_string_precision(duration / 1000.0, precision) + "ms";
    } else {
        return to_string_precision(duration_us, precision) + "us";
    }
}

inline std::string throughput_str(double tp, int precision = 6){
    std::ostringstream out;

    out << std::setprecision(precision);

    if(tp > 1000.0 * 1000.0 * 1000.0 * 1000.0){
        out << tp / (1000.0 * 1000.0 * 1000.0 * 1000.0) << "T";
    } else if(tp > 1000 * 1000 * 1000){
        out << tp / (1000 * 1000 * 1000) << "G";
    } else if(tp > 1000 * 1000){
        out << tp / (1000 * 1000) << "M";
    } else if(tp > 1000){
        out << tp / (1000) << "K";
    } else {
        out << tp;
    }

    return out.str();
}

} //end of namespace cpm

#endif //CPM_DURATION_HPP
