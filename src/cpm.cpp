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
#include <set>

#include <stdio.h>
#include <dirent.h>

#include "cxxopts.hpp"

#include "cpm/io.hpp"
#include "cpm/rapidjson.hpp"
#include "cpm/data.hpp"
#include "cpm/raw_theme.hpp"
#include "cpm/bootstrap_theme.hpp"

//TODO Use all the sizes for section graphs to handle timeouts in some of the graphs
//TODO Align correctly sections when compiler graphs are generated

namespace {

std::string dark_unica_theme =
#include "dark_unica.inc.js"
;

cpm::document_t read_document(const std::string& folder, const std::string& file){
    FILE* pFile = fopen((folder + "/" + file).c_str(), "rb");
    char buffer[65536];

    rapidjson::FileReadStream is(pFile, buffer, sizeof(buffer));
    cpm::document_t doc;
    doc.ParseStream<0>(is);

    return doc;
}

std::vector<cpm::document_t> read(const std::string& source_folder){
    std::vector<cpm::document_t> documents;

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
        [](cpm::document_t& lhs, cpm::document_t& rhs){ return lhs["timestamp"].GetInt() < rhs["timestamp"].GetInt(); });

    return documents;
}

//Select relevant documents
std::vector<cpm::document_cref> select_documents(std::vector<cpm::document_t>& documents, cpm::document_t& base){
    std::vector<cpm::document_cref> relevant;

    for(auto& doc : documents){
        //Two documents are relevant if the configuration
        //is the same
        if(std::string(doc["compiler"].GetString()) == std::string(base["compiler"].GetString())){
            relevant.push_back(std::cref(doc));
        }
    }

    return relevant;
}

template<typename Theme>
void header(Theme& theme, std::ostream& stream){
    stream << "<!DOCTYPE html>\n";
    stream << "<html lang=\"en\">\n";
    stream << "<head>\n";

    stream << "<title>" << theme.data.documents.back()["name"].GetString() << "</title>\n";
    stream << "<meta charset=\"UTF-8\">\n";

    //We need JQuery
    stream << "<script src=\"http://code.jquery.com/jquery-1.11.3.min.js\"></script>\n";
    stream << "<script src=\"http://code.jquery.com/jquery-migrate-1.2.1.min.js\"></script>\n";

    //We need Highcharts
    stream << "<script src=\"http://code.highcharts.com/highcharts.js\"></script>\n";
    stream << "<script src=\"http://code.highcharts.com/modules/exporting.js\"></script>\n";

    theme.include(stream);

    stream << "</head>\n";

    stream << "<body>\n";

    theme.header(stream);
}

template<typename Theme>
void footer(Theme& theme, std::ostream& stream){
    theme.footer(stream);

    stream << "</body>\n";
    stream << "</html>\n";
}

template<typename Theme>
void information(Theme& theme, std::ostream& stream, const cpm::document_t& doc){
    theme.before_information(stream);

    stream << "<h1>" << doc["name"].GetString() << "</h1>\n";

    stream << "<ul>\n";
    stream << "<li>Tag: " << doc["tag"].GetString() << "</li>\n";
    stream << "<li>Compiler: " << doc["compiler"].GetString() << "</li>\n";
    stream << "<li>Operating System: " << doc["os"].GetString() << "</li>\n";
    stream << "<li>Time: " << doc["time"].GetString() << "</li>\n";
    stream << "</ul>\n";

    theme.after_information(stream);
}

template<typename Theme>
void compiler_buttons(Theme& theme, std::ostream& stream, const cpm::reports_data& data, const cpm::document_t& base){
    if(data.compilers.size() > 1){
        std::string current_compiler{base["compiler"].GetString()};

        theme.compiler_buttons(stream, current_compiler);
    }
}

