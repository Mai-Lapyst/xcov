#include "report.h"
#include "highlight/highlight.h"
#include "utils.h"

#include <fstream>
#include <sstream>
#include <iostream>

namespace xcov {

    BranchCoverage::BranchCoverage() {}

    BranchCoverage::BranchCoverage(bool fallthrough, long count, bool throw_flag)
        : fallthrough(fallthrough), count(count), throw_flag(throw_flag)
    {}

    void to_json(nlohmann::json& j, const BranchCoverage& t) {
        j["fallthrough"] = t.fallthrough;
        j["count"] = t.count;
        j["throw"] = t.throw_flag;
    }

    void from_json(const nlohmann::json& j, BranchCoverage& t) {
        j.at("fallthrough").get_to(t.fallthrough);
        j.at("count").get_to(t.count);
        j.at("throw").get_to(t.throw_flag);
    }

    // -------------------------------------------------------------------------------- //

    LineCoverage::LineCoverage() {}

    LineCoverage::LineCoverage(long count, long line_number, bool unexecuted_block, std::string function_name)
        : count(count), line_number(line_number), unexecuted_block(unexecuted_block), function_name(function_name)
    {}

    void LineCoverage::addBranchData(BranchCoverage& newBranch) {
        this->branches.push_back(newBranch);
    }

    void to_json(nlohmann::json& j, const LineCoverage& t) {
        j["branches"] = t.branches;
        j["count"] = t.count;
        j["line_number"] = t.line_number;
        j["unexecuted_block"] = t.unexecuted_block;
        j["function_name"] = t.function_name;
    }

    void from_json(const nlohmann::json& j, LineCoverage& t) {
        j.at("branches").get_to(t.branches);
        j.at("count").get_to(t.count);
        j.at("line_number").get_to(t.line_number);
        j.at("unexecuted_block").get_to(t.unexecuted_block);
        j.at("function_name").get_to(t.function_name);
    }

    // -------------------------------------------------------------------------------- //

    FunctionCoverage::FunctionCoverage() {}

    FunctionCoverage::FunctionCoverage(
        long blocks, long blocks_executed,
        long start_column, long end_column, long start_line, long end_line,
        std::string name, std::string demangled_name, long execution_count
    )
        : blocks(blocks), blocks_executed(blocks_executed), start_column(start_column), end_column(end_column),
            start_line(start_line), end_line(end_line), name(name), demangled_name(demangled_name), execution_count(execution_count)
    {}

    // -------------------------------------------------------------------------------- //

    CoverageStats::CoverageStats(long hits, long total) : hits(hits), total(total) {
        this->recalcCoverage();
    }

    void CoverageStats::recalcCoverage() {
        this->coverage = ( ((double)hits) / ((double)total) ) * 100;
    }

    void CoverageStats::operator+=(const CoverageStats& other) {
        this->hits += other.hits;
        this->total += other.total;
        this->recalcCoverage();
    }

    void to_json(nlohmann::json& j, const CoverageStats& t) {
        j["hits"] = t.hits;
        j["total"] = t.total;
        j["coverage"] = t.coverage;

        if (t.total == 0) {
            j["coverage_level"] = "";
        }
        else {
            j["coverage_level"] = t.coverage >= 90 ? "coverage-high" : (t.coverage >= 75 ? "coverage-mid" : "coverage-low");
        }
    }

    void from_json(const nlohmann::json& j, CoverageStats& t) {
        j.at("hits").get_to(t.hits);
        j.at("total").get_to(t.total);
        j.at("coverage").get_to(t.coverage);
    }

    // -------------------------------------------------------------------------------- //

    void to_json(nlohmann::json& j, const SourceLine& t) {
        j["branches"] = t.branches;
        j["count"] = t.count;
        j["line_number"] = t.line_number;
        j["unexecuted_block"] = t.unexecuted_block;
        j["coverage_present"] = t.coverage_present;
        j["is_excluded"] = t.is_excluded;
        j["source_line"] = t.source_line;

        j["state"] = "";
        if (t.is_excluded) {
            j["state"] = "excluded";
        }
        else if (t.coverage_present) {
            j["state"] = t.count > 0 ? "hit" : "miss";
        }

        int takenBranches = 0;
        for (auto& branch : t.branches) {
            if (branch.count > 0) { takenBranches++; }
        }
        j["takenBranchesCount"] = takenBranches;
    }

    void from_json(const nlohmann::json& j, SourceLine& t) {
        j.at("branches").get_to(t.branches);
        j.at("count").get_to(t.count);
        j.at("line_number").get_to(t.line_number);
        j.at("unexecuted_block").get_to(t.unexecuted_block);
        j.at("coverage_present").get_to(t.coverage_present);
        j.at("source_line").get_to(t.source_line);
    }

    // -------------------------------------------------------------------------------- //

