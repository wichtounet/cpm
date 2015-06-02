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
#include "cpm/bootstrap_tabs_theme.hpp"

namespace {

using json_value = const rapidjson::Value&;

bool str_equal(const char* lhs, const char* rhs){
    return std::strcmp(lhs, rhs) == 0;
}

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

std::vector<cpm::document_t> read(const std::string& source_folder, cxxopts::Options& options){
    std::vector<cpm::document_t> documents;

    struct dirent* entry;
    DIR* dp = opendir(source_folder.c_str());

    if(!dp){
        return documents;
    }

    while((entry = readdir(dp))){
        if(str_equal(entry->d_name, ".") || str_equal(entry->d_name, "..")){
            continue;
        }

        if(entry->d_type == DT_REG){
            documents.push_back(read_document(source_folder, entry->d_name));
        }
    }

    if(options.count("sort-by-size")){
        std::sort(documents.begin(), documents.end(),
            [](cpm::document_t& lhs, cpm::document_t& rhs){ 
                if(std::string(lhs["tag"].GetString()) < std::string(rhs["tag"].GetString())){
                    return true;
                } else if(std::string(lhs["tag"].GetString()) > std::string(rhs["tag"].GetString())){
                    return false;
                } else {
                    //If same tag, sort by time
                    return lhs["timestamp"].GetInt() < rhs["timestamp"].GetInt();
                }
            }
        );
    } else {
        std::cout << documents.size() << std::endl;
        std::sort(documents.begin(), documents.end(),
            [](cpm::document_t& lhs, cpm::document_t& rhs){ return lhs["timestamp"].GetInt() < rhs["timestamp"].GetInt(); });
    }

    return documents;
}

//Select relevant documents
std::vector<cpm::document_cref> select_documents(const std::vector<cpm::document_t>& documents, const cpm::document_t& base){
    std::vector<cpm::document_cref> relevant;

    for(auto& doc : documents){
        //Two documents are relevant if the configuration is the same
        if(str_equal(doc["compiler"].GetString(), base["compiler"].GetString()) && str_equal(doc["configuration"].GetString(), base["configuration"].GetString())){
            relevant.push_back(std::cref(doc));
        }
    }

    return relevant;
}

template<typename Theme>
void header(Theme& theme){
    theme << "<!DOCTYPE html>\n";
    theme << "<html lang=\"en\">\n";
    theme << "<head>\n";

    //Metas
    theme << "<meta charset=\"UTF-8\">\n";
    theme << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
    theme << "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n";

    theme << "<title>" << theme.data.documents.back()["name"].GetString() << "</title>\n";

    //We need JQuery
    theme << "<script src=\"http://code.jquery.com/jquery-1.11.3.min.js\"></script>\n";
    theme << "<script src=\"http://code.jquery.com/jquery-migrate-1.2.1.min.js\"></script>\n";

    //We need Highcharts
    theme << "<script src=\"http://code.highcharts.com/highcharts.js\"></script>\n";
    theme << "<script src=\"http://code.highcharts.com/modules/exporting.js\"></script>\n";

    theme.include();

    theme << "</head>\n";

    theme << "<body>\n";

    theme.header();
}

template<typename Theme>
void footer(Theme& theme){
    theme.footer();

    theme << "</body>\n";
    theme << "</html>\n";
}

template<typename Theme>
void information(Theme& theme, const cpm::document_t& doc){
    theme.before_information();

    theme << "<h1>" << doc["name"].GetString() << "</h1>\n";

    theme << "<ul>\n";
    theme << "<li>Tag: " << doc["tag"].GetString() << "</li>\n";
    theme << "<li>Compiler: " << doc["compiler"].GetString() << "</li>\n";
    theme << "<li>Operating System: " << doc["os"].GetString() << "</li>\n";
    theme << "<li>Time: " << doc["time"].GetString() << "</li>\n";
    theme << "</ul>\n";

    theme.after_information();
}

template<typename Theme>
void compiler_buttons(Theme& theme){
    if(theme.data.compilers.size() > 1){
        theme.compiler_buttons();
    }
}

template<typename Theme>
void configuration_buttons(Theme& theme){
    if(theme.data.configurations.size() > 1){
        theme.configuration_buttons();
    }
}

template<typename Theme>
void start_graph(Theme& theme, const std::string& id, const std::string& title){
    theme << "<script>\n";

    theme << "$(function () {\n";
    theme << "$('#" << id << "').highcharts({\n";
    theme << "title: { text: '" << title << "', x: -20 },\n";
}

template<typename Theme>
void end_graph(Theme& theme){
    theme << "});\n";
    theme << "});\n";

    theme << "</script>\n";
}

template<typename Theme>
void y_axis_configuration(Theme& theme){
    theme << "yAxis: {\n";
    theme << "title: { text: 'Time [us]' },\n";
    theme << "plotLines: [{ value: 0, width: 1, color: '#808080'}]\n";
    theme << "},\n";

    theme << "tooltip: { valueSuffix: 'us' },\n";
}

template<typename Theme, typename Elements>
void json_array_string(Theme& theme, const Elements& elements){
    theme << "[";

    std::string comma = "";
    for(auto& element : elements){
        theme << comma << "'" << element << "'";
        comma = ",";
    }

    theme << "]";
}

template<typename Theme, typename Elements>
void json_array_int(Theme& theme, const Elements& elements){
    theme << "[";

    std::string comma = "";
    for(auto& element : elements){
        theme << comma << element;
        comma = ",";
    }

    theme << "]";
}

template<typename T>
std::vector<std::string> string_collect(const T& parent, const char* attr){
    std::vector<std::string> values;
    for(auto& r : parent){
        values.emplace_back(r[attr].GetString());
    }
    return values;
}

template<typename T>
std::vector<int> int_collect(const T& parent, const char* attr){
    std::vector<int> values;
    for(auto& r : parent){
        values.emplace_back(r[attr].GetInt());
    }
    return values;
}

template<typename Theme>
void generate_run_graph(Theme& theme, std::size_t& id, const rapidjson::Value& result){
    theme.before_graph(id);
    start_graph(theme, std::string("chart_") + std::to_string(id), std::string("Last run:") + result["title"].GetString());

    theme << "xAxis: { categories: \n";

    json_array_string(theme, string_collect(result["results"], "size"));

    theme << "},\n";

    y_axis_configuration(theme);

    theme << "legend: { enabled: false },\n";

    theme << "series: [\n";
    theme << "{\n";

    theme << "name: '',\n";
    theme << "data: ";

    json_array_int(theme, int_collect(result["results"], "duration"));

    theme << "\n}\n";
    theme << "]\n";

    end_graph(theme);
    theme.after_graph();
    ++id;
}

template<typename Theme, typename Filter>
void generate_compare_graph(Theme& theme, std::size_t& id, json_value base_result, const std::string& title, const char* attr, Filter f){
    theme.before_graph(id);
    start_graph(theme, std::string("chart_") + std::to_string(id), title + base_result["title"].GetString());

    theme << "xAxis: { categories: \n";

    json_array_string(theme, string_collect(base_result["results"], "size"));

    theme << "},\n";

    y_axis_configuration(theme);

    theme << "legend: { align: 'left', verticalAlign: 'top', floating: false, borderWidth: 0, y: 20 },\n";

    theme << "series: [\n";

    std::string comma = "";
    for(auto& document : theme.data.documents){
        if(f(document)){
            for(auto& result : document["results"]){
                if(str_equal(result["title"].GetString(), base_result["title"].GetString())){
                    theme << comma << "{\n";
                    theme << "name: '" << document[attr].GetString() << "',\n";
                    theme << "data: ";

                    json_array_int(theme, int_collect(result["results"], "duration"));

                    theme << "\n}\n";

                    comma = ",";
                }
            }
        }
    }

    theme << "]\n";

    end_graph(theme);
    theme.after_graph();
    ++id;
}

template<typename Theme>
void generate_compiler_graph(Theme& theme, std::size_t& id, const rapidjson::Value& base_result, const cpm::document_t& base){
    generate_compare_graph(theme, id, base_result, "Compiler:", "compiler", [&base](const cpm::document_t& doc){
        return  str_equal(doc["tag"].GetString(), base["tag"].GetString())
            &&  str_equal(doc["configuration"].GetString(), base["configuration"].GetString());
    });
}

template<typename Theme>
void generate_configuration_graph(Theme& theme, std::size_t& id, const rapidjson::Value& base_result, const cpm::document_t& base){
    generate_compare_graph(theme, id, base_result, "Configuration:", "configuration", [&base](const cpm::document_t& doc){
        return  str_equal(doc["tag"].GetString(), base["tag"].GetString())
            &&  str_equal(doc["compiler"].GetString(), base["compiler"].GetString());
    });
}

std::pair<bool,int> find_same_duration(const rapidjson::Value& base_result, const rapidjson::Value& r, const cpm::document_t& doc){
    for(auto& p_r : doc["results"]){
        if(str_equal(p_r["title"].GetString(), base_result["title"].GetString())){
            for(auto& p_r_r : p_r["results"]){
                if(str_equal(p_r_r["size"].GetString(), r["size"].GetString())){
                    return std::make_pair(true, p_r_r["duration"].GetInt());
                }
            }
        }
    }

    return std::make_pair(false, 0);
}

template<typename Theme>
std::pair<bool,double> compare(Theme& theme, const rapidjson::Value& base_result, const rapidjson::Value& r, const cpm::document_t& doc){
    bool found;
    int previous;
    std::tie(found, previous) = find_same_duration(base_result, r, doc);

    if(found){
        auto current = r["duration"].GetInt();

        double diff = 0.0;
        if(current < previous){
            diff = -1.0 * (100.0 * (static_cast<double>(previous) / current) - 100.0);
            theme.green_cell(std::to_string(diff) + "%");
        } else if(current > previous){
            diff = (100.0 * (static_cast<double>(current) / previous) - 100.0);
            theme.red_cell("+" + std::to_string(diff) + "%");
        } else {
            theme.cell("+0%");
        }

        return std::make_pair(true, diff);
    } else {
        return std::make_pair(false, 0.0);
    }
}

template<typename Theme>
void generate_summary_table(Theme& theme, const rapidjson::Value& base_result, const cpm::document_t& base){
    theme.before_summary();

    theme << "<tr>\n";
    theme << "<th>Size</th>\n";
    theme << "<th>Time</th>\n";
    theme << "<th>Previous</th>\n";
    theme << "<th>First</th>\n";

    if(theme.data.compilers.size() > 1){
        theme << "<th>Best compiler</th>\n";
        theme << "<th>Max difference</th>\n";
    }

    theme << "</tr>\n";

    double previous_acc = 0;
    double first_acc = 0;

    for(auto& r : base_result["results"]){
        theme << "<tr>\n";

        theme << "<td>" << r["size"].GetString() << "</td>\n";
        theme << "<td>" << r["duration"].GetInt() << "</td>\n";

        bool previous_found = false;
        double diff = 0.0;

        auto documents = select_documents(theme.data.documents, base);

        for(std::size_t i = 0; i < documents.size() - 1; ++i){
            if(&static_cast<const cpm::document_t&>(documents[i+1]) == &base){
                auto& doc = static_cast<const cpm::document_t&>(documents[i]);
                std::tie(previous_found, diff) = compare(theme, base_result, r, doc);

                if(previous_found){
                    previous_acc += diff;
                    break;
                }
            }
        }

        if(!previous_found){
            theme.cell("N/A");
        }

        previous_found = false;

        if(documents.size() > 1){
            auto& doc = static_cast<const cpm::document_t&>(documents[0]);
            std::tie(previous_found, diff) = compare(theme, base_result, r, doc);

            first_acc += diff;
        }

        if(!previous_found){
            theme.cell("N/A");
        }

        if(theme.data.compilers.size() > 1){
            std::string best_compiler = base["compiler"].GetString();
            auto best_duration = r["duration"].GetInt();
            auto worse_duration = r["duration"].GetInt();

            for(auto& doc : theme.data.documents){
                //Filter different tag
                if(!str_equal(doc["tag"].GetString(), base["tag"].GetString())){
                    continue;
                }

                //Filter different configuration
                if(!str_equal(doc["configuration"].GetString(), base["configuration"].GetString())){
                    continue;
                }

                bool found;
                int duration;
                std::tie(found, duration) = find_same_duration(base_result, r, doc);

                if(found){
                    if(duration < best_duration){
                        best_duration = duration;
                        best_compiler = doc["compiler"].GetString();
                    } else if(duration > worse_duration){
                        worse_duration = duration;
                    }
                }
            }

            theme.cell(best_compiler);

            auto max_diff = (100.0 * (static_cast<double>(worse_duration) / best_duration) - 100.0);

            theme.cell(std::to_string(max_diff) + "%");
        }

        theme << "</tr>\n";
    }

    theme << "<tr>\n";

    theme.cell("&nbsp;");
    theme.cell("&nbsp;");

    previous_acc /= base_result["results"].Size();
    first_acc /= base_result["results"].Size();

    if(previous_acc < 1e-5 && previous_acc > -1e-5){
        theme.cell("+0%");
    } else if(previous_acc < 0.0){
        theme.green_cell(std::to_string(previous_acc) + "%");
    } else if(previous_acc > 0.0){
        theme.red_cell("+" + std::to_string(previous_acc) + "%");
    }

    if(first_acc < 1e-5 && first_acc > -1e-5){
        theme.cell("+0%");
    } else if(first_acc < 0.0){
        theme.green_cell(std::to_string(first_acc) + "%");
    } else if(first_acc > 0.0){
        theme.red_cell("+" + std::to_string(first_acc) + "%");
    }

    theme.cell("&nbsp;");

    theme << "</tr>\n";

    theme.after_summary();
}

template<typename Theme>
void generate_time_graph(Theme& theme, std::size_t& id, const rapidjson::Value& result, const std::vector<cpm::document_cref>& documents){
    theme.before_graph(id);
    start_graph(theme, std::string("chart_") + std::to_string(id), std::string("Time:") + result["title"].GetString());

    theme << "xAxis: { type: 'datetime', title: { text: 'Date' } },\n";

    y_axis_configuration(theme);

    if(!theme.options.count("time-sizes")){
        theme << "legend: { enabled: false },\n";
    }

    theme << "series: [\n";

    if(theme.options.count("time-sizes")){
        std::string comma = "";
        for(auto& r : result["results"]){
            theme << comma << "{\n";

            theme << "name: '" << r["size"].GetString() << "',\n";
            theme << "data: [";

            std::string inner_comma = "";

            for(auto& document_r : documents){
                auto& document = static_cast<const cpm::document_t&>(document_r);

                for(auto& o_result : document["results"]){
                    if(str_equal(o_result["title"].GetString(), result["title"].GetString())){
                        for(auto& o_rr : o_result["results"]){
                            if(o_rr["size"].GetString() == r["size"].GetString()){
                                theme << inner_comma << "[" << document["timestamp"].GetInt() * 1000 << ",";
                                theme << o_rr["duration"].GetInt() << "]";
                                inner_comma = ",";
                            }
                        }
                    }
                }
            }

            theme << "]\n";
            theme << "}\n";
            comma =",";
        }
    } else {
        theme << "{\n";

        theme << "name: '',\n";
        theme << "data: [";

        std::string comma = "";

        for(auto& document_r : documents){
            auto& document = static_cast<const cpm::document_t&>(document_r);

            for(auto& o_result : document["results"]){
                if(str_equal(o_result["title"].GetString(), result["title"].GetString())){
                    theme << comma << "[" << document["timestamp"].GetInt() * 1000 << ",";
                    auto& o_r_results = o_result["results"];
                    theme << o_r_results[o_r_results.Size() - 1]["duration"].GetInt() << "]";
                    comma = ",";
                }
            }
        }

        theme << "]\n";
        theme << "}\n";
    }

    theme << "]\n";

    end_graph(theme);
    theme.after_graph();
    ++id;
}

std::vector<std::string> gather_sizes(const rapidjson::Value& section){
    std::vector<std::string> sizes;
    std::set<std::string> set_sizes;

    for(auto& r : section["results"]){
        for(auto& rr : r["results"]){
            auto s = rr["size"].GetString();
            if(!set_sizes.count(s)){
                set_sizes.insert(s);
                sizes.push_back(s);
            }
        }
    }

    return sizes;
}

template<typename Theme>
void generate_section_run_graph(Theme& theme, std::size_t& id, const rapidjson::Value& section){
    theme.before_graph(id);
    start_graph(theme, std::string("chart_") + std::to_string(id), std::string("Last run:") + section["name"].GetString());

    theme << "xAxis: { categories: \n";

    auto sizes = gather_sizes(section);

    json_array_string(theme, sizes);

    theme << "},\n";

    y_axis_configuration(theme);

    theme << "legend: { align: 'left', verticalAlign: 'top', floating: false, borderWidth: 0, y: 20 },\n";

    theme << "series: [\n";

    std::string comma = "";
    for(auto& r : section["results"]){
        theme << comma << "{\n";

        theme << "name: '" << r["name"].GetString() << "',\n";
        theme << "data: ";

        json_array_int(theme, int_collect(r["results"], "duration"));

        theme << "\n}\n";
        comma = ",";
    }

    theme << "]\n";

    end_graph(theme);
    theme.after_graph();
    ++id;
}

template<typename Theme>
void generate_section_time_graph(Theme& theme, std::size_t& id, const rapidjson::Value& section, const std::vector<cpm::document_cref>& documents){
    theme.before_graph(id);
    start_graph(theme, std::string("chart_") + std::to_string(id), std::string("Time:") + section["name"].GetString());

    theme << "xAxis: { type: 'datetime', title: { text: 'Date' } },\n";

    y_axis_configuration(theme);

    theme << "legend: { align: 'left', verticalAlign: 'top', floating: false, borderWidth: 0, y: 20 },\n";

    theme << "series: [\n";

    std::string comma = "";
    for(auto& r : section["results"]){
        theme << comma << "{\n";

        theme << "name: '" << r["name"].GetString() << "',\n";
        theme << "data: [";

        std::string comma_inner = "";

        for(auto& r_doc_r : documents){
            auto& r_doc = static_cast<const cpm::document_t&>(r_doc_r);

            for(auto& r_section : r_doc["sections"]){
                if(str_equal(r_section["name"].GetString(), section["name"].GetString())){
                    for(auto& r_r : r_section["results"]){
                        if(str_equal(r_r["name"].GetString(), r["name"].GetString())){
                            theme << comma_inner << "[" << r_doc["timestamp"].GetInt() * 1000 << ",";
                            auto& r_r_results = r_r["results"];
                            theme << r_r_results[r_r_results.Size() - 1]["duration"].GetInt() << "]";
                            comma_inner = ",";
                        }
                    }
                }
            }
        }

        theme << "]\n";
        theme << "}\n";
        comma = ",";
    }

    theme << "]\n";

    end_graph(theme);
    theme.after_graph();
    ++id;
}

template<typename Theme, typename Filter>
void generate_section_compare_graph(Theme& theme, std::size_t& id, const rapidjson::Value& section, const std::string& title, const char* attr, Filter f){
    std::size_t sub_id = 0;

    theme.before_sub_graphs(id, string_collect(section["results"], "name"));

    for(auto& r : section["results"]){
        theme.before_sub_graph(id, sub_id++);

        start_graph(theme,
            std::string("chart_") + std::to_string(id) + "-" + std::to_string(sub_id - 1),
            title + section["name"].GetString() + "-" + r["name"].GetString());

        theme << "xAxis: { categories: \n";

        auto sizes = gather_sizes(section);

        json_array_string(theme, sizes);

        theme << "},\n";

        y_axis_configuration(theme);

        theme << "legend: { align: 'left', verticalAlign: 'top', floating: false, borderWidth: 0, y: 20 },\n";

        theme << "series: [\n";

        std::string comma = "";
        for(auto& document : theme.data.documents){
            if(f(document)){
                for(auto& o_section : document["sections"]){
                    if(str_equal(o_section["name"].GetString(), section["name"].GetString())){
                        for(auto& o_r : o_section["results"]){
                            if(str_equal(o_r["name"].GetString(), r["name"].GetString())){
                                theme << comma << "{\n";
                                theme << "name: '" << document[attr].GetString() << "',\n";
                                theme << "data: ";

                                json_array_int(theme, int_collect(o_r["results"], "duration"));

                                theme << "\n}\n";

                                comma = ",";
                            }
                        }
                    }
                }
            }
        }

        theme << "]\n";

        end_graph(theme);

        theme.after_sub_graph();
    }

    theme.after_sub_graphs();

    ++id;
}

template<typename Theme>
void generate_section_compiler_graph(Theme& theme, std::size_t& id, const rapidjson::Value& section, const cpm::document_t& base){
    generate_section_compare_graph(theme, id, section, "Compiler:", "compiler", [&base](const cpm::document_t& doc){
        return  str_equal(doc["tag"].GetString(), base["tag"].GetString())
            &&  str_equal(doc["configuration"].GetString(), base["configuration"].GetString());
    });
}

template<typename Theme>
void generate_section_configuration_graph(Theme& theme, std::size_t& id, const rapidjson::Value& section, const cpm::document_t& base){
    generate_section_compare_graph(theme, id, section, "Configuration:", "configuration", [&base](const cpm::document_t& doc){
        return  str_equal(doc["tag"].GetString(), base["tag"].GetString())
            &&  str_equal(doc["compiler"].GetString(), base["compiler"].GetString());
    });
}

std::pair<bool, int> find_same_duration_section(json_value base_result, json_value base_section, json_value r, const cpm::document_t& doc){
    for(auto& section : doc["sections"]){
        if(str_equal(section["name"].GetString(), base_section["name"].GetString())){
            for(auto& result : section["results"]){
                if(str_equal(result["name"].GetString(), base_result["name"].GetString())){
                    for(auto& p_r_r : result["results"]){
                        if(str_equal(p_r_r["size"].GetString(), r["size"].GetString())){
                            return std::make_pair(true, p_r_r["duration"].GetInt());
                        }
                    }
                }
            }
        }
    }

    return std::make_pair(false, 0);
}

template<typename Theme>
std::pair<bool,double> compare_section(Theme& theme, json_value base_result, json_value base_section, json_value r, const cpm::document_t& doc){
    bool found;
    int previous;
    std::tie(found, previous) = find_same_duration_section(base_result, base_section, r, doc);

    if(found){
        auto current = r["duration"].GetInt();

        double diff = 0.0;
        if(current < previous){
            diff = -1.0 * (100.0 * (static_cast<double>(previous) / current) - 100.0);
            theme.green_cell(std::to_string(diff) + "%");
        } else if(current > previous){
            diff = 1.0 * (100.0 * (static_cast<double>(current) / previous) - 100.0);
            theme.red_cell("+" + std::to_string(diff) + "%");
        } else {
            theme.cell("+0%");
        }

        return std::make_pair(true, diff);
    }

    return std::make_pair(false, 0.0);
}

template<typename Theme>
void generate_section_summary_table(Theme& theme, std::size_t id, json_value base_section, const cpm::document_t& base){
    std::size_t sub_id = 0;
    theme.before_sub_graphs(id * 1000000, string_collect(base_section["results"], "name"));

    for(auto& base_result : base_section["results"]){
        theme.before_sub_summary(id * 1000000, sub_id++);

        theme << "<tr>\n";
        theme << "<th>Size</th>\n";
        theme << "<th>Time</th>\n";
        theme << "<th>Previous</th>\n";
        theme << "<th>First</th>\n";

        if(theme.data.compilers.size() > 1){
            theme << "<th>Best compiler</th>\n";
            theme << "<th>Max difference</th>\n";
        }

        theme << "</tr>\n";

        double previous_acc = 0;
        double first_acc = 0;

        for(auto& r : base_result["results"]){
            theme << "<tr>\n";
            theme << "<td>" << r["size"].GetString() << "</td>\n";
            theme << "<td>" << r["duration"].GetInt() << "</td>\n";

            bool previous_found = false;
            double diff = 0.0;

            auto documents = select_documents(theme.data.documents, base);

            for(std::size_t i = 0; i < documents.size() - 1; ++i){
                if(&static_cast<const cpm::document_t&>(documents[i+1]) == &base){
                    auto& doc = static_cast<const cpm::document_t&>(documents[i]);
                    std::tie(previous_found, diff) = compare_section(theme, base_result, base_section, r, doc);

                    if(previous_found){
                        previous_acc += diff;
                        break;
                    }
                }
            }

            if(!previous_found){
                theme << "<td>N/A</td>\n";
            }

            previous_found = false;

            if(documents.size() > 1){
                auto& doc = static_cast<const cpm::document_t&>(documents[0]);
                std::tie(previous_found, diff) = compare_section(theme, base_result, base_section, r, doc);

                first_acc += diff;
            }

            if(!previous_found){
                theme << "<td>N/A</td>\n";
            }

            if(theme.data.compilers.size() > 1){
                std::string best_compiler = base["compiler"].GetString();
                auto best_duration = r["duration"].GetInt();
                auto worse_duration = r["duration"].GetInt();

                for(auto& doc : theme.data.documents){
                    //Filter different tag
                    if(!str_equal(doc["tag"].GetString(), base["tag"].GetString())){
                        continue;
                    }

                    //Filter different configuration
                    if(!str_equal(doc["configuration"].GetString(), base["configuration"].GetString())){
                        continue;
                    }

                    bool found;
                    int duration;
                    std::tie(found, duration) = find_same_duration_section(base_result, base_section, r, doc);

                    if(found){
                        if(duration < best_duration){
                            best_duration = duration;
                            best_compiler = doc["compiler"].GetString();
                        } else if(duration > worse_duration){
                            worse_duration = duration;
                        }
                    }
                }

                theme.cell(best_compiler);

                auto max_diff = (100.0 * (static_cast<double>(worse_duration) / best_duration) - 100.0);

                theme.cell(std::to_string(max_diff) + "%");
            }

            theme << "</tr>\n";
        }

        theme << "<tr>\n";
        theme << "<td>&nbsp;</td>\n";
        theme << "<td>&nbsp;</td>\n";

        previous_acc /= base_result["results"].Size();
        first_acc /= base_result["results"].Size();

        if(previous_acc < 0.0){
            theme.green_cell(std::to_string(previous_acc) + "%");
        } else if(previous_acc > 0.0){
            theme.red_cell("+" + std::to_string(previous_acc) + "%");
        } else {
            theme.cell("+0%");
        }

        if(first_acc < 0.0){
            theme.green_cell(std::to_string(first_acc) + "%");
        } else if(first_acc > 0.0){
            theme.red_cell("+" + std::to_string(first_acc) + "%");
        } else {
            theme.cell("+0%");
        }

        theme << "</tr>\n";

        theme.after_sub_summary();
    }

    theme.after_sub_graphs();
}

template<typename Theme>
void generate_standard_page(const std::string& target_folder, const std::string& file, const cpm::reports_data& data, const cpm::document_t& doc, const std::vector<cpm::document_cref>& documents, cxxopts::Options& options){
    bool time_graphs = !options.count("disable-time") && documents.size() > 1;
    bool compiler_graphs = !options.count("disable-compiler") && data.compilers.size() > 1;
    bool configuration_graphs = !options.count("disable-configuration") && data.configurations.size() > 1;
    bool summary_table = !options.count("disable-summary");

    std::string target_file = target_folder + "/" + file;

    std::ofstream stream(target_file);

    Theme theme(data, options, stream, doc["compiler"].GetString(), doc["configuration"].GetString());

    //Header of the page
    header(theme);

    //Configure the highcharts theme
    if(options["hctheme"].as<std::string>() == "dark_unica"){
        theme << "<script>\n" << "\n";
        theme << dark_unica_theme << "\n";
        theme << "</script>\n";
    }

    //Information block about the last run
    information(theme, doc);

    //Compiler selection
    compiler_buttons(theme);

    //Configuration selection
    configuration_buttons(theme);

    std::size_t id = 1;
    for(const auto& result : doc["results"]){
        theme.before_result(result["title"].GetString());

        generate_run_graph(theme, id, result);

        if(time_graphs){
            generate_time_graph(theme, id, result, documents);
        }

        if(compiler_graphs){
            generate_compiler_graph(theme, id, result, doc);
        }

        if(configuration_graphs){
            generate_configuration_graph(theme, id, result, doc);
        }

        if(summary_table){
            generate_summary_table(theme, result, doc);
        }

        theme.after_result();
    }

    for(auto& section : doc["sections"]){
        theme.before_result(section["name"].GetString(), compiler_graphs);

        generate_section_run_graph(theme, id, section);

        if(time_graphs){
            generate_section_time_graph(theme, id, section, documents);
        }

        if(compiler_graphs){
            generate_section_compiler_graph(theme, id, section, doc);
        }

        if(configuration_graphs){
            generate_section_configuration_graph(theme, id, section, doc);
        }

        if(summary_table){
            generate_section_summary_table(theme, id, section, doc);
        }

        theme.after_result();
    }

    footer(theme);
}

template<typename Theme>
void generate_pages(const std::string& target_folder, cpm::reports_data& data, cxxopts::Options& options){
    //Select the base document
    auto& base = data.documents.back();

    //Generate the index
    generate_standard_page<Theme>(target_folder, "index.html", data, base, select_documents(data.documents, base), options);

    std::set<std::string> pages;

    //Generate the compiler pages
    std::for_each(data.documents.rbegin(), data.documents.rend(), [&](cpm::document_t& d){
        auto file = cpm::filify(d["compiler"].GetString(), d["configuration"].GetString());
        if(!pages.count(file)){
            generate_standard_page<Theme>(target_folder, file, data, d, select_documents(data.documents, d), options);
            pages.insert(file);
        }
    });
}

} //end of anonymous namespace

