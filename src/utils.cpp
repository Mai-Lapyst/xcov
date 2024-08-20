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

#include "utils.h"
#include <memory>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <time.h>

#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#if defined(__GNUG__) || defined(__clang__)
    #include <cxxabi.h>
#else
    #error "Platform not fully supported; missing symbol demangling support!"
#endif

namespace xcov {
    namespace utils {

        std::string makeCmd(std::string cmd, std::vector<std::string>& arguments) {
            std::string fullCmd = "";
            fullCmd += cmd;
            for (size_t i = 0; i < arguments.size(); i++) {
                fullCmd += " ";
                fullCmd += arguments.at(i);
            }
            return fullCmd;
        }

        void exec_process(std::string cmd, std::vector<std::string>& arguments, bool verbose) {
            namespace bp = boost::process;
            namespace asio = boost::asio;

            asio::io_service ios;
            std::future<std::string> data;
            std::error_code ec;
            bp::child c(
                bp::search_path(cmd), arguments,
                bp::std_in.close(),
                bp::std_out > data,
                bp::std_err > data,
                ios, ec
            );

            ios.run();

            if (ec) {
                std::cerr << "Error: " << cmd << " failed with errorcode " << ec << std::endl;
                std::string str = data.get();
                boost::trim(str);
                if (!str.empty()) {
                    std::cerr << str;
                }
                exit(EXIT_FAILURE);
            }

            if (verbose) {
                std::cout << "[Info] $ " << utils::makeCmd(cmd, arguments) << std::endl;
                std::string str = data.get();
                boost::trim(str);
                if (!str.empty()) {
                    std::cout << "\033[0;35m" << str << "\033[0m" << std::endl;
                }
            }
        }

        char* formatDateTime(time_t time, const char* format) {
            struct tm* tstruct;
            tstruct = localtime(&time);

            int len = 64; // pre-allocate 64 bytes for the string
            char* buf = (char*) calloc(sizeof(char), len);
            while (true) {
                int r = strftime(buf, len, format, tstruct);
                if (r == 0) {
                    len += 32;  // try in 32 bytes steps
                    buf = (char*) realloc(buf, len);
                    continue;
                }
                else {
                    break;
                }
            }
            return buf;
        }

        std::string demangleCpp(std::string mangledName) {
            int status;
            std::unique_ptr<char[], void (*)(void *)> result{
                abi::__cxa_demangle(mangledName.data(), NULL, NULL, &status), std::free};
            return (status == 0) ? result.get() : mangledName;
        }
    }
}