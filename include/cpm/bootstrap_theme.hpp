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
        stream << "<div id=\"chart_" << id << "\" style=\"height: 400px;\"></div>\n";
    }

    void after_graph(std::ostream& stream){
        stream << "</div>\n";
    }

    void before_result(std::ostream& stream, const std::string& title, bool sub = false){
        stream << "<div class=\"page-header\">\n";
        stream << "<h2>" << title << "</h2>\n";
        stream << "</div>\n";

        if(sub){
            stream << "<div class=\"row\" style=\"display:flex; align-items: flex-end\">\n";
        } else {
            stream << "<div class=\"row\">\n";
        }
    }

    void after_result(std::ostream& stream){
        stream << "</div>\n";
    }

    void before_sub_graphs(std::ostream& stream, std::size_t id, std::vector<std::string> graphs){
        //TODO Improve this to select correctly the width
        if(data.compilers.size() > 1 && !options.count("disable-compiler")){
            stream << "<div class=\"col-xs-4\">\n";
        } else {
            stream << "<div class=\"col-xs-6\">\n";
        }

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

    void after_sub_graphs(std::ostream& stream){
        stream << "</div>\n";
        stream << "</div>\n";
        stream << "</div>\n";
    }

    void before_sub_graph(std::ostream& stream, std::size_t id, std::size_t sub){
        auto sub_id = std::string("sub") + std::to_string(id) + "-" + std::to_string(sub);
        std::string active;
        if(sub == 0){
            active = " active";
        }
        stream << "<div role=\"tabpanel\" class=\"tab-pane" << active << "\" id=\"" << sub_id << "\">\n";

        stream << "<div id=\"chart_" << id << "-" << sub << "\" style=\"height: 400px;\"></div>\n";
    }

    void after_sub_graph(std::ostream& stream){
        stream << "</div>\n";
    }
};

} //end of namespace cpm

#endif //CPM_BOOTSTRAP_THEME_HPP
