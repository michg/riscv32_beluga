/*
 *  utilities
 */

#include <stddef.h>        /* size_t */
#include <string.h>        /* strcpy, strrchr */
#include <cbl/assert.h>    /* assert */
#include <cbl/memory.h>    /* MEM_ALLOC, MEM_RESIZE, MEM_FREE */
#include <cdsl/hash.h>     /* hash_string, hash_new */
#ifdef HAVE_REALPATH
#include <stddef.h>    /* NULL */
#include <stdlib.h>    /* realpath, free */
#endif    /* HAVE_REALPATH */

#include "util.h"


/* range table for wcwidth() */
struct interval {
    int first;
    int last;
};


/* declares realpath() for bootstrapping */
char *realpath(const char *, char *);


/*
 *  resolves a path
 */
const char *(rpath)(const char *path)
{
#ifdef HAVE_REALPATH
    const char *p;

    assert(path);

    if ((p = realpath(path, NULL)) == NULL)
#endif    /* HAVE_REALPATH */
        return hash_string(path);
#ifdef HAVE_REALPATH
    path = hash_string(p);
    free((char *)p);

    return path;
#endif    /* HAVE_REALPATH */
}


#define next()                 \
    do {                       \
        assert(*p);            \
        t = *p++ ^ 0x80;       \
        if (t > 0x3F)          \
            goto illegal;      \
        cp = (cp << 6) + t;    \
    } while(0)

/*
 *  converts a utf-8-encoded character to a utf-32 code point
 */
unsigned long (utf8to32)(const char **ps)
{
    unsigned long cp;
    unsigned char t;
    const unsigned char *p = (const unsigned char *)*ps;

    assert(ps);

    switch(*p & 0xF0) {
        case 0x00:
        case 0x10:
        case 0x20:
        case 0x30:
        case 0x40:
        case 0x50:
        case 0x60:
        case 0x70:
            cp = *p++;
            break;
        case 0xC0:
        case 0xD0:
            cp = *p++ & 0x1F;
            next();
            if (cp < 0x80)
                goto illegal;
            break;
        case 0xE0:
            cp = *p++ & 0xF;
            next();
            next();
            if (cp < 0x800)
                goto illegal;
            break;
        case 0xF0:
            if (*p > 0xF4)
                goto illegal;
            cp = *p++ & 0x7;
            next();
            next();
            next();
            if (cp < 0x10000 || cp > 0x10FFFF)
                goto illegal;
            break;
        default:
        illegal:
            p++;
            cp = (unsigned long)-1;
            break;
    }

    *ps = (const char *)p;
    return cp;
}

#undef next


/*
 *  helps wcwidth() for table search
 */
static int bisearch(unsigned long ucs, const struct interval *table, int max) {
    int min = 0, mid;

    if (ucs < table[0].first || ucs > table[max].last)
        return 0;
    while (max >= min) {
        mid = (min + max) / 2;
        if (ucs > table[mid].last)
            min = mid + 1;
        else if (ucs < table[mid].first)
            max = mid - 1;
        else
            return 1;
    }

    return 0;
}


/*
 *  wcwidth() by Markus Kuhn (see https://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c);
 *  ASSUMPTION: control characters are printed out as a space
 */
