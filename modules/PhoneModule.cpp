// ============================================================
// modules/PhoneModule.cpp
// ============================================================
#include "PhoneModule.h"
#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>

using json = nlohmann::json;

void PhoneModule::Run(const std::string& pivot, ResultSet& results) {
    NormalizeAndParse(pivot, results);
    QueryNumverify(pivot, results);
    SuggestSocialPivots(pivot, results);
}

void PhoneModule::NormalizeAndParse(const std::string& phone, ResultSet& results) {
    // Strip everything except digits and leading +
    std::string stripped;
    for (char c : phone) {
        if (isdigit(c) || (c == '+' && stripped.empty()))
            stripped += c;
    }
    results.Add("Phone", "Normalized", stripped, "Local Parse");

    // Rough country code detection
    if (stripped.size() > 1 && stripped[0] == '+') {
        if (stripped.substr(0, 2) == "+1")
            results.Add("Phone", "Country Code", "+1 (US/Canada)", "Local Parse");
        else if (stripped.substr(0, 3) == "+44")
            results.Add("Phone", "Country Code", "+44 (United Kingdom)", "Local Parse");
        else if (stripped.substr(0, 3) == "+61")
            results.Add("Phone", "Country Code", "+61 (Australia)", "Local Parse");
        else if (stripped.substr(0, 3) == "+49")
            results.Add("Phone", "Country Code", "+49 (Germany)", "Local Parse");
        else if (stripped.substr(0, 3) == "+91")
            results.Add("Phone", "Country Code", "+91 (India)", "Local Parse");
        else
            results.Add("Phone", "Country Code",
                        stripped.substr(0, 3) + " (unrecognized)", "Local Parse");
    }

    // NANP area code intel (US)
    if (stripped.size() >= 4 && (stripped[0] != '+' || stripped.substr(0, 2) == "+1")) {
        std::string areaStart = (stripped[0] == '+') ? stripped.substr(2, 3)
                                                     : stripped.substr(0, 3);
        results.Add("Phone", "Area Code (NANP)", areaStart,
                    "Local Parse — cross-ref areacodelocations.com");
    }
}

void PhoneModule::QueryNumverify(const std::string& phone, ResultSet& results) {
    if (std::string(NUMVERIFY_KEY) == "YOUR_NUMVERIFY_API_KEY") {
        results.Add("Carrier", "Numverify", "API key not configured", "Numverify");
        return;
    }

    // Strip to digits only for query
    std::string digits;
    for (char c : phone) if (isdigit(c)) digits += c;

    HttpClient client;
    std::wstring key  = std::wstring(std::string(NUMVERIFY_KEY).begin(),
                                      std::string(NUMVERIFY_KEY).end());
    std::wstring num  = std::wstring(digits.begin(), digits.end());
    std::wstring path = L"/api/validate?access_key=" + key +
                        L"&number=" + num + L"&format=1";

    auto resp = client.Get(L"apilayer.net", path);
    if (!resp.success) {
        results.AddError("Numverify: HTTP " + std::to_string(resp.statusCode));
        return;
    }

    try {
        auto j = json::parse(resp.body);
        if (!j.value("valid", false)) {
            results.Add("Phone", "Valid", "false", "Numverify");
            return;
        }

        auto addStr = [&](const std::string& cat,
                          const std::string& label,
                          const std::string& jkey) {
            if (j.contains(jkey) && j[jkey].is_string() && !j[jkey].get<std::string>().empty())
                results.Add(cat, label, j[jkey].get<std::string>(), "Numverify");
        };

        results.Add("Phone", "Valid", "true", "Numverify");
        addStr("Phone",   "International Format", "international_format");
        addStr("Phone",   "Local Format",          "local_format");
        addStr("Phone",   "Country",               "country_name");
        addStr("Phone",   "Country Code",          "country_code");
        addStr("Phone",   "Location",              "location");
        addStr("Carrier", "Carrier Name",          "carrier");
        addStr("Carrier", "Line Type",             "line_type");
    } catch (const std::exception& ex) {
        results.AddError(std::string("Numverify parse: ") + ex.what());
    }
}

void PhoneModule::SuggestSocialPivots(const std::string& phone, ResultSet& results) {
    // Strip non-digits
    std::string digits;
    for (char c : phone) if (isdigit(c)) digits += c;

    // WhatsApp deep link
    results.Add("Social Pivot", "WhatsApp",
                "https://wa.me/" + digits, "Generated Pivot");
    // Telegram phone lookup (manual)
    results.Add("Social Pivot", "Telegram",
                "Search +"+digits+" in Telegram manually", "Generated Pivot");
    // Truecaller (manual — they block automated queries)
    results.Add("Social Pivot", "Truecaller",
                "https://www.truecaller.com/search/us/" + digits, "Generated Pivot");
    // Google dorking suggestion
    results.Add("Social Pivot", "Google Dork",
                "site:facebook.com | site:linkedin.com \"" + phone + "\"",
                "Generated Pivot");
}
