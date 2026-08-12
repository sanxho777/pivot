// ============================================================
// BaseModule.h
// ============================================================
#pragma once
#include <string>
#include <vector>
#include <map>

struct Finding {
    std::string category;   // e.g. "Geolocation", "Breach", "Carrier"
    std::string key;
    std::string value;
    std::string source;
};

struct ResultSet {
    std::string pivot;
    std::string pivotType;
    std::vector<Finding> findings;
    std::vector<std::string> errors;

    void Add(const std::string& cat, const std::string& key,
             const std::string& val, const std::string& src) {
        findings.push_back({cat, key, val, src});
    }

    void AddError(const std::string& err) {
        errors.push_back(err);
    }
};

class BaseModule {
public:
    virtual ~BaseModule() = default;
    virtual std::string Name() const = 0;
    virtual void Run(const std::string& pivot, ResultSet& results) = 0;
};
