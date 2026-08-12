// ============================================================
// HttpClient.h
// ============================================================
#pragma once
#include <string>
#include <map>
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

struct HttpResponse {
    int statusCode;
    std::string body;
    bool success;
};

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    HttpResponse Get(const std::wstring& host,
                     const std::wstring& path,
                     const std::map<std::wstring, std::wstring>& headers = {},
                     INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT,
                     bool useSSL = true);

private:
    HINTERNET hSession_;

    std::string WideToUtf8(const std::wstring& wide);
};
