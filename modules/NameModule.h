// ============================================================
// modules/NameModule.h
// ============================================================
#pragma once
#include "../BaseModule.h"
#include "../HttpClient.h"

class NameModule : public BaseModule {
public:
    std::string Name() const override { return "Name Intelligence"; }
    void Run(const std::string& pivot, ResultSet& results) override;

private:
    void GenerateSearchDorks(const std::string& name, ResultSet& results);
    void GenerateUsernameCandidates(const std::string& name, ResultSet& results);
    void GenerateEmailCandidates(const std::string& name, ResultSet& results);
};
