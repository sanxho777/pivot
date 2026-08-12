// ============================================================
// modules/NameModule.cpp
// ============================================================
#include "NameModule.h"
#include <sstream>
#include <algorithm>
#include <regex>

void NameModule::Run(const std::string& pivot, ResultSet& results) {
    GenerateSearchDorks(pivot, results);
    GenerateUsernameCandidates(pivot, results);
    GenerateEmailCandidates(pivot, results);
}

void NameModule::GenerateSearchDorks(const std::string& name, ResultSet& results) {
    // URL-encode spaces as +
    std::string encoded = name;
    std::replace(encoded.begin(), encoded.end(), ' ', '+');

    results.Add("Search Dork", "Google",
                "https://www.google.com/search?q=\"" + name + "\"",
                "Generated Pivot");
    results.Add("Search Dork", "LinkedIn",
                "https://www.linkedin.com/search/results/people/?keywords=" + encoded,
                "Generated Pivot");
    results.Add("Search Dork", "Facebook",
                "https://www.facebook.com/search/people?q=" + encoded,
                "Generated Pivot");
    results.Add("Search Dork", "Twitter/X",
                "https://twitter.com/search?q=\"" + name + "\"&f=user",
                "Generated Pivot");
    results.Add("Search Dork", "Instagram",
                "https://www.google.com/search?q=site:instagram.com+\"" + name + "\"",
                "Generated Pivot");
    results.Add("Search Dork", "Whitepages",
                "https://www.whitepages.com/name/" + encoded,
                "Generated Pivot");
    results.Add("Search Dork", "Spokeo",
                "https://www.spokeo.com/" + encoded,
                "Generated Pivot");
    results.Add("Search Dork", "Pipl",
                "https://pipl.com/search/?q=" + encoded,
                "Generated Pivot");
    results.Add("Search Dork", "Intelius",
                "https://www.intelius.com/results.php?ReportType=1&searchFirstName=" +
                name.substr(0, name.find(' ')) +
                "&searchLastName=" + name.substr(name.find(' ') + 1),
                "Generated Pivot");
}

void NameModule::GenerateUsernameCandidates(const std::string& name, ResultSet& results) {
    std::istringstream ss(name);
    std::vector<std::string> parts;
    std::string tok;
    while (ss >> tok) {
        std::transform(tok.begin(), tok.end(), tok.begin(), ::tolower);
        parts.push_back(tok);
    }

    if (parts.empty()) return;

    std::vector<std::string> candidates;

    if (parts.size() >= 2) {
        std::string f = parts[0], l = parts.back();
        candidates = {
            f + l,                          // johnsmith
            f + "." + l,                    // john.smith
            f + "_" + l,                    // john_smith
            f[0] + l,                       // jsmith
            f + std::string(1, l[0]),       // johns
            l + f,                          // smithjohn
            l + "." + f,                    // smith.john
            f + l.substr(0, 3),             // johnsmi
            l + std::string(1, f[0]),       // smithj
            std::string(1,f[0]) + "." + l   // j.smith
        };
    } else {
        candidates.push_back(parts[0]);
    }

    for (auto& c : candidates) {
        results.Add("Username Candidate", c,
                    "https://whatsmyname.app/?q=" + c,
                    "Pattern Generation");
    }
}

void NameModule::GenerateEmailCandidates(const std::string& name, ResultSet& results) {
    std::istringstream ss(name);
    std::vector<std::string> parts;
    std::string tok;
    while (ss >> tok) {
        std::transform(tok.begin(), tok.end(), tok.begin(), ::tolower);
        parts.push_back(tok);
    }

    if (parts.size() < 2) return;

    std::string f = parts[0], l = parts.back();
    std::vector<std::string> domains = {
        "gmail.com", "outlook.com", "yahoo.com", "icloud.com", "protonmail.com"
    };
    std::vector<std::string> patterns = {
        f + "." + l,
        f + l,
        f[0] + l,
        f + std::string(1, l[0]),
        l + "." + f
    };

    for (auto& pat : patterns) {
        for (auto& dom : domains) {
            results.Add("Email Candidate", pat + "@" + dom,
                        "Verify: https://hunter.io/email-verifier/" + pat + "@" + dom,
                        "Pattern Generation");
        }
    }
}
