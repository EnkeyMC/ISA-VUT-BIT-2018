
#ifndef ISA_SSLWRAPER_H
#define ISA_SSLWRAPER_H

#include <string>

using std::string;

typedef struct bio_st BIO;

class SSLWrapper {
public:
    static void init();

    SSLWrapper();
    ~SSLWrapper();

    void connect(const string &hostname);
    void close();
    string read(int length);
    void write(const string &msg);

private:
    BIO * bio;
    string hostname;

};


#endif //ISA_SSLWRAPER_H
