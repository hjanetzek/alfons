/*
 * Based on The New Chronotext Toolkit
 * Copyright (C) 2014, Ariel Malka - All rights reserved.
 *
 * Adaption to Alfons
 * Copyright (C) 2015, Hannes Janetzek
 *
 * The following source-code is distributed under the Simplified BSD License.
 */

#pragma once

#include <hb.h>

#include <vector>
#include <map>
#include <set>
#include <string>

namespace alfons {

class LangHelper {
public:
    LangHelper();

    /*
     * Expects a list languages separated by colons
     */
    void setDefaultLanguages(const std::string& languages);

    /*
     * Determines the scripts used to write @lang
     *
     * Quoting pango:
     * Most languages use only one script for writing, but there are
     * some that use two (Latin and Cyrillic for example), and a few
     * use three (Japanese for example).
     */
    const std::vector<hb_script_t>& getScriptsForLang(const std::string& lang) const;

    /*
     * Determines if @script may be used to write @lang
     */
    bool includesScript(const std::string& lang, hb_script_t script) const;

    /*
     * Returns the resolved language if @script may be used to write one of the
     * "default languages"
     */
    const std::string& getDefaultLanguage(hb_script_t script) const;

    /*
     * Quoting pango:
     * Given a script, finds a language tag that is reasonably
     * representative of that script. This will usually be the
     * most widely spoken or used language written in that script:
     * for instance, the sample language for %HB_SCRIPT_CYRILLIC
     * is <literal>ru</literal> (Russian), the sample language
     * for %HB_SCRIPT_ARABIC is <literal>ar</literal>.
     *
     * For some scripts, no sample language will be returned because
     * there is no language that is sufficiently representative.
     * The best example of this is %HB_SCRIPT_HAN, where various
     * different  variants of written Chinese, Japanese, and Korean
     * all use  significantly different sets of Han characters and
     * forms of shared characters. No sample language can be provided
     * for many historical scripts as well.
     */
    const std::string& getSampleLanguage(hb_script_t script) const;

    /*
     * Trying to detect a language for @script by asking 3 questions:
     * 1. Can @script be used to write @langHint?
     */
    bool matchLanguage(hb_script_t script, const std::string& langHint) const;

    /*
     * 2. Can @script be used to write one of the "default languages"?
     * 3. Is there a predominant language that is likely for @script?
     */
    const std::string& detectLanguage(hb_script_t script) const;

protected:
    std::map<std::string, std::vector<hb_script_t>> scriptMap;
    std::map<hb_script_t, std::string> sampleLanguageMap;
    std::set<std::string> defaultLanguageSet;
};
}
