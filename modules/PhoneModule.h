// ============================================================
// modules/PhoneModule.h
// ============================================================
#pragma once
#include "../BaseModule.h"
#include "../HttpClient.h"

class PhoneModule : public BaseModule {
public:
    std::string Name() const override { return "Phone Intelligence"; }
    void Run(const std::string& pivot, ResultSet& results) override;

private:
    void NormalizeAndParse(const std::string& phone, ResultSet& results);
    void QueryNumverify(const std::string& phone, ResultSet& results);
    void SuggestSocialPivots(const std::string& phone, ResultSet& results);

    static constexpr const char* NUMVERIFY_KEY = "YOUR_NUMVERIFY_API_KEY";
};
