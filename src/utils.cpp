#include <ostream>
#include "utils.h"
#include "pugixml.hpp"

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

std::ostream &operator<<(std::ostream &os, const pugi::xml_node &node) {
    if (!node)
        os << "node_null";
    else {
        os << node.name();
        if (node.text())
            os << " (" << node.text().get() << ")";
    }

    return os;
}
