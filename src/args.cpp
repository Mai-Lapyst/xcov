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

#include "args.h"
#include <iostream>

namespace xcov {

    CliArgs gCliArgs;

    #define VERSION_STRING      "v1.1.0"

    #define TITLE_OPT_ID        1000
    #define CONFIG_OPT_ID       1001
    #define FILENAMEING_OPT_ID  1002
    #define LANGDEF_OPT_ID      1003
    #define OUTFORMAT_OPT_ID    1004
    #define REPORTFORMAT_OPT_ID 1005

    struct option long_options[] = {
        { "help", no_argument, NULL, 'h' },
        { "version", no_argument, NULL, 'V' },
        { "verbose", no_argument, NULL, 'v' },
        { "output", required_argument, NULL, 'o' },
        { "root", required_argument, NULL, 'r' },
        { "title", required_argument, NULL, TITLE_OPT_ID },
        { "template", required_argument, NULL, 't' },
        { "config", required_argument, NULL, CONFIG_OPT_ID },
        { "filenameing", required_argument, NULL, FILENAMEING_OPT_ID },

        { "lang-def", required_argument, NULL, LANGDEF_OPT_ID },
        { "out-format", required_argument, NULL, OUTFORMAT_OPT_ID },

        { "format", required_argument, NULL, REPORTFORMAT_OPT_ID },

        { "keep-gcov", no_argument, (int*)&(gCliArgs.keep_gcov), 1 },
        {0, 0, 0, 0}
    };

    void printUsage() {
        puts("Usage: xcov [OPTIONS]");
        puts("Available options:");
        puts(" -v, --verbose        Produce more output (if set before other options this even logs them!)");
        puts(" -V, --version        Prints out version");
        puts(" -o, --output DIR     Sets the directory to output to");
        puts(" -r, --root DIR       Searches after gcov files in the specified dir rather than from pwd");
        puts(" --title TITLE        Sets the title for the output");
        puts(" -t, --template       Sets the template/theme path to use");
        puts(" --config FILE        Specifies an config file to load");
        puts(" --filenameing        The mode how reportfiles should be named. (default: path)");
        puts("                       Available:");
        puts("                        - hash       : adds an hashvalue to the end of the filename");
        puts("                        - pathprefix : adds an prefix to the filename thats based on the original path (. instead of /)");
        puts("                                        i.e.: the file 'src/main.cpp' will become '<output dir>/src.main.cpp.html'");
        puts("                        - path       : just creates the folders of the old path relative to the new path.");
        puts("                                        i.e.: if your file was 'src/main.cpp' the output is written to '<output dir>/src/main.cpp.html'");
        puts(" --format FORMAT      Sets the report format to use.");
        puts("                       Available:");
        puts("                         - html : Prints a html report to <reportDir>");

        puts("Source-highlight options:");
        puts(" --lang-def FILE      Specifies an definition file to use for the highlighting (default: cpp)");
        puts(" --out-format FILE    Use the given file to generate the output then used inside the report (default: html)");
    }

    void printVersion() {
        puts("xcov version " VERSION_STRING);
    }

    void parseArguments(int argc, char** argv) {
        while (1) {
            int option_index = 0;
            int c = getopt_long(argc, argv, "hVvo:r:t:", long_options, &option_index);
            if (c == -1) {
                break;
            }

            switch (c) {
                case 0:
                    // option has set an flag
                    break;

                case 'h': {
                    printUsage();
                    exit(1);
                }

                case 'V': {
                    printVersion();
                    exit(1);
                }

                case 'v': {
                    gCliArgs.verbose_flag = true;
                    gCliArgs.verbose_level++;
                    break;
                }

                case 'o': {
                    gCliArgs.hasOutputDir = true;
                    gCliArgs.outputDir = fs::absolute(optarg);
                    PRINT_VERBOSE("Setting output folder: %s\n", optarg);
                    break;
                }

                case 'r': {
                    gCliArgs.hasRootDir = true;
                    gCliArgs.rootDir = optarg;
                    if (!fs::exists(gCliArgs.rootDir) || !fs::is_directory(gCliArgs.rootDir)) {
                        fprintf(stderr, "Error: cannot use non-existing or non-directory path %s as root directory!\n", gCliArgs.rootDir.c_str());
                        exit(1);
                    }
                    PRINT_VERBOSE("Setting root folder: %s\n", fs::absolute(gCliArgs.rootDir).c_str());
                    break;
                }

                case TITLE_OPT_ID: {
                    gCliArgs.hasTitle = true;
                    gCliArgs.title = optarg;
                    PRINT_VERBOSE("Setting title: %s\n", optarg);
                    break;
                }

                case 't': {
                    gCliArgs.hasTemplateDir = true;
                    gCliArgs.templateDir = fs::absolute(optarg);
                    if (!fs::exists(gCliArgs.templateDir) || !fs::is_directory(gCliArgs.templateDir)) {
                        fprintf(stderr, "Error: cannot use non-existing or non-directory path %s as template!\n", gCliArgs.templateDir.c_str());
                        exit(1);
                    }
                    break;
                }

                case CONFIG_OPT_ID: {
                    gCliArgs.hasConfigFile = true;
                    gCliArgs.configFile = optarg;
                    if (!fs::exists(gCliArgs.configFile) || fs::is_directory(gCliArgs.configFile)) {
                        fprintf(stderr, "Error: cannot use non-existing path or an directory %s as config file!\n", gCliArgs.configFile.c_str());
                        exit(1);
                    }
                    break;
                }

                case FILENAMEING_OPT_ID: {
                    std::string val = optarg;
                    gCliArgs.hasNameingStrategy = true;
                    if (val == "hash") {
                        gCliArgs.fileNameingStrategy = FNS_HASHSUFFIX;
                    }
                    else if (val == "pathprefix") {
                        gCliArgs.fileNameingStrategy = FNS_PATHPREFIX;
                    }
                    else if (val == "path") {
                        gCliArgs.fileNameingStrategy = FNS_PATH;
                    }
                    else {
                        std::cout << "Error: invalid filenameing strategy: " << val << std::endl;
                        exit(1);
                    }
                    break;
                }

                case LANGDEF_OPT_ID: {
                    gCliArgs.hasLangDef = true;
                    gCliArgs.langDef = optarg;
                    PRINT_VERBOSE("Setting language definition file for source-highlight: %s\n", optarg);
                    break;
                }

                case OUTFORMAT_OPT_ID: {
                    gCliArgs.hasOutFormat = true;
                    gCliArgs.outFormat = optarg;
                    PRINT_VERBOSE("Setting output definition file for source-highlight: %s\n", optarg);
                    break;
                }

                case REPORTFORMAT_OPT_ID: {
                    gCliArgs.hasReportFormat = true;
                    gCliArgs.reportFormat = optarg;
                    PRINT_VERBOSE("Setting report format: %s\n", optarg);
                    break;
                }

                case '?':
                    break;
            
                default:
                    PRINT_VERBOSE("Got unknown return from getopt_long(): %d\n", c);
                    exit(1);
            }
        }

    }

}