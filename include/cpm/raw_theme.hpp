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

    raw_theme(const cpm::reports_data& data, cxxopts::Options& options) : data(data), options(options) {}

    void include(std::ostream& /*stream*/){}
    void header(std::ostream& /*stream*/){}
    void footer(std::ostream& /*stream*/){}
    void before_information(std::ostream& /*stream*/){}
    void after_information(std::ostream& /*stream*/){}

    void compiler_buttons(std::ostream& stream, const std::string& /*current_compiler*/){
        stream << "<div>\n";
        stream << R"=====(<span>Select compiler: </span>)=====";
        for(auto& compiler : data.compilers){
            stream << "<a href=\"" << cpm::filify(compiler)  << "\">" << compiler << "</a>\n";
        }
        stream << "</div>\n";
    }

    void before_graph(std::ostream& stream, std::size_t id){
        stream << "<div id=\"chart_" << id << "\" style=\"float:left; width:600px; height: 400px; margin: 5 auto; padding-right: 10px; \"></div>\n";
    }

    void after_graph(std::ostream& /*stream*/){}

    void before_result(std::ostream& stream, const std::string& title, bool /*sub */ = false){
        stream << "<h2 style=\"clear:both\">" << title << "</h2>\n";
    }

    void after_result(std::ostream& /*stream*/){}

    void before_sub_graphs(std::ostream& /*stream*/, std::size_t /*id*/, std::vector<std::string> /*graphs*/){}

    void after_sub_graphs(std::ostream& /*stream*/){}

    void before_sub_graph(std::ostream& stream, std::size_t id, std::size_t sub){
        stream << "<div id=\"chart_" << id << "-" << sub << "\" style=\"float:left; width:600px; height: 400px; margin: 5 auto; padding-right: 10px; padding-bottom: 10px; \"></div>\n";
    }

    void after_sub_graph(std::ostream& stream){
        stream << "</div>\n";
    }
};

} //end of namespace cpm

#endif //CPM_RAW_THEME_HPP
