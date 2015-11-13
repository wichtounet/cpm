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

    std::string current_compiler;
    std::string current_configuration;

    raw_theme(const reports_data& data, cxxopts::Options& options, std::ostream& stream, std::string compiler, std::string configuration)
        : data(data), options(options), stream(stream), current_compiler(std::move(compiler)), current_configuration(std::move(configuration)) {}

    void include(){}
    void header(){}
    void footer(){}

    void before_information(std::string name){
        stream << "<h1>" << name << "</h1>\n";
        stream << "<ul>\n";
    }

    void after_information(){
        stream << "</ul>\n";
    }

    void compiler_buttons(){
        stream << "<div>\n";
        stream << R"=====(<span>Select compiler: </span>)=====";
        for(auto& compiler : data.compilers){
            stream << "<a href=\"" << cpm::filify(compiler, current_configuration)  << "\">" << compiler << "</a>\n";
        }
        stream << "</div>\n";
    }

    void configuration_buttons(){
        stream << "<div>\n";
        stream << R"=====(<span>configuration: </span>)=====";
        for(auto& configuration : data.configurations){
            stream << "<a href=\"" << cpm::filify(current_compiler, configuration)  << "\">" << configuration << "</a>\n";
        }
        stream << "</div>\n";
    }

    void after_buttons(){
        //Nothing to be done here
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

    template<typename T>
    raw_theme& operator<<(const T& v){
        stream << v;
        return *this;
    }
};

} //end of namespace cpm

#endif //CPM_RAW_THEME_HPP
