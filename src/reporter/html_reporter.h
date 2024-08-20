/*
 * xcov - eXtended COVerage reporter for gcov
 * Copyright (C) 2022-2024 Mai-Lapyst
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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