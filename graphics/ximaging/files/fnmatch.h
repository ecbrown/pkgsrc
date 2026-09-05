#ifndef XIMAGING_OPENVMS_FNMATCH_H
#define XIMAGING_OPENVMS_FNMATCH_H

#define FNM_NOMATCH 1

int ximaging_fnmatch(const char *, const char *, int);
#define fnmatch ximaging_fnmatch

#endif
