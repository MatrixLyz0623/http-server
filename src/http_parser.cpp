#include "http_parser.h"
#include <sstream>
#include <algorithm>

std::string trim(const std::string& s) {
    auto start = std::find_if_not(s.begin(), s.end(), ::isspace);
    auto end = std::find_if_not(s.rbegin(), s.rend(), ::isspace).base();
    return (start < end ? std::string(start, end) : "");
}

bool HttpRequestParser::parse(const std::string& buffer) {
    std::istringstream stream(buffer);
    std::string line;

    if (std::getline(stream, line)) {
        std::istringstream linestream(line);
        linestream >> method >> path;
    }

    while (std::getline(stream, line) && line != "\r") {
        auto pos = line.find(":");
        if (pos != std::string::npos) {
            std::string key = trim(line.substr(0, pos));
            std::string value = trim(line.substr(pos + 1));
            header[key] = value;
        }
    }
    if(header.find("Content-Length") != header.end()) {
        ssize_t headerEnd = buffer.find("\r\n\r\n");
        body = buffer.substr(headerEnd+4);
    }

    return true;
}

const std::string& HttpRequestParser::get_method() const { return method; }
const std::string& HttpRequestParser::get_path() const { return path; }
std::string HttpRequestParser::get_header(const std::string& key) const {
    auto it = header.find(key);
    return it != header.end() ? it->second : "";
}

const std::string& HttpRequestParser::get_body() const {return body;}
