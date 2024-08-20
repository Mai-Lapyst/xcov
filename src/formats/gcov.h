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
/**
 * @brief contains objects for deserializing gcov's json format
 */

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "report.h"
#include "config.h"

namespace xcov {

    void loadGcovData(Report& report, Config& conf);

    class GCovBranch {
    public:
        bool fallthrough;
        long count;
        bool throw_flag;

        friend void to_json(nlohmann::json& j, const GCovBranch& t);
        friend void from_json(const nlohmann::json& j, GCovBranch& t);
    };

    class GCovLine {
    public:
        std::vector<GCovBranch> branches;
        long count;
        long line_number;
        bool unexecuted_block;
        std::string function_name;

        friend void to_json(nlohmann::json& j, const GCovLine& t);
        friend void from_json(const nlohmann::json& j, GCovLine& t);
    };

    class GCovFunction {
    public:
        long blocks;
        long end_column;
        long start_line;
        std::string name;
        long blocks_executed;
        long execution_count;
        std::string demangled_name;
        long start_column;
        long end_line;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(
            GCovFunction,
            blocks, end_column, start_line, name, blocks_executed, execution_count,
            demangled_name, start_column, end_line
        );
    };

    class GCovSourceFile {
    public:
        std::vector<GCovLine> lines;
        std::vector<GCovFunction> functions;
        std::string file;

        friend void to_json(nlohmann::json& j, const GCovSourceFile& t);
        friend void from_json(const nlohmann::json& j, GCovSourceFile& t);
    };

    class GCovFile {
    public:
        std::string gcc_version;
        std::vector<GCovSourceFile> files;
        std::string format_version;
        std::string current_working_directory;
        std::string data_file;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(
            GCovFile,
            gcc_version, files, format_version,
            current_working_directory, data_file
        );
    };

}