int (wcwidth)(unsigned long ucs)
{
    static const struct interval combining[] = {
        { 0x0300, 0x036F }, { 0x0483, 0x0486 }, { 0x0488, 0x0489 },
        { 0x0591, 0x05BD }, { 0x05BF, 0x05BF }, { 0x05C1, 0x05C2 },
        { 0x05C4, 0x05C5 }, { 0x05C7, 0x05C7 }, { 0x0600, 0x0603 },
        { 0x0610, 0x0615 }, { 0x064B, 0x065E }, { 0x0670, 0x0670 },
        { 0x06D6, 0x06E4 }, { 0x06E7, 0x06E8 }, { 0x06EA, 0x06ED },
        { 0x070F, 0x070F }, { 0x0711, 0x0711 }, { 0x0730, 0x074A },
        { 0x07A6, 0x07B0 }, { 0x07EB, 0x07F3 }, { 0x0901, 0x0902 },
        { 0x093C, 0x093C }, { 0x0941, 0x0948 }, { 0x094D, 0x094D },
        { 0x0951, 0x0954 }, { 0x0962, 0x0963 }, { 0x0981, 0x0981 },
        { 0x09BC, 0x09BC }, { 0x09C1, 0x09C4 }, { 0x09CD, 0x09CD },
        { 0x09E2, 0x09E3 }, { 0x0A01, 0x0A02 }, { 0x0A3C, 0x0A3C },
        { 0x0A41, 0x0A42 }, { 0x0A47, 0x0A48 }, { 0x0A4B, 0x0A4D },
        { 0x0A70, 0x0A71 }, { 0x0A81, 0x0A82 }, { 0x0ABC, 0x0ABC },
        { 0x0AC1, 0x0AC5 }, { 0x0AC7, 0x0AC8 }, { 0x0ACD, 0x0ACD },
        { 0x0AE2, 0x0AE3 }, { 0x0B01, 0x0B01 }, { 0x0B3C, 0x0B3C },
        { 0x0B3F, 0x0B3F }, { 0x0B41, 0x0B43 }, { 0x0B4D, 0x0B4D },
        { 0x0B56, 0x0B56 }, { 0x0B82, 0x0B82 }, { 0x0BC0, 0x0BC0 },
        { 0x0BCD, 0x0BCD }, { 0x0C3E, 0x0C40 }, { 0x0C46, 0x0C48 },
        { 0x0C4A, 0x0C4D }, { 0x0C55, 0x0C56 }, { 0x0CBC, 0x0CBC },
        { 0x0CBF, 0x0CBF }, { 0x0CC6, 0x0CC6 }, { 0x0CCC, 0x0CCD },
        { 0x0CE2, 0x0CE3 }, { 0x0D41, 0x0D43 }, { 0x0D4D, 0x0D4D },
        { 0x0DCA, 0x0DCA }, { 0x0DD2, 0x0DD4 }, { 0x0DD6, 0x0DD6 },
        { 0x0E31, 0x0E31 }, { 0x0E34, 0x0E3A }, { 0x0E47, 0x0E4E },
        { 0x0EB1, 0x0EB1 }, { 0x0EB4, 0x0EB9 }, { 0x0EBB, 0x0EBC },
        { 0x0EC8, 0x0ECD }, { 0x0F18, 0x0F19 }, { 0x0F35, 0x0F35 },
        { 0x0F37, 0x0F37 }, { 0x0F39, 0x0F39 }, { 0x0F71, 0x0F7E },
        { 0x0F80, 0x0F84 }, { 0x0F86, 0x0F87 }, { 0x0F90, 0x0F97 },
        { 0x0F99, 0x0FBC }, { 0x0FC6, 0x0FC6 }, { 0x102D, 0x1030 },
        { 0x1032, 0x1032 }, { 0x1036, 0x1037 }, { 0x1039, 0x1039 },
        { 0x1058, 0x1059 }, { 0x1160, 0x11FF }, { 0x135F, 0x135F },
        { 0x1712, 0x1714 }, { 0x1732, 0x1734 }, { 0x1752, 0x1753 },
        { 0x1772, 0x1773 }, { 0x17B4, 0x17B5 }, { 0x17B7, 0x17BD },
        { 0x17C6, 0x17C6 }, { 0x17C9, 0x17D3 }, { 0x17DD, 0x17DD },
        { 0x180B, 0x180D }, { 0x18A9, 0x18A9 }, { 0x1920, 0x1922 },
        { 0x1927, 0x1928 }, { 0x1932, 0x1932 }, { 0x1939, 0x193B },
        { 0x1A17, 0x1A18 }, { 0x1B00, 0x1B03 }, { 0x1B34, 0x1B34 },
        { 0x1B36, 0x1B3A }, { 0x1B3C, 0x1B3C }, { 0x1B42, 0x1B42 },
        { 0x1B6B, 0x1B73 }, { 0x1DC0, 0x1DCA }, { 0x1DFE, 0x1DFF },
        { 0x200B, 0x200F }, { 0x202A, 0x202E }, { 0x2060, 0x2063 },
        { 0x206A, 0x206F }, { 0x20D0, 0x20EF }, { 0x302A, 0x302F },
        { 0x3099, 0x309A }, { 0xA806, 0xA806 }, { 0xA80B, 0xA80B },
        { 0xA825, 0xA826 }, { 0xFB1E, 0xFB1E }, { 0xFE00, 0xFE0F },
        { 0xFE20, 0xFE23 }, { 0xFEFF, 0xFEFF }, { 0xFFF9, 0xFFFB },
        { 0x10A01, 0x10A03 }, { 0x10A05, 0x10A06 }, { 0x10A0C, 0x10A0F },
        { 0x10A38, 0x10A3A }, { 0x10A3F, 0x10A3F }, { 0x1D167, 0x1D169 },
        { 0x1D173, 0x1D182 }, { 0x1D185, 0x1D18B }, { 0x1D1AA, 0x1D1AD },
        { 0x1D242, 0x1D244 }, { 0xE0001, 0xE0001 }, { 0xE0020, 0xE007F },
        { 0xE0100, 0xE01EF }
    };

    if (ucs == 0)
        return 0;
    if (ucs < 32 || (ucs >= 0x7F && ucs < 0xA0))
        return 1;
    if (bisearch(ucs, combining, sizeof(combining)/sizeof(struct interval) - 1))
        return 0;

    return 1 + (ucs >= 0x1100 &&
                   (ucs <= 0x115F || ucs == 0x2329 || ucs == 0x232A ||
                       (ucs >= 0x2E80 && ucs <= 0xA4CF && ucs != 0x303F) ||
                       (ucs >= 0xAC00 && ucs <= 0xD7A3) ||
                       (ucs >= 0xF900 && ucs <= 0xFAFF) ||
                       (ucs >= 0xFE10 && ucs <= 0xFE19) ||
                       (ucs >= 0xFE30 && ucs <= 0xFE6f) ||
                       (ucs >= 0xFF00 && ucs <= 0xFF60) ||
                       (ucs >= 0xFFE0 && ucs <= 0xFFE6) ||
                       (ucs >= 0x20000 && ucs <= 0x2FFFD) ||
                       (ucs >= 0x30000 && ucs <= 0x3FFFD)));
}


