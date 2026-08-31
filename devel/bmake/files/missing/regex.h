/*
 * bmake's :C modifier needs POSIX regular expressions.  Systems without a
 * native <regex.h> use the implementation built by pkgsrc's libnbcompat.
 */
#ifndef BMAKE_MISSING_REGEX_H
#define BMAKE_MISSING_REGEX_H

#include <nbcompat/regex.h>

#endif
