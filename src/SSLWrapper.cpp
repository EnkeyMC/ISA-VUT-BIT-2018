#include "SSLWrapper.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sstream>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


#include "exceptions.h"
#include "debug.h"

#define READ_BUFF_LEN 1024
#define TIMEOUT_SEC 10

void SSLWrapper::init() {
    SSL_load_error_strings();
    SSL_library_init();
    ERR_load_BIO_strings();
//    OpenSSL_add_all_algorithms();
}

SSLWrapper::SSLWrapper(const string &ca, const string &cadir) : ca(ca), cadir(cadir) {
    this->bio = nullptr;
    this->ssl_ctx = nullptr;
    this->ssl = nullptr;
}

SSLWrapper::~SSLWrapper() {
    FIPS_mode_set(0);
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();

    if (this->bio != nullptr)
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

string SSLWrapper::read() {
    char buffer[READ_BUFF_LEN+1];
    std::ostringstream osstream;
    int read;

    do {
        read = BIO_read(this->bio, buffer, READ_BUFF_LEN);
        if (read > 0) {
            buffer[read] = '\0';
            osstream << buffer;
        } else if (read < 0 && !this->should_retry())
            throw SSLException("Chyba čtení dat ze serveru " + this->url.get_original());
        else if (this->should_retry()) {
            this->wait();
        }
    } while (read != 0);

    return osstream.str();
}

void SSLWrapper::write(const string &msg) {
    int ret;
    do {
        ret = BIO_puts(this->bio, msg.c_str());
        if (ret == 0)
            throw SSLException("Server "+this->url.get_hostname()+" ukončil spojení");
        if (this->should_retry())
            this->wait(false);
    } while (ret < 0);
}

void SSLWrapper::close() {
    BIO_reset(this->bio);

    if (this->ssl_ctx != nullptr)
        SSL_CTX_free(this->ssl_ctx);
    this->ssl_ctx = nullptr;
}

void SSLWrapper::setup_ssl() {
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
        throw SSLException("Nepodařilo se připojit k serveru " + url.get_hostname() + " (Chybová zpráva: " + this->get_error_str() + ")");

    this->connect_bio();
}

void SSLWrapper::connect_secure() {
    this->bio = BIO_new_ssl_connect(this->ssl_ctx);
    BIO_get_ssl(this->bio, &this->ssl);
    SSL_set_mode(this->ssl, SSL_MODE_AUTO_RETRY);
    SSL_set_tlsext_host_name(this->ssl, this->url.get_hostname().c_str());

    BIO_set_conn_hostname(this->bio, (this->url.get_hostname() + ":" + this->url.get_port()).c_str());

    this->connect_bio();

    if (SSL_get_verify_result(this->ssl))
        throw SSLException("Nepodařilo se ověřit platnost certifikátu serveru "+this->url.get_hostname());
}

void SSLWrapper::connect_bio() {
    BIO_set_nbio(this->bio, 1);

    bool connected = false;
    do {
        if (BIO_do_connect(this->bio) > 0)
            connected = true;
        if (this->should_retry())
            this->wait(false);
    } while (!connected);
}

string SSLWrapper::get_error_str() {
#ifdef NDEBUG
    return string(ERR_reason_error_string(ERR_get_error()));
#else
    return string(ERR_error_string(ERR_get_error(), nullptr));
#endif
}

bool SSLWrapper::should_retry() const {
    return (bool) BIO_should_retry(this->bio);
}

void SSLWrapper::wait(bool read) const {
    int select_ret, bio_fd;

    bio_fd = (int) BIO_get_fd(this->bio, nullptr);
    if (bio_fd < 0)
        throw std::runtime_error("Nepodařilo se získat socket");

    fd_set fds;
    FD_SET(bio_fd, &fds);

    struct timeval timeout{.tv_sec = TIMEOUT_SEC, .tv_usec = 0};
    if (read)
        select_ret = select(bio_fd+1, &fds, nullptr, nullptr, &timeout);
    else
        select_ret = select(bio_fd+1, nullptr, &fds, nullptr, &timeout);

    if (select_ret == -1)
        throw std::runtime_error("Chyba funkce select() u požadavku na URL " + this->url.get_original());
    else if (select_ret == 0)
        throw SSLTimeoutException("Vypršel čas pro požadavek " + this->url.get_original());
}
