// ============================================================
// modules/EmailModule.cpp
// ============================================================
#include "EmailModule.h"
#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>

using json = nlohmann::json;

void EmailModule::Run(const std::string& pivot, ResultSet& results) {
    ExtractDomainIntel(pivot, results);
    CheckHIBP(pivot, results);
    CheckHunter(pivot, results);
}

void EmailModule::ExtractDomainIntel(const std::string& email, ResultSet& results) {
    // Extract domain
    auto atPos = email.find('@');
    if (atPos == std::string::npos) return;
    std::string domain = email.substr(atPos + 1);
    results.Add("Domain", "Extracted Domain", domain, "Local Parse");

    // Classify provider
    std::map<std::string, std::string> known = {
        {"gmail.com", "Google"}, {"yahoo.com", "Yahoo"},
        {"outlook.com", "Microsoft"}, {"hotmail.com", "Microsoft"},
        {"protonmail.com", "Proton (Encrypted)"}, {"icloud.com", "Apple"},
        {"aol.com", "AOL"}, {"zoho.com", "Zoho"}
    };

    auto it = known.find(domain);
    results.Add("Domain", "Provider",
                it != known.end() ? it->second : "Custom/Corporate", "Local Parse");

    // Username extraction
    std::string username = email.substr(0, atPos);
    results.Add("Identity", "Username Candidate", username, "Local Parse");

    // Check for name patterns in username (john.doe, jdoe, john_doe)
    std::regex namePat(R"(([a-zA-Z]{2,})[\.\-_]([a-zA-Z]{2,}))");
    std::smatch m;
    if (std::regex_search(username, m, namePat)) {
        results.Add("Identity", "Possible First Name",
                    m[1].str(), "Username Pattern Analysis");
        results.Add("Identity", "Possible Last Name",
                    m[2].str(), "Username Pattern Analysis");
    }

    // MX record hint via DNS TXT — lightweight check using ip-api isn't applicable here
    // We'll at minimum flag domain for manual WHOIS
    results.Add("Domain", "WHOIS Recommended",
                "https://whois.domaintools.com/" + domain, "Local Parse");
}

void EmailModule::CheckHIBP(const std::string& email, ResultSet& results) {
    if (std::string(HIBP_KEY) == "YOUR_HIBP_API_KEY") {
        results.Add("Breach", "HIBP", "API key not configured", "HaveIBeenPwned");
        return;
    }

    HttpClient client;
    // URL-encode the @ symbol
    std::string encoded = email;
    auto pos = encoded.find('@');
    if (pos != std::string::npos) encoded.replace(pos, 1, "%40");

    std::wstring path = L"/api/v3/breachedaccount/" +
                        std::wstring(encoded.begin(), encoded.end()) +
                        L"?truncateResponse=false";

    auto resp = client.Get(L"haveibeenpwned.com", path,
                           {{L"hibp-api-key",
                             std::wstring(std::string(HIBP_KEY).begin(),
                                          std::string(HIBP_KEY).end())},
                            {L"User-Agent", L"PivotHarvest"}});

    if (resp.statusCode == 404) {
        results.Add("Breach", "Status", "No breaches found", "HaveIBeenPwned");
        return;
    }
    if (!resp.success) {
        results.AddError("HIBP: HTTP " + std::to_string(resp.statusCode));
        return;
    }

    try {
        auto j = json::parse(resp.body);
        results.Add("Breach", "Total Breaches Found",
                    std::to_string(j.size()), "HaveIBeenPwned");
        for (auto& breach : j) {
            std::string name  = breach.value("Name", "Unknown");
            std::string date  = breach.value("BreachDate", "Unknown");
            std::string count = std::to_string(breach.value("PwnCount", 0));
            results.Add("Breach", name,
                        "Date: " + date + " | Records: " + count,
                        "HaveIBeenPwned");
        }
    } catch (const std::exception& ex) {
        results.AddError(std::string("HIBP parse: ") + ex.what());
    }
}

void EmailModule::CheckHunter(const std::string& email, ResultSet& results) {
    if (std::string(HUNTER_KEY) == "YOUR_HUNTER_API_KEY") {
        results.Add("Professional", "Hunter.io", "API key not configured", "Hunter.io");
        return;
    }

    HttpClient client;
    std::wstring key = std::wstring(std::string(HUNTER_KEY).begin(),
                                    std::string(HUNTER_KEY).end());
    std::wstring em  = std::wstring(email.begin(), email.end());
    std::wstring path = L"/v2/email-verifier?email=" + em + L"&api_key=" + key;

    auto resp = client.Get(L"api.hunter.io", path);
    if (!resp.success) {
        results.AddError("Hunter.io: HTTP " + std::to_string(resp.statusCode));
        return;
    }

    try {
        auto j = json::parse(resp.body);
        auto& data = j["data"];
        auto addStr = [&](const std::string& cat,
                          const std::string& label,
                          const std::string& jkey) {
            if (data.contains(jkey) && !data[jkey].is_null()) {
                if (data[jkey].is_string())
                    results.Add(cat, label, data[jkey].get<std::string>(), "Hunter.io");
                else if (data[jkey].is_boolean())
                    results.Add(cat, label, data[jkey].get<bool>() ? "true" : "false", "Hunter.io");
            }
        };
        addStr("Email Validation", "Status",         "status");
        addStr("Email Validation", "Score",           "score");
        addStr("Identity",         "First Name",      "first_name");
        addStr("Identity",         "Last Name",       "last_name");
        addStr("Professional",     "Position",        "position");
        addStr("Professional",     "Company",         "company");
        addStr("Professional",     "LinkedIn",        "linkedin");
        addStr("Technical",        "Disposable",      "disposable");
        addStr("Technical",        "Webmail",         "webmail");
        addStr("Technical",        "MX Valid",        "mx_records");
        addStr("Technical",        "SMTP Valid",      "smtp_server");
    } catch (const std::exception& ex) {
        results.AddError(std::string("Hunter parse: ") + ex.what());
    }
}
