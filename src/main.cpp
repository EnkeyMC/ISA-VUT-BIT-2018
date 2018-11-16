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


#define EXIT_ARG_ERROR 1
#define EXIT_CONN_ERROR 2
#define EXIT_UNSUPORTED_STATUS 3
#define EXIT_HTTP_ERROR 4
#define EXIT_APP_ERROR 5
#define EXIT_INTERN_ERROR 99

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
void print_feed(const Feed &feed, const Params &params);


int main(int argc, char** argv) {
    try {
        run_program(argc, argv);
    } catch (const ArgumentException &e) {
        cerr << "Chyba: " << e.what() << endl << endl;
        print_help(argv[0]);
        return EXIT_ARG_ERROR;
    } catch (const UrlException &e) {
        cerr << "Chyba: " << e.what() << endl << endl;
        print_help(argv[0]);
        return EXIT_ARG_ERROR;
    } catch (const SSLException &e) {
        cerr << "Chyba: " << e.what() << endl;
        return EXIT_CONN_ERROR;
    } catch (const UnsupportedHttpStatusException &e) {
        cerr << "Chyba: " << e.what() << endl;
        return EXIT_UNSUPORTED_STATUS;
    } catch (const HttpException &e) {
        cerr << "Chyba: " << e.what() << endl;
        return EXIT_HTTP_ERROR;
    } catch (const ApplicationException &e) {
        cerr << "Chyba: " << e.what() << endl;
        return EXIT_APP_ERROR;
    } catch (const exception &e) {
        cerr << "Interní chyba: " << e.what() << endl;
        return EXIT_INTERN_ERROR;
    }

    return EXIT_SUCCESS;
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
    }

    if (urls.empty())
        throw ArgumentException("Soubor " + params.feedfile + " neobsahuje žádné URL");

    SSLWrapper::init();
    SSLWrapper ssl{params.certfile, params.certaddr};
    Feed feed;
    bool first = true;
    int errors = 0;
    for (const Url &url : urls) {

        try {
            feed = get_feed(&ssl, url);
            
            if (urls.size() > 1 && !first)
                cout << endl;
            first = false;
            
            print_feed(feed, params);
        } catch (const ApplicationException &e) {
            if (urls.size() == 1)
                throw;
            cerr << "Chyba zdroje ("+url.get_original()+"): "+e.what()+" (přeskakuji)" << endl;
            errors++;
        }            
    }
    
    if (errors == urls.size())
        throw ApplicationException("Žádné zdroje se nepodařilo načíst");
}

Params get_params(int argc, char** argv) {
    Params params{};

    if (argc == 1) {
        throw ArgumentException("Chybí povinné argumenty");
    }

    int opt;
    opterr = 0;

    do {
        while ((opt = getopt(argc, argv, "+:f:c:C:Tau")) != -1) {
            switch (opt) {
                case 'f':
                    if (!params.feedfile.empty()) {
                        throw ArgumentException("Přepínač -f lze zadat pouze jednou");
                    }
                    params.feedfile = string(optarg);
                    break;

                case 'c':
                    if (!params.certfile.empty()) {
                        throw ArgumentException("Přepínač -c lze zadat pouze jednou");
                    }
                    params.certfile = string(optarg);
                    break;

                case 'C':
                    if (!params.certaddr.empty()) {
                        throw ArgumentException("Přepínač -C lze zadat pouze jednou");
                    }
                    params.certaddr = string(optarg);
                    break;

                case 'T':
                    if (params.show_time) {
                        throw ArgumentException("Přepínač -t lze zadat pouze jednou");
                    }
                    params.show_time = true;
                    break;

                case 'a':
                    if (params.show_author) {
                        throw ArgumentException("Přepínač -a lze zadat pouze jednou");
                    }
                    params.show_author = true;
                    break;

                case 'u':
                    if (params.show_url) {
                        throw ArgumentException("Přepínač -u lze zadat pouze jednou");
                    }
                    params.show_url = true;
                    break;

                case ':':
                    throw ArgumentException("Přepínači "+string(argv[optind-1])+" chybí argument");

                default:
                    throw ArgumentException("Neznámý přepínač "+string(argv[optind-1]));
            }
        }

        if (optind < argc) {
            if (!params.url.empty()) {
                throw ArgumentException("Lze specifikovat pouze jedno URL");
            }
            params.url = string(argv[optind]);
            optind++;
        }
    } while (optind < argc);

    if (params.url.empty() && params.feedfile.empty()) {
        throw ArgumentException("Chybí jeden z povinných argumentů");
    } else if (!params.url.empty() && !params.feedfile.empty()) {
        throw ArgumentException("Nelze kombinovat URL argument a přepínač -f");
    }

    return params;
}

void print_help(const string &exe_name) {
    cerr << "Použití:" << endl;
    cerr << " " << exe_name << " <URL | -f <feedfile>> [-c <cerfile>] [-C <certaddr>] [-T] [-a] [-u]" << endl << endl;
    cerr << "Stáhne RSS nebo Atom zdroj a vypíše informace o něm na standardní výstup." << endl << endl;
    cerr << "Přepínače:" << endl;
    cerr << " -f <feedfile>   jako zdroje použije URL ze zadaného souboru" << endl;
    cerr << " -c <certfile>   certfile musí být soubor s certifikáty, které budou použity pro ověření validity certifikátů serverů" << endl;
    cerr << " -C <certaddr>   použije zadaný adresář pro ověření validity certifikátů při připojování k serverům" << endl;
    cerr << " -T              zobrazí informaci o času vytvoření nebo upravení záznamu zdroje" << endl;
    cerr << " -a              zobrazí autora záznamu zdroje" << endl;
    cerr << " -u              zobrazí URL záznamu zdroje" << endl;
}

vector<Url> parse_feedfile(const string &feedfile) {
    ifstream file{feedfile};
    string line;
    Url url;
    vector<Url> urls;

    if (!file.is_open())
        throw ArgumentException("Soubor " + feedfile + " se nepodařilo otevřít");

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
                cerr << line << " není validní URL, přeskočeno (chyba: " << e.what() << ")" << endl;
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
    Feed feed{url.get_original()};
    feed.parse(content);
    return feed;
}

void print_feed(const Feed &feed, const Params &params) {
    cout << "*** " << feed.get_title() << " ***" << endl;

    auto entries = feed.get_entries();
    bool first = true;
    for (const auto &entry : entries) {
        if (!first && (params.show_url || params.show_author || params.show_time))
            cout << endl;
        first = false;
        cout << entry.title << endl;
        if (params.show_url)
            cout << "URL: " << entry.url << endl;
        if (params.show_time)
            cout << "Aktualizace: " << entry.time << endl;
        if (params.show_author)
            cout << "Autor: " << entry.author << endl;
    }
}
