//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_BOOTSTRAP_TABS_THEME_HPP
#define CPM_BOOTSTRAP_TABS_THEME_HPP

#include "cpm/io.hpp"
#include "cpm/data.hpp"
#include "cpm/bootstrap_theme.hpp"

namespace cpm {

struct bootstrap_tabs_theme : bootstrap_theme {
    std::size_t uid = 0;
    std::size_t tid = 0;

    bootstrap_tabs_theme(const reports_data& data, cxxopts::Options& options, std::ostream& stream, std::string compiler, std::string configuration) : bootstrap_theme(data, options, stream, compiler, configuration) {}

    void before_result(const std::string& title, bool sub = false){
        bootstrap_theme::before_result(title, sub);

        stream << "<div class=\"col-xs-12\">\n";
        stream << "<div role=\"tabpanel\">\n";
        stream << "<ul class=\"nav nav-tabs\" role=\"tablist\">\n";

        stream
            << "<li class =\"active\" role=\"presentation\"><a href=\"#tab_" << uid << "_0\" aria-controls=\"tab_"
            << uid << "_0\" role=\"tab\" data-toggle=\"tab\">Last results</a></li>\n";

        if(data.documents.size() > 1 && !options.count("disable-time")){
            stream
                << "<li role=\"presentation\"><a href=\"#tab_" << uid << "_1\" aria-controls=\"tab_"
                << uid << "_1\" role=\"tab\" data-toggle=\"tab\">Results over time</a></li>\n";
        }

        if(data.compilers.size() > 1 && !options.count("disable-compiler")){
            stream
                << "<li role=\"presentation\"><a href=\"#tab_" << uid << "_2\" aria-controls=\"tab_"
                << uid << "_2\" role=\"tab\" data-toggle=\"tab\">Results over compiler</a></li>\n";
        }

        if(!options.count("disable-summary")){
            stream
                << "<li role=\"presentation\"><a href=\"#tab_" << uid << "_3\" aria-controls=\"tab_"
                << uid << "_3\" role=\"tab\" data-toggle=\"tab\">Summary</a></li>\n";
        }

        stream << "</ul>\n";
        stream << "<div class=\"tab-content\">\n";
    }

    void after_result(){
        stream << "</div>\n";
        stream << "</div>\n";
        stream << "</div>\n";

        bootstrap_theme::after_result();

        ++uid;
        tid = 0;
    }

    virtual void start_column(const std::string& /*style*/) override {
        if(tid == 0){
            stream << "<div role=\"tabpanel\" class=\"tab-pane active\" id=\"tab_" << uid << "_" << tid++ << "\">\n";
        } else {
            stream << "<div role=\"tabpanel\" class=\"tab-pane\" id=\"tab_" << uid << "_" << tid++ << "\">\n";
        }
    }

    virtual void close_column(){
        stream << "</div>\n";
    }

    template<typename T>
    bootstrap_tabs_theme& operator<<(const T& v){
        stream << v;
        return *this;
    }
};

} //end of namespace cpm

#endif //CPM_BOOTSTRAP_THEME_HPP
