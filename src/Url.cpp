
#include "Url.h"

#include <regex>
#include <algorithm>

#include "exceptions.h"

Url::Url() {
    this->port = "80";
    this->protocol = "http";
    this->path = "/";
}

void Url::from_string(const string &url) {
    this->original = url;

    size_t pos = url.find("://");
    if (pos == string::npos) {
        pos = 0;
    } else {
        this->protocol = url.substr(0, pos);
        std::transform(this->protocol.begin(), this->protocol.end(), this->protocol.begin(), ::tolower);
        pos += 3;
    }

    size_t port_pos = url.find(':', pos);
    size_t path_pos = url.find('/', pos);
    if (port_pos != string::npos) {
        this->hostname = url.substr(pos, port_pos - pos);
        this->port = url.substr(port_pos + 1, path_pos - port_pos - 1);
    } else {
        this->hostname = url.substr(pos, path_pos - pos);
        if (this->is_https())
            this->port = "443";
    }

    if (path_pos != string::npos)
        this->path = url.substr(path_pos, string::npos);
}

void Url::validate() const {
    if (!std::regex_match(this->original, std::regex(R"(^([\w\d]+:\/\/)?[\w\d\.\-]+(:\d{1,5})?(\/.*)?$)")))
        throw UrlException("URL " + this->original + " není validní");

    if (this->protocol != "http" && this->protocol != "https")
        throw UrlException("Podporované jsou pouze protokoly HTTP a HTTPS, " + this->original + " používá " + this->get_protocol());

    try {
        int port;
        port = std::stoi(this->port);

        if (port < 0 || port > 65535)
            throw UrlException("URL port je mimo rozsah (" + this->port + ")");
    } catch (const std::out_of_range &e) {
        throw UrlException("URL port je mimo rozsah (" + this->port + ")");
    }
}

const string &Url::get_protocol() const {
    return this->protocol;
}

const string &Url::get_hostname() const {
    return this->hostname;
}

const string &Url::get_port() const {
    return this->port;
}

const string &Url::get_path() const {
    return this->path;
}

bool Url::is_https() const {
    return this->protocol == "https";
}

const string &Url::get_original() const {
    return this->original;
}

bool Url::operator==(const Url &url) const {
    return url.protocol == this->protocol
            && url.hostname == this->hostname
            && url.port == this->port
            && url.path == this->path;
}

bool Url::operator!=(const Url &url) const {
    return !(*this == url);
}

std::ostream &operator<<(std::ostream &os, const Url &url) {
    os << url.get_original();
    return os;
}
