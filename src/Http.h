
#ifndef ISA_HTTP_H
#define ISA_HTTP_H

#include <string>

#include "SSLWrapper.h"
#include "Url.h"

using std::string;

class Http {
public:
    /**
     * HTTP request to url of given type
     * @param ssl ssl bio socket pointer connected to host
     * @param url url to request
     * @param type request type (Http::GET by default)
     * @return response data
     */
    static string request(SSLWrapper* ssl, const Url &url, const string &type = Http::GET);

    static const string GET;
};


#endif //ISA_HTTP_H
