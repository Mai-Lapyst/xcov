#pragma once

#include <vector>
#include "nlohmann/json.hpp"
#include "config.h"

namespace xcov {

    class BranchCoverage {
    public:
        bool fallthrough;
        long count;
        bool throw_flag;

        friend void to_json(nlohmann::json& j, const BranchCoverage& t);
        friend void from_json(const nlohmann::json& j, BranchCoverage& t);
    };

    /**
     * @brief Represents one sourceline that we have coverage data about
     */
    class LineCoverage {
    public:
        std::vector<BranchCoverage> branches;

        // count of how many times this line was triggered; a count of <=0 means it wasn't triggered at all
        long count;

        // the linenumber of the sourcefile; starts at 1
        long line_number;

        bool unexecuted_block;

        // function this line is contained in; usually the mangled name
        std::string function_name;

        LineCoverage();
        LineCoverage(long count, long line_number, bool unexecuted_block, std::string function_name);

        friend void to_json(nlohmann::json& j, const LineCoverage& t);
        friend void from_json(const nlohmann::json& j, LineCoverage& t);
    };

    /**
     * @brief Represents one function that we have coverage data about
     */
    class FunctionCoverage {
    public:
        long blocks, blocks_executed;

        // position in the file
        long start_column, end_column;
        long start_line, end_line;

        // name (mangled), and demangled name of the function
        std::string name, demangled_name;

        // how many times the function was called; an count of <=0 means that the function wasn't called at all
        long execution_count;

        FunctionCoverage();
        FunctionCoverage(
            long blocks, long blocks_executed,
            long start_column, long end_column, long start_line, long end_line,
            std::string name, std::string demangled_name, long execution_count
        );

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(
            FunctionCoverage,
            blocks, end_column, start_line, name, blocks_executed, execution_count,
            demangled_name, start_column, end_line
        );
    };

    /**
     * @brief represents the coverage statistics; used by both Report and Sourcefile to condense the contained data
     */
    class CoverageStats {
    public:
        long hits, total;
        double coverage;

        CoverageStats() : CoverageStats(0, 0) {};
        CoverageStats(long hits, long total);

        /**
         * @brief recalculates the coverate rate
         */
        void recalcCoverage();

        void operator+=(const CoverageStats& other);
    
        friend void to_json(nlohmann::json& j, const CoverageStats& t);
        friend void from_json(const nlohmann::json& j, CoverageStats& t);
    };

    /**
     * @brief Represents one source line; used when we expand the line data to generate an source listing
     */
    class SourceLine {
    public:
        // These fields contain data copied from the corresponding LineCoverage object
        std::vector<BranchCoverage> branches;
        long count = 0;
        long line_number;
        bool unexecuted_block = true;

        // if this field is true, the fields above contain valid data; if not, the above fields should NOT be read
        bool coverage_present;

        // the (highlighted) source line
        std::string source_line;

        SourceLine(LineCoverage coverageData, long line_number, std::string source_line)
            : branches(coverageData.branches), count(coverageData.count),
                line_number(line_number), unexecuted_block(coverageData.unexecuted_block),
                coverage_present(true), source_line(source_line)
        {}

        SourceLine(long line_number, std::string source_line)
            : line_number(line_number), coverage_present(false), source_line(source_line)
        {}

        friend void to_json(nlohmann::json& j, const SourceLine& t);
        friend void from_json(const nlohmann::json& j, SourceLine& t);
    };

    /**
     * @brief Represents one source file that we have data about
     */
    class SourceFile {
    public:
        // relative path of the file
        std::string path;

        // time the report was generated; usually the same date as the report
        time_t datetime;

        std::vector<LineCoverage> lines;
        std::vector<FunctionCoverage> funcs;

        CoverageStats lineStats, funcStats, branchStats;

        bool expandLines(std::vector<SourceLine>& sourceLines, Config& conf);

        void addLineData(LineCoverage& newLine);
        void addFuncData(FunctionCoverage& newFunc);

        void recalcStats();

        friend void to_json(nlohmann::json& j, const SourceFile& t);
        friend void from_json(const nlohmann::json& j, SourceFile& t);
    };

    /**
     * @brief Represents an report to be generated
     */
    class Report {
    public:
        time_t datetime;
        CoverageStats lineStats, funcStats, branchStats;

        std::vector<SourceFile> sourceFiles;

        SourceFile& findOrCreate(std::string file);

        void recalcStats();

        friend void to_json(nlohmann::json& j, const Report& t);
        friend void from_json(const nlohmann::json& j, Report& t);
    };

    /**
     * @brief Object that handles the report generation
     */
    class Reporter {
    protected:
        FileNameingStrategy nameingStrategy;

        fs::path generateFilename(fs::path name, std::string suffix);

    public:
        Reporter(Config& conf);

        virtual ~Reporter() {};

        virtual void generateReport(Report& report) = 0;
    };

}