// ============================================================
// modules/IPModule.h
// ============================================================
#pragma once
#include "../BaseModule.h"
#include "../HttpClient.h"

class IPModule : public BaseModule {
public:
    std::string Name() const override { return "IP Intelligence"; }
    void Run(const std::string& pivot, ResultSet& results) override;

private:
    void QueryGeoIP(const std::string& ip, ResultSet& results);
    void QueryReverseDNS(const std::string& ip, ResultSet& results);
    void QueryShodan(const std::string& ip, ResultSet& results);

    // Slot your Shodan API key here
    static constexpr const char* SHODAN_KEY = "YOUR_SHODAN_API_KEY";
};
