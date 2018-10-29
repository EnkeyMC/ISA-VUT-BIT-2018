
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
     * @param ssl ssl bio socket pointer connected to host
     * @param url url to request
     * @return response data
     */
    static string get_request(SSLWrapper* ssl, const Url &url);

private:
    typedef struct {
        size_t offset;
        int length;
    } LineFeedPos;
    
    static void read_response(SSLWrapper* ssl, HttpResponse &response);
    static void parse_header(const string &header, HttpResponse &http_response);
    static string read_chunked_content(const string &content_start);
    static string create_get_request(const Url &url);
    static LineFeedPos find_line_feeds(const string &str, int n = 1, size_t pos = 0);
};


#endif //ISA_HTTP_H
