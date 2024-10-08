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

#include "config.h"

#include <fstream>
#include <stdexcept>
#include <regex>
#include <iostream>
#include <stack>

namespace xcov {

    std::regex sectionRegex("^\\[([\\w]+)\\]$");
    std::regex keyRegex("^([\\w]+)\\s*=\\s*\"([^\"]*)\"\\s*($|#)");

    enum ConfigSection {
        SEC_NULL,           // state BEFORE we have any section
        SEC_INVALID,        // state when we have encountered an non-existing section

        // normal sections wich we can have keys for
        SEC_MAIN,
        SEC_HIGHLIGHT,
        SEC_SOURCES,
    };

    void Config::loadFrom(fs::path path) {

        std::ifstream file(path);
        if (!file) {
            throw std::runtime_error("could not load configuration file: " + path.string());
        }

        ConfigSection section = SEC_INVALID;
        std::string line;
        while (std::getline(file, line)) {

            if (line.empty()) {
                continue;
            }
            else if (line.at(0) == '#') {
                // comment line
                continue;
            }

            std::smatch match;
            if ( std::regex_match(line, match, sectionRegex) ) {
                // we change section

                std::string sectionName = match[1];
                if (sectionName == "main") {
                    section = SEC_MAIN;
                }
                else if (sectionName == "source_highlight") {
                    section = SEC_HIGHLIGHT;
                }
                else if (sectionName == "sources") {
                    section = SEC_SOURCES;
                }
                else {
                    std::cerr << "[Config " << path.string() << "] Warning: unknown section '" << sectionName << "'; keys in it will be ignored" << std::endl;
                }
                continue;
            }

            if ( std::regex_match(line, match, keyRegex) ) {
                // we have an key

                std::string key = match[1];
                std::string val = match[2];

                #define CONFIG_KEY(keyname) \
                    if (key == #keyname) { this->keyname = val; continue; }

                #define XCONFIG_KEY(keyname, fieldname) \
                    if (key == keyname) { this->fieldname = val; continue; }

                switch (section) {
                    case SEC_NULL:
                        std::cerr << "[Config " << path.string() << "] Warning: found key before any section: '" << line << "'; key will be ignored" << std::endl;
                        break;
                    case SEC_INVALID:
                        break;
                    
                    case SEC_MAIN: {
                        CONFIG_KEY(rootDir);
                        CONFIG_KEY(reportDir);
                        CONFIG_KEY(reportPath);
                        CONFIG_KEY(reportFormat);
                        CONFIG_KEY(title);
                        CONFIG_KEY(templatePath);
                        if (key == "nameingStrategy") {
                            if (val == "hash") {
                                this->nameingStrategy = FNS_HASHSUFFIX;
                            }
                            else if (val == "pathprefix") {
                                this->nameingStrategy = FNS_PATHPREFIX;
                            }
                            else if (val == "path") {
                                this->nameingStrategy = FNS_PATH;
                            }
                            else {
                                std::cerr << "[Config " << path.string() << "] Warning: invalid value '" << val << "' for key nameingStrategy" << std::endl;
                            }
                            continue;
                        }

                        std::cerr << "[Config " << path.string() << "] Warning: unknown key '" << key << "' for section 'main'" << std::endl;
                        break;
                    }

                    case SEC_HIGHLIGHT: {
                        CONFIG_KEY(langDef);
                        CONFIG_KEY(outFormat);
                        XCONFIG_KEY("dataDir", sh_dataDir);

                        std::cerr << "[Config " << path.string() << "] Warning: unknown key '" << key << "' for section 'source_highlight'" << std::endl;
                        break;
                    }

                    case SEC_SOURCES: {
                        if (key == "exclude") {
                            this->source_excludes.push_back( std::regex(val, std::regex_constants::ECMAScript) );
                            continue;
                        }

                        std::cerr << "[Config " << path.string() << "] Warning: unknown key '" << key << "' for section 'sources'" << std::endl;
                        break;
                    }
                }

                #undef CONFIG_KEY
                #undef XCONFIG_KEY
                continue;
            }

            std::cerr << "[Config " << path.string() << "] Warning: could not parse line: '" << line << "'" << std::endl;
        }

        file.close();
    }

    bool Config::isExcludedSource(std::string source_file) {
        for (auto& regex : this->source_excludes) {
            std::smatch match;
            if (std::regex_search(source_file, match, regex)) {
                return true;
            }
        }
        return false;
    }

    void loadConfig(Config& conf) {
        // first apply global options
        fs::path globalConfPath = "/usr/local/etc/xcov/default.cfg";
        if (fs::exists(globalConfPath)) {
            std::cout << "[Config] Trying to load global config from " << globalConfPath << std::endl;
            conf.loadFrom(globalConfPath);
        }

        // then apply the user's options
        char* homePath = getenv("HOME");
        if (homePath != NULL) {
            fs::path userConfPath = fs::path(homePath) / ".config/xcov/user.cfg";
            if (fs::exists(userConfPath)) {
                std::cout << "[Config] Trying to load user config from " << userConfPath << std::endl;
                conf.loadFrom(userConfPath);
            }
        }

        // now go through all folders from root (/) to the current directory we are in
        // and look for a "xcov.cfg" file; if any present, apply it

        fs::path pwd = fs::current_path();
        std::stack<fs::path> dirPaths;
        while (true) {
            fs::path filePath = pwd / "xcov.cfg";
            if (fs::exists(filePath)) {
                dirPaths.push(filePath);
            }

            if (pwd.root_path() == pwd) {
                break; // we are root
            }
            pwd = pwd.parent_path();
        }

        while (!dirPaths.empty()) {
            fs::path filePath = dirPaths.top();
            dirPaths.pop();

            std::cout << "[Config] Trying to load folder config from " << filePath << std::endl;
            conf.loadFrom(filePath);
        }
    }

}