#include "http_response.h"
#include <iostream>
#include <sstream>

HttpResponse::HttpResponse(const HttpResponse& other)
    : status_code_(other.status_code_),
      status_text_(other.status_text_),
      content_type_(other.content_type_),
      body_(other.body_),
      headers_(other.headers_) {
    std::cout << "copy construct" << std::endl;
}

HttpResponse::HttpResponse(HttpResponse&& other)
    : status_code_(other.status_code_),
      status_text_(std::move(other.status_text_)),
      content_type_(std::move(other.content_type_)),
      body_(std::move(other.body_)),
      headers_(std::move(other.headers_)) {
    std::cout << "move construct" << std::endl;
}

HttpResponse::HttpResponse(int status_code,
                           std::string status_text,
                           std::string content_type,
                           std::string body
                           )
    : status_code_(status_code),
      status_text_(std::move(status_text)),
      content_type_(std::move(content_type)),
      body_(std::move(body))
       {
    std::cout << "construct" << std::endl;
    if (!content_type_.empty()) {
        headers_["Content-Type"] = content_type_;
    }
    headers_["Content-Length"] = std::to_string(body_.size());
}

HttpResponse HttpResponse::ok(const std::string& body) {
    std::cout << "call OK" << std::endl;
    return HttpResponse(200, "OK", "text/plain", body);
}

HttpResponse HttpResponse::notFound() {
    return HttpResponse(404, "NotFound", "text/plain", "404 Not Found");
}

std::string HttpResponse::to_string() const {
    std::ostringstream os;
    os << "HTTP/1.1 " << status_code_ << " " << status_text_ << "\r\n";

    if (!content_type_.empty()) {
        os << "Content-Type: " << content_type_ << "\r\n";
    }

    // 写入自定义 headers
    for (const auto& [key, value] : headers_) {
        os << key << ": " << value << "\r\n";
    }

    // 确保 Content-Length 总是写入（如果未指定）
    if (headers_.count("Content-Length") == 0) {
        os << "Content-Length: " << body_.size() << "\r\n";
    }

    os << "\r\n";        // 空行分隔 header 与 body
    os << body_;         // 写入 body 内容
    return os.str();
}