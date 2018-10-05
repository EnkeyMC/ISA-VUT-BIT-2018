#include "exceptions.h"

ApplicationException::ApplicationException(const std::string &__arg) : runtime_error(__arg) {}

ArgumentException::ArgumentException(const std::string &__arg) : ApplicationException(__arg) {}

SSLException::SSLException(const std::string &__arg) : ApplicationException(__arg) {}

UrlException::UrlException(const std::string &__arg) : ApplicationException(__arg) {}
