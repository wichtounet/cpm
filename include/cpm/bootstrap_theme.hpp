//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
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

    bootstrap_theme(const reports_data& data, cxxopts::Options& options) : data(data), options(options) {}

    void include(std::ostream& stream){
        stream << "<script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/js/bootstrap.min.js\"></script>\n";
        stream << "<link href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap.min.css\" rel=\"stylesheet\">\n";
        stream << "<link href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap-theme.min.css\" rel=\"stylesheet\">\n";
    }

    void header(std::ostream& stream){
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
            <li><a href="http://github.com/wichtounet/cpm">Generated with CPM</a></li>
            </ul>
            </div>
            </div>
            </nav>
        )=====";
    }

    void footer(std::ostream& stream){
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
    }

    void before_information(std::ostream& stream){
        stream << "<div class=\"jumbotron\">\n";
        stream << "<div class=\"container-fluid\">\n";
    }

    void after_information(std::ostream& stream){
        stream << "</div>\n";
        stream << "</div>\n";
        stream << "<div class=\"container-fluid\">\n";
    }

    void compiler_buttons(std::ostream& stream, const std::string& current_compiler){
        stream << R"=====(<div class="row">)=====";
        stream << R"=====(<div class="col-xs-12">)=====";

        stream << R"=====(<span>Select compiler: </span>)=====";

        stream << R"=====(<div class="btn-group" role="group">)=====";
        for(auto& compiler : data.compilers){
            if(compiler == current_compiler){
                stream << "<a class=\"btn btn-primary\" href=\"" << cpm::filify(compiler)  << "\">" << compiler << "</a>\n";
            } else {
                stream << "<a class=\"btn btn-default\" href=\"" << cpm::filify(compiler)  << "\">" << compiler << "</a>\n";
            }
        }
        stream << "</div>\n";

        stream << "</div>\n";
        stream << "</div>\n";
    }

    void before_graph(std::ostream& stream, std::size_t id){
        //TODO Improve this to select correctly the width
        if(data.compilers.size() > 1 && !options.count("disable-compiler")){
            stream << "<div class=\"col-xs-4\">\n";
        } else {
            stream << "<div class=\"col-xs-6\">\n";
        }
        stream << "<div id=\"chart_" << id << "\" style=\"min-width:400px; height: 400px;\"></div>\n";
    }

    void after_graph(std::ostream& stream){
        stream << "</div>\n";
    }

    void before_result(std::ostream& stream, const std::string& title){
        stream << "<div class=\"page-header\">\n";
        stream << "<h2>" << title << "</h2>\n";
        stream << "</div>\n";
        stream << "<div class=\"row\">\n";
    }

    void after_result(std::ostream& stream){
        stream << "</div>\n";
    }
};

} //end of namespace cpm

#endif //CPM_BOOTSTRAP_THEME_HPP
