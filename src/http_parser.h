#pragma once
#include <string>
#include <unordered_map>

class HttpRequestParser {
public:
    bool parse(const std::string& buffer);
    const std::string& get_method() const;
    const std::string& get_path() const;
    const std::string& get_body() const;
    std::string get_header(const std::string& key) const;

private:
    std::string method;
    std::string path;
    std::string body;
    std::unordered_map<std::string , std::string> header;
};

std::string trim(const std::string& s);
