#include <ostream>
#include "utils.h"


bool str_starts_with(const string &str, const string &prefix) {
    return str.substr(0, prefix.length()) == prefix;
}

string trim(const string &str) {
    if (str.empty())
        return str;
    size_t start = str.find_first_not_of(" \t\r");
    size_t end = str.find_last_not_of(" \t\r") + 1;
    return str.substr(start, end - start);
}
