#include "openvms-motif.h"

#undef _XmMgrTraversal
#undef realpath

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

char *
ximaging_realpath(const char *path, char *resolved_path)
{
    struct stat st;
    char *absolute_path;
    char *cwd;
    size_t length;

    if (path == NULL)
    {
        errno = EINVAL;
        return NULL;
    }

    absolute_path = realpath(path, resolved_path);
    if (absolute_path != NULL)
        return absolute_path;

    if (stat(path, &st) != 0)
        return NULL;

    if (path[0] == '/')
    {
        absolute_path = strdup(path);
    }
    else
    {
        cwd = getcwd(NULL, 0);
        if (cwd == NULL)
            return NULL;
        if (strcmp(path, ".") == 0)
        {
            absolute_path = cwd;
        }
        else
        {
            length = strlen(cwd) + strlen(path) + 2;
            absolute_path = malloc(length);
            if (absolute_path != NULL)
                snprintf(absolute_path, length, "%s/%s", cwd, path);
            free(cwd);
        }
    }

    if (absolute_path == NULL)
    {
        errno = ENOMEM;
        return NULL;
    }
    if (resolved_path == NULL)
        return absolute_path;

    strcpy(resolved_path, absolute_path);
    free(absolute_path);
    return resolved_path;
}

void
ximaging_mgr_traversal(Widget widget, Cardinal direction)
{
    (void)_XmMgrTraversal(widget, (XmTraversalDirection)direction);
}

void
ximaging_font_list_extents(XmFontList fonts, int *height, int *ascent,
                           int *descent)
{
    XmFontContext context;
    XmStringCharSet charset = NULL;
    XFontStruct *font = NULL;
    int a = 12;
    int d = 4;

    if (fonts != NULL && XmFontListInitFontContext(&context, fonts))
    {
        if (XmFontListGetNextFont(context, &charset, &font) && font != NULL)
        {
            a = font->ascent;
            d = font->descent;
        }
        if (charset != NULL)
            XtFree(charset);
        XmFontListFreeFontContext(context);
    }
    if (height != NULL)
        *height = a + d;
    if (ascent != NULL)
        *ascent = a;
    if (descent != NULL)
        *descent = d;
}

char *
ximaging_xmstring_unparse(XmString value)
{
    char *text = NULL;

    if (!XmStringGetLtoR(value, XmSTRING_DEFAULT_CHARSET, &text))
        return XtNewString("");
    return text;
}
