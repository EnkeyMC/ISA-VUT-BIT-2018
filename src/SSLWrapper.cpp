
#include "SSLWrapper.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "exceptions.h"

void SSLWrapper::init() {
    SSL_load_error_strings();
    SSL_library_init();
    ERR_load_BIO_strings();
//    OpenSSL_add_all_algorithms();
}

SSLWrapper::SSLWrapper() {
    this->bio = nullptr;
    this->ssl_ctx = nullptr;
    this->ssl = nullptr;
}

SSLWrapper::~SSLWrapper() {
    FIPS_mode_set(0);
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();
    BIO_free_all(this->bio);
    this->bio = nullptr;
    if (this->ssl_ctx != nullptr)
        SSL_CTX_free(this->ssl_ctx);
}

void SSLWrapper::connect(const Url &url) {
    this->url = url;

    if (this->ssl_ctx != nullptr)
        this->connect_secure();
    else
        this->connect_insecure();
}

string SSLWrapper::read(int length) {
    char buffer[length+1];
    int read = BIO_read(this->bio, buffer, length);
    if (read <= 0)
        throw SSLException("Error reading data from " + this->url.get_hostname());
    buffer[read] = '\0';
    return string(buffer);
}

void SSLWrapper::write(const string &msg) {
    if (BIO_puts(this->bio, msg.c_str()) <= 0)
        throw SSLException("Error sending data to " + this->url.get_hostname());
}

void SSLWrapper::close() {
    BIO_reset(this->bio);
}

void SSLWrapper::setup_ssl(const string &ca, const string &cadir) {
    this->ssl_ctx = SSL_CTX_new(SSLv23_client_method());
    if (this->ssl_ctx == nullptr)
        throw SSLException(this->get_error_str());

    if (ca.empty() && cadir.empty()) {
        if (SSL_CTX_set_default_verify_paths(this->ssl_ctx) == 0)
            throw SSLException(this->get_error_str());
    } else {
        const char *cafile_p, *cadir_p;
        cafile_p = ca.empty() ? nullptr : ca.c_str();
        cadir_p = cadir.empty() ? nullptr : cadir.c_str();

        if (!SSL_CTX_load_verify_locations(this->ssl_ctx, cafile_p, cadir_p)) {
            throw SSLException(this->get_error_str());
        }
    }
}

void SSLWrapper::connect_insecure() {
    this->bio = BIO_new_connect((this->url.get_hostname() + ":" + this->url.get_port()).c_str());
    if (this->bio == nullptr)
        throw SSLException("Could not open connection to " + url.get_hostname() + " (Reason: " + this->get_error_str() + ")");

    if (BIO_do_connect(this->bio) <= 0)
        throw SSLException("Could not connect to " + url.get_hostname() + " (Reason: " + this->get_error_str() + ")");
}

void SSLWrapper::connect_secure() {
    this->bio = BIO_new_ssl_connect(this->ssl_ctx);
    BIO_get_ssl(this->bio, &this->ssl);
    SSL_set_mode(this->ssl, SSL_MODE_AUTO_RETRY);
    SSL_set_tlsext_host_name(this->ssl, this->url.get_hostname().c_str());

    BIO_set_conn_hostname(this->bio, (this->url.get_hostname() + ":" + this->url.get_port()).c_str());

    if (BIO_do_connect(this->bio) <= 0) {
        throw SSLException("Could not connect to " + url.get_hostname() + " (Reason: " + this->get_error_str() + ")");
    }
}

string SSLWrapper::get_error_str() {
#ifdef NDEBUG
    return string(ERR_reason_error_string(ERR_get_error()));
#else
    return string(ERR_error_string(ERR_get_error(), NULL));
#endif
}

bool SSLWrapper::should_retry() const {
    return (bool) BIO_should_retry(this->bio);
}
