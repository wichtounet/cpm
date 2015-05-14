//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

//Allow range-based for loop on rapidjson objects

namespace rapidjson {

inline auto begin(rapidjson::Value& value){
    return value.Begin();
}

inline auto end(rapidjson::Value& value){
    return value.End();
}

} //end of namespace rapidjson

int main(){
    std::string file = "results/41.cpm";

    FILE* pFile = fopen(file.c_str(), "rb");
    char buffer[65536];

    rapidjson::FileReadStream is(pFile, buffer, sizeof(buffer));
    rapidjson::Document doc;
    doc.ParseStream<0>(is);

    std::cout << doc["name"].GetString() << std::endl;

    auto& results = doc["results"];

    for(auto& result : results){

        std::cout << result["title"].GetString() << std::endl;
    }

    return 0;
}