void start_graph(std::ostream& stream, const std::string& id, const std::string& title){
    stream << "<script>\n";

    stream << "$(function () {\n";
    stream << "$('#" << id << "').highcharts({\n";
    stream << "title: { text: '" << title << "', x: -20 },\n";
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

template<typename Theme>
void generate_run_graph(Theme& theme, std::ostream& stream, std::size_t& id, const rapidjson::Value& result){
    theme.before_graph(stream, id);
    start_graph(stream, std::string("chart_") + std::to_string(id), std::string("Last run:") + result["title"].GetString());

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
    theme.after_graph(stream);
    ++id;
}

template<typename Theme>
void generate_compiler_graph(Theme& theme, std::ostream& stream, std::size_t& id, const rapidjson::Value& base_result, const cpm::document_t& base){
    theme.before_graph(stream, id);
    start_graph(stream, std::string("chart_") + std::to_string(id), std::string("Compiler:") + base_result["title"].GetString());

    stream << "xAxis: { categories: [\n";

    std::string comma = "";
    for(auto& r : base_result["results"]){
        stream << comma << "'" << r["size"].GetString() << "'";
        comma = ",";
    }

    stream << "]},\n";

    y_axis_configuration(stream);

    stream << "legend: { align: 'left', verticalAlign: 'top', floating: false, borderWidth: 0, y: 20 },\n";

    stream << "series: [\n";

    comma = "";
    for(auto& document : theme.data.documents){
        //Filter different tag
        if(std::string(document["tag"].GetString()) != std::string(base["tag"].GetString())){
            continue;
        }

        for(auto& result : document["results"]){
            if(std::string(result["title"].GetString()) == std::string(base_result["title"].GetString())){
                stream << comma << "{\n";
                stream << "name: '" << document["compiler"].GetString() << "',\n";
                stream << "data: [";

                std::string inner_comma = "";
                for(auto& r : result["results"]){
                    stream << inner_comma << r["duration"].GetInt();
                    inner_comma = ",";
                }

                stream << "]\n";
                stream << "}\n";

                comma = ",";
            }
        }
    }

    stream << "]\n";

    end_graph(stream);
    theme.after_graph(stream);
    ++id;
}

template<typename Theme>
void generate_time_graph(Theme& theme, std::ostream& stream, std::size_t& id, const rapidjson::Value& result, const std::vector<cpm::document_cref>& documents){
    theme.before_graph(stream, id);
    start_graph(stream, std::string("chart_") + std::to_string(id), std::string("Time:") + result["title"].GetString());

    stream << "xAxis: { type: 'datetime', title: { text: 'Date' } },\n";

    y_axis_configuration(stream);

    if(!theme.options.count("time-sizes")){
        stream << "legend: { enabled: false },\n";
    }

    stream << "series: [\n";

    if(theme.options.count("time-sizes")){
        std::string comma = "";
        for(auto& r : result["results"]){
            stream << comma << "{\n";

            stream << "name: '" << r["size"].GetString() << "',\n";
            stream << "data: [";

            std::string inner_comma = "";

            for(auto& document_r : documents){
                auto& document = static_cast<const cpm::document_t&>(document_r);

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

        for(auto& document_r : documents){
            auto& document = static_cast<const cpm::document_t&>(document_r);

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
    theme.after_graph(stream);
    ++id;
}

template<typename Theme>
void generate_section_run_graph(Theme& theme, std::ostream& stream, std::size_t& id, const rapidjson::Value& section){
    theme.before_graph(stream, id);
    start_graph(stream, std::string("chart_") + std::to_string(id), std::string("Last run:") + section["name"].GetString());

    stream << "xAxis: { categories: [\n";

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
    theme.after_graph(stream);
    ++id;
}

template<typename Theme>
void generate_section_time_graph(Theme& theme, std::ostream& stream, std::size_t& id, const rapidjson::Value& section, const std::vector<cpm::document_cref>& documents){
    theme.before_graph(stream, id);
    start_graph(stream, std::string("chart_") + std::to_string(id), std::string("Time:") + section["name"].GetString());

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

        for(auto& r_doc_r : documents){
            auto& r_doc = static_cast<const cpm::document_t&>(r_doc_r);

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
    theme.after_graph(stream);
    ++id;
}

template<typename T>
std::vector<std::string> string_collect(const T& parent, const char* attr){
    std::vector<std::string> values;
    for(auto& r : parent){
        values.emplace_back(r[attr].GetString());
    }
    return values;
}

template<typename Theme>
void generate_section_compiler_graph(Theme& theme, std::ostream& stream, std::size_t& id, const rapidjson::Value& section, const cpm::document_t& base){
    std::size_t sub_id = 0;

    theme.before_sub_graphs(stream, id, string_collect(section["results"], "name"));

    for(auto& r : section["results"]){
        theme.before_sub_graph(stream, id, sub_id++);

        start_graph(stream,
            std::string("chart_") + std::to_string(id) + "-" + std::to_string(sub_id - 1),
            std::string("Compiler:") + section["name"].GetString() + "-" + r["name"].GetString());

        stream << "xAxis: { categories: [\n";

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
        for(auto& document : theme.data.documents){
            //Filter different tag
            if(std::string(document["tag"].GetString()) != std::string(base["tag"].GetString())){
                continue;
            }

            for(auto& o_section : document["sections"]){
                if(std::string(o_section["name"].GetString()) == std::string(section["name"].GetString())){

                    for(auto& o_r : o_section["results"]){
                        if(std::string(o_r["name"].GetString()) == std::string(r["name"].GetString())){
                            stream << comma << "{\n";
                            stream << "name: '" << document["compiler"].GetString() << "',\n";
                            stream << "data: [";

                            std::string inner_comma = "";
                            for(auto& o_r_r : o_r["results"]){
                                stream << inner_comma << o_r_r["duration"].GetInt();
                                inner_comma = ",";
                            }

                            stream << "]\n";
                            stream << "}\n";

                            comma = ",";
                        }
                    }
                }
            }
        }

        stream << "]\n";

        end_graph(stream);

        theme.after_sub_graph(stream);
    }

    theme.after_sub_graphs(stream);

    ++id;
}

template<typename Theme>
void generate_standard_page(Theme& theme, const std::string& target_folder, const std::string& file, const cpm::reports_data& data, const cpm::document_t& doc, const std::vector<cpm::document_cref>& documents, cxxopts::Options& options){
    bool time_graphs = !options.count("disable-time") && documents.size() > 1;
    bool compiler_graphs = !options.count("disable-compiler") && data.compilers.size() > 1;

    std::string target_file = target_folder + "/" + file;

    std::ofstream stream(target_file);

    //Header of the page
    header(theme, stream);

    //Configure the highcharts theme
    if(options["hctheme"].as<std::string>() == "dark_unica"){
        stream << "<script>\n" << "\n";
        stream << dark_unica_theme << "\n";
        stream << "</script>\n";
    }

    //Information block about the last run
    information(theme, stream, doc);

    //Compiler selection
    compiler_buttons(theme, stream, data, doc);

    std::size_t id = 1;
    for(const auto& result : doc["results"]){
        theme.before_result(stream, result["title"].GetString());

        generate_run_graph(theme, stream, id, result);

        if(time_graphs){
            generate_time_graph(theme, stream, id, result, documents);
        }

        if(compiler_graphs){
            generate_compiler_graph(theme, stream, id, result, doc);
        }

        theme.after_result(stream);
    }

    for(auto& section : doc["sections"]){
        theme.before_result(stream, section["name"].GetString(), compiler_graphs);

        generate_section_run_graph(theme, stream, id, section);

        if(time_graphs){
            generate_section_time_graph(theme, stream, id, section, documents);
        }

        if(compiler_graphs){
            generate_section_compiler_graph(theme, stream, id, section, doc);
        }

        theme.after_result(stream);
    }

    footer(theme, stream);
}

template<typename Theme>
void generate_pages(const std::string& target_folder, cpm::reports_data& data, cxxopts::Options& options){
    Theme theme(data, options);

    //Generate the index
    auto& base = data.documents.back();
    generate_standard_page(theme, target_folder, "index.html", data, base, select_documents(data.documents, base), options);

    std::set<std::string> compilers(data.compilers);

    //Generate the compiler pages
    std::for_each(data.documents.rbegin(), data.documents.rend(), [&](cpm::document_t& d){
        std::string compiler{d["compiler"].GetString()};
        if(compilers.count(compiler)){
            generate_standard_page(theme, target_folder, cpm::filify(compiler), data, d, select_documents(data.documents, d), options);
            compilers.erase(compiler);
        }
    });
}

} //end of anonymous namespace

int main(int argc, char* argv[]){
    cxxopts::Options options(argv[0], "  results_folder");

    try {
        options.add_options()
            ("s,time-sizes", "Display multiple sizes in the time graphs")
            ("t,theme", "Theme name [raw,bootstrap]", cxxopts::value<std::string>()->default_value("raw"))
            ("c,hctheme", "Highcharts Theme name [std,dark_unica]", cxxopts::value<std::string>()->default_value("dark_unica"), "theme_name")
            ("o,output", "Output folder", cxxopts::value<std::string>()->default_value("reports"), "output_folder")
            ("input", "Input results", cxxopts::value<std::string>())
            ("d,disable-time", "Disable time graphs")
            ("disable-compiler", "Disable compiler graphs")
            ("disable-summary", "Disable summary table")
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

    cpm::reports_data data;

    //Get all the documents
    data.documents = read(source_folder);

    if(data.documents.empty()){
        std::cout << "Unable to read any files" << std::endl;
        return -1;
    }

    //Collect the list of compilers
    for(auto& doc : data.documents){
        data.compilers.insert(doc["compiler"].GetString());
    }

    if(options["theme"].as<std::string>() == "raw"){
        generate_pages<cpm::raw_theme>(target_folder, data, options);
    } else {
        generate_pages<cpm::bootstrap_theme>(target_folder, data, options);
    }

    return 0;
}
