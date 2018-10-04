#ifndef ISA_EXCEPTIONS_H
#define ISA_EXCEPTIONS_H

#include <stdexcept>


class ApplicationException : public std::runtime_error {
public:
    explicit ApplicationException(const std::string &__arg);

};

class ArgumentException : public ApplicationException {
public:
    explicit ArgumentException(const std::string &__arg);
};


#endif //ISA_EXCEPTIONS_H
