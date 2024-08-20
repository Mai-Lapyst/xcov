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

#include "highlight.h"

// #include <srchilite/sourcehighlight.h>
// #include <srchilite/fileutil.h>
#include <srchilite/formattermanager.h>
// #include <srchilite/textstyles.h>
// #include <srchilite/preformatter.h>
#include <srchilite/parsestyles.h>
#include <srchilite/textstyleformatter.h>
#include <srchilite/textstyleformatterfactory.h>
#include <srchilite/outlangdefparserfun.h>
#include <srchilite/langdefmanager.h>
#include <srchilite/regexrulefactory.h>
#include <srchilite/highlightstate.h>
#include <srchilite/sourcehighlighter.h>
#include <srchilite/bufferedoutput.h>
#include <srchilite/sourcefilehighlighter.h>
// #include <srchilite/linenumgenerator.h>
// #include <srchilite/ioexception.h>
// #include <srchilite/docgenerator.h>
// #include <srchilite/srcuntabifier.h>
#include <srchilite/langmap.h>
#include <srchilite/parserexception.h>
// #include <srchilite/ctagsmanager.h>
// #include <srchilite/ctagsformatter.h>
// #include <srchilite/highlightstateprinter.h>
// #include <srchilite/langelemsprinter.hpp>
// #include <srchilite/langelems.h>
// #include <srchilite/verbosity.h>
#include <srchilite/settings.h>

#include <iostream>

namespace xcov {

    using namespace srchilite;

    SourceLineHighlighter::SourceLineHighlighter(const std::string& _outputLang)
        : outputLang(_outputLang), dataDir(Settings::retrieveDataDir()), styleFile("default.style"),
            formatterManager(0), preFormatter(0),
            langDefManager(new LangDefManager(new RegexRuleFactory)),
            // ctagsManager(0), ctagsFormatter(0),
            optimize(true)
    {}

    SourceLineHighlighter::~SourceLineHighlighter() {
        if (formatterManager)
            delete formatterManager;

        if (preFormatter)
            delete preFormatter;

        delete langDefManager->getRuleFactory();
        delete langDefManager;

        // if (ctagsFormatter)
        //     delete ctagsFormatter;
    }

    void SourceLineHighlighter::initialize() {
        if (formatterManager) { return; }

        
        TextStylesPtr textStyles;
        try {
            textStyles = parse_outlang_def(dataDir.c_str(), outputLang.c_str());
        }
        catch (ParserException& e) {
            std::cerr << "SourceLineHighlighter: " << e << std::endl;
            throw e;
        }

        FormatterPtr defaultFormatter(new TextStyleFormatter("$text"));
        formatterManager = new FormatterManager(defaultFormatter);

        preFormatter = new PreFormatter(textStyles->charTranslator);

        // if (ctagsManager) {
        //     ctagsFormatter = ctagsManager->createCTagsFormatter(
        //             textStyles->refstyle);
        //     ctagsFormatter->setPreFormatter(preFormatter);
        // }

        // TextStyleFormatterFactory formatterFactory(textStyles, preFormatter, ctagsFormatter, formatterManager);
        TextStyleFormatterFactory formatterFactory(textStyles, preFormatter, NULL, formatterManager);

        // We dont need the bgColor here...
        std::string bgColor;

        if (styleCssFile.size()) {
            parseCssStyles(dataDir, styleCssFile, &formatterFactory, bgColor);
        }
        else {
            parseStyles(dataDir, styleFile, &formatterFactory, bgColor);
        }

        formatterFactory.addDefaultFormatter();

        if (styleDefaultFile.size()) {
            LangMap defaultStyles(dataDir, styleDefaultFile);
            defaultStyles.open();
            for (LangMap::const_iterator it = defaultStyles.begin(); it != defaultStyles.end(); ++it) {
                formatterFactory.createMissingFormatter(it->first, it->second);
            }
        }

        formatterCollection = formatterFactory.getFormatterCollection();

        for (TextStyleFormatterCollection::const_iterator it = formatterCollection.begin(); it != formatterCollection.end(); ++it) {
            (*it)->setPreFormatter(preFormatter);
        }
    }

    void SourceLineHighlighter::highlight(
        std::istream &input, std::ostream &output,
        const std::string &inputLang, const std::string &inputFileName
    ) {
        initialize();

        HighlightStatePtr highlightState = langDefManager->getHighlightState(dataDir, inputLang);

        SourceHighlighter highlighter(highlightState);
        highlighter.setFormatterManager(formatterManager);
        highlighter.setOptimize(optimize);

        BufferedOutput bufferedOutput(output);

        if (!optimize)
            bufferedOutput.setAlwaysFlush(true);

        updateBufferedOutput(&bufferedOutput);

        SourceFileHighlighter fileHighlighter(inputFileName, &highlighter, &bufferedOutput);

        fileHighlighter.setPreformatter(preFormatter);

        fileHighlighter.setContextFormatter(formatterManager->getFormatter("context").get());

        fileHighlighter.highlight(input);
    }

    void SourceLineHighlighter::updateBufferedOutput(BufferedOutput *output) {
        for (TextStyleFormatterCollection::const_iterator it =
                formatterCollection.begin(); it != formatterCollection.end(); ++it) {
            (*it)->setBufferedOutput(output);
        }
    }

}