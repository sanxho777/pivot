// ============================================================
// modules/EmailModule.h
// ============================================================
#pragma once
#include "../BaseModule.h"
#include "../HttpClient.h"

class EmailModule : public BaseModule {
public:
    std::string Name() const override { return "Email Intelligence"; }
    void Run(const std::string& pivot, ResultSet& results) override;

private:
    void ExtractDomainIntel(const std::string& email, ResultSet& results);
    void CheckHIBP(const std::string& email, ResultSet& results);
    void CheckHunter(const std::string& email, ResultSet& results);

    static constexpr const char* HIBP_KEY    = "YOUR_HIBP_API_KEY";
    static constexpr const char* HUNTER_KEY  = "YOUR_HUNTER_API_KEY";
};
