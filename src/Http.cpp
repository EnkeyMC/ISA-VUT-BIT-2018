
#include "Http.h"

const string Http::GET = "GET";

string Http::request(SSLWrapper *ssl, const Url &url, const string &type) {
    if (url.is_https())
        ssl->setup_ssl();

    ssl->connect(url.get_hostname() + ":" + url.get_port());
    ssl->write("GET "+url.get_path()+" HTTP/1.1\nHost: "+url.get_hostname()+"\n\n");
    string out = ssl->read(4096);
    ssl->close();

    return out;
}
