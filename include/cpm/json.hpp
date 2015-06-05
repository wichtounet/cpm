//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_JSON_HPP
#define CPM_JSON_HPP

#include <unistd.h>
#include <sys/stat.h>

namespace cpm {

template<typename T>
void write_value(std::ofstream& stream, std::size_t& indent, const std::string& tag, const T& value, bool comma = true){
    if(comma){
        stream << std::string(indent, ' ') << "\"" << tag << "\": \"" << value << "\",\n";
    } else {
        stream << std::string(indent, ' ') << "\"" << tag << "\": \"" << value << "\"\n";
    }
}

template<>
void write_value(std::ofstream& stream, std::size_t& indent, const std::string& tag, const std::size_t& value, bool comma){
    if(comma){
        stream << std::string(indent, ' ') << "\"" << tag << "\": " << value << ",\n";
    } else {
        stream << std::string(indent, ' ') << "\"" << tag << "\": " << value << "\n";
    }
}

template<>
void write_value(std::ofstream& stream, std::size_t& indent, const std::string& tag, const int64_t& value, bool comma){
    if(comma){
        stream << std::string(indent, ' ') << "\"" << tag << "\": " << value << ",\n";
    } else {
        stream << std::string(indent, ' ') << "\"" << tag << "\": " << value << "\n";
    }
}

template<>
void write_value(std::ofstream& stream, std::size_t& indent, const std::string& tag, const long long& value, bool comma){
    if(comma){
        stream << std::string(indent, ' ') << "\"" << tag << "\": " << value << ",\n";
    } else {
        stream << std::string(indent, ' ') << "\"" << tag << "\": " << value << "\n";
    }
}

template<>
void write_value(std::ofstream& stream, std::size_t& indent, const std::string& tag, const double& value, bool comma){
    stream << std::fixed;
    if(comma){
        stream << std::string(indent, ' ') << "\"" << tag << "\": " << value << ",\n";
    } else {
        stream << std::string(indent, ' ') << "\"" << tag << "\": " << value << "\n";
    }
    stream << std::scientific;
}

inline void start_array(std::ofstream& stream, std::size_t& indent, const std::string& tag){
    stream << std::string(indent, ' ') << "\"" << tag << "\": " << "[" << "\n";
    indent += 2;
}

inline void close_array(std::ofstream& stream, std::size_t& indent, bool comma){
    indent -= 2;

    if(comma){
        stream << std::string(indent, ' ') << "]," << "\n";
    } else {
        stream << std::string(indent, ' ') << "]" << "\n";
    }
}

inline void start_sub(std::ofstream& stream, std::size_t& indent){
    stream << std::string(indent, ' ') << "{" << "\n";
    indent += 2;
}

inline void close_sub(std::ofstream& stream, std::size_t& indent, bool comma){
    indent -= 2;

    if(comma){
        stream << std::string(indent, ' ') << "}," << "\n";
    } else {
        stream << std::string(indent, ' ') << "}" << "\n";
    }
}

} //end of namespace cpm

#endif //CPM_JSON_HPP
