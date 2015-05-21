//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <stdio.h>
#include <dirent.h>

#include "cxxopts.hpp"

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

#include "cpm/io.hpp"

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
#include "dark_unica.inc"
;

rapidjson::Document read_document(const std::string& folder, const std::string& file){
    FILE* pFile = fopen((folder + "/" + file).c_str(), "rb");
    char buffer[65536];

    rapidjson::FileReadStream is(pFile, buffer, sizeof(buffer));
    rapidjson::Document doc;
    doc.ParseStream<0>(is);

    return doc;
}

void header(std::ostream& stream){
    stream << "<html>\n";
    stream << "<head>\n";

    stream << "<script src=\"http://code.jquery.com/jquery-1.11.3.min.js\"></script>\n";
    stream << "<script src=\"http://code.jquery.com/jquery-migrate-1.2.1.min.js\"></script>\n";
    stream << "<script src=\"http://code.highcharts.com/highcharts.js\"></script>\n";
    stream << "<script src=\"http://code.highcharts.com/modules/exporting.js\"></script>\n";

    stream << "</head>\n";

    stream << "<body>\n";
}

void footer(std::ostream& stream){
    stream << "</body>\n";
    stream << "</html>\n";
}

void information(std::ostream& stream, rapidjson::Document& doc){
    stream << "<h1>" << doc["name"].GetString() << "</h1>\n";

    stream << "<ul>\n";
    stream << "<li>Compiler: " << doc["compiler"].GetString() << "</li>\n";
    stream << "<li>Operating System: " << doc["os"].GetString() << "</li>\n";
    stream << "<li>Time: " << doc["time"].GetString() << "</li>\n";
    stream << "</ul>\n";
}

std::vector<rapidjson::Document> read(const std::string& source_folder){
    std::vector<rapidjson::Document> documents;

    struct dirent* entry;
    DIR* dp = opendir(source_folder.c_str());

    if(!dp){
        return documents;
    }

    while((entry = readdir(dp))){
        if(std::string(entry->d_name) == "." || std::string(entry->d_name) == ".."){
            continue;
        }

        documents.push_back(read_document(source_folder, entry->d_name));
    }

    std::sort(documents.begin(), documents.end(),
        [](rapidjson::Document& lhs, rapidjson::Document& rhs){ return lhs["timestamp"].GetInt() < rhs["timestamp"].GetInt(); });

    return documents;
}

void start_graph(std::ostream& stream, cxxopts::Options& /*options*/, std::size_t& id, const std::string& title){
    stream << "<div id=\"chart_" << id << "\" style=\"float:left; width:600px; height: 400px; margin: 5 auto; padding-right: 10px; \"></div>\n";

    stream << "<script>\n";

    stream << "$(function () {\n";
    stream << "$('#chart_" << id << "').highcharts({\n";
    stream << "title: { text: '" << title << "', x: -20 },\n";

    ++id;
}

void end_graph(std::ostream& stream){
    stream << "});\n";
    stream << "});\n";

    stream << "</script>\n";
}

void y_axis_configuration(std::ostream& stream){
    stream << "yAxis: {\n";
    stream << "title: { text: 'Time [us]' },\n";
    stream << "plotLines: [{ value: 0, width: 1, color: '#808080'}]\n";
    stream << "},\n";

    stream << "tooltip: { valueSuffix: 'us' },\n";
}

void generate_run_graph(std::ostream& stream, cxxopts::Options& options, std::size_t& id, rapidjson::Value& result){
    start_graph(stream, options, id, std::string("Last run:") + result["title"].GetString());

    stream << "xAxis: { categories: [\n";

    std::string comma = "";
    for(auto& r : result["results"]){
        stream << comma << "'" << r["size"].GetString() << "'";
        comma = ",";
    }

    stream << "]},\n";

    y_axis_configuration(stream);

    stream << "legend: { enabled: false },\n";

    stream << "series: [\n";
    stream << "{\n";

    stream << "name: '',\n";
    stream << "data: [";

    comma = "";
    for(auto& r : result["results"]){
        stream << comma << r["duration"].GetInt();
        comma = ",";
    }

    stream << "]\n";
    stream << "}\n";
    stream << "]\n";

    end_graph(stream);
}

