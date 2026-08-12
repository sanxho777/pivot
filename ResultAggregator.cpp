// ============================================================
// ResultAggregator.cpp
// ============================================================
#include "ResultAggregator.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <map>
#include <iomanip>
#include <windows.h>

using json = nlohmann::json;

static void SetColor(WORD color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void ResultAggregator::Print(const ResultSet& results, std::ostream& out) {
    // Header
    SetColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    out << "\n╔══════════════════════════════════════════════════════════╗\n";
    out << "║           P I V O T H A R V E S T   R E S U L T S       ║\n";
    out << "╚══════════════════════════════════════════════════════════╝\n\n";
    SetColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    out << "  Pivot : " << results.pivot << "\n";
    out << "  Type  : " << results.pivotType << "\n";
    out << "  Total : " << results.findings.size() << " findings\n\n";

    // Group by category
    std::map<std::string, std::vector<const Finding*>> grouped;
    for (auto& f : results.findings)
        grouped[f.category].push_back(&f);

    for (auto& [cat, items] : grouped) {
        SetColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        out << "  ┌─ " << cat << " ";
        out << std::string(std::max(0, 52 - (int)cat.size()), '─') << "┐\n";
        SetColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

        for (auto* f : items) {
            out << "  │  ";
            SetColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            out << std::left << std::setw(28) << f->key;
            SetColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            out << f->value << "\n";
            out << "  │  ";
            SetColor(FOREGROUND_RED | FOREGROUND_BLUE);
            out << std::string(28, ' ') << "[src: " << f->source << "]\n";
            SetColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
        out << "  └" << std::string(55, '─') << "┘\n\n";
    }

    if (!results.errors.empty()) {
        SetColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        out << "  ── Errors ──────────────────────────────────────────────\n";
        for (auto& e : results.errors)
            out << "  ! " << e << "\n";
        SetColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        out << "\n";
    }
}

void ResultAggregator::ExportCSV(const ResultSet& results, const std::string& filepath) {
    std::ofstream f(filepath);
    f << "Pivot,PivotType,Category,Key,Value,Source\n";
    for (auto& r : results.findings) {
        // basic CSV escaping
        auto esc = [](std::string s) {
            if (s.find(',') != std::string::npos ||
                s.find('"') != std::string::npos) {
                std::string out = "\"";
                for (char c : s) {
                    if (c == '"') out += "\"\"";
                    else out += c;
                }
                out += "\"";
                return out;
            }
            return s;
        };
        f << esc(results.pivot) << ","
          << esc(results.pivotType) << ","
          << esc(r.category) << ","
          << esc(r.key) << ","
          << esc(r.value) << ","
          << esc(r.source) << "\n";
    }
}

void ResultAggregator::ExportJSON(const ResultSet& results, const std::string& filepath) {
    json j;
    j["pivot"]     = results.pivot;
    j["pivotType"] = results.pivotType;
    j["findings"]  = json::array();
    for (auto& r : results.findings) {
        j["findings"].push_back({
            {"category", r.category},
            {"key",      r.key},
            {"value",    r.value},
            {"source",   r.source}
        });
    }
    j["errors"] = results.errors;
    std::ofstream f(filepath);
    f << j.dump(2);
}
