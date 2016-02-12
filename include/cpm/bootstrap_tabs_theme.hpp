//=======================================================================
// Copyright (c) 2015-2016 Baptiste Wicht
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

    std::vector<std::string> matches;

    bootstrap_tabs_theme(const reports_data& data, cxxopts::Options& options, std::ostream& stream, std::string compiler, std::string configuration) : bootstrap_theme(data, options, stream, compiler, configuration) {}

    void before_result(const std::string& title, bool sub, const std::vector<cpm::document_cref>& documents){
        bootstrap_theme::before_result(title, sub, documents);

        stream << "<div class=\"col-xs-12\">\n";
        stream << "<div role=\"tabpanel\">\n";
        stream << "<ul class=\"nav nav-tabs\" role=\"tablist\">\n";

        std::size_t tab_id = 0;

        stream
            << "<li class =\"active\" role=\"presentation\"><a href=\"#tab_" << uid << '_' << tab_id << "\" aria-controls=\"tab_"
            << uid << '_' << tab_id << "\" role=\"tab\" data-toggle=\"tab\">Last results</a></li>\n";
        ++tab_id;

        if(documents.size() > 1 && !options.count("disable-time")){
            stream
                << "<li role=\"presentation\"><a href=\"#tab_" << uid << '_' << tab_id << "\" aria-controls=\"tab_"
                << uid << '_' << tab_id << "\" role=\"tab\" data-toggle=\"tab\">Results over time</a></li>\n";
            ++tab_id;
        }

        if(data.compilers.size() > 1 && !options.count("disable-compiler")){
            stream
                << "<li role=\"presentation\"><a href=\"#tab_" << uid << '_' << tab_id << "\" aria-controls=\"tab_"
                << uid << '_' << tab_id << "\" role=\"tab\" data-toggle=\"tab\">Compilers</a></li>\n";
            ++tab_id;
        }

        if(data.configurations.size() > 1 && !options.count("disable-configuration")){
            stream
                << "<li role=\"presentation\"><a href=\"#tab_" << uid << '_' << tab_id << "\" aria-controls=\"tab_"
                << uid << '_' << tab_id << "\" role=\"tab\" data-toggle=\"tab\">Configurations</a></li>\n";
            ++tab_id;
        }

        if(!options.count("disable-summary")){
            stream
                << "<li role=\"presentation\"><a href=\"#tab_" << uid << '_' << tab_id << "\" aria-controls=\"tab_"
                << uid << '_' << tab_id << "\" role=\"tab\" data-toggle=\"tab\">Summary</a></li>\n";
            ++tab_id;
        }

        stream << "</ul>\n";
        stream << "<div class=\"tab-content\">\n";

        matches.clear();

        if(uid == 0){
            stream << "<script>\n";
            stream << "$(function () {\n";
            stream << "$('a[data-toggle=\"tab\"]').on( 'shown.bs.tab', function (e) {\n";
            stream << "$(\".cpm_chart\").each(function(){\n";
            stream << "var chart = $(this).highcharts();\n";
            stream << "chart.reflow();\n";
            stream << "});\n";
            stream << "});\n";
            stream << "});\n";
            stream << "</script>\n";
        }
    }

    void after_result(){
        stream << "<script>\n";

        stream << "$(function () {\n";
        stream << "var max_height = 0;\n";

        for(auto& match : matches){
            stream << "max_height = Math.max(max_height, $('#" << match << "').height());\n";
        }

        for(auto& match : matches){
            stream << "$('#" << match << "').height(max_height);\n";
        }

        stream << "});\n";

        stream << "</script>\n";

        stream << "</div>\n";
        stream << "</div>\n";
        stream << "</div>\n";

        bootstrap_theme::after_result();

        ++uid;
        tid = 0;
    }

    virtual void start_column(const std::string& /*style*/) override {
        auto tab_id = std::string("tab_") + std::to_string(uid) + "_" + std::to_string(tid);

        if(tid == 0){
            stream << "<div role=\"tabpanel\" class=\"tab-pane active\" id=\"" << tab_id << "\">\n";
        } else {
            stream << "<div role=\"tabpanel\" class=\"tab-pane\" id=\"" << tab_id << "\">\n";
        }

        matches.push_back(tab_id);

        ++tid;
    }

    virtual void close_column(){
        stream << "</div>\n";
    }

    void before_graph(std::size_t id){
        start_column("");

        stream << "<div id=\"chart_" << id << "\" class=\"cpm_chart\" style=\"height: 400px;\"></div>\n";
    }

    template<typename T>
    bootstrap_tabs_theme& operator<<(const T& v){
        stream << v;
        return *this;
    }
};

} //end of namespace cpm

#endif //CPM_BOOTSTRAP_THEME_HPP
