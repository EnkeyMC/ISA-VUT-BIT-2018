#include <iostream>
#include <libxml/xmlreader.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

int main() {
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();

    std::cout << "Hello, World!" << std::endl;
    return 0;
}