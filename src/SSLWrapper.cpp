
#include "SSLWrapper.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "exceptions.h"

void SSLWrapper::init() {
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();
}

SSLWrapper::SSLWrapper() {
    this->bio = nullptr;
    this->hostname = "";
}

SSLWrapper::~SSLWrapper() {
    BIO_free_all(this->bio);
    this->bio = nullptr;
}

void SSLWrapper::connect(const string &hostname) {
    this->hostname = hostname;
    this->bio = BIO_new_connect(hostname.c_str());
    if (this->bio == nullptr)
        throw SSLException("Could not open connection to " + hostname);

    if (BIO_do_connect(this->bio) <= 0)
        throw SSLException("Could not connect to " + hostname);
}

string SSLWrapper::read(int length) {
    char buffer[length+1];
    int read = BIO_read(this->bio, buffer, length);
    if (read <= 0)
        throw SSLException("Error reading data from " + this->hostname);
    return string(buffer);
}

void SSLWrapper::write(const string &msg) {
    if (BIO_puts(this->bio, msg.c_str()) <= 0)
        throw SSLException("Error sending data to " + this->hostname);
}

void SSLWrapper::close() {
    BIO_reset(this->bio);
}
