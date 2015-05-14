//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>

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

namespace {

std::string dark_unica_theme = 
#include "theme.inc"
;

rapidjson::Document read_document(const std::string& source_file){
    FILE* pFile = fopen(source_file.c_str(), "rb");
    char buffer[65536];

    rapidjson::FileReadStream is(pFile, buffer, sizeof(buffer));
    rapidjson::Document doc;
    doc.ParseStream<0>(is);

    return doc;
}

void header(std::ostream& stream){
    stream << "<html>" << std::endl;
    stream << "<head>" << std::endl;

    stream << "<script src=\"http://code.jquery.com/jquery-1.11.3.min.js\"></script>" << std::endl;
    stream << "<script src=\"http://code.jquery.com/jquery-migrate-1.2.1.min.js\"></script>" << std::endl;
    stream << "<script src=\"http://code.highcharts.com/highcharts.js\"></script>" << std::endl;
    stream << "<script src=\"http://code.highcharts.com/modules/exporting.js\"></script>" << std::endl;

    stream << "</head>" << std::endl;

    stream << "<body>" << std::endl;
}

void footer(std::ostream& stream){
    stream << "</body>" << std::endl;
    stream << "</html>" << std::endl;
}

void information(std::ostream& stream, rapidjson::Document& doc){
    stream << "<h1>" << doc["name"].GetString() << "</h1>" << std::endl;

    stream << "<ul>" << std::endl;
    stream << "<li>Compiler: " << doc["compiler"].GetString() << "</li>" << std::endl;
    stream << "<li>Operating System: " << doc["os"].GetString() << "</li>" << std::endl;
    stream << "<li>Time: " << doc["time"].GetString() << "</li>" << std::endl;
    stream << "</ul>" << std::endl;
}

} //end of anonymous namespace

int main(){
    std::string source_file = "results/1.cpm";
    std::string target_file = "reports/index.html";

    auto doc = read_document(source_file);

    std::ofstream stream(target_file);

    header(stream);

    information(stream, doc);

    //TODO The theme should probably be made configurable
    stream << "<script>" << std::endl;
    stream << dark_unica_theme << std::endl;
    stream << "</script>" << std::endl;

    std::size_t id = 1;
    for(auto& result : doc["results"]){
        stream << "<h2>" << result["title"].GetString() << "</h2>" << std::endl;

        stream << "<div id=\"chart_" << id << "\" style=\"min-width: 310px; max-width:700px; height: 400px; margin: 0 auto\"></div>" << std::endl;

        stream << "<script>" << std::endl;

        stream << "$(function () {" << std::endl;
        stream << "$('#chart_" << id << "').highcharts({" << std::endl;
        stream << "title: { text: '" << result["title"].GetString() << "', x: -20 }," << std::endl;

        stream << "xAxis: { categories: [" << std::endl;

        std::string comma = "";
        for(auto& r : result["results"]){
            stream << comma << "'" << r["size"].GetInt() << "'";
            comma = ",";
        }

        stream << "]}," << std::endl;

        stream << "yAxis: {" << std::endl;
        stream << "title: { text: 'Time [us]' }," << std::endl;
        stream << "plotLines: [{ value: 0, width: 1, color: '#808080'}]" << std::endl;
        stream << "}," << std::endl;

        stream << "tooltip: { valueSuffix: 'us' }," << std::endl;
        stream << "legend: { enabled: false }," << std::endl;

        stream << "series: [" << std::endl;
        stream << "{" << std::endl;

        stream << "name: ''," << std::endl;
        stream << "data: [";

        comma = "";
        for(auto& r : result["results"]){
            stream << comma << r["duration"].GetInt();
            comma = ",";
        }

        stream << "]" << std::endl;
        stream << "}" << std::endl;
        stream << "]" << std::endl;

        stream << "});" << std::endl;
        stream << "});" << std::endl;

        stream << "</script>" << std::endl;

        ++id;
    }
    
    for(auto& section : doc["sections"]){
        stream << "<h2>" << section["name"].GetString() << "</h2>" << std::endl;

        stream << "<div id=\"chart_" << id << "\" style=\"min-width: 310px; max-width:700px; height: 400px; margin: 0 auto\"></div>" << std::endl;

        stream << "<script>" << std::endl;

        stream << "$(function () {" << std::endl;
        stream << "$('#chart_" << id << "').highcharts({" << std::endl;
        stream << "title: { text: '" << section["name"].GetString() << "', x: -20 }," << std::endl;

        stream << "xAxis: { categories: [" << std::endl;

        //TODO Use all the sizes

        std::string comma = "";
        for(auto& r : section["results"][static_cast<rapidjson::SizeType>(0)]["results"]){
            stream << comma << "'" << r["size"].GetInt() << "'";
            comma = ",";
        }

        stream << "]}," << std::endl;

        stream << "yAxis: {" << std::endl;
        stream << "title: { text: 'Time [us]' }," << std::endl;
        stream << "plotLines: [{ value: 0, width: 1, color: '#808080'}]" << std::endl;
        stream << "}," << std::endl;

        stream << "tooltip: { valueSuffix: 'us' }," << std::endl;
        stream << "legend: { align: 'left', verticalAlign: 'top', floating: false, borderWidth: 0, y: 20 }," << std::endl;

        stream << "series: [" << std::endl;

        comma = "";
        for(auto& r : section["results"]){
            stream << comma << "{" << std::endl;

            stream << "name: '" << r["name"].GetString() << "'," << std::endl;
            stream << "data: [";

            std::string comma_inner = "";
            for(auto& rr : r["results"]){
                stream << comma_inner << rr["duration"].GetInt();
                comma_inner = ",";
            }

            stream << "]" << std::endl;
            stream << "}" << std::endl;
            comma = ",";
        }

        stream << "]" << std::endl;

        stream << "});" << std::endl;
        stream << "});" << std::endl;

        stream << "</script>" << std::endl;

        ++id;
    }

    footer(stream);

    return 0;
}
