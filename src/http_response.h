#pragma once
#include <string>

class HttpResponse {
public:
    static HttpResponse ok(const std::string& body = "");
    static HttpResponse notFound();

    HttpResponse(const HttpResponse& other);
    HttpResponse(HttpResponse&& other);
    HttpResponse(int status_code,
                 std::string status_text,
                 std::string content_type,
                 std::string body);

    std::string to_string() const;

private:
    int status_code_;
    std::string status_text_;
    std::string content_type_;
    std::string body_;
};
