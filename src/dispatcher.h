#pragma once
#include "http_parser.h"
#include "http_response.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

class Dispatcher {
public:
    using Handler = std::function<HttpResponse(const HttpRequestParser&)>;

    void registerHandler(const std::string& method, const std::string& path, Handler handler);
    void registerHandlerPrefix(const std::string& method, const std::string& prefix, Handler handler);
    HttpResponse dispatch(const HttpRequestParser& req) const;

private:
    std::unordered_map<std::string, std::unordered_map<std::string, Handler>> exactRoutes_;
    std::unordered_map<std::string, std::vector<std::pair<std::string, Handler>>> prefixRoutes_;
};
