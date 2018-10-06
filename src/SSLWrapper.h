
#ifndef ISA_SSLWRAPER_H
#define ISA_SSLWRAPER_H

#include <string>

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
     */
    SSLWrapper();
    /**
     * Destructor
     */
    ~SSLWrapper();

    /**
     * Connect to host
     * @param hostname
     */
    void connect(const string &hostname);

    /**
     * Close connection
     */
    void close();

    /**
     * Read message from host
     * @param length max message length
     * @return message
     *
     * @throws SSLException on error
     */
    string read(int length);
    /**
     * Write message to host
     * @param msg message
     *
     * @throws SSLException on error
     */
    void write(const string &msg);

private:
    BIO * bio;
    string hostname;

};


#endif //ISA_SSLWRAPER_H