    void SourceFile::applyLineExclusions() {
        std::ifstream srcFile(this->path);
        if (!srcFile) {
            printf("Error: could not open source file %s\n", this->path.c_str());
            return;
        }

        std::string line;
        long line_number = 1;
        bool is_inside_exluded = false;
        while (std::getline(srcFile, line)) {
            if (line.find("LCOV_EXCL_START") != std::string::npos) {
                is_inside_exluded = true;
            }
            if (line.find("LCOV_EXCL_STOP") != std::string::npos) {
                is_inside_exluded = false;
            }

            bool is_line_excluded = is_inside_exluded || (line.find("LCOV_EXCL_LINE") != std::string::npos);
            if (is_line_excluded) {
                std::vector<LineCoverage>::iterator lineCovData = std::find_if(
                    this->lines.begin(), this->lines.end(),
                    [line_number](LineCoverage& entry) {
                        return entry.line_number == line_number;
                    }
                );
                if (lineCovData != this->lines.end()) {
                    // change linestats
                    this->lineStats.total--;
                    if (lineCovData->count > 0) {
                        this->lineStats.hits--;
                    }
                    this->lineStats.recalcCoverage();

                    // change branchstats
                    this->branchStats.total -= lineCovData->branches.size();
                    for (auto& br : lineCovData->branches) {
                        if (br.count > 0) { this->branchStats.hits--; }
                    }
                    this->branchStats.recalcCoverage();

                    this->lines.erase(lineCovData);
                }
            }

            line_number++;
        }

        srcFile.close();
    }

    bool SourceFile::expandLines(std::vector<SourceLine>& sourceLines, Config& conf) {
        std::ifstream srcFile(this->path);
        if (!srcFile) {
            printf("Error: could not open source file %s\n", this->path.c_str());
            return false;
        }

        // TODO: this highlighter currently is mandatory; if an outputhandler dosnt need it (i.e. markdown), this is not very performant...

        SourceLineHighlighter sourcehighlight(conf.outFormat);

        std::stringstream strstream;
        sourcehighlight.highlight(srcFile, strstream, conf.langDef);
        strstream.seekg(0, std::ios::beg);
        strstream.clear();

        std::string line;
        long line_number = 1;
        bool is_inside_exluded = false;
        while (std::getline(strstream, line)) {
            if (line.find("LCOV_EXCL_START") != std::string::npos) {
                is_inside_exluded = true;
            }
            if (line.find("LCOV_EXCL_STOP") != std::string::npos) {
                is_inside_exluded = false;
            }

            bool is_line_excluded = is_inside_exluded || (line.find("LCOV_EXCL_LINE") != std::string::npos);

            std::vector<LineCoverage>::iterator gcovData = std::find_if(
                this->lines.begin(), this->lines.end(),
                [line_number](LineCoverage& entry) {
                    return entry.line_number == line_number;
                }
            );

            if (gcovData != this->lines.end()) {
                sourceLines.push_back(SourceLine( *gcovData, line_number, line, is_line_excluded ));
            }
            else {
                sourceLines.push_back(SourceLine( line_number, line, is_line_excluded ));
            }
            line_number++;
        }

        srcFile.close();
        return true;
    }

    void SourceFile::addLineData(LineCoverage& newLine) {
        auto it = std::find_if(
            this->lines.begin(), this->lines.end(),
            [&newLine](LineCoverage& line) {
                return line.line_number == newLine.line_number;
            }
        );

        if (it == this->lines.end()) {
            // not present
            this->lines.push_back(newLine);
            return;
        }

        std::string demangled_old_name = utils::demangleCpp(it->function_name);
        std::string demangled_new_name = utils::demangleCpp(newLine.function_name);

        // TODO: check first if it really is a c++ symbol (prefix _ZN)
        // TODO: support other name-mangeling formats like D

        if (demangled_old_name != demangled_new_name) {
            // Same line, but another function!
            // This happens when, for example, a lambda is used in certain ways in C++
            // 
            // The current behavior is to increment the line-count and simply copy over the branches
            // to the old line.
            // 
            // TODO: maybe we should restructure this so one line can hold multiple sets of branchdata

            std::cerr << "Warning: " << this->path
                << " got coverage data for two different function on same line: "
                << demangled_old_name << ", " << demangled_new_name
                << std::endl;

            (*it).count += newLine.count;
            for (auto& br : newLine.branches) {
                (*it).branches.push_back(br);
            }
        }
        else {
            (*it).count += newLine.count;

            if (newLine.branches.size() != (*it).branches.size()) {
                throw std::runtime_error("Cannot add linedata with different count of branches");
            } else {
                for (size_t i = 0; i < newLine.branches.size(); i++) {
                    auto& oldBr = (*it).branches.at(i);
                    auto& newBr = newLine.branches.at(i);

                    // TODO: what about the other fields?

                    oldBr.count += newBr.count;
                }
            }
        }
    }

