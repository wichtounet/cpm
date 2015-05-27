//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_RAPIDJSON_HPP
#define CPM_RAPIDJSON_HPP

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

inline auto begin(const rapidjson::Value& value){
    return value.Begin();
}

inline auto end(const rapidjson::Value& value){
    return value.End();
}

} //end of namespace rapidjson

#endif //CPM_RAPIDJSON_HPP
