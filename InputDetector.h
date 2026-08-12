// ============================================================
// InputDetector.h
// ============================================================
#pragma once
#include <string>

enum class PivotType {
    IP_ADDRESS,
    EMAIL,
    PHONE,
    NAME,
    UNKNOWN
};

class InputDetector {
public:
    static PivotType Classify(const std::string& input);
    static std::string TypeName(PivotType type);

private:
    static bool IsIPv4(const std::string& s);
    static bool IsEmail(const std::string& s);
    static bool IsPhone(const std::string& s);
};
