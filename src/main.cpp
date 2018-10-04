#include <iostream>
#include <string>
#include <getopt.h>

#include "exceptions.h"

using namespace std;

typedef struct {
    string url;
    string feedfile;
    string certfile;
    string certaddr;
    bool show_time;
    bool show_author;
    bool show_url;
    bool help;
} Params;

Params get_params(int argc, char** argv);
void print_help(const string &exe_name);

int main(int argc, char** argv) {
    Params params;

    try {
        params = get_params(argc, argv);
    } catch (const ArgumentException &e) {
        cerr << "Argument error: " << e.what() << endl;
        return 1;
    }

    if (params.help) {
        print_help(argv[0]);
        return 1;
    }

    return 0;
}

Params get_params(int argc, char** argv) {
    Params params{};

    if (argc == 1) {
        params.help = true;
        return params;
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
    cout << "Usage:" << endl;
    cout << " " << exe_name << " <URL | -f <feedfile>> [-c <cerfile>] [-C <certaddr>] [-T] [-a] [-u]" << endl << endl;
    cout << "Downloads given RSS or Atom feed and prints information to standard output." << endl << endl;
    cout << "Options:" << endl;
    cout << " -f <feedfile>   file with multiple URLs to RSS or Atom feed" << endl;
    cout << " -c <certfile>   file with certificates used to check SSL/TLS certificate validity" << endl;
    cout << " -C <certaddr>   scans given directory for certificates for SSL/TLS certificate validity check" << endl;
    cout << " -T              show information about creation or modification time of record (if available)" << endl;
    cout << " -a              show author or his e-mail address (if available)" << endl;
    cout << " -u              show asociated URL to each record (if available)" << endl;
}
