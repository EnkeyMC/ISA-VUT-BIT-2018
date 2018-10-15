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

class SSLException : public ApplicationException {
public:
    explicit SSLException(const std::string &__arg);
};

class UrlException : public ApplicationException {
public:
    explicit UrlException(const std::string &__arg);
};

class HttpException : public ApplicationException {
public:
    explicit HttpException(const std::string &__arg);
};

class UnsupportedHttpStatusException : public HttpException {
public:
    explicit UnsupportedHttpStatusException(const std::string &__arg);
};


#endif //ISA_EXCEPTIONS_H
