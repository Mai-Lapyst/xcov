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

    HtmlReporter reporter(conf);
    reporter.generateReport(report);
    return 0;
}