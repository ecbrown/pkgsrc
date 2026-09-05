/* Small POSIX-style matcher for XImaging's selection-pattern dialog. */
#include "fnmatch.h"

static int
match_class(const char **pattern, unsigned char value)
{
    const char *p = *pattern;
    int matched = 0;
    int negate = 0;

    if (*p == '!' || *p == '^')
    {
        negate = 1;
        ++p;
    }
    while (*p != '\0' && *p != ']')
    {
        unsigned char first = (unsigned char)*p++;
        if (first == '\\' && *p != '\0')
            first = (unsigned char)*p++;
        if (*p == '-' && p[1] != '\0' && p[1] != ']')
        {
            unsigned char last;
            ++p;
            last = (unsigned char)*p++;
            if (last == '\\' && *p != '\0')
                last = (unsigned char)*p++;
            if (first <= value && value <= last)
                matched = 1;
        }
        else if (first == value)
            matched = 1;
    }
    if (*p != ']')
        return -1;
    *pattern = p + 1;
    return negate ? !matched : matched;
}

static int
match_pattern(const char *pattern, const char *string)
{
    while (*pattern != '\0')
    {
        if (*pattern == '*')
        {
            while (*pattern == '*')
                ++pattern;
            if (*pattern == '\0')
                return 1;
            do
            {
                if (match_pattern(pattern, string))
                    return 1;
            } while (*string++ != '\0');
            return 0;
        }
        if (*string == '\0')
            return 0;
        if (*pattern == '?')
        {
            ++pattern;
            ++string;
            continue;
        }
        if (*pattern == '[')
        {
            const char *next = pattern + 1;
            int result = match_class(&next, (unsigned char)*string);
            if (result < 0)
            {
                if (*string != '[')
                    return 0;
                ++pattern;
            }
            else
            {
                if (!result)
                    return 0;
                pattern = next;
            }
            ++string;
            continue;
        }
        if (*pattern == '\\' && pattern[1] != '\0')
            ++pattern;
        if (*pattern++ != *string++)
            return 0;
    }
    return *string == '\0';
}

int
ximaging_fnmatch(const char *pattern, const char *string, int flags)
{
    (void)flags;
    return match_pattern(pattern, string) ? 0 : FNM_NOMATCH;
}
