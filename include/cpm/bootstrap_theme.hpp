//=======================================================================
// Copyright (c) 2015-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_BOOTSTRAP_THEME_HPP
#define CPM_BOOTSTRAP_THEME_HPP

#include "cpm/io.hpp"
#include "cpm/data.hpp"

namespace cpm {

struct bootstrap_theme {
    const reports_data& data;
    cxxopts::Options& options;
    std::ostream& stream;
    std::string current_compiler;
    std::string current_configuration;

    std::size_t current_column = 0;

    bootstrap_theme(const reports_data& data, cxxopts::Options& options, std::ostream& stream, std::string compiler, std::string configuration)
        : data(data), options(options), stream(stream), current_compiler(std::move(compiler)), current_configuration(std::move(configuration)) {}

    void include(){
        stream << "<script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/js/bootstrap.min.js\"></script>\n";
        stream << "<link href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap.min.css\" rel=\"stylesheet\">\n";
        stream << "<link href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap-theme.min.css\" rel=\"stylesheet\">\n";

        stream << R"=====(
            <style>
                .jumbotron {
                    padding-top: 38px;
                    padding-bottom: 8px;
                    margin-bottom: 5px;
                }

                .page-header {
                    margin-top: 10px;
                }
            </style>
        )=====";
    }

