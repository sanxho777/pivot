// ============================================================
// main.cpp
// ============================================================
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <windows.h>

#include "InputDetector.h"
#include "ResultAggregator.h"
#include "modules/IPModule.h"
#include "modules/EmailModule.h"
#include "modules/PhoneModule.h"
#include "modules/NameModule.h"

static void PrintBanner() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(h, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << R"(
  ██████╗ ██╗██╗   ██╗ ██████╗ ████████╗
  ██╔══██╗██║██║   ██║██╔═══██╗╚══██╔══╝
  ██████╔╝██║██║   ██║██║   ██║   ██║   
  ██╔═══╝ ██║╚██╗ ██╔╝██║   ██║   ██║   
  ██║     ██║ ╚████╔╝ ╚██████╔╝   ██║   
  ╚═╝     ╚═╝  ╚═══╝   ╚═════╝    ╚═╝   
  ██╗  ██╗ █████╗ ██████╗ ██╗   ██╗███████╗███████╗████████╗
  ██║  ██║██╔══██╗██╔══██╗██║   ██║██╔════╝██╔════╝╚══██╔══╝
  ███████║███████║██████╔╝██║   ██║█████╗  ███████╗   ██║   
  ██╔══██║██╔══██║██╔══██╗╚██╗ ██╔╝██╔══╝  ╚════██║   ██║   
  ██║  ██║██║  ██║██║  ██║ ╚████╔╝ ███████╗███████║   ██║   
  ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝  ╚═══╝  ╚══════╝╚══════╝   ╚═╝   
)" << "\n";
    SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << "  OSINT Pivot Engine  |  v1.0  |  Windows 11\n";
    std::cout << "  ─────────────────────────────────────────────────────\n\n";
}

int main(int argc, char* argv[]) {
    // Enable UTF-8 console output
    SetConsoleOutputCP(CP_UTF8);
    PrintBanner();

    std::string pivot;

    if (argc > 1) {
        pivot = argv[1];
    } else {
        std::cout << "  Enter pivot (email / IP / phone / name): ";
        std::getline(std::cin, pivot);
    }

    if (pivot.empty()) {
        std::cerr << "  [!] No input provided.\n";
        return 1;
    }

    PivotType type = InputDetector::Classify(pivot);
    std::string typeName = InputDetector::TypeName(type);

    std::cout << "\n  Classified as: " << typeName << "\n";
    std::cout << "  Running intelligence modules...\n\n";

    ResultSet results;
    results.pivot     = pivot;
    results.pivotType = typeName;

    // Load appropriate module(s)
    std::vector<std::unique_ptr<BaseModule>> modules;

    switch (type) {
        case PivotType::IP_ADDRESS:
            modules.push_back(std::make_unique<IPModule>());
            break;
        case PivotType::EMAIL:
            modules.push_back(std::make_unique<EmailModule>());
            break;
        case PivotType::PHONE:
            modules.push_back(std::make_unique<PhoneModule>());
            break;
        case PivotType::NAME:
            modules.push_back(std::make_unique<NameModule>());
            break;
        default:
            std::cerr << "  [!] Could not classify input. Try quoting a full name.\n";
            return 1;
    }

    for (auto& mod : modules) {
        std::cout << "  [~] " << mod->Name() << "...\n";
        mod->Run(pivot, results);
    }

    ResultAggregator agg;
    agg.Print(results);

    // Export options
    std::cout << "  Export results? [j]son / [c]sv / [n]o: ";
    char choice;
    std::cin >> choice;
    if (choice == 'j' || choice == 'J') {
        agg.ExportJSON(results, "results.json");
        std::cout << "  Saved to results.json\n";
    } else if (choice == 'c' || choice == 'C') {
        agg.ExportCSV(results, "results.csv");
        std::cout << "  Saved to results.csv\n";
    }

    return 0;
}
