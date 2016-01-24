/*
 * Based on The New Chronotext Toolkit
 * Copyright (C) 2014, Ariel Malka - All rights reserved.
 *
 * Adaption to Alfons
 * Copyright (C) 2015, Hannes Janetzek
 *
 * The following source-code is distributed under the Simplified BSD License.
 */

/*
 * Includes data from pango:
 * https://developer.gnome.org/pango/stable/pango-Scripts-and-Languages.html
 */

#include "langHelper.h"

#include <assert.h>

namespace alfons {
const std::string DEFAULT_LANGUAGES = "en:zh-cn"; // GIVING PRIORITY (BY DEFAULT) TO CHINESE OVER JAPANESE

struct HBScriptForLang {
    const char lang[7];
    hb_script_t scripts[3];
};

/*
 * DATA FROM pango-script-lang-table.h
 */
static const HBScriptForLang HB_SCRIPT_FOR_LANG[] = {
    {"aa", {HB_SCRIPT_LATIN /*62*/}},
    {"ab", {HB_SCRIPT_CYRILLIC /*90*/}},
    {"af", {HB_SCRIPT_LATIN /*69*/}},
    {"ak", {HB_SCRIPT_LATIN /*70*/}},
    {"am", {HB_SCRIPT_ETHIOPIC /*264*/}},
    {"an", {HB_SCRIPT_LATIN /*66*/}},
    {"ar", {HB_SCRIPT_ARABIC /*125*/}},
    {"as", {HB_SCRIPT_BENGALI /*64*/}},
    {"ast", {HB_SCRIPT_LATIN /*66*/}},
    {"av", {HB_SCRIPT_CYRILLIC /*67*/}},
    {"ay", {HB_SCRIPT_LATIN /*60*/}},
    {"az-az", {HB_SCRIPT_LATIN /*66*/}},
    {"az-ir", {HB_SCRIPT_ARABIC /*129*/}},
    {"ba", {HB_SCRIPT_CYRILLIC /*82*/}},
    {"be", {HB_SCRIPT_CYRILLIC /*68*/}},
    {"ber-dz", {HB_SCRIPT_LATIN /*70*/}},
    {"ber-ma", {HB_SCRIPT_TIFINAGH /*32*/}},
    {"bg", {HB_SCRIPT_CYRILLIC /*60*/}},
    {"bh", {HB_SCRIPT_DEVANAGARI /*68*/}},
    {"bho", {HB_SCRIPT_DEVANAGARI /*68*/}},
    {"bi", {HB_SCRIPT_LATIN /*58*/}},
    {"bin", {HB_SCRIPT_LATIN /*76*/}},
    {"bm", {HB_SCRIPT_LATIN /*60*/}},
    {"bn", {HB_SCRIPT_BENGALI /*63*/}},
    {"bo", {HB_SCRIPT_TIBETAN /*95*/}},
    {"br", {HB_SCRIPT_LATIN /*64*/}},
    {"bs", {HB_SCRIPT_LATIN /*62*/}},
    {"bua", {HB_SCRIPT_CYRILLIC /*70*/}},
    {"byn", {HB_SCRIPT_ETHIOPIC /*255*/}},
    {"ca", {HB_SCRIPT_LATIN /*74*/}},
    {"ce", {HB_SCRIPT_CYRILLIC /*67*/}},
    {"ch", {HB_SCRIPT_LATIN /*58*/}},
    {"chm", {HB_SCRIPT_CYRILLIC /*76*/}},
    {"chr", {HB_SCRIPT_CHEROKEE /*85*/}},
    {"co", {HB_SCRIPT_LATIN /*84*/}},
    {"crh", {HB_SCRIPT_LATIN /*68*/}},
    {"cs", {HB_SCRIPT_LATIN /*82*/}},
    {"csb", {HB_SCRIPT_LATIN /*74*/}},
    {"cu", {HB_SCRIPT_CYRILLIC /*103*/}},
    {"cv", {HB_SCRIPT_CYRILLIC /*72*/, HB_SCRIPT_LATIN /*2*/}},
    {"cy", {HB_SCRIPT_LATIN /*78*/}},
    {"da", {HB_SCRIPT_LATIN /*70*/}},
    {"de", {HB_SCRIPT_LATIN /*59*/}},
    {"dv", {HB_SCRIPT_THAANA /*49*/}},
    {"dz", {HB_SCRIPT_TIBETAN /*95*/}},
    {"ee", {HB_SCRIPT_LATIN /*96*/}},
    {"el", {HB_SCRIPT_GREEK /*69*/}},
    {"en", {HB_SCRIPT_LATIN /*72*/}},
    {"eo", {HB_SCRIPT_LATIN /*64*/}},
    {"es", {HB_SCRIPT_LATIN /*66*/}},
    {"et", {HB_SCRIPT_LATIN /*64*/}},
    {"eu", {HB_SCRIPT_LATIN /*56*/}},
    {"fa", {HB_SCRIPT_ARABIC /*129*/}},
    {"fat", {HB_SCRIPT_LATIN /*70*/}},
    {"ff", {HB_SCRIPT_LATIN /*62*/}},
    {"fi", {HB_SCRIPT_LATIN /*62*/}},
    {"fil", {HB_SCRIPT_LATIN /*84*/}},
    {"fj", {HB_SCRIPT_LATIN /*52*/}},
    {"fo", {HB_SCRIPT_LATIN /*68*/}},
    {"fr", {HB_SCRIPT_LATIN /*84*/}},
    {"fur", {HB_SCRIPT_LATIN /*66*/}},
    {"fy", {HB_SCRIPT_LATIN /*75*/}},
    {"ga", {HB_SCRIPT_LATIN /*80*/}},
    {"gd", {HB_SCRIPT_LATIN /*70*/}},
    {"gez", {HB_SCRIPT_ETHIOPIC /*218*/}},
    {"gl", {HB_SCRIPT_LATIN /*66*/}},
    {"gn", {HB_SCRIPT_LATIN /*70*/}},
    {"gu", {HB_SCRIPT_GUJARATI /*68*/}},
    {"gv", {HB_SCRIPT_LATIN /*54*/}},
    {"ha", {HB_SCRIPT_LATIN /*60*/}},
    {"haw", {HB_SCRIPT_LATIN /*62*/}},
    {"he", {HB_SCRIPT_HEBREW /*27*/}},
    {"hi", {HB_SCRIPT_DEVANAGARI /*68*/}},
    {"hne", {HB_SCRIPT_DEVANAGARI /*68*/}},
    {"ho", {HB_SCRIPT_LATIN /*52*/}},
    {"hr", {HB_SCRIPT_LATIN /*62*/}},
    {"hsb", {HB_SCRIPT_LATIN /*72*/}},
    {"ht", {HB_SCRIPT_LATIN /*56*/}},
    {"hu", {HB_SCRIPT_LATIN /*70*/}},
    {"hy", {HB_SCRIPT_ARMENIAN /*77*/}},
    {"hz", {HB_SCRIPT_LATIN /*56*/}},
    {"ia", {HB_SCRIPT_LATIN /*52*/}},
    {"id", {HB_SCRIPT_LATIN /*54*/}},
    {"ie", {HB_SCRIPT_LATIN /*52*/}},
    {"ig", {HB_SCRIPT_LATIN /*58*/}},
    {"ii", {HB_SCRIPT_YI /*1165*/}},
    {"ik", {HB_SCRIPT_CYRILLIC /*68*/}},
    {"io", {HB_SCRIPT_LATIN /*52*/}},
    {"is", {HB_SCRIPT_LATIN /*70*/}},
    {"it", {HB_SCRIPT_LATIN /*72*/}},
    {"iu", {HB_SCRIPT_CANADIAN_ABORIGINAL /*161*/}},
    {"ja", {HB_SCRIPT_HAN /*6356*/, HB_SCRIPT_KATAKANA /*88*/, HB_SCRIPT_HIRAGANA /*85*/}},
    {"jv", {HB_SCRIPT_LATIN /*56*/}},
    {"ka", {HB_SCRIPT_GEORGIAN /*33*/}},
    {"kaa", {HB_SCRIPT_CYRILLIC /*78*/}},
    {"kab", {HB_SCRIPT_LATIN /*70*/}},
    {"ki", {HB_SCRIPT_LATIN /*56*/}},
    {"kj", {HB_SCRIPT_LATIN /*52*/}},
    {"kk", {HB_SCRIPT_CYRILLIC /*77*/}},
    {"kl", {HB_SCRIPT_LATIN /*81*/}},
    {"km", {HB_SCRIPT_KHMER /*63*/}},
    {"kn", {HB_SCRIPT_KANNADA /*70*/}},
    {"ko", {HB_SCRIPT_HANGUL /*2443*/}},
    {"kok", {HB_SCRIPT_DEVANAGARI /*68*/}},
    {"kr", {HB_SCRIPT_LATIN /*56*/}},
    {"ks", {HB_SCRIPT_ARABIC /*145*/}},
    {"ku-am", {HB_SCRIPT_CYRILLIC /*64*/}},
    {"ku-iq", {HB_SCRIPT_ARABIC /*32*/}},
    {"ku-ir", {HB_SCRIPT_ARABIC /*32*/}},
    {"ku-tr", {HB_SCRIPT_LATIN /*62*/}},
    {"kum", {HB_SCRIPT_CYRILLIC /*66*/}},
    {"kv", {HB_SCRIPT_CYRILLIC /*70*/}},
    {"kw", {HB_SCRIPT_LATIN /*64*/}},
    {"kwm", {HB_SCRIPT_LATIN /*52*/}},
    {"ky", {HB_SCRIPT_CYRILLIC /*70*/}},
    {"la", {HB_SCRIPT_LATIN /*68*/}},
    {"lb", {HB_SCRIPT_LATIN /*75*/}},
    {"lez", {HB_SCRIPT_CYRILLIC /*67*/}},
    {"lg", {HB_SCRIPT_LATIN /*54*/}},
    {"li", {HB_SCRIPT_LATIN /*62*/}},
    {"ln", {HB_SCRIPT_LATIN /*78*/}},
    {"lo", {HB_SCRIPT_LAO /*55*/}},
    {"lt", {HB_SCRIPT_LATIN /*70*/}},
    {"lv", {HB_SCRIPT_LATIN /*78*/}},
    {"mai", {HB_SCRIPT_DEVANAGARI /*68*/}},
    {"mg", {HB_SCRIPT_LATIN /*56*/}},
    {"mh", {HB_SCRIPT_LATIN /*62*/}},
    {"mi", {HB_SCRIPT_LATIN /*64*/}},
    {"mk", {HB_SCRIPT_CYRILLIC /*42*/}},
    {"ml", {HB_SCRIPT_MALAYALAM /*68*/}},
    {"mn-cn", {HB_SCRIPT_MONGOLIAN /*130*/}},
    {"mn-mn", {HB_SCRIPT_CYRILLIC /*70*/}},
    {"mo", {HB_SCRIPT_CYRILLIC /*66*/, HB_SCRIPT_LATIN /*62*/}},
    {"mr", {HB_SCRIPT_DEVANAGARI /*68*/}},
    {"ms", {HB_SCRIPT_LATIN /*52*/}},
    {"mt", {HB_SCRIPT_LATIN /*72*/}},
    {"my", {HB_SCRIPT_MYANMAR /*48*/}},
    {"na", {HB_SCRIPT_LATIN /*60*/}},
    {"nb", {HB_SCRIPT_LATIN /*70*/}},
    {"nds", {HB_SCRIPT_LATIN /*59*/}},
    {"ne", {HB_SCRIPT_DEVANAGARI /*68*/}},
    {"ng", {HB_SCRIPT_LATIN /*52*/}},
    {"nl", {HB_SCRIPT_LATIN /*82*/}},
    {"nn", {HB_SCRIPT_LATIN /*76*/}},
    {"no", {HB_SCRIPT_LATIN /*70*/}},
    {"nr", {HB_SCRIPT_LATIN /*52*/}},
    {"nso", {HB_SCRIPT_LATIN /*58*/}},
    {"nv", {HB_SCRIPT_LATIN /*70*/}},
    {"ny", {HB_SCRIPT_LATIN /*54*/}},
    {"oc", {HB_SCRIPT_LATIN /*70*/}},
    {"om", {HB_SCRIPT_LATIN /*52*/}},
    {"or", {HB_SCRIPT_ORIYA /*68*/}},
    {"os", {HB_SCRIPT_CYRILLIC /*66*/}},
    {"ota", {HB_SCRIPT_ARABIC /*37*/}},
    {"pa-in", {HB_SCRIPT_GURMUKHI /*63*/}},
    {"pa-pk", {HB_SCRIPT_ARABIC /*145*/}},
    {"pap-an", {HB_SCRIPT_LATIN /*72*/}},
    {"pap-aw", {HB_SCRIPT_LATIN /*54*/}},
    {"pl", {HB_SCRIPT_LATIN /*70*/}},
    {"ps-af", {HB_SCRIPT_ARABIC /*49*/}},
    {"ps-pk", {HB_SCRIPT_ARABIC /*49*/}},
    {"pt", {HB_SCRIPT_LATIN /*82*/}},
    {"qu", {HB_SCRIPT_LATIN /*54*/}},
    {"rm", {HB_SCRIPT_LATIN /*66*/}},
    {"rn", {HB_SCRIPT_LATIN /*52*/}},
    {"ro", {HB_SCRIPT_LATIN /*62*/}},
    {"ru", {HB_SCRIPT_CYRILLIC /*66*/}},
    {"rw", {HB_SCRIPT_LATIN /*52*/}},
    {"sa", {HB_SCRIPT_DEVANAGARI /*68*/}},
    {"sah", {HB_SCRIPT_CYRILLIC /*76*/}},
    {"sc", {HB_SCRIPT_LATIN /*62*/}},
    {"sco", {HB_SCRIPT_LATIN /*56*/}},
    {"sd", {HB_SCRIPT_ARABIC /*54*/}},
    {"se", {HB_SCRIPT_LATIN /*66*/}},
    {"sel", {HB_SCRIPT_CYRILLIC /*66*/}},
    {"sg", {HB_SCRIPT_LATIN /*72*/}},
    {"sh", {HB_SCRIPT_CYRILLIC /*94*/, HB_SCRIPT_LATIN /*62*/}},
    {"shs", {HB_SCRIPT_LATIN /*46*/}},
    {"si", {HB_SCRIPT_SINHALA /*73*/}},
    {"sid", {HB_SCRIPT_ETHIOPIC /*281*/}},
    {"sk", {HB_SCRIPT_LATIN /*86*/}},
    {"sl", {HB_SCRIPT_LATIN /*62*/}},
    {"sm", {HB_SCRIPT_LATIN /*52*/}},
    {"sma", {HB_SCRIPT_LATIN /*60*/}},
    {"smj", {HB_SCRIPT_LATIN /*60*/}},
    {"smn", {HB_SCRIPT_LATIN /*68*/}},
    {"sms", {HB_SCRIPT_LATIN /*80*/}},
    {"sn", {HB_SCRIPT_LATIN /*52*/}},
    {"so", {HB_SCRIPT_LATIN /*52*/}},
    {"sq", {HB_SCRIPT_LATIN /*56*/}},
    {"sr", {HB_SCRIPT_CYRILLIC /*60*/}},
    {"ss", {HB_SCRIPT_LATIN /*52*/}},
    {"st", {HB_SCRIPT_LATIN /*52*/}},
    {"su", {HB_SCRIPT_LATIN /*54*/}},
    {"sv", {HB_SCRIPT_LATIN /*68*/}},
    {"sw", {HB_SCRIPT_LATIN /*52*/}},
    {"syr", {HB_SCRIPT_SYRIAC /*45*/}},
    {"ta", {HB_SCRIPT_TAMIL /*48*/}},
    {"te", {HB_SCRIPT_TELUGU /*70*/}},
    {"tg", {HB_SCRIPT_CYRILLIC /*78*/}},
    {"th", {HB_SCRIPT_THAI /*73*/}},
    {"ti-er", {HB_SCRIPT_ETHIOPIC /*255*/}},
    {"ti-et", {HB_SCRIPT_ETHIOPIC /*281*/}},
    {"tig", {HB_SCRIPT_ETHIOPIC /*221*/}},
    {"tk", {HB_SCRIPT_LATIN /*68*/}},
    {"tl", {HB_SCRIPT_LATIN /*84*/}},
    {"tn", {HB_SCRIPT_LATIN /*58*/}},
    {"to", {HB_SCRIPT_LATIN /*52*/}},
    {"tr", {HB_SCRIPT_LATIN /*70*/}},
    {"ts", {HB_SCRIPT_LATIN /*52*/}},
    {"tt", {HB_SCRIPT_CYRILLIC /*76*/}},
    {"tw", {HB_SCRIPT_LATIN /*70*/}},
    {"ty", {HB_SCRIPT_LATIN /*64*/}},
    {"tyv", {HB_SCRIPT_CYRILLIC /*70*/}},
    {"ug", {HB_SCRIPT_ARABIC /*125*/}},
    {"uk", {HB_SCRIPT_CYRILLIC /*72*/}},
    {"ur", {HB_SCRIPT_ARABIC /*145*/}},
    {"uz", {HB_SCRIPT_LATIN /*52*/}},
    {"ve", {HB_SCRIPT_LATIN /*62*/}},
    {"vi", {HB_SCRIPT_LATIN /*186*/}},
    {"vo", {HB_SCRIPT_LATIN /*54*/}},
    {"vot", {HB_SCRIPT_LATIN /*62*/}},
    {"wa", {HB_SCRIPT_LATIN /*70*/}},
    {"wal", {HB_SCRIPT_ETHIOPIC /*281*/}},
    {"wen", {HB_SCRIPT_LATIN /*76*/}},
    {"wo", {HB_SCRIPT_LATIN /*66*/}},
    {"xh", {HB_SCRIPT_LATIN /*52*/}},
    {"yap", {HB_SCRIPT_LATIN /*58*/}},
    {"yi", {HB_SCRIPT_HEBREW /*27*/}},
    {"yo", {HB_SCRIPT_LATIN /*114*/}},
    {"za", {HB_SCRIPT_LATIN /*52*/}},
    {"zh-cn", {HB_SCRIPT_HAN /*6763*/}},
    {"zh-hk", {HB_SCRIPT_HAN /*2213*/}},
    {"zh-mo", {HB_SCRIPT_HAN /*2213*/}},
    {"zh-sg", {HB_SCRIPT_HAN /*6763*/}},
    {"zh-tw", {HB_SCRIPT_HAN /*13063*/}},
    {"zu", {HB_SCRIPT_LATIN /*52*/}}};

static void
initScriptMap(std::map<std::string, std::vector<hb_script_t>>& scriptMap) {
    size_t entryCount = sizeof(HB_SCRIPT_FOR_LANG) / sizeof(HBScriptForLang);

    for (size_t i = 0; i < entryCount; i++) {
        auto entry = HB_SCRIPT_FOR_LANG[i];

        size_t scriptCount = sizeof(entry.scripts) / sizeof(hb_script_t);
        std::vector<hb_script_t> scripts;

        for (size_t j = 0; j < scriptCount; j++) {
            if (entry.scripts[j]) {
                scripts.push_back(entry.scripts[j]);
            } else {
                break;
            }
        }

        assert(scripts.size() > 0);
        scriptMap[entry.lang] = scripts;
    }

    /*
     * DEFAULT-VALUE
     */
    std::vector<hb_script_t> invalid;
    invalid.push_back(HB_SCRIPT_INVALID);
    scriptMap[""] = invalid;
}

static void
initSampleLanguageMap(std::map<hb_script_t, std::string>& sampleLanguageMap) {
    sampleLanguageMap[HB_SCRIPT_ARABIC] = "ar";
    sampleLanguageMap[HB_SCRIPT_ARMENIAN] = "hy";
    sampleLanguageMap[HB_SCRIPT_BENGALI] = "bn";
    sampleLanguageMap[HB_SCRIPT_CHEROKEE] = "chr";
    sampleLanguageMap[HB_SCRIPT_COPTIC] = "cop";
    sampleLanguageMap[HB_SCRIPT_CYRILLIC] = "ru";
    sampleLanguageMap[HB_SCRIPT_DEVANAGARI] = "hi";
    sampleLanguageMap[HB_SCRIPT_ETHIOPIC] = "am";
    sampleLanguageMap[HB_SCRIPT_GEORGIAN] = "ka";
    sampleLanguageMap[HB_SCRIPT_GREEK] = "el";
    sampleLanguageMap[HB_SCRIPT_GUJARATI] = "gu";
    sampleLanguageMap[HB_SCRIPT_GURMUKHI] = "pa";
    sampleLanguageMap[HB_SCRIPT_HANGUL] = "ko";
    sampleLanguageMap[HB_SCRIPT_HEBREW] = "he";
    sampleLanguageMap[HB_SCRIPT_HIRAGANA] = "ja";
    sampleLanguageMap[HB_SCRIPT_KANNADA] = "kn";
    sampleLanguageMap[HB_SCRIPT_KATAKANA] = "ja";
    sampleLanguageMap[HB_SCRIPT_KHMER] = "km";
    sampleLanguageMap[HB_SCRIPT_LAO] = "lo";
    sampleLanguageMap[HB_SCRIPT_LATIN] = "en";
    sampleLanguageMap[HB_SCRIPT_MALAYALAM] = "ml";
    sampleLanguageMap[HB_SCRIPT_MONGOLIAN] = "mn";
    sampleLanguageMap[HB_SCRIPT_MYANMAR] = "my";
    sampleLanguageMap[HB_SCRIPT_ORIYA] = "or";
    sampleLanguageMap[HB_SCRIPT_SINHALA] = "si";
    sampleLanguageMap[HB_SCRIPT_SYRIAC] = "syr";
    sampleLanguageMap[HB_SCRIPT_TAMIL] = "ta";
    sampleLanguageMap[HB_SCRIPT_TELUGU] = "te";
    sampleLanguageMap[HB_SCRIPT_THAANA] = "dv";
    sampleLanguageMap[HB_SCRIPT_THAI] = "th";
    sampleLanguageMap[HB_SCRIPT_TIBETAN] = "bo";
    sampleLanguageMap[HB_SCRIPT_CANADIAN_ABORIGINAL] = "iu";
    sampleLanguageMap[HB_SCRIPT_TAGALOG] = "tl";
    sampleLanguageMap[HB_SCRIPT_HANUNOO] = "hnn";
    sampleLanguageMap[HB_SCRIPT_BUHID] = "bku";
    sampleLanguageMap[HB_SCRIPT_TAGBANWA] = "tbw";
    sampleLanguageMap[HB_SCRIPT_UGARITIC] = "uga";
    sampleLanguageMap[HB_SCRIPT_BUGINESE] = "bug";
    sampleLanguageMap[HB_SCRIPT_SYLOTI_NAGRI] = "syl";
    sampleLanguageMap[HB_SCRIPT_OLD_PERSIAN] = "peo";
    sampleLanguageMap[HB_SCRIPT_NKO] = "nqo";

    /*
     * Default-value
     */
    sampleLanguageMap[HB_SCRIPT_INVALID] = "";
};

LangHelper::LangHelper() {
    initScriptMap(scriptMap);
    initSampleLanguageMap(sampleLanguageMap);
    setDefaultLanguages(DEFAULT_LANGUAGES);
}

void LangHelper::setDefaultLanguages(const std::string& languages) {
    defaultLanguageSet.clear();
#if 0
    for (auto &lang : split(languages, ":")) {defaultLanguageSet.insert(lang);
    }
#endif
}

const std::vector<hb_script_t>&
LangHelper::getScriptsForLang(const std::string& lang) const {
    auto it = scriptMap.find(lang);

    if (it == scriptMap.end()) {
        it = scriptMap.find("");
    }

    return it->second;
}

bool LangHelper::includesScript(const std::string& lang, hb_script_t script) const {
    for (auto& value : getScriptsForLang(lang)) {
        if (value == script) {
            return true;
        }
    }

    return false;
}

const std::string&
LangHelper::getDefaultLanguage(hb_script_t script) const {
    const static std::string EMPTY = "";

    for (auto& lang : defaultLanguageSet) {
        for (auto& value : getScriptsForLang(lang)) {
            if (value == script) {
                return lang;
            }
        }
    }

    return EMPTY;
}

const std::string&
LangHelper::getSampleLanguage(hb_script_t script) const {
    auto it = sampleLanguageMap.find(script);

    if (it == sampleLanguageMap.end()) {
        it = sampleLanguageMap.find(HB_SCRIPT_INVALID);
    }

    return it->second;
}

auto LangHelper::matchLanguage(hb_script_t script, const std::string& langHint) const -> bool {
    // 1. Can @script be used to write @langHint?
    if (!langHint.empty() && includesScript(langHint, script)) {
        return true;
    }
    return false;
}

auto LangHelper::detectLanguage(hb_script_t script) const -> const std::string& {

    // 2. Can @script be used to write one of the "default languages"?
    const auto& defaultLanguage = getDefaultLanguage(script);
    if (!defaultLanguage.empty()) {
        return defaultLanguage;
    }
    // 3. Is there a predominant language that is likely for @script?
    return getSampleLanguage(script);
}
}
