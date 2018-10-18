#include <iostream>
#include <getopt.h>
#include <vector>
#include <fstream>
#include <sstream>

#include "exceptions.h"
#include "SSLWrapper.h"
#include "Url.h"
#include "Http.h"
#include "utils.h"
#include "Feed.h"
#include "debug.h"


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

void run_program(int argc, char** argv);
Params get_params(int argc, char** argv);
void print_help(const string &exe_name);
vector<Url> parse_feedfile(const string &feedfile);
void remove_comment(string &str);
Feed get_feed(SSLWrapper *ssl, const Url &url);

int main(int argc, char** argv) {
    try {
        run_program(argc, argv);
    } catch (const ArgumentException &e) {
        cerr << "Argument error: " << e.what() << endl << endl;
        print_help(argv[0]);
        return 1;
    } catch (const UrlException &e) {
        cerr << "Argument error: " << e.what() << endl << endl;
        print_help(argv[0]);
        return 1;
    } catch (const SSLException &e) {
        cerr << e.what() << endl;
        return 2;
    } catch (const UnsupportedHttpStatusException &e) {
        cerr << e.what() << endl;
        return 3;
    } catch (const HttpException &e) {
        cerr << e.what() << endl;
        return 4;
    } catch (const exception &e) {
        cerr << e.what() << endl;
        return 99;
    }

    return 0;
}

void run_program(int argc, char **argv) {
    Params params;
    vector<Url> urls{};

    params = get_params(argc, argv);

    if (params.feedfile.empty()) {
        Url url{};
        url.from_string(params.url);
        url.validate();
        urls.push_back(url);
    } else {
        urls = parse_feedfile(params.feedfile);
    } debug(urls);

    SSLWrapper::init();
    SSLWrapper ssl{params.certfile, params.certaddr};
    Feed feed;
    for (const Url &url : urls) {
        feed = get_feed(&ssl, url);
        cout << feed << endl; // TODO error printing
    }
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

vector<Url> parse_feedfile(const string &feedfile) {
    ifstream file{feedfile};
    string line;
    Url url;
    vector<Url> urls;

    if (!file.is_open())
        throw ArgumentException("File " + feedfile + " could not be opened");

    while (getline(file, line)) {
        remove_comment(line);
        line = trim(line);
        if (!line.empty()) {
            url = Url();
            url.from_string(line);

            try {
                url.validate();
                urls.push_back(url);
            } catch (const UrlException &e) {
                cerr << "Ignoring URL " << line << ", error occurred: " << e.what() << endl;
            }
        }
    }

    return urls;
}

void remove_comment(string &str) {
    size_t hash_pos = str.find(" #");
    if (str.length() > 0 && str[0] == '#')
        hash_pos = 0;

    if (hash_pos != string::npos)
        str.erase(hash_pos);
}

Feed get_feed(SSLWrapper *ssl, const Url &url) {
    string content = Http::get_request(ssl, url);

    return {};
}
