//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_RAW_THEME_HPP
#define CPM_RAW_THEME_HPP

#include "cpm/io.hpp"
#include "cpm/data.hpp"

namespace cpm {

struct raw_theme {
    const cpm::reports_data& data;
    cxxopts::Options& options;
    std::ostream& stream;

    raw_theme(const cpm::reports_data& data, cxxopts::Options& options, std::ostream& stream) : data(data), options(options), stream(stream) {}

    void include(){}
    void header(){}
    void footer(){}
    void before_information(){}
    void after_information(){}

    void compiler_buttons(const std::string& /*current_compiler*/){
        stream << "<div>\n";
        stream << R"=====(<span>Select compiler: </span>)=====";
        for(auto& compiler : data.compilers){
            stream << "<a href=\"" << cpm::filify(compiler)  << "\">" << compiler << "</a>\n";
        }
        stream << "</div>\n";
    }

    void before_graph(std::size_t id){
        stream << "<div id=\"chart_" << id << "\" style=\"float:left; width:600px; height: 400px; margin: 5 auto; padding-right: 10px; \"></div>\n";
    }

    void after_graph(){}

    void before_result(const std::string& title, bool /*sub */ = false){
        stream << "<h2 style=\"clear:both\">" << title << "</h2>\n";
    }

    void after_result(){}

    void before_sub_graphs(std::size_t /*id*/, std::vector<std::string> /*graphs*/){}

    void after_sub_graphs(){}

    void before_sub_graph(std::size_t id, std::size_t sub){
        stream << "<div id=\"chart_" << id << "-" << sub << "\" style=\"float:left; width:600px; height: 400px; margin: 5 auto; padding-right: 10px; padding-bottom: 10px; \"></div>\n";
    }

    void after_sub_graph(){}

    void before_summary(){
        stream << "<table>\n";
    }

    void after_summary(){
        stream << "</table>\n";
    }

    void before_sub_summary(std::size_t /*id*/, std::size_t /*sub*/){
        stream << "<table>\n";
    }

    void after_sub_summary(){
        stream << "</table>\n";
    }

    void cell(const std::string& v){
        stream << "<td>" << v << "</td>\n";
    }

    void red_cell(const std::string& v){
        stream << "<td style=\"color:red;\">" << v << "</td>\n";
    }

    void green_cell(const std::string& v){
        stream << "<td style=\"color:green;\">" << v << "</td>\n";
    }
};

} //end of namespace cpm

#endif //CPM_RAW_THEME_HPP