    void SourceFile::addFuncData(FunctionCoverage& newFunc) {
        auto it = std::find_if(
            this->funcs.begin(), this->funcs.end(),
            [&newFunc](FunctionCoverage& func) {
                return (func.start_line == newFunc.start_line)
                    && (func.start_column == newFunc.start_column)
                    && (func.demangled_name == newFunc.demangled_name);
                // we need to use the demangled_name here instead of name
                // since functions can be compiled in twice (from headers)
                // but really mean the same function
            }
        );

        if (it == this->funcs.end()) {
            // not present
            this->funcs.push_back(newFunc);
            // std::cout << "[SourceFile(" << this->file << ")::addFuncData()] adding new function '" << newFunc.demangled_name << "'; new function count: " << this->funcs.size() << std::endl;
        }
        else {
            (*it).execution_count += newFunc.execution_count;
            // std::cout << "[SourceFile(" << this->file << ")::addFuncData()] adding existing function '" << newFunc.demangled_name << "'" << std::endl;
        }
    }

    void SourceFile::recalcStats() {
        long branchCount = 0, branchHits = 0;

        this->lineStats = CoverageStats(0, this->lines.size());
        for (LineCoverage& line : this->lines) {
            if (line.count > 0) {
                this->lineStats.hits++;
            }

            branchCount += line.branches.size();
            for (BranchCoverage& branch : line.branches) {
                if (branch.count > 0) {
                    branchHits++;
                }
            }
        }
        this->lineStats.recalcCoverage();

        this->branchStats = CoverageStats(branchHits, branchCount);

        this->funcStats = CoverageStats(0, this->funcs.size());
        for (FunctionCoverage& func : this->funcs) {
            if (func.execution_count > 0) {
                this->funcStats.hits++;
            }
        }
        this->funcStats.recalcCoverage();
    }

    void to_json(nlohmann::json& j, const SourceFile& t) {
        j["lines"] = t.lines;
        j["functions"] = t.funcs;
        j["path"] = t.path;
        j["lineStats"] = t.lineStats;
        j["funcStats"] = t.funcStats;
        j["branchStats"] = t.branchStats;
    }

    void from_json(const nlohmann::json& j, SourceFile& t) {
        j.at("lines").get_to(t.lines);
        j.at("functions").get_to(t.funcs);
        j.at("path").get_to(t.path);
        j.at("lineStats").get_to(t.lineStats);
        j.at("funcStats").get_to(t.funcStats);
        j.at("branchStats").get_to(t.branchStats);
    }

    // -------------------------------------------------------------------------------- //

    void Report::recalcStats() {
        this->lineStats = CoverageStats();
        this->funcStats = CoverageStats();
        this->branchStats = CoverageStats();

        for (SourceFile& srcFile : this->sourceFiles) {
            srcFile.recalcStats();
            srcFile.applyLineExclusions();
            this->lineStats += srcFile.lineStats;
            this->funcStats += srcFile.funcStats;
            this->branchStats += srcFile.branchStats;
        }
    }

    void Report::sortFiles() {
        std::sort(
            this->sourceFiles.begin(), this->sourceFiles.end(),
            [] (const SourceFile& a, const SourceFile& b) {
                return a.path < b.path;
            }
        );
    }

    SourceFile& Report::findOrCreate(std::string path) {
        for (SourceFile& srcFile : this->sourceFiles) {
            if (srcFile.path == path) {
                return srcFile;
            }
        }

        // std::cout << "[Report] created new sourcefile: " << file << std::endl;

        SourceFile srcFile = SourceFile();
        srcFile.path = path;
        srcFile.datetime = this->datetime;
        this->sourceFiles.push_back(srcFile);
        return this->sourceFiles.back();
    }

    void to_json(nlohmann::json& j, const Report& t) {
        j["date"] = t.datetime;
        j["lineStats"] = t.lineStats;
        j["funcStats"] = t.funcStats;
        j["branchStats"] = t.branchStats;
        j["files"] = t.sourceFiles;
    }

    void from_json(const nlohmann::json& j, Report& t) {
        j.at("date").get_to(t.datetime);
        j.at("lineStats").get_to(t.lineStats);
        j.at("funcStats").get_to(t.funcStats);
        j.at("branchStats").get_to(t.branchStats);
        j.at("files").get_to(t.sourceFiles);
    }

    // -------------------------------------------------------------------------------- //

    Reporter::Reporter(Config& conf) {
        this->nameingStrategy = conf.nameingStrategy;
    }

    fs::path Reporter::generateFilename(fs::path name, std::string suffix) {
        switch (this->nameingStrategy) {
            case FNS_HASHSUFFIX: {
                size_t hashValue = std::hash<std::string>()(name.string());
                std::stringstream stream;
                stream << std::setfill('0') << std::setw(sizeof(size_t)*2) << std::hex << hashValue;
                return name.filename().string() + '_' + stream.str() + suffix;
            }
            case FNS_PATHPREFIX: {
                std::string s = name.string();
                std::replace(s.begin(), s.end(), '/', '.');
                return s + suffix;
            }
            case FNS_PATH: {
                return name.string() + suffix;
            }
            default:
                throw std::runtime_error("Invalid nameing strategy: " + this->nameingStrategy);
        }
    }

}