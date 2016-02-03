//=======================================================================
// Copyright (c) 2015-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_IO_HPP
#define CPM_IO_HPP

#include <unistd.h>
#include <algorithm>
#include <iomanip>

#include <sys/stat.h>

namespace cpm {

inline std::size_t max_path_length(){
    auto path_max = pathconf(".", _PC_PATH_MAX);
    if (path_max == -1){
        path_max = 1024;
    } else if (path_max > 10240){
        path_max = 10240;
    }
    return path_max;
}

inline std::string get_cwd(){
    auto path_max = max_path_length();

    char* buf = static_cast<char*>(malloc(sizeof(char) * path_max));
    if(!buf){
        return "";
    }

    char* ptr = getcwd(buf, path_max);
    if (!ptr) {
        return "";
    }

    std::string cwd = buf;

    free(buf);

    return cwd;
}

inline std::string make_absolute(const std::string& folder){
    auto path_max = max_path_length();

    char* buf = static_cast<char*>(malloc(sizeof(char) * path_max));
    if(!buf){
        return folder;
    }

    char* res = realpath(folder.c_str(), buf);

    if(!res){
        return folder;
    }

    std::string abs = buf;

    free(buf);

    return abs;
}

inline std::string get_free_file(const std::string& base_folder){
    std::size_t result_name = 0;
    std::string result_folder;

    struct stat buffer;
    do {
        ++result_name;
        result_folder = base_folder + std::to_string(result_name) + ".cpm";
    } while(stat(result_folder.c_str(), &buffer) == 0);

    return std::to_string(result_name);
}

inline bool folder_exists(const std::string& folder){
    struct stat buffer;
    if (stat(folder.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode)){
        return true;
    } else {
        return false;
    }
}

inline std::string url_encode(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (auto i = value.begin(), n = value.end(); i != n; ++i) {
        auto c = *i;

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << '%' << std::setw(2) << int((unsigned char) c);
    }

    return escaped.str();
}

inline std::string filify(std::string name){
    std::string n{std::move(name)};

    //Replace all spaces
    std::replace(n.begin(), n.end(), ' ', '_');

    //Replace all forward slashes
    std::replace(n.begin(), n.end(), '/', '|');

    //Append extension
    n += ".html";

    return n;
}

inline std::string filify(std::string compiler, std::string configuration){
    std::string n1{std::move(compiler)};
    std::string n2{std::move(configuration)};

    auto n = n1 + "-" + n2;

    return filify(n);
}

inline std::string filify(std::string compiler, std::string configuration, std::string bench){
    std::string n1{std::move(compiler)};
    std::string n2{std::move(configuration)};
    std::string n3{std::move(bench)};

    auto n = n1 + "-" + n2 + "-" + n3;

    return filify(n);
}

} //end of namespace cpm

#endif //CPM_IO_HPP
