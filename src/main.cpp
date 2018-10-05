#include <iostream>
#include <getopt.h>

#include "exceptions.h"
#include "SSLWrapper.h"
#include "Url.h"

using namespace std;

typedef struct {
    string url;
    string feedfile;
    string certfile;
    string certaddr;
    bool show_time{};
    bool show_author{};
    bool show_url{};
} Params;

Params get_params(int argc, char** argv);
void print_help(const string &exe_name);

int main(int argc, char** argv) {
    Params params;

    try {
        params = get_params(argc, argv);
    } catch (const ArgumentException &e) {
        cerr << "Argument error: " << e.what() << endl << endl;
        print_help(argv[0]);
        return 1;
    }

    SSLWrapper::init();

    auto* connection = new SSLWrapper();
    connection->connect("www.fit.vutbr.cz:80");
    connection->write("GET /news/news-rss.php HTTP/1.1\nHost: www.fit.vutbr.cz\n\n");
//    cout << connection->read(4096) << endl;

    Url url{};
    url.from_string(params.url);
    url.validate();
    cout << url.get_protocol() << " " << url.get_hostname() << " " << url.get_port() << " " << url.get_path() << endl;

    return 0;
}

Params get_params(int argc, char** argv) {
    Params params{};

    if (argc == 1) {
        throw ArgumentException("Required arguments missing");
    }

    int opt;
    opterr = 1;

    while ((opt = getopt(argc, argv, "f:c:C:tau")) != -1) {
        switch (opt) {
            case 'f':
                if (!params.feedfile.empty()) {
                    throw ArgumentException("Option -f can only be present once");
                }
                params.feedfile = string(optarg);
                break;

            case 'c':
                if (!params.certfile.empty()) {
                    throw ArgumentException("Option -c can only be present once");
                }
                params.certfile = string(optarg);
                break;

            case 'C':
                if (!params.certaddr.empty()) {
                    throw ArgumentException("Option -C can only be present once");
                }
                params.certaddr = string(optarg);
                break;

            case 't':
                if (params.show_time) {
                    throw ArgumentException("Option -t can only be present once");
                }
                params.show_time = true;
                break;

            case 'a':
                if (params.show_author) {
                    throw ArgumentException("Option -a can only be present once");
                }
                params.show_author = true;
                break;

            case 'u':
                if (params.show_url) {
                    throw ArgumentException("Option -u can only be present once");
                }
                params.show_url = true;
                break;

            default:
                throw ArgumentException("Invalid option");
        }
    }

    for (int index = optind; index < argc; index++) {
        if (!params.url.empty()) {
            throw ArgumentException("You can specify only one URL");
        }
        params.url = string(argv[index]);
    }


    if (params.url.empty() && params.feedfile.empty()) {
        throw ArgumentException("You have to specify URL or feedfile (option -f)");
    } else if (!params.url.empty() && !params.feedfile.empty()) {
        throw ArgumentException("You cannot specify URL and feedfile (option -f) at the same time");
    }

    return params;
}

void print_help(const string &exe_name) {
    cerr << "Usage:" << endl;
    cerr << " " << exe_name << " <URL | -f <feedfile>> [-c <cerfile>] [-C <certaddr>] [-T] [-a] [-u]" << endl << endl;
    cerr << "Downloads given RSS or Atom feed and prints information to standard output." << endl << endl;
    cerr << "Options:" << endl;
    cerr << " -f <feedfile>   file with multiple URLs to RSS or Atom feed" << endl;
    cerr << " -c <certfile>   file with certificates used to check SSL/TLS certificate validity" << endl;
    cerr << " -C <certaddr>   scans given directory for certificates for SSL/TLS certificate validity check" << endl;
    cerr << " -T              show information about creation or modification time of record (if available)" << endl;
    cerr << " -a              show author or his e-mail address (if available)" << endl;
    cerr << " -u              show asociated URL to each record (if available)" << endl;
}
