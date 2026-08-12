// ============================================================
// modules/IPModule.cpp
// ============================================================
#include "IPModule.h"
#include <nlohmann/json.hpp>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

using json = nlohmann::json;

void IPModule::Run(const std::string& pivot, ResultSet& results) {
    QueryGeoIP(pivot, results);
    QueryReverseDNS(pivot, results);
    QueryShodan(pivot, results);
}

void IPModule::QueryGeoIP(const std::string& ip, ResultSet& results) {
    HttpClient client;
    std::wstring path = L"/json/" + std::wstring(ip.begin(), ip.end());
    auto resp = client.Get(L"ip-api.com", path,
                           {}, INTERNET_DEFAULT_HTTP_PORT, false);

    if (!resp.success) {
        results.AddError("GeoIP: request failed (" + std::to_string(resp.statusCode) + ")");
        return;
    }

    try {
        auto j = json::parse(resp.body);
        if (j.value("status", "") == "success") {
            auto add = [&](const std::string& k, const std::string& jk) {
                if (j.contains(jk) && !j[jk].is_null())
                    results.Add("Geolocation", k, j[jk].get<std::string>(), "ip-api.com");
            };
            auto addNum = [&](const std::string& k, const std::string& jk) {
                if (j.contains(jk) && !j[jk].is_null())
                    results.Add("Geolocation", k,
                                std::to_string(j[jk].get<double>()), "ip-api.com");
            };

            add("Country",      "country");
            add("Country Code", "countryCode");
            add("Region",       "regionName");
            add("City",         "city");
            add("ZIP",          "zip");
            addNum("Latitude",  "lat");
            addNum("Longitude", "lon");
            add("Timezone",     "timezone");
            add("ISP",          "isp");
            add("Organization", "org");
            add("AS Number",    "as");
            add("Hostname",     "reverse");
        } else {
            results.AddError("GeoIP: " + j.value("message", "unknown error"));
        }
    } catch (const std::exception& ex) {
        results.AddError(std::string("GeoIP parse error: ") + ex.what());
    }
}

void IPModule::QueryReverseDNS(const std::string& ip, ResultSet& results) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &sa.sin_addr);

    char hostname[NI_MAXHOST] = {0};
    int ret = getnameinfo(reinterpret_cast<sockaddr*>(&sa),
                          sizeof(sa), hostname, NI_MAXHOST,
                          nullptr, 0, NI_NAMEREQD);
    if (ret == 0 && strlen(hostname) > 0) {
        results.Add("DNS", "Reverse Hostname", hostname, "WinSock/getnameinfo");
    } else {
        results.Add("DNS", "Reverse Hostname", "(no PTR record)", "WinSock/getnameinfo");
    }

    WSACleanup();
}

void IPModule::QueryShodan(const std::string& ip, ResultSet& results) {
    if (std::string(SHODAN_KEY) == "YOUR_SHODAN_API_KEY") return;

    HttpClient client;
    std::wstring path = L"/shodan/host/" +
                        std::wstring(ip.begin(), ip.end()) +
                        L"?key=" +
                        std::wstring(std::string(SHODAN_KEY).begin(),
                                     std::string(SHODAN_KEY).end());

    auto resp = client.Get(L"api.shodan.io", path);
    if (!resp.success) {
        results.AddError("Shodan: " + std::to_string(resp.statusCode));
        return;
    }

    try {
        auto j = json::parse(resp.body);
        if (j.contains("ports")) {
            std::string portList;
            for (auto& p : j["ports"]) {
                if (!portList.empty()) portList += ", ";
                portList += std::to_string(p.get<int>());
            }
            results.Add("Exposure", "Open Ports", portList, "Shodan");
        }
        if (j.contains("hostnames")) {
            for (auto& h : j["hostnames"])
                results.Add("Exposure", "Hostname", h.get<std::string>(), "Shodan");
        }
        if (j.contains("vulns")) {
            for (auto& [cve, _] : j["vulns"].items())
                results.Add("Vulnerabilities", "CVE", cve, "Shodan");
        }
        if (j.contains("os") && !j["os"].is_null())
            results.Add("System", "OS", j["os"].get<std::string>(), "Shodan");
    } catch (const std::exception& ex) {
        results.AddError(std::string("Shodan parse: ") + ex.what());
    }
}
