#include "compress.h"
#include <zlib.h>
#include <sstream>
#include <unordered_map>
#include <algorithm>
std::string trim(std::string str) {
    auto start = std::find_if_not(str.begin(), str.end(), ::isspace);
    auto end = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
    return (start < end ? std::string(start, end) : "");
}
CompressionType select_compression(const std::string& accept_encoding)
{
    static std::unordered_map<std::string ,CompressionType> supperCompresstype{
        {"gzip", CompressionType::Gzip}
    };
    std::istringstream iss(accept_encoding);
    std::string type;
    while(std::getline(iss, type, ',')) {
        type = trim(type);
        if(supperCompresstype.find(type) != supperCompresstype.end()) {
            return supperCompresstype[type];
        }
    }
    return CompressionType::None;
}

std::string gzip_compress(const std::string& input)
{
    z_stream zs{};
    deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                 15 + 16, 8, Z_DEFAULT_STRATEGY);

    zs.next_in = (Bytef*)input.data();
    zs.avail_in = input.size();

    char outbuffer[32768];
    std::string outstring;
    int ret;

    do {
        zs.next_out = (Bytef*)outbuffer;
        zs.avail_out = sizeof(outbuffer);
        ret = deflate(&zs, Z_FINISH);
        outstring.append(outbuffer, sizeof(outbuffer) - zs.avail_out);
    } while (ret == Z_OK);

    deflateEnd(&zs);
    return outstring;
}