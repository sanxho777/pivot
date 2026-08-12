// ============================================================
// HttpClient.cpp
// ============================================================
#include "HttpClient.h"
#include <stdexcept>
#include <sstream>

HttpClient::HttpClient() {
    hSession_ = WinHttpOpen(
        L"PivotHarvest/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );
    if (!hSession_)
        throw std::runtime_error("WinHttpOpen failed");
}

HttpClient::~HttpClient() {
    if (hSession_) WinHttpCloseHandle(hSession_);
}

HttpResponse HttpClient::Get(const std::wstring& host,
                              const std::wstring& path,
                              const std::map<std::wstring, std::wstring>& headers,
                              INTERNET_PORT port,
                              bool useSSL) {
    HttpResponse resp{0, "", false};

    HINTERNET hConnect = WinHttpConnect(hSession_, host.c_str(), port, 0);
    if (!hConnect) return resp;

    DWORD flags = useSSL ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect, L"GET", path.c_str(),
        nullptr, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, flags
    );
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        return resp;
    }

    for (auto& [k, v] : headers) {
        std::wstring hdr = k + L": " + v + L"\r\n";
        WinHttpAddRequestHeaders(hRequest, hdr.c_str(), (ULONG)-1L,
                                 WINHTTP_ADDREQ_FLAG_ADD);
    }

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        !WinHttpReceiveResponse(hRequest, nullptr)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        return resp;
    }

    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest,
                        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX,
                        &statusCode, &statusCodeSize,
                        WINHTTP_NO_HEADER_INDEX);
    resp.statusCode = static_cast<int>(statusCode);

    std::string body;
    DWORD bytesAvail = 0;
    while (WinHttpQueryDataAvailable(hRequest, &bytesAvail) && bytesAvail > 0) {
        std::vector<char> buf(bytesAvail + 1, 0);
        DWORD bytesRead = 0;
        WinHttpReadData(hRequest, buf.data(), bytesAvail, &bytesRead);
        body.append(buf.data(), bytesRead);
    }

    resp.body = body;
    resp.success = (statusCode >= 200 && statusCode < 300);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    return resp;
}

std::string HttpClient::WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) return {};
    int sz = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string out(sz - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, out.data(), sz, nullptr, nullptr);
    return out;
}
