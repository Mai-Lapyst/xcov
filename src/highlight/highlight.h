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

#include <string>
#include <srchilite/textstyleformattercollection.h>

namespace srchilite {
    class FormatterManager;
    class PreFormatter;
    class LangDefManager;
    class BufferedOutput;
    class LineNumGenerator;
    // class DocGenerator;
    class CharTranslator;
    class HighlightEventListener;
    class CTagsManager;
    class CTagsFormatter;
    class LineRanges;
    class RegexRanges;
}

namespace xcov {

    using namespace srchilite;

    /**
     * @brief This class wraps gnu source-highlight to helps us highlight individual lines.
     * We could use srchilite::SourceHighlight with an std::stringstream for that too, but it would not give
     * us the ability to let people re-use their existing .outlang files, since they would be needed to be refactored
     * (clearing doctemplate / nodoctemplate).
     * 
     * NOTE: to ensure correct behaviour, you should always highlight lines in order!
     */
    class SourceLineHighlighter {

        std::string outputLang;
        std::string dataDir;
        std::string styleFile;
        std::string styleCssFile;
        std::string styleDefaultFile;
        std::string inputLang;
        std::string css;
        FormatterManager *formatterManager;
        PreFormatter *preFormatter;
        TextStyleFormatterCollection formatterCollection;
        LangDefManager *langDefManager;
        // CTagsManager *ctagsManager;
        // CTagsFormatter *ctagsFormatter;
        bool optimize;
        void updateBufferedOutput(BufferedOutput *output);

    public:
        SourceLineHighlighter(const std::string& outputLang = "html.outlang");
        ~SourceLineHighlighter();

        void initialize();
        void highlight(std::istream &input, std::ostream &output, const std::string &inputLang, const std::string &inputFileName = "");
        void checkLangDef(const std::string &langFile);
        void checkOutLangDef(const std::string &langFile);
        void printHighlightState(const std::string &langFile, std::ostream &os);
        void printLangElems(const std::string &langFile, std::ostream &os);
        const std::string createOutputFileName(const std::string &inputFile);
    };

}