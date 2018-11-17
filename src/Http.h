
#ifndef ISA_HTTP_H
#define ISA_HTTP_H

#include <string>
#include <ostream>

#include "SSLWrapper.h"
#include "Url.h"

using std::string;

typedef struct {
    int status_code;
    string status_reason;
    string transfer_coding;
    string content_type;
    size_t content_len;
    string content;
} HttpResponse;

std::ostream& operator<<(std::ostream& os, const HttpResponse& response);

class Http {
public:
    /**
     * HTTP GET request
     * @param ssl SSLWrapper instance to use
     * @param url url to request
     * @return response data
     */
    static string get_request(SSLWrapper* ssl, const Url &url);

private:
    typedef struct {
        size_t offset;
        int length;
    } LineFeedPos;

    /**
     * Read HTTP response into response parameter
     * @param ssl SSLWrapper instance to use
     * @param response response output parameter
     */
    static void read_response(SSLWrapper* ssl, HttpResponse &response);

    /**
     * Parse HTTP header from string into http_response
     * @param header string with HTTP header
     * @param http_response parsed header output parameter
     */
    static void parse_header(const string &header, HttpResponse &http_response);

    /**
     * Read HTTP content in chunked encoding
     * @param chunked_content content in chunked encoding
     * @return parsed content
     */
    static string read_chunked_content(const string &chunked_content);

    /**
     * Create HTTP GET request string for given URL
     * @param url URL of the request
     * @return request string
     */
    static string create_get_request(const Url &url);

    /**
     * Find first sequence of line feeds in string, searches for LF and CRLF
     * @param str searched string
     * @param n length of line feed sequence to find
     * @param pos position to search from
     * @return position and length of line feed sequence in LineFeedPos structure
     */
    static LineFeedPos find_line_feeds(const string &str, int n = 1, size_t pos = 0);
};


#endif //ISA_HTTP_H
