
#include "Http.h"

#include <sstream>
#include <memory>

#include "debug.h"
#include "utils.h"
#include "exceptions.h"

#define READ_BUFF_LEN 1024
#define CRLF "\r\n"
#define CRLF_LEN 2

using std::istringstream;
using std::getline;

string Http::get_request(SSLWrapper *ssl, const Url &url){
    HttpResponse response{};

    if (url.is_https())
        ssl->setup_ssl();

    ssl->connect(url);
    ssl->write(create_get_request(url));
    read_response(ssl, response);
    ssl->close();

    if (response.status_code < 200 || (response.status_code > 200 && response.status_code < 400))
        throw UnsupportedHttpStatusException("Unsupported HTTP response code: "+response.status_reason);
    else if (response.status_code >= 400)
        throw HttpException("Server returned error status code: "+response.status_reason);

    return std::move(response.content);
}

void Http::read_response(SSLWrapper *ssl, HttpResponse &response) {
    string response_str{};
    size_t header_end;

    do {
        response_str += ssl->read(READ_BUFF_LEN);
        header_end = response_str.find(CRLF CRLF);
    } while (header_end == string::npos);

    parse_header(response_str.substr(0, header_end), response);

    if (response.transfer_coding == "chunked") {
        response.content = read_chunked_content(ssl, response_str.substr(header_end + 4));
    } else if (response.content_len > 0) {
        while (ssl->should_retry() || response_str.length() - header_end - 4 < response.content_len) {
            response_str += ssl->read(READ_BUFF_LEN);
        }

        response.content = response_str.substr(header_end + 4);
    }


    debug(response);
}

void Http::parse_header(const string &header, HttpResponse &http_response) {
    istringstream str_stream{header};
    string line;

    while (getline(str_stream, line)) {
        if (str_starts_with(line, "HTTP/")) {
            http_response.status_code = std::stoi(line.substr(9, 3));
            http_response.status_reason = trim(line.substr(9));
        } else if (str_starts_with(line, "Content-Type:")) {
            http_response.content_type = trim(line.substr(13, line.find(';', 13) - 13));
        } else if (str_starts_with(line, "Content-Length:")) {
            http_response.content_len = std::stoul(trim(line.substr(15)));
        } else if (str_starts_with(line, "Transfer-Encoding:")) {
            http_response.transfer_coding = trim(line.substr((18)));
        }
    }
}

string Http::read_chunked_content(SSLWrapper *ssl, const string &content_start) {
    std::ostringstream content{};
    string buffer = content_start;
    size_t chunk_size;

    do {
        if (buffer.find(CRLF) == string::npos)  // If we don't have the whole chunk-size in buffer, read more
            buffer += ssl->read(READ_BUFF_LEN);

        chunk_size = std::stoul(buffer.substr(0, buffer.find_first_of(" \t" CRLF)), nullptr, 16);
        buffer.erase(0, buffer.find(CRLF) + CRLF_LEN);

        while (buffer.length() < chunk_size + CRLF_LEN) {
            buffer += ssl->read(READ_BUFF_LEN);
        }

        content << buffer.substr(0, chunk_size);
        buffer.erase(0, chunk_size + CRLF_LEN);
    } while(chunk_size != 0);

    return content.str();
}

string Http::create_get_request(const Url &url) {
    std::ostringstream ostream{};

    ostream << "GET " << url.get_path() << " HTTP/1.1" CRLF;
    ostream << "Host: " << url.get_hostname() << CRLF;
//    ostream << "Accept: text/xml" CRLF;
    ostream << "Connection: close" CRLF;
    ostream << "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:59.0) Gecko/20100101 Firefox/59.0" CRLF;
    ostream << "Cache-Control: no-cache, no-store, must-revalidate" CRLF;
    ostream << CRLF;

    return ostream.str();
}

std::ostream &operator<<(std::ostream &os, const HttpResponse &response) {
    os << "Status code: " << response.status_code << std::endl;
    os << "Status reason: " << response.status_reason << std::endl;
    os << "Transfer coding: " << response.transfer_coding << std::endl;
    os << "Content type: " << response.content_type << std::endl;
    os << "Content length: " << response.content_len << std::endl;
    os << "Content: " << response.content << std::endl;

    return os;
}
