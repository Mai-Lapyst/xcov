#include "formats/gcov.h"
#include "highlight/highlight.h"
#include "utils.h"

#include <ftw.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

namespace xcov {

    std::vector<fs::path> gcda_files;
    std::vector<fs::path> gcda_stems;
    std::vector<fs::path> gcno_files;
    std::vector<fs::path> gcov_file_list;
    std::vector<fs::path> gGcovOutputFiles;

    int findGcovSources(const char* fpath, const struct stat* sb, int typeflags, struct FTW* ftwbuf) {
        switch (typeflags) {
            // file (0)
            case FTW_F: {
                fs::path path = fpath;
                path = fs::absolute(path);

                if (path.extension() == ".gcda") {
                    gcda_files.push_back(path);
                    gcda_stems.push_back(path.stem());
                }
                else if (path.extension() == ".gcno") {
                    gcno_files.push_back(path);
                }
                return FTW_CONTINUE;
            }

            case FTW_D:     // dir (1)
                return 0;

            case FTW_DNR:   // directory which can't be read (2)
                return FTW_SKIP_SUBTREE;

            case FTW_NS:    // stat failed on fpath (3)
                return FTW_SKIP_SUBTREE;

            case FTW_SL:    // symbolic link (4)
                return 0;

            case FTW_SLN:   // symbolic link to nonexisting file
                return 0;

            default:
                return FTW_CONTINUE;
        }
    }

    void loadGcovData(Report& report, Config& conf) {
        bool keep_gcov = false;

        std::cout << "[loadGcovData] Begin searching inside: " << conf.rootDir.string() << std::endl;
        int r = nftw(conf.rootDir.c_str(), findGcovSources, 20, FTW_ACTIONRETVAL);

        std::cout << "[loadGcovData] Done searching; found: " << gcda_files.size() << " gcda files and " << gcno_files.size() << " gcno files" << std::endl;

        // copy all gcda files into our filelist!
        gcov_file_list = gcda_files;
        gcda_files.clear(); // safe memory!

        if (gcov_file_list.size() < gcno_files.size()) {
            // only check which gcno files we need if we really need them!

            for (fs::path gcno_file : gcno_files) {
                if (std::find(gcda_stems.begin(), gcda_stems.end(), gcno_file.stem()) != gcda_stems.end()) {
                    // found the gcno stem inside our gcda stem cache... we dont need it!
                    continue;
                }
                else {
                    gcov_file_list.push_back(gcno_file);
                }
            }

            std::cout << "[loadGcovData] Done diffing gcda and gcno stems; now using " << gcov_file_list.size() << " files in total" << std::endl;
        }

        gcno_files.clear();
        gcda_stems.clear();

        auto cache_path = fs::current_path() / ".xcov_cache";
        if (fs::exists(cache_path)) {
            // delete cache if already there...
            fs::remove_all(cache_path);
        }

        fs::create_directories(cache_path);

        for (fs::path path : gcov_file_list) {

            // gcov produces an file named <input>.gcov.json.gz; move it to our cache...
            fs::path output_file = path.filename().string() + ".gcov.json.gz";

            if (!keep_gcov) {
                // TODO: we should probably feature test these flags...
                std::vector<std::string> args = {
                    fs::absolute(path).string(),
                    "--branch-count",               // output counts for branching instead of procents
                    "--branch-probabilities",
                    "--demangled-names",            // demangle the names for us
                    "--object-directory", fs::absolute(path).parent_path().string(),
                    "--json-format",                // format as json; this also gives us an .json.gz file
                    "--relative-only"               // skips system files
                };

                utils::exec_process("gcov", args, conf.verboseLevel >= 2);
                fs::rename(fs::current_path() / output_file, cache_path / output_file);
            }

            gGcovOutputFiles.push_back( cache_path / output_file );
        }

        // fill in the report...
        for (fs::path gcovOutFile : gGcovOutputFiles) {

            // file: <gcno/gcda>.gcov.json
            fs::path jsonFilepath = gcovOutFile.replace_extension();

            if (!keep_gcov) {
                // call gzip to unzip this mess...
                std::vector<std::string> gzip_args = {
                    "--keep", "--force", "--uncompress", gcovOutFile,
                };
                utils::exec_process("gzip", gzip_args, conf.verboseLevel >= 2);
            }

            nlohmann::json j;
            std::ifstream jsonFile(jsonFilepath);
            if (!jsonFile) {
                printf("Could not open file: %s\n", jsonFilepath.c_str());
                continue;
            }

            jsonFile >> j;
            jsonFile.close();

            GCovFile gCovFile = j;
            for (GCovSourceFile& gcovSrcFile : gCovFile.files) {
                if (gcovSrcFile.lines.size() == 0 && gcovSrcFile.functions.size() == 0) {
                    continue;
                }

                SourceFile& srcFile = report.findOrCreate(gcovSrcFile.file);

                for (GCovLine& line : gcovSrcFile.lines) {
                    LineCoverage data = LineCoverage(line.count, line.line_number, line.unexecuted_block, line.function_name);
                    // TODO: add branch data
                    srcFile.addLineData(data);
                }

                for (GCovFunction& func : gcovSrcFile.functions) {
                    FunctionCoverage data = FunctionCoverage(
                        func.blocks, func.blocks_executed,
                        func.start_column, func.end_column,
                        func.start_line, func.end_line,
                        func.name, func.demangled_name,
                        func.execution_count
                    );
                    srcFile.addFuncData(data);
                }

                //report.addSourceFile(srcFile);
            }
        }

    }

    // -------------------------------------------------------------------------------- //

    void to_json(nlohmann::json& j, const GCovBranch& t) {
        j["fallthrough"] = t.fallthrough;
        j["count"] = t.count;
        j["throw"] = t.throw_flag;
    }

    void from_json(const nlohmann::json& j, GCovBranch& t) {
        j.at("fallthrough").get_to(t.fallthrough);
        j.at("count").get_to(t.count);
        j.at("throw").get_to(t.throw_flag);
    }

    // -------------------------------------------------------------------------------- //

    void to_json(nlohmann::json& j, const GCovLine& t) {
        j["branches"] = t.branches;
        j["count"] = t.count;
        j["line_number"] = t.line_number;
        j["unexecuted_block"] = t.unexecuted_block;
        j["function_name"] = t.function_name;
    }

    void from_json(const nlohmann::json& j, GCovLine& t) {
        j.at("branches").get_to(t.branches);
        j.at("count").get_to(t.count);
        j.at("line_number").get_to(t.line_number);
        j.at("unexecuted_block").get_to(t.unexecuted_block);

        // Sometimes an line can have no function_name key; this crashes then the program...
        if (j.contains("function_name")) {
            j.at("function_name").get_to(t.function_name);
        }
    }

    // -------------------------------------------------------------------------------- //

    void to_json(nlohmann::json& j, const GCovSourceFile& t) {
        j["lines"] = t.lines;
        j["functions"] = t.functions;
        j["file"] = t.file;
    }

    void from_json(const nlohmann::json& j, GCovSourceFile& t) {
        j.at("file").get_to(t.file);
        j.at("lines").get_to(t.lines);
        j.at("functions").get_to(t.functions);
    }

}