    void header(){
        stream << R"=====(
            <nav id="myNavbar" class="navbar navbar-default navbar-inverse navbar-fixed-top" role="navigation">
            <div class="container-fluid">
            <div class="navbar-header">
            <button type="button" class="navbar-toggle" data-toggle="collapse" data-target="#navbarCollapse">
            <span class="sr-only">Toggle navigation</span>
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
            </button>
            <a class="navbar-brand" href="#">Results</a>
            </div>
            <div class="collapse navbar-collapse" id="navbarCollapse">
            <ul class="nav navbar-nav">
            <li class="active"><a href="index.html">Home</a></li>
            <li><a href="https://github.com/wichtounet/cpm">Generated with CPM</a></li>
            </ul>
            </div>
            </div>
            </nav>
        )=====";
    }

    void footer(){
        stream << R"=====(
            <hr>
            <div class="row">
            <div class="col-xs-12">
            <footer>
            <p>Generated with <a href="https://github.com/wichtounet/cpm">CPM</a></p>
            </footer>
            </div>
            </div>
            </div>
        )=====";

        if(options.count("pages")){
            stream << "</div>\n";
            stream << "</div>\n";
        }
    }

    void before_information(std::string name){
        stream << "<div class=\"jumbotron\">\n";
        stream << "<div class=\"container-fluid\">\n";
        stream << "<h1>" << name << "</h1>\n";
        stream << "<div class=\"row\">\n";
        stream << "<div class=\"col-xs-3\">\n";
        stream << "<ul>\n";
    }

    void after_information(){
        stream << "</ul>\n";
        stream << "</div>\n"; //col-xs-3

        if(data.compilers.size() > 1 || data.configurations.size() > 1){
            stream << "<div class=\"col-xs-9\">\n";
        }
    }

    void compiler_buttons(){
        stream << R"=====(<div class="row">)=====";
        stream << R"=====(<div class="col-xs-12">)=====";

        stream << R"=====(<span>Select compiler: </span>)=====";
        stream << R"=====(<div class="btn-group" role="group">)=====";

        if(options.count("pages")){
            for(auto& compiler : data.compilers){
                auto file = cpm::filify(compiler, current_configuration, data.sub_part);
                if(compiler == current_compiler){
                    stream << "<a class=\"btn btn-primary\" href=\"" << file << "\">" << compiler << "</a>\n";
                } else {
                    stream << "<a class=\"btn btn-default\" href=\"" << file << "\">" << compiler << "</a>\n";
                }
            }
        } else {
            for(auto& compiler : data.compilers){
                if(compiler == current_compiler){
                    stream << "<a class=\"btn btn-primary\" href=\"" << cpm::filify(compiler, current_configuration)  << "\">" << compiler << "</a>\n";
                } else {
                    stream << "<a class=\"btn btn-default\" href=\"" << cpm::filify(compiler, current_configuration)  << "\">" << compiler << "</a>\n";
                }
            }
        }

        stream << "</div>\n"; //btn-group

        stream << "</div>\n"; //col-xs-12
        stream << "</div>\n"; //row
    }

    void configuration_buttons(){
        if(data.compilers.size() > 1){
            stream << R"=====(<div class="row" style="padding-top:5px;">)=====";
        } else {
            stream << R"=====(<div class="row">)=====";
        }

        stream << R"=====(<div class="col-xs-12">)=====";

        stream << R"=====(<span>Select configuration: </span>)=====";

        stream << R"=====(<div class="btn-group" role="group">)=====";

        if(options.count("pages")){
            for(auto& configuration : data.configurations){
                auto file = cpm::filify(current_compiler, configuration, data.sub_part);
                if(configuration == current_configuration){
                    stream << "<a class=\"btn btn-primary\" href=\"" << file  << "\">" << configuration << "</a>\n";
                } else {
                    stream << "<a class=\"btn btn-default\" href=\"" << file  << "\">" << configuration << "</a>\n";
                }
            }
        } else {
            for(auto& configuration : data.configurations){
                if(configuration == current_configuration){
                    stream << "<a class=\"btn btn-primary\" href=\"" << cpm::filify(current_compiler, configuration)  << "\">" << configuration << "</a>\n";
                } else {
                    stream << "<a class=\"btn btn-default\" href=\"" << cpm::filify(current_compiler, configuration)  << "\">" << configuration << "</a>\n";
                }
            }
        }

        stream << "</div>\n"; //btn-group
        stream << "</div>\n"; //col-xs-12
        stream << "</div>\n"; //row
    }

    void after_buttons(){
        if(data.compilers.size() > 1 || data.configurations.size() > 1){
            stream << "</div>\n"; //col-xs-9
        }

        stream << "</div>\n"; //row
        stream << "</div>\n"; //container-fluid
        stream << "</div>\n"; //jumbotron

        if(options.count("pages")){
            stream << R"=====(
                <div class=container-fluid>
                <div class="row">
                <div class="col-xs-2">
                <ul class="nav nav-stacked" id="sidebar">
            )=====";

            for(auto& link : data.files){
                stream << "<li><a href=\"" << link.second << "\">" << link.first << "</a></li>" << std::endl;
            }

            stream << R"=====(
                </ul>
                </div>
                <div class="col-xs-10">
                <div class=container-fluid>
            )=====";
        } else {
            stream << "<div class=\"container-fluid\">\n";
        }
    }

    virtual void start_column(const std::string& style = ""){
        std::size_t columns = 1; //Always the first grapah

        if(data.documents.size() > 1 && !options.count("disable-time")){
            ++columns;
        }

        if(data.compilers.size() > 1 && !options.count("disable-compiler")){
            ++columns;
        }

        if(data.configurations.size() > 1 && !options.count("disable-configuration")){
            ++columns;
        }

        if(!options.count("disable-summary")){
            ++columns;
        }

        if(columns < 4){
            stream << "<div class=\"col-xs-" << 12 / columns << "\"" << style << ">\n";
        } else if(columns == 4){
            if(current_column == 2){
                stream << "</div>\n";
                stream << "<div class=\"row\" style=\"display:flex; margin-top: 10px;\">\n";
            }

            stream << "<div class=\"col-xs-6\"" << style << ">\n";
        } else if(columns == 5){
            if(current_column == 3){
                stream << "</div>\n";
                stream << "<div class=\"row\" style=\"display:flex; margin-top: 10px;\">\n";
            }

            if(current_column < 3){
                stream << "<div class=\"col-xs-4\"" << style << ">\n";
            } else if(current_column == 3){
                stream << "<div class=\"col-xs-4\"" << style << ">\n";
            } else if(current_column == 4){
                stream << "<div class=\"col-xs-8\"" << style << ">\n";
            }
        }

        ++current_column;
    }

    virtual void close_column(){
        stream << "</div>\n";
    }

    void before_graph(std::size_t id){
        start_column();

        stream << "<div id=\"chart_" << id << "\" style=\"height: 400px;\"></div>\n";
    }

    void after_graph(){
        close_column();
    }

    void before_result(const std::string& title, bool sub, const std::vector<cpm::document_cref>& /*documents*/){
        stream << "<div class=\"page-header\">\n";
        stream << "<h2>" << title << "</h2>\n";
        stream << "</div>\n";

        if(sub){
            stream << "<div class=\"row\" style=\"display:flex; align-items: flex-end\">\n";
        } else {
            stream << "<div class=\"row\">\n";
        }

        current_column = 0;
    }

    void after_result(){
        stream << "</div>\n";
    }

    void before_sub_graphs(std::size_t id, std::vector<std::string> graphs){
        start_column("style=\"align-self: flex-start; \"");

        stream << "<div role=\"tabpanel\">\n";
        stream << "<ul class=\"nav nav-tabs\" role=\"tablist\">\n";

        std::string active = "class=\"active\"";
        std::size_t sub = 0;
        for(auto& g : graphs){
            auto sub_id = std::string("sub") + std::to_string(id) + "-" + std::to_string(sub++);
            stream << "<li " << active << " role=\"presentation\"><a href=\"#" << sub_id << "\" aria-controls=\""
                << sub_id << "\" role=\"tab\" data-toggle=\"tab\">" << g << "</a></li>\n";
            active = "";
        }

        stream << "</ul>\n";
        stream << "<div class=\"tab-content\">\n";
    }

    void after_sub_graphs(){
        stream << "</div>\n";
        stream << "</div>\n";

        close_column();
    }

    void before_sub_graph(std::size_t id, std::size_t sub){
        auto sub_id = std::string("sub") + std::to_string(id) + "-" + std::to_string(sub);
        std::string active;
        if(sub == 0){
            active = " active";
        }
        stream << "<div role=\"tabpanel\" class=\"tab-pane" << active << "\" id=\"" << sub_id << "\">\n";

        stream << "<div id=\"chart_" << id << "-" << sub << "\" style=\"height: 400px;\"></div>\n";
    }

    void after_sub_graph(){
        stream << "</div>\n";
    }

    void before_summary(){
        start_column();

        stream << "<table class=\"table\">\n";
    }

    void after_summary(){
        stream << "</table>\n";

        close_column();
    }

    void before_sub_summary(std::size_t id, std::size_t sub){
        auto sub_id = std::string("sub") + std::to_string(id) + "-" + std::to_string(sub);
        std::string active;
        if(sub == 0){
            active = " active";
        }

        stream << "<div role=\"tabpanel\" class=\"tab-pane" << active << "\" id=\"" << sub_id << "\">\n";
        stream << "<table class=\"table\">\n";
    }

    void after_sub_summary(){
        stream << "</table>\n";
        stream << "</div>\n";
    }

    void cell(const std::string& v){
        stream << "<td>" << v << "</td>\n";
    }

    void red_cell(const std::string& v){
        stream << "<td class=\"danger\">"<< v << "</td>\n";
    }

    void green_cell(const std::string& v){
        stream << "<td class=\"success\">" << v << "</td>\n";
    }

    template<typename T>
    bootstrap_theme& operator<<(const T& v){
        stream << v;
        return *this;
    }
};

} //end of namespace cpm

#endif //CPM_BOOTSTRAP_THEME_HPP
