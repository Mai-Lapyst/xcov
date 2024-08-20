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

#include "utils.h"
#include "args.h"
#include "reporter/html_reporter.h"
#include "formats/gcov.h"

#include <ftw.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include "config.h"

void applyCliArgs(xcov::Config& conf) {
    using namespace xcov;

    conf.logVerbose = gCliArgs.verbose_flag;
    conf.verboseLevel = gCliArgs.verbose_level;

    if (gCliArgs.hasOutputDir) {
        conf.reportDir = gCliArgs.outputDir;
    }
    if (gCliArgs.hasRootDir) {
        conf.rootDir = gCliArgs.rootDir;
    }
    if (gCliArgs.hasTitle) {
        conf.title = gCliArgs.title;
    }
    if (gCliArgs.hasTemplateDir) {
        conf.templatePath = gCliArgs.templateDir;
    }
    if (gCliArgs.hasNameingStrategy) {
        conf.nameingStrategy = gCliArgs.fileNameingStrategy;
    }
    if (gCliArgs.hasLangDef) {
        conf.langDef = gCliArgs.langDef;
    }
    if (gCliArgs.hasOutFormat) {
        conf.outFormat = gCliArgs.outFormat;
    }
    if (gCliArgs.hasReportFormat) {
        conf.reportFormat = gCliArgs.reportFormat;
    }
}

int main(int argc, char** argv) {
    using namespace xcov;

    parseArguments(argc, argv);

    Config conf;
    loadConfig(conf);
    if (gCliArgs.hasConfigFile) {
        conf.loadFrom(gCliArgs.configFile);
    }
    applyCliArgs(conf);

    Report report;
    report.datetime = time(NULL);
    loadGcovData(report, conf);
    report.recalcStats();
    report.sortFiles();

    if (conf.reportFormat == "html") {
        HtmlReporter reporter(conf);
        reporter.generateReport(report);
    }
    else {
        std::cerr << "Error: unknown report format '" << conf.reportFormat << "'" << std::endl;
        return 1;
    }
    return 0;
}