/*
 *  implements strnlen (which conforms to POSIX.1-2008)
 */
size_t (snlen)(const char *s, size_t max)
{
    const char *t;

    assert(s);

    for (t = s; *(unsigned char *)t && t-s < max; t++)
        continue;

    return t - s;
}


/*
 *  accumulates strings managing a buffer for them;
 *  calls to snbuf() must not be interleaved;
 *  used in:
 *    - inc_add() from inc.c to tokenize paths;
 *    - build() from inc.c to construct full paths;
 *    - concat() from mcr.c to splice tokens
 */
char *(snbuf)(size_t len, int cp)
{
    static char buf[64],    /* size must be power of 2 */
                *p = buf;
    static size_t blen = sizeof(buf);

    assert(p);

    if (len <= blen)
        return p;
    else if (len != (size_t)-1) {
        blen = (len + sizeof(buf)-1) & ~(sizeof(buf)-1);
        if (p == buf) {
            p = MEM_ALLOC(blen);
            if (cp)
                strcpy(p, buf);
        } else
            MEM_RESIZE(p, blen);
    } else if (p != buf)
        MEM_FREE(p);

    return p;
}


/*
 *  extracts a basename from a filename
 */
const char *(basename)(const char *f, int dsep)
{
    const char *d, *s;

    assert(f);
    assert(dsep != '\0');

    d = strrchr(f, '.');
    s = strrchr(f, dsep);

    s = (s)? s+1: f;
    if (!d || d < s || s == d || d[1] == '\0')
        return s;
    return hash_new(s, d-s);
}


/*
 *  extracts an extension from a filename
 */
const char *(extname)(const char *f, int dsep)
{
    const char *p;

    assert(f);
    assert(dsep != '\0');

    p = strrchr(f, '.');
    return (p && p > f && p[-1] != dsep)? p+1: "";
}

/* end of util.c */
