
#ifndef ISA_SSLWRAPER_H
#define ISA_SSLWRAPER_H

#include <string>
#include <openssl/ossl_typ.h>
#include <openssl/ssl.h>
#include "Url.h"

using std::string;

typedef struct bio_st BIO;

class SSLWrapper {
public:
    /**
     * Initialize OpenSSL library. Needs to be called before using this class.
     */
    static void init();

    /**
     * Constructor
     * @param ca file with certificates
     * @param cadir directory with certificates
     */
    explicit SSLWrapper(const string &ca = "", const string &cadir = "");
    /**
     * Destructor
     */
    ~SSLWrapper();

    /**
     * Connect to host
     * @param url
     */
    void connect(const Url &url);

    /**
     * Close connection
     */
    void close();

    /**
     * Read message from host
     * @return message
     *
     * @throws SSLException on error
     */
    string read();
    /**
     * Write message to host
     * @param msg message
     *
     * @throws SSLException on error
     */
    void write(const string &msg);

    /**
     * Setup SSL for connection
     */
    void setup_ssl();

    /**
     * Indicates there is more data to be read in buffer
     * @return true if there is more data, false otherwise
     */
    bool should_retry() const;

private:

    /**
     * Wait for available data in BIO
     * @param read true = wait for read, false = wait for write
     */
    void wait(bool read = true) const;

    /**
     * Connect wihtout SSL/TLS
     */
    void connect_insecure();

    /**
     * Connect with SSL/TLS
     */
    void connect_secure();

    /**
     * Connect BIO socket
     */
    void connect_bio();

    /**
     * Get error string after OpenSSL returned error
     * @return error reason string
     */
    string get_error_str();

    BIO * bio;
    SSL_CTX * ssl_ctx;
    SSL * ssl;
    Url url;
    const string ca;
    const string cadir;
};


#endif //ISA_SSLWRAPER_H
