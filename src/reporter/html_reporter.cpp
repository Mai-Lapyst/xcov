#include "reporter/html_reporter.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

namespace xcov {

    HtmlReporter::HtmlReporter(Config& conf) : Reporter(conf) {

        this->outputDir = conf.reportDir;
        if (this->outputDir.empty()) {
            std::cerr << "Error: HtmlReporter needs reportDir to be set!" << std::endl;
            std::exit(1);
        }

        this->tmplDir = conf.templatePath;
        this->title = conf.title;

        env.add_callback("expandLines", 1, [&conf](inja::Arguments& args) {
            SourceFile srcFile = args.at(0)->get<SourceFile>();
            std::vector<SourceLine> sourceLines;
            srcFile.expandLines(sourceLines, conf);
            return sourceLines;
        });

        env.add_callback("roundPercentage", 2, [](inja::Arguments& args) {
            double d = args.at(0)->get<double>();
            int precision = args.at(1)->get<int>();
            if (std::isnan(d)) {
                return std::string("-");
            }
            std::ostringstream stream;
            stream << std::fixed << std::setprecision(precision) << d;
            return stream.str();
        });

        env.add_callback("filename", 1, [this](inja::Arguments& args) {
            std::string path = args.at(0)->get<std::string>();
            return this->generateFilename(path, "").string();
        });

        fs::path sourceFileTemplPath = this->tmplDir / "sourcefile.html";
        if (!fs::exists(sourceFileTemplPath)) {
            throw std::runtime_error("Could not find sourcefile template: " + sourceFileTemplPath.string());
        }
        this->sourceFileTempl = env.parse_template(sourceFileTemplPath);

        fs::path indexTemplPath = this->tmplDir / "index.html";
        if (!fs::exists(sourceFileTemplPath)) {
            throw std::runtime_error("Could not find index template: " + indexTemplPath.string());
        }
        this->indexTempl = env.parse_template(indexTemplPath);
    }

    void HtmlReporter::generateReport(Report& report) {
        // ensure that output dir exists
        fs::create_directories(this->outputDir);

        this->generateIndex(report);
        for (SourceFile& srcFile : report.sourceFiles) {
            this->generateFromSourcefile(srcFile);
        }

        // copy css file(s)
        fs::copy_file(this->tmplDir / "theme.css", this->outputDir / "theme.css", fs::copy_options::overwrite_existing);
        fs::copy_file(this->tmplDir / "bootstrap.min.css", this->outputDir / "bootstrap.min.css", fs::copy_options::overwrite_existing);
    }

    void HtmlReporter::generateIndex(Report& report) {
        fs::path outFilePath = this->outputDir / "index.html";
        std::ofstream outFile(outFilePath);

        nlohmann::json j;
        j["title"] = this->title;
        j["report"] = report;

        char* dateStr = xcov::utils::formatDateTime(report.datetime, "%c");
        j["date"] = dateStr;

        this->env.render_to(outFile, this->indexTempl, j);

        outFile.close();
        free(dateStr);
    }

    void HtmlReporter::generateFromSourcefile(SourceFile& file) {
        using namespace xcov::utils;

        fs::path outFilepath = this->outputDir / this->generateFilename(file.path, ".html");
        fs::create_directories(outFilepath.parent_path());

        std::ofstream outFile(outFilepath);

        std::cout << "[HtmlReporter::generateFromSourcefile(" << file.path << ")] writing to " << outFilepath << std::endl;

        nlohmann::json j;
        j["title"] = this->title;
        j["file"] = file;
        j["rootPath"] = fs::relative(this->outputDir, outFilepath.parent_path());

        char* dateStr = utils::formatDateTime(file.datetime, "%c");
        j["date"] = dateStr;

        this->env.render_to(outFile, this->sourceFileTempl, j);

        outFile.close();
        free(dateStr);
    }

}