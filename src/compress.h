#pragma once
#include <string>

enum class CompressionType {
    None,
    Gzip,
    Brotli,
    Deflate
};

CompressionType select_compression(const std::string& accept_encoding);
std::string gzip_compress(const std::string& input);