void generate_time_graph(std::ostream& stream, cxxopts::Options& options, std::size_t& id, rapidjson::Value& result, std::vector<rapidjson::Document>& documents){
    start_graph(stream, options, id, std::string("Time:") + result["title"].GetString());

    stream << "xAxis: { type: 'datetime', title: { text: 'Date' } },\n";

    y_axis_configuration(stream);

    if(!options.count("time-sizes")){
        stream << "legend: { enabled: false },\n";
    }

    stream << "series: [\n";

    if(options.count("time-sizes")){
        std::string comma = "";
        for(auto& r : result["results"]){
            stream << comma << "{\n";

            stream << "name: '" << r["size"].GetString() << "',\n";
            stream << "data: [";

            std::string inner_comma = "";

            for(auto& document : documents){
                for(auto& o_result : document["results"]){
                    if(std::string(o_result["title"].GetString()) == std::string(result["title"].GetString())){
                        for(auto& o_rr : o_result["results"]){
                            if(o_rr["size"].GetString() == r["size"].GetString()){
                                stream << inner_comma << "[" << document["timestamp"].GetInt() * 1000 << ",";
                                stream << o_rr["duration"].GetInt() << "]";
                                inner_comma = ",";
                            }
                        }
                    }
                }
            }

            stream << "]\n";
            stream << "}\n";
            comma =",";
        }
    } else {
        stream << "{\n";

        stream << "name: '',\n";
        stream << "data: [";

        std::string comma = "";

        for(auto& document : documents){
            for(auto& o_result : document["results"]){
                if(std::string(o_result["title"].GetString()) == std::string(result["title"].GetString())){
                    stream << comma << "[" << document["timestamp"].GetInt() * 1000 << ",";
                    auto& o_r_results = o_result["results"];
                    stream << o_r_results[o_r_results.Size() - 1]["duration"].GetInt() << "]";
                    comma = ",";
                }
            }
        }

        stream << "]\n";
        stream << "}\n";
    }

    stream << "]\n";

    end_graph(stream);
}

void generate_section_run_graph(std::ostream& stream, cxxopts::Options& options, std::size_t& id, rapidjson::Value& section){
    start_graph(stream, options, id, std::string("Last run:") + section["name"].GetString());

    stream << "xAxis: { categories: [\n";

    //TODO Use all the sizes

    std::string comma = "";
    for(auto& r : section["results"][static_cast<rapidjson::SizeType>(0)]["results"]){
        stream << comma << "'" << r["size"].GetString() << "'";
        comma = ",";
    }

    stream << "]},\n";

    y_axis_configuration(stream);

    stream << "legend: { align: 'left', verticalAlign: 'top', floating: false, borderWidth: 0, y: 20 },\n";

    stream << "series: [\n";

    comma = "";
    for(auto& r : section["results"]){
        stream << comma << "{\n";

        stream << "name: '" << r["name"].GetString() << "',\n";
        stream << "data: [";

        std::string comma_inner = "";
        for(auto& rr : r["results"]){
            stream << comma_inner << rr["duration"].GetInt();
            comma_inner = ",";
        }

        stream << "]\n";
        stream << "}\n";
        comma = ",";
    }

    stream << "]\n";

    end_graph(stream);
}
void generate_section_time_graph(std::ostream& stream, cxxopts::Options& options, std::size_t& id, rapidjson::Value& section, std::vector<rapidjson::Document>& documents){
    start_graph(stream, options, id, std::string("Time:") + section["name"].GetString());

    stream << "xAxis: { type: 'datetime', title: { text: 'Date' } },\n";

    y_axis_configuration(stream);

    stream << "legend: { align: 'left', verticalAlign: 'top', floating: false, borderWidth: 0, y: 20 },\n";

    stream << "series: [\n";

    std::string comma = "";
    for(auto& r : section["results"]){
        stream << comma << "{\n";

        stream << "name: '" << r["name"].GetString() << "',\n";
        stream << "data: [";

        std::string comma_inner = "";

        for(auto& r_doc : documents){
            for(auto& r_section : r_doc["sections"]){
                if(std::string(r_section["name"].GetString()) == std::string(section["name"].GetString())){
                    for(auto& r_r : r_section["results"]){
                        if(std::string(r_r["name"].GetString()) == std::string(r["name"].GetString())){
                            stream << comma_inner << "[" << r_doc["timestamp"].GetInt() * 1000 << ",";
                            auto& r_r_results = r_r["results"];
                            stream << r_r_results[r_r_results.Size() - 1]["duration"].GetInt() << "]";
                            comma_inner = ",";
                        }
                    }
                }
            }
        }

        stream << "]\n";
        stream << "}\n";
        comma = ",";
    }

    stream << "]\n";

    end_graph(stream);
}

} //end of anonymous namespace

