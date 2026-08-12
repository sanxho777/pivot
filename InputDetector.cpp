// ============================================================
// InputDetector.cpp
// ============================================================
#include "InputDetector.h"
#include <regex>

PivotType InputDetector::Classify(const std::string& input) {
    if (IsIPv4(input))   return PivotType::IP_ADDRESS;
    if (IsEmail(input))  return PivotType::EMAIL;
    if (IsPhone(input))  return PivotType::PHONE;
    // Name is the fallback — anything with only letters/spaces/hyphens
    std::regex nameRx(R"(^[A-Za-z\s\-'\.]{2,60}$)");
    if (std::regex_match(input, nameRx)) return PivotType::NAME;
    return PivotType::UNKNOWN;
}

std::string InputDetector::TypeName(PivotType type) {
    switch (type) {
        case PivotType::IP_ADDRESS: return "IP Address";
        case PivotType::EMAIL:      return "Email";
        case PivotType::PHONE:      return "Phone Number";
        case PivotType::NAME:       return "Full Name";
        default:                    return "Unknown";
    }
}

bool InputDetector::IsIPv4(const std::string& s) {
    std::regex rx(R"(^(\d{1,3}\.){3}\d{1,3}$)");
    if (!std::regex_match(s, rx)) return false;
    std::istringstream ss(s);
    std::string tok;
    while (std::getline(ss, tok, '.')) {
        int v = std::stoi(tok);
        if (v < 0 || v > 255) return false;
    }
    return true;
}

bool InputDetector::IsEmail(const std::string& s) {
    std::regex rx(R"(^[^\s@]+@[^\s@]+\.[^\s@]{2,}$)");
    return std::regex_match(s, rx);
}

bool InputDetector::IsPhone(const std::string& s) {
    std::regex rx(R"(^[\+]?[(]?[0-9]{1,4}[)]?[-\s\.]?[(]?[0-9]{1,4}[)]?[-\s\.]?[0-9]{4,9}$)");
    return std::regex_match(s, rx);
}
