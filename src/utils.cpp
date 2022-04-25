#include "utils.h"
#include <memory>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <time.h>

#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

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

    }
}