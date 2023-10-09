#pragma once

#include <filesystem>
#include <regex>

#ifndef DEFAULT_TEMPL_PATH
    #warning "No DEFAULT_TEMPL_PATH defined, assuming development-environment ($pwd/xcov_data/themes/default)"
    #define DEFAULT_TEMPL_PATH \
        (fs::current_path() / "xcov_data/themes/default")
#endif

namespace xcov {

    namespace fs = std::filesystem;

    enum FileNameingStrategy {
        FNS_HASHSUFFIX,
        FNS_PATHPREFIX,
        FNS_PATH
    };

    class Config {
    public:
        // dir to search from, default is the pwd
        fs::path rootDir = fs::current_path();

        // dir to save the reports in; needs to be specified explictily
        fs::path reportDir;

        // title for the report
        std::string title = "Code Coverage Report";

        // template to use; default should be provided through the system's config or compile time options
        fs::path templatePath = DEFAULT_TEMPL_PATH;

        // how reportfiles should be named
        FileNameingStrategy nameingStrategy = FNS_PATH;

        bool logVerbose = false;
        int verboseLevel = 0;

        // ----------------------------------------
        //      Source filters
        // ----------------------------------------

        std::vector<std::regex> source_excludes;

        // ----------------------------------------
        //      GNU source-highlight
        // ----------------------------------------

        // language definition file to use; can also be a filename from your
        // GNU source-highlight data folder (given that the datadir is configured right)
        std::string langDef = "cpp.lang";

        // the definition file for GNU source-highlight that defines what to generate
        std::string outFormat = "html.outlang";

        // the datadir for GNU source-highlight; if an empty string,
        // let source-highlight figure it out for you
        std::string sh_dataDir = "";

        // ----------------------------------------

        /**
         * @brief Loads an configuration from a specified file
         * NOTE: calling loadFrom() multiple times causes values from previous calls to be overriden;
         * but only those that are actually defined inside the file currently beeing loaded. This way we
         * can implement system, user and folder configurations.
         * 
         * @param path the path to load from
         */
        void loadFrom(fs::path path);

        /**
         * @brief Checks wether or not a given sourcefile is excluded
         * 
         * @param source_file the sourcefile to test
         * @return true if the file is excluded
         * @return false otherwise
         */
        bool isExcludedSource(std::string source_file);

    };

    void loadConfig(Config& conf);

}