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