#pragma once

#include <sstream>
#include <ostream>
#include <vector>
#include <string>
#include <filesystem>

namespace xcov {
    namespace utils {

        /**
         * @brief makes an single string sutiable for std::system() or similar out ot the cmd and its arguments
         * Used for logging
         * 
         * @param cmd 
         * @param arguments 
         * @return std::string 
         */
        std::string makeCmd(std::string cmd, std::vector<std::string>& arguments);

        /**
         * @brief executes an process named by <cmd>; uses internally boost::process::search_path() to find the correct binary
         * and executes it then via boost::process::child
         * 
         * NOTE: this method calls std::exit() when the process invoked fails
         * 
         * @param cmd the command to run
         * @param arguments the arguments to the given command
         * @param verbose if true, we log the command invokation & output; if false we suppress them
         */
        void exec_process(std::string cmd, std::vector<std::string>& arguments, bool verbose);

        /**
         * @brief returns the givem datetime formated by the given format
         * 
         * @param time the time to format
         * @param format the format as for strftime (see https://www.cplusplus.com/reference/ctime/strftime/)
         * @return char* an string that holds the current datetime; owned by caller
         */
        char* formatDateTime(time_t time, const char* format);

        /**
         * @brief Demangles a symbol via the c++ abi
         * 
         * @param mangledName the mangled name
         * @return std::string the demangled name
         */
        std::string demangleCpp(std::string mangledName);

    }
}