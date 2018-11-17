
#include "Http.h"

#include <sstream>
#include <memory>

#include "debug.h"
#include "utils.h"
#include "exceptions.h"

#define CRLF "\r\n"
#define CRLF_LEN 2
#define LF '\n'

using std::istringstream;
using std::getline;

string Http::get_request(SSLWrapper *ssl, const Url &url) {
    HttpResponse response{};

    if (url.is_https())
        ssl->setup_ssl();

    ssl->connect(url);
    ssl->write(create_get_request(url));
    read_response(ssl, response);
    ssl->close();

    if (response.status_code < 200 || (response.status_code > 200 && response.status_code < 400))
        throw UnsupportedHttpStatusException("Server vrátil nepodporovaný status kód: " + response.status_reason);
    else if (response.status_code >= 400)
        throw HttpException("Server vrátil chybu: " + response.status_reason);

    return std::move(response.content);
}

void Http::read_response(SSLWrapper *ssl, HttpResponse &response) {
    string response_str{};
    LineFeedPos header_end{};

    response_str = ssl->read();

    header_end = find_line_feeds(response_str, 2);
    
    if (header_end.offset == string::npos)
        throw HttpException("Odpověď serveru není validní HTTP odpověď");

    parse_header(response_str.substr(0, header_end.offset), response);
    if (response.status_reason.empty())
        throw HttpException("Odpověď serveru není validní HTTP odpověď");
    if (response.status_code != 200)
        return;

    if (response.transfer_coding == "chunked") {
        response.content = read_chunked_content(response_str.substr(header_end.offset + header_end.length));
    } else {
        response.content = response_str.substr(header_end.offset + header_end.length);
    }
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

string Http::read_chunked_content(const string &chunked_content) {
    std::ostringstream content{};
    string buffer = chunked_content;
    size_t chunk_size;
    LineFeedPos lf_pos{};

    do {
        chunk_size = std::stoul(buffer.substr(0, buffer.find_first_of(" \t" CRLF)), nullptr, 16);
        lf_pos = find_line_feeds(buffer);
        buffer.erase(0, lf_pos.offset + lf_pos.length);

        content << buffer.substr(0, chunk_size);
        lf_pos = find_line_feeds(buffer, 1, chunk_size);
        buffer.erase(0, chunk_size + lf_pos.length);
    } while (chunk_size != 0);

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

Http::LineFeedPos Http::find_line_feeds(const string &str, int n, size_t pos) {
    string lfs {};
    for (int i = 0; i < n; i++)
        lfs += CRLF;
    
    size_t position = str.find(lfs, pos);
    LineFeedPos result{};
    if (position != string::npos) {
        result.offset = position;
        result.length = CRLF_LEN*n;
        return result;
    }
    
    lfs = "";
    for (int i = 0; i < n; i++)
        lfs += LF;
    
    position = str.find(lfs, pos);
    result.offset = position;
    result.length = n;
    return result;
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
