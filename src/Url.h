
#ifndef ISA_URL_H
#define ISA_URL_H

#include <string>

using std::string;

class Url {
public:
    Url();

    void from_string(const string &url);
    void validate() const;

    const string & get_protocol() const;
    const string & get_hostname() const;
    const string & get_port() const;
    const string & get_path() const;

private:
    string original;

    string protocol;
    string hostname;
    string port;
    string path;
};


#endif //ISA_URL_H
