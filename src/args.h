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

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <filesystem>
#include "config.h"

namespace xcov {
    namespace fs = std::filesystem;

    struct CliArgs {
        int verbose_level = 0; // 0 = normal; 1 = "debug"; 2 = show output of called binaries
        bool verbose_flag = false;

        bool hasOutputDir = false; fs::path outputDir;
        bool hasRootDir = false; fs::path rootDir;
        bool hasTitle = false; std::string title;
        bool hasTemplateDir = false; fs::path templateDir;
        bool hasConfigFile = false; fs::path configFile;

        bool hasNameingStrategy;
        FileNameingStrategy fileNameingStrategy = FNS_PATH;

        bool hasLangDef = false; std::string langDef;
        bool hasOutFormat = false; std::string outFormat;
        bool hasReportFormat = false; std::string reportFormat;

        bool keep_gcov = false;
    };
    extern CliArgs gCliArgs;
    extern std::vector<fs::path> gGcovOutputFiles;

    #define PRINT_VERBOSE(format, ...) \
        if (gCliArgs.verbose_flag) { printf(format, ## __VA_ARGS__); }

    extern struct option long_options[];

    void printUsage();

    void parseArguments(int argc, char** argv);

}