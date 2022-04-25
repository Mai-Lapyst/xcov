#pragma once

#include "report.h"
#include "config.h"
#include "inja/inja.hpp"

namespace xcov {

    class HtmlReporter : public Reporter {
    private:
        inja::Environment env;
        inja::Template indexTempl;
        inja::Template funcIndexTempl;
        inja::Template sourceFileTempl;

        fs::path outputDir;
        fs::path tmplDir;
        std::string title;

    public:
        HtmlReporter(Config& conf);

        void generateReport(Report& report) override;

        void generateIndex(Report& stats);
        void generateFromSourcefile(SourceFile& file);
    };

}