int main(int argc, char* argv[]){
    cxxopts::Options options(argv[0], "  results_folder");

    try {
        options.add_options()
            ("time-sizes", "Display multiple sizes in the time graphs")
            ("t,theme", "Theme name [raw,bootstrap,boostrap-tabs]", cxxopts::value<std::string>()->default_value("raw"))
            ("c,hctheme", "Highcharts Theme name [std,dark_unica]", cxxopts::value<std::string>()->default_value("dark_unica"), "theme_name")
            ("o,output", "Output folder", cxxopts::value<std::string>()->default_value("reports"), "output_folder")
            ("input", "Input results", cxxopts::value<std::string>())
            ("d,disable-time", "Disable time graphs")
            ("s,sort-by-tag", "Sort by tag instaed of time")
            ("disable-compiler", "Disable compiler graphs")
            ("disable-configuration", "Disable configuration graphs")
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
    data.documents = read(source_folder, options);

    if(data.documents.empty()){
        std::cout << "Unable to read any files" << std::endl;
        return -1;
    }

    //Collect the list of compilers
    for(auto& doc : data.documents){
        data.compilers.insert(doc["compiler"].GetString());
    }

    //Collect the list of configurations
    for(auto& doc : data.documents){
        data.configurations.insert(doc["configuration"].GetString());
    }

    if(options["theme"].as<std::string>() == "raw"){
        generate_pages<cpm::raw_theme>(target_folder, data, options);
    } else if(options["theme"].as<std::string>() == "bootstrap-tabs"){
        generate_pages<cpm::bootstrap_tabs_theme>(target_folder, data, options);
    } else {
        generate_pages<cpm::bootstrap_theme>(target_folder, data, options);
    }

    return 0;
}
