// ============================================================
// ResultAggregator.h
// ============================================================
#pragma once
#include "BaseModule.h"
#include <iostream>
#include <map>

class ResultAggregator {
public:
    void Print(const ResultSet& results, std::ostream& out = std::cout);
    void ExportCSV(const ResultSet& results, const std::string& filepath);
    void ExportJSON(const ResultSet& results, const std::string& filepath);
};
