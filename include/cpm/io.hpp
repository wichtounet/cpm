//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_IO_HPP
#define CPM_IO_HPP

#include <unistd.h>
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

bool folder_exists(const std::string& folder){
    struct stat buffer;
    if (stat(folder.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode)){
        return true;
    } else {
        return false;
    }
}

} //end of namespace cpm

#endif //CPM_IO_HPP