int main(int argc, char* argv[]){
    cxxopts::Options options(argv[0], "  results_folder");

    try {
        options.add_options()
            ("s,time-sizes", "Display multiple sizes in the time graphs")
            ("t,theme", "Theme name [raw]", cxxopts::value<std::string>()->default_value("raw"))
            ("c,hctheme", "Highcharts Theme name [std,dark_unica]", cxxopts::value<std::string>()->default_value("dark_unica"), "theme_name")
            ("o,output", "Output folder", cxxopts::value<std::string>()->default_value("reports"), "output_folder")
            ("input", "Input results", cxxopts::value<std::string>())
            ("d,disable-time", "Disable time graphs")
            ("h,help", "Print help")
            ;

        options.parse_positional("input");
        options.parse(argc, argv);

        if (options.count("help")){
            std::cout << options.help({""}) << std::endl;
            return 0;
        }

        if (!options.count("input")){
            std::cout << "cpm: No input provided, exiting" << std::endl;
            return 0;
        }
    } catch (const cxxopts::OptionException& e){
        std::cout << "cpm: error parsing options: " << e.what() << std::endl;
        return -1;
    }

    //Get the entered folders
    auto source_folder = options["input"].as<std::string>();
    auto target_folder = options["output"].as<std::string>();

    if(!cpm::folder_exists(source_folder)){
        std::cout << "cpm: The input folder does not exists, exiting" << std::endl;
        return -1;
    }

    if(!cpm::folder_exists(target_folder)){
        std::cout << "cpm: The target folder does not exists, exiting" << std::endl;
        return -1;
    }

    bool time_graphs = !options.count("disable-time");

    std::string target_file = target_folder + "/index.html";

    auto documents = read(source_folder);

    if(documents.empty()){
        std::cout << "Unable to read any files" << std::endl;
        return -1;
    }

    auto& doc = documents.front();

    std::ofstream stream(target_file);

    header(stream);

    information(stream, doc);

    //Configure the highcharts theme
    if(options["hctheme"].as<std::string>() == "dark_unica"){
        stream << "<script>\n" << "\n";
        stream << dark_unica_theme << "\n";
        stream << "</script>\n";
    }

    std::size_t id = 1;
    for(auto& result : doc["results"]){
        stream << "<h2 style=\"clear:both\">" << result["title"].GetString() << "</h2>\n";

        generate_run_graph(stream, options, id, result);

        if(time_graphs){
            generate_time_graph(stream, options, id, result, documents);
        }
    }

    for(auto& section : doc["sections"]){
        stream << "<h2 style=\"clear:both\">" << section["name"].GetString() << "</h2>\n";

        generate_section_run_graph(stream, options, id, section);

        if(time_graphs){
            generate_section_time_graph(stream, options, id, section, documents);
        }
    }

    footer(stream);

    return 0;
}
