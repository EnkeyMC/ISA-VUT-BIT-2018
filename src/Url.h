
#ifndef ISA_URL_H
#define ISA_URL_H

#include <string>

using std::string;

class Url {
public:
    /**
     * Create blank URL
     */
    Url();

    /**
     * Parse URL from string
     * @param url url string
     */
    void from_string(const string &url);
    /**
     * Validate this URL
     * @throws UrlException if URL is not valid
     */
    void validate() const;

    /**
     * Get protocol part of URL
     * @return URL protocol
     */
    const string & get_protocol() const;
    /**
     * Get hostname part of URL
     * @return URL hostname
     */
    const string & get_hostname() const;
    /**
     * Get URL port
     * @return URL port
     */
    const string & get_port() const;
    /**
     * Get path part of URL
     * @return URL path
     */
    const string & get_path() const;

private:
    string original;

    string protocol;
    string hostname;
    string port;
    string path;
};


#endif //ISA_URL_H
