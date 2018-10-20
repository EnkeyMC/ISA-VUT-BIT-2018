#ifndef ISA_UTILS_H
#define ISA_UTILS_H

#include <string>
#include <vector>

using std::string;

namespace pugi {
    class xml_node;
}

/**
 * Trim leading and trailing whitespaces
 * @param str string to trim
 * @return trimmed string
 */
string trim(const string &str);

/**
 * Check if str starts with prefix
 * @param str string
 * @param prefix prefix
 * @return true if str starts with prefix
 */
bool str_starts_with(const string &str, const string &prefix);

template<typename T>
std::ostream& operator<<(std::ostream &os, const std::vector<T> &vector) {
    for (const auto &item : vector)
        os << item << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream &os, const pugi::xml_node &node);



#endif //ISA_UTILS_H
