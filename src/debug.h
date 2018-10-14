#ifndef ISA_DEBUG_H
#define ISA_DEBUG_H

#ifndef NDEBUG
#include <iostream>

using std::cerr;
using std::endl;

#define debug(var) cerr << __FILE__ << ":" << __LINE__ << ": " << #var << endl << (var) << endl;

#else

#define debug(var)

#endif


#endif //ISA_DEBUG_H
