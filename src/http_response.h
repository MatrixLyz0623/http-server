#pragma once
#include <string>
#include <unordered_map>

class HttpResponse {
public:
    static HttpResponse ok(const std::string& body = "");
    static HttpResponse notFound();

    HttpResponse(const HttpResponse& other);
    HttpResponse(HttpResponse&& other);
    HttpResponse(int status_code,
                 std::string status_text,
                 std::string content_type,
                 std::string body
                 );

    std::string to_string() const;
    std::unordered_map<std::string, std::string>& headers() {
        return headers_;
    }
private:
    int status_code_;
    std::string status_text_;
    std::string content_type_;
    std::string body_;
    std::unordered_map<std::string, std::string> headers_;
};
