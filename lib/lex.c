/*
 *  primitive lexical analyzer
 */

#include <ctype.h>         /* isdigit, isxdigit, isprint, tolower */
#include <stddef.h>        /* NULL */
#include <string.h>        /* memcpy, memset, strchr, strlen */
#include <cbl/arena.h>     /* ARENA_CALLOC, ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */

#include "common.h"
#include "err.h"
#include "in.h"
#include "lmap.h"
#include "main.h"
#include "strg.h"
#include "lex.h"

/* puts character into token buffer */
#define putbuf(c) (((pbuf == buf+bsize-1)? resize(): (void)0), *pbuf++ = (c))

/* allocates buffer for token spelling */
#define NEWBUF(c)                                \
    (bsize = IN_MAXTOKEN,                        \
     buf = ARENA_CALLOC(strg_line, 1, bsize),    \
     buf[0] = (c),                               \
     pbuf = buf + 1,                             \
     t->f.alloc = 1)

/* handles escaped newlines */
#define BSNL(x)                     \
    do {                            \
        do {                        \
            putbuf('\n');           \
            y++;                    \
        } while(*++rcp == '\n');    \
        (x) = 1;                    \
    } while(0)

/* sets token properties */
#define SETTOK(i, c, y, x)                \
    (t->id = (i),                         \
     t->spell = buf,                      \
     t->f.clean = (c),                    \
     ((lmap_t *)t->pos)->u.n.dy = (y),    \
     ((lmap_t *)t->pos)->u.n.dx = (x))

/* macros to return a token */
#define RETURN(i, s)       \
    do {                   \
        t->id = (i);       \
        t->spell = (s);    \
        return t;          \
    } while(0)

#define RETADJ(x, i, s)                       \
    do {                                      \
        wx += (x);                            \
        t->id = (i);                          \
        t->spell = (s);                       \
        ((lmap_t *)t->pos)->u.n.dx += (x);    \
        in_cp += (x);                         \
        return t;                             \
    } while(0)

#define RETDRT(i, s) if (unclean(t, (i), (s))) return t
#define RETFNL(i)    do { unclean(t, (i), "\0"); return t; } while(0)


int lex_inc;      /* true while parsing #include */
int lex_direc;    /* true while parsing directives */


static int dy = 0;              /* adjusts py for line splicing */
static sz_t wx = 1;             /* x counted by wcwidth() */
static sz_t bsize;              /* size of token buffer */
static char *buf, *pbuf;        /* pointers to maintain token buffer */
static int fromstr;             /* true while input coming from string */
static const lmap_t *posstr;    /* source locus used when fromstr */


/*
 *  enlarges a token buffer
 */
static void resize(void)
{
    char *p = buf;

    buf = ARENA_ALLOC(strg_line, bsize+IN_MAXTOKEN);
    memcpy(buf, p, pbuf-p);
    pbuf = buf + bsize - 1;
    memset(pbuf, 0, IN_MAXTOKEN+1);
    bsize += IN_MAXTOKEN;
}


/*
 *  recognizes "unclean" tokens;
 *  ASSUMPTION: warning for trigraphs issued once per file
 */
static int unclean(lex_t *t, int id, const char *s)
{
    int c;
    int y = 0;
    sz_t x = wx;
    register const char *rcp = in_cp;

    assert(t);
    assert(s);

    pbuf = buf + ((buf[0] == '?')? 3: 1);
    for (s++; *s != '\0'; s++) {
        if (*rcp == '\n')
            BSNL(x);
        c = *rcp++, x++;
        if (c == '?' && rcp[0] == '?' && main_opt()->trigraph &&
            (c = in_trigraph(rcp-1)) != '?' && (main_opt()->trigraph & 1) && c == *s) {
            putbuf('?');
            putbuf('?');
            putbuf(rcp[1]);
            rcp += 2, x += 2;
        } else if (c == *s)
            putbuf(c);
        else
            return 0;
    }
    *pbuf = '\0';    /* token buffer reused */

    dy += y;
    wx = x;
    in_cp = rcp;
    SETTOK(id, (buf[1] == '\0'), y, x);

    return 1;
}


/*
 *  recognizes string literals
 */
static int scon(lex_t *t)
{
    int q, c;
    int y = 0;
    int w = (buf[0] == 'L');
    register const char *rcp = in_cp;

    assert(t);

    if (w) {
        if (*rcp == '\n')
            BSNL(wx);
        q = *rcp++;
        putbuf(q);
    } else
        q = buf[0];
    assert(q == '\'' || q == '"');

    while (*rcp != q && *rcp != '\0') {
        if (*rcp == '\n') {
            t->f.clean = 0;
            BSNL(wx);
            continue;
        }
        c = *rcp++;
        if (c == '\\') {
            putbuf(c);
            if (*rcp == '\n') {
                t->f.clean = 0;
                BSNL(wx);
            }
            c = *rcp;
            if (c != '\0')
                rcp++;
        }
        if (c == '?' && rcp[0] == '?' && main_opt()->trigraph && in_trigraph(rcp-1) != '?' &&
            (main_opt()->trigraph & 1)) {
            t->f.clean = 0;
            if (rcp[1] == '/') {
                putbuf(c);
                putbuf(rcp[0]);
                putbuf(rcp[1]);
                rcp += 2;
                if (*rcp == '\n')
                    BSNL(wx);
                c = *rcp;
                if (c != '\0')
                    rcp++;
            }
        }
        putbuf(c);
    }
    ((lmap_t *)t->pos)->u.n.dy = y, dy += y;
    ((lmap_t *)t->pos)->u.n.dx = wx = in_getwx(wx, in_cp, rcp, NULL)+1;
    in_cp = rcp + 1;
    if (*rcp == q) {
        putbuf(q);
        if (q == '\'' && (buf[w+1] == '\'' || buf[w+1] == '\n')) {
            for (pbuf = &buf[w+1]; *pbuf == '\n'; pbuf++)
                continue;
            if (*pbuf == '\'')
                err_dpos((fromstr)? posstr: t->pos, ERR_CONST_EMPTYCHAR);
        }
    } else {
        in_cp--, ((lmap_t *)t->pos)->u.n.dx--, wx--;
        err_dpos((fromstr)? posstr: t->pos, ERR_LEX_UNCLOSESTR, q);
    }

    return (q == '"')? LEX_SCON: LEX_CCON;
}


/*
 *  recognizes comments
 */
static int comment(lex_t *t)
{
    int c;
    int y = 0;
    sz_t x = wx;
    register const char *rcp = in_cp;
    const char *slash = rcp - 1;

    assert(t);

    if (*rcp == '\n') {
        do { y++; } while(*++rcp == '\n');
        x = 1;
    }
    if (*rcp == '*' && !fromstr) {    /* block comments */
        c = 0;
        dy += y, wx = x;
        rcp++;    /* skips * */
        while (!(c == '*' && *rcp == '/')) {
            if (*rcp == '\0') {
                c = 0;
                y++, dy = 0, wx = 1;
                in_nextline();
                rcp = in_cp;
                if (rcp == in_limit)
                    break;
                continue;
            }
            if (*rcp == '\n') {
                do { dy++, y++; } while(*++rcp == '\n');
                wx = 1;
                continue;
            }
            if (*rcp == '/')
                slash = rcp;
            else if (c == '/' && *rcp == '*')
                err_dline(slash, 2, ERR_LEX_CMTINCMT);
            c = *rcp++;
        }
        ((lmap_t *)t->pos)->u.n.dy = y;
        if (rcp < in_limit) {
            rcp++;    /* skips / */
            ((lmap_t *)t->pos)->u.n.dx = wx = in_getwx(wx, in_cp, rcp, NULL);
            in_cp = rcp;
            return 1;    /* returns space */
        } else {
            ((lmap_t *)t->pos)->u.n.dx = wx;
            err_dpos(t->pos, ERR_LEX_UNCLOSECMT);
            return 2;    /* returns newline */
        }
    }
    if (*rcp == '/' && !fromstr) {
        if (main_opt()->std != 1) {    /* line comments supported */
            dy += y, wx = x;
            while (*++rcp != '\0') {
                if (*rcp == '\n') {
                    err_dline(rcp, 1, ERR_LEX_BSNLINCMT);
                    do { dy++, y++; } while(*++rcp == '\n');
                    wx = 1;
                    rcp--;
                }
            }
            ((lmap_t *)t->pos)->u.n.dy = y;
            ((lmap_t *)t->pos)->u.n.dx = wx = in_getwx(wx, in_cp, rcp, NULL);
            in_cp = rcp;
            return 1;    /* returns space */
        } else
            err_dline(slash, 2, ERR_LEX_C99CMT);
    }
    return 0;    /* not comments */
}


/*
 *  recognizes pp-numbers
 */
static void ppnum(lex_t *t)
{
    int c;
    int y;
    register const char *rcp = in_cp;

    assert(t);

    c = y = 0;
    while (1) {
        while(ISCH_IP(*rcp) || ((*rcp == '-' || *rcp == '+') && tolower(c) == 'e')) {
            c = *rcp++;
            putbuf(c);
        }
        ((lmap_t *)t->pos)->u.n.dx += rcp-in_cp;
        in_cp = rcp;
        if (*rcp != '\n') {
            wx = ((lmap_t *)t->pos)->u.n.dx;
            return;
        }
        BSNL(wx);
        if (!(ISCH_IP(*rcp) || ((*rcp == '-' || *rcp == '+') && tolower(c) == 'e'))) {
            pbuf[in_cp-rcp] = '\0';
            in_cp = rcp;
            dy += y;
            break;
        }
        in_cp = rcp;
        t->f.clean = 0;
        ((lmap_t *)t->pos)->u.n.dy = y, dy += y;
        ((lmap_t *)t->pos)->u.n.dx = 1;
    }
}


/*
 *  recognizes identifiers
 */
static void id(lex_t *t)
{
    int y = 0;
    register const char *rcp = in_cp;

    assert(t);

    while (1) {
        while(ISCH_I(*rcp))
            putbuf(*rcp++);
        ((lmap_t *)t->pos)->u.n.dx += rcp-in_cp;
        in_cp = rcp;
        if (*rcp != '\n') {
            wx = ((lmap_t *)t->pos)->u.n.dx;
            return;
        }
        BSNL(wx);
        if (!ISCH_I(*rcp)) {
            pbuf[in_cp-rcp] = '\0';
            in_cp = rcp;
            dy += y;
            break;
        }
        in_cp = rcp;
        t->f.clean = 0;
        ((lmap_t *)t->pos)->u.n.dy = y, dy += y;
        ((lmap_t *)t->pos)->u.n.dx = 1;
    }
}


/*
 *  recognizes header names
 */
static int header(lex_t *t)
{
    int c;
    int y = 0;
    sz_t x = wx;
    int clean = 1;
    int q = buf[0];
    const char *incp = in_cp;
    register const char *rcp = incp;

    assert(q == '"' || q == '<');
    assert(t);

    if (q == '<')
        q = '>';
    while (*rcp != q && *rcp != '\0') {
        if (*rcp == '\n') {
            clean = 0;
            BSNL(x);
            continue;
        }
        c = *rcp++;
        if (c == '?' && rcp[0] == '?' && main_opt()->trigraph && in_trigraph(rcp-1) != '?' &&
            (main_opt()->trigraph & 1))
            clean = 0;
        putbuf(c);
    }

    if (*rcp == q) {
        putbuf(q);
        dy += y;
        in_cp = ++rcp;
        wx = in_getwx(x, incp, rcp, NULL);
        SETTOK(LEX_HEADER, clean, y, wx);
        return 1;
    } else if (q == '"') {
        assert(!fromstr);
        putbuf(q);
        dy += y;
        in_cp = rcp;
        wx = in_getwx(x, incp, rcp, NULL);
        SETTOK(LEX_HEADER, clean, y, wx);
        err_dpos(t->pos, ERR_LEX_UNCLOSEHDR, q);
        return 1;
    }

    return 0;
}


/*
 *  looks ahead a character after line splicing
 */
static int getchr(void)
{
    register const char *p = in_cp;

    while (*p == '\n')
        p++;

    return *p;
}


/*
 *  retrieves a token from the input stream
 */
lex_t *(lex_next)(void)
{
    lex_t *t;

    assert(in_cp);
    assert(in_limit);

    t = ARENA_CALLOC(strg_line, sizeof(*t), 1);
    t->pos = lmap_add(dy, wx);
    t->f.clean = 1;
    t->next = t;

    while (1) {
        register const char *rcp = in_cp;

        in_cp = rcp + 1, wx++;
        switch(*rcp++) {
            /* whitespaces */
            case '\n':    /* line splicing */
                assert(!fromstr);
                ((lmap_t *)t->pos)->u.n.py++, dy++;
                ((lmap_t *)t->pos)->u.n.wx = wx = 1;
                ((lmap_t *)t->pos)->u.n.dx = 1+1;
                break;
            case '\0':    /* line end */
                dy = 0, wx = 1;
                if (fromstr || in_cp > in_limit) {
                    in_cp = in_limit;
                    RETURN(LEX_EOI, "");
                }
                if (!lex_inc)
                    in_nextline();
                RETURN(LEX_NEWLINE, "\n");
            case '\v':    /* ISCH_SP() */
            case '\f':
            case '\r':
            case ' ':
            case '\t':
                NEWBUF(rcp[-1]);
                while (1) {
                    int y = 0;
                    while (1) {
                        if (rcp[-1] != ' ' && rcp[-1] != '\t' && !fromstr)
                            err_dline(rcp-1, 1, (lex_direc)? ERR_PP_SPHTDIREC: ERR_LEX_STRAYWS,
                                      (rcp[-1] == '\v')? "\\v": (rcp[-1] == '\f')? "\\f": "\\r");
                        if (!ISCH_SP(*rcp))
                            break;
                        putbuf(*rcp++);
                    }
                    ((lmap_t *)t->pos)->u.n.dx += rcp-in_cp;
                    in_cp = rcp;
                    if (*rcp != '\n') {
                        wx = t->pos->u.n.dx;
                        RETURN(LEX_SPACE, buf);
                    }
                    BSNL(wx);
                    if (!ISCH_SP(*rcp)) {
                        pbuf[in_cp-rcp] = '\0';
                        in_cp = rcp;
                        dy += y;
                        break;
                    }
                    in_cp = rcp;
                    t->f.clean = 0;
                    ((lmap_t *)t->pos)->u.n.dy += y, dy += y;
                    ((lmap_t *)t->pos)->u.n.dx = 1;
                    putbuf(*rcp++);
                }
                RETURN(LEX_SPACE, buf);
            /* punctuations */
            case '!':    /* != ! */
                if (*rcp == '=')
                    RETADJ(1, LEX_NEQ, "!=");
                if (*rcp != '\n')
                    RETURN('!', "!");
                NEWBUF('!');
                RETDRT(LEX_NEQ, "!=");
                RETFNL('!');
            case '"':    /* string literals and header */
                if (lex_inc)
                    goto header;
                NEWBUF('"');
            strlit:
                RETURN(scon(t), buf);
            case '#':    /* ## # #??= */
                if (*rcp == '#')
                    RETADJ(1, LEX_DSHARP, "##");
                if (*rcp != '\n' && *rcp != '?')
                    RETURN(LEX_SHARP, "#");
                NEWBUF('#');
                RETDRT(LEX_DSHARP, "##");
                RETFNL(LEX_SHARP);
            case '%':    /* %= %> %:%: %: % */
                if (*rcp == '=')
                    RETADJ(1, LEX_CREM, "%=");
                if (*rcp == '>')
                    RETADJ(1, '}', "%>");
                if (*rcp == ':') {
                    if (rcp[1] == '%' && rcp[2] == ':')
                        RETADJ(3, LEX_DSHARP, "%:%:");
                    if (rcp[1] != '\n')
                        RETADJ(1, LEX_SHARP, "%:");
                } else if (*rcp != '\n')
                    RETURN('%', "%");
                NEWBUF('%');
                RETDRT(LEX_CREM, "%=");
                RETDRT('}', "%>");
                RETDRT(LEX_DSHARP, "%:%:");
                RETDRT(LEX_SHARP, "%:");
                RETFNL('%');
            case '&':    /* && &= & */
                if (*rcp == '&')
                    RETADJ(1, LEX_ANDAND, "&&");
                if (*rcp == '=')
                    RETADJ(1, LEX_CBAND, "&=");
                if (*rcp != '\n')
                    RETURN('&', "&");
                NEWBUF('&');
                RETDRT(LEX_ANDAND, "&&");
                RETDRT(LEX_CBAND, "&=");
                RETFNL('&');
            case '\'':    /* character constant */
                NEWBUF('\'');
                goto strlit;
            case '*':    /* *= * */
                if (*rcp == '=')
                    RETADJ(1, LEX_CMUL, "*=");
                if (*rcp != '\n')
                    RETURN('*', "*");
                NEWBUF('*');
                RETDRT(LEX_CMUL, "*=");
                RETFNL('*');
            case '+':    /* ++ += + */
                if (*rcp == '+')
                    RETADJ(1, LEX_INCR, "++");
                if (*rcp == '=')
                    RETADJ(1, LEX_CADD, "+=");
                if (*rcp != '\n')
                    RETURN('+', "+");
                NEWBUF('+');
                RETDRT(LEX_INCR, "++");
                RETDRT(LEX_CADD, "+=");
                RETFNL('+');
            case '-':    /* -> -- -= - */
                if (*rcp == '>')
                    RETADJ(1, LEX_DEREF, "->");
                if (*rcp == '-')
                    RETADJ(1, LEX_DECR, "--");
                if (*rcp == '=')
                    RETADJ(1, LEX_CSUB, "-=");
                if (*rcp != '\n')
                    RETURN('-', "-");
                NEWBUF('-');
                RETDRT(LEX_DEREF, "->");
                RETDRT(LEX_DECR, "--");
                RETDRT(LEX_CSUB, "-=");
                RETFNL('-');
            case '.':    /* ... . pp-numbers */
                if (rcp[0] == '.' && rcp[1] == '.')
                    RETADJ(2, LEX_ELLIPSIS, "...");
                if (isdigit(*(unsigned char *)rcp))
                    goto ppnum;
                if (*rcp != '\n' && *rcp != '.')
                    RETURN('.', ".");
                if (*rcp == '\n' && isdigit((unsigned char)getchr()))
                    goto ppnum;
                NEWBUF('.');
                RETDRT(LEX_ELLIPSIS, "...");
                RETFNL('.');
            case '/':    /* block-comments line-comments /= / */
                switch(comment(t)) {
                    case 1:
                        RETURN(LEX_SPACE, " ");
                    case 2:
                        RETURN(LEX_NEWLINE, "\n");
                }
                if (*rcp == '=')
                    RETADJ(1, LEX_CDIV, "/=");
                if (*rcp != '\n')
                    RETURN('/', "/");
                NEWBUF('/');
                RETDRT(LEX_CDIV, "/=");
                RETFNL('/');
            case ':':    /* :> : */
                if (*rcp == '>')
                    RETADJ(1, ']', ":>");
                if (*rcp != '\n')
                    RETURN(':', ":");
                NEWBUF(':');
                RETDRT(']', ":>");
                RETFNL(':');
            case '<':    /* header <= << <<= <: <% < */
                if (lex_inc) {
            header:
                    NEWBUF(rcp[-1]);
                    if (header(t))
                        return t;
                }
                switch (*rcp) {
                    case '=':
                        RETADJ(1, LEX_LEQ, "<=");
                    case '<':
                        if (rcp[1] == '=')
                            RETADJ(2, LEX_CLSHFT, "<<=");
                        if (rcp[1] != '\n')
                            RETADJ(1, LEX_LSHFT, "<<");
                        break;
                    case ':':
                        RETADJ(1, '[', "<:");
                    case '%':
                        RETADJ(1, '{', "<%");
                }
                if (*rcp != '\n' && *rcp != '<')
                    RETURN('<', "<");
                NEWBUF('<');
                RETDRT(LEX_LEQ, "<=");
                RETDRT(LEX_CLSHFT, "<<=");
                RETDRT(LEX_LSHFT, "<<");
                RETDRT('[', "<:");
                RETDRT('{', "<%");
                RETFNL('<');
            case '=':    /* == = */
                if (*rcp == '=')
                    RETADJ(1, LEX_EQEQ, "==");
                if (*rcp != '\n')
                    RETURN('=', "=");
                NEWBUF('=');
                RETDRT(LEX_EQEQ, "==");
                RETFNL('=');
            case '>':    /* >= >>= >> > */
                if (*rcp == '=')
                    RETADJ(1, LEX_GEQ, ">=");
                if (*rcp == '>') {
                    if (rcp[1] == '=')
                        RETADJ(2, LEX_CRSHFT, ">>=");
                    if (rcp[1] != '\n')
                        RETADJ(1, LEX_RSHFT, ">>");
                }
                if (*rcp != '\n' && *rcp != '>')
                    RETURN('>', ">");
                NEWBUF('>');
                RETDRT(LEX_GEQ, ">=");
                RETDRT(LEX_CRSHFT, ">>=");
                RETDRT(LEX_RSHFT, ">>");
                RETFNL('>');
            case '^':    /* ^= ^ */
                if (*rcp == '=')
                    RETADJ(1, LEX_CBXOR, "^=");
                if (*rcp != '\n')
                    RETURN('^', "^");
                NEWBUF('^');
                RETDRT(LEX_CBXOR, "^=");
                RETFNL('^');
            case '|':    /* || |= | |??! */
                if (*rcp == '|')
                    RETADJ(1, LEX_OROR, "||");
                if (*rcp == '=')
                    RETADJ(1, LEX_CBOR, "|=");
                if (*rcp != '\n' && *rcp != '?')
                    RETURN('|', "|");
                NEWBUF('|');
                RETDRT(LEX_OROR, "||");
                RETDRT(LEX_CBOR, "|=");
                RETFNL('|');
            case '?':    /* trigraphs ? */
                {
                    int c;
                    if (rcp[0] == '?' && main_opt()->trigraph &&
                        (c = in_trigraph(rcp-1)) != '?' && (main_opt()->trigraph & 1)) {
                        NEWBUF('?');
                        putbuf('?');
                        putbuf(rcp[1]);
                        in_cp = rcp+2, wx += 2;
                        switch(c) {
                            case '#':
                                RETDRT(LEX_DSHARP, "##");
                                RETFNL(LEX_SHARP);
                            case '\\':
                                RETFNL(LEX_UNKNOWN);
                            case '^':
                                RETDRT(LEX_CBXOR, "^=");
                                RETFNL('^');
                            case '|':
                                RETDRT(LEX_OROR, "||");
                                RETDRT(LEX_CBOR, "|=");
                                RETFNL('|');
                            case '[':
                            case ']':
                            case '{':
                            case '}':
                            case '~':
                                RETFNL(c);
                            default:
                                assert(!"invalid trigraph -- should never reach here");
                                break;
                        }
                    }
                }
                RETURN('?', "?");
            /* one-char tokens */
            case '(':
                RETURN('(', "(");
            case ')':
                RETURN(')', ")");
            case ',':
                RETURN(',', ",");
            case ';':
                RETURN(';', ";");
            case '~':
                RETURN('~', "~");
            case '[':
                RETURN('[', "[");
            case ']':
                RETURN(']', "]");
            case '}':
                RETURN('}', "}");
            case '{':
                RETURN('{', "{");
            case 'L':    /* L'x' L"x" ids */
                if (*rcp == '\'' || *rcp == '"') {
                    NEWBUF('L');
                    goto strlit;
                }
                if (*rcp == '\n') {
                    int c = getchr();
                    if (c == '\'' || c == '"') {
                        NEWBUF('L');
                        goto strlit;
                    }
                }
                /* no break */
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
            case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
            case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
            case 'y': case 'z':    /* ids */
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
            case 'I': case 'J': case 'K': case 'M': case 'N': case 'O': case 'P': case 'Q':
            case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y':
            case 'Z':
            case '_':
                NEWBUF(rcp[-1]);
                id(t);
                RETURN(LEX_ID, buf);
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
            case '8': case '9':    /* pp-numbers */
            ppnum:
                NEWBUF(rcp[-1]);
                ppnum(t);
                RETURN(LEX_PPNUM, buf);
            default:    /* unknown chars */
                NEWBUF(rcp[-1]);
                if (FIRSTUTF8(*rcp))
                    RETURN(LEX_UNKNOWN, buf);
                do {
                    putbuf(*rcp++);
                } while(!FIRSTUTF8(*rcp));
                ((lmap_t *)t->pos)->u.n.dx = wx = in_getwx(wx-1, in_cp-1, rcp, NULL);
                in_cp = rcp;
                RETURN(LEX_UNKNOWN, buf);
        }
    }

    /* assert(!"impossible control flow -- should never reach here");
       RETURN(LEX_EOI, ""); */
}


/*
 *  recognizes an escape sequence
 */
ux_t (lex_bs)(lex_t *t, const char *ss, const char **pp, ux_t lim, const char *w)
{
    int c;
    int ovf;
    ux_t n;
    char m[] = "\\x\0";
    const char *p, *s, *hex = "0123456789abcdef";

    assert(t);
    assert(ss);
    assert(pp);
    assert(*pp && **pp == '\\');
    assert(w);

    s = (*pp)++;    /* skips \ */
    switch(*(*pp)++) {
        case 'a':
            return xiu('\a');
        case 'b':
            return xiu('\b');
        case 'f':
            return xiu('\f');
        case 'n':
            return xiu('\n');
        case 'r':
            return xiu('\r');
        case 't':
            return xiu('\t');
        case 'v':
            return xiu('\v');
        case '\'':
        case '"':
        case '\\':
        case '\?':
            break;
        case 'x':    /* \xh...h */
            ovf = 0;
            p = *pp;
            if (!isxdigit(*(unsigned char *)p)) {
                m[2] = *p;
                if (isprint(*(unsigned char *)p))
                    err_dpos(lmap_spell(t, ss, s, p+1), ERR_PP_INVESC, m, w);
                else
                    err_dpos(lmap_spell(t, ss, s, p), ERR_PP_INVESCNP, w);
                return xO;
            }
            c = 0;
            n = xO;
            do {
                c = strchr(hex, tolower(*p++)) - hex;
                if (xt(xba(n, xbc(xsrl(lim, 4)))))
                    ovf = 1;
                n = xau(xsl(n, 4), xiu(c));
            } while(isxdigit(*(unsigned char *)p));
            if (ovf)
                err_dpos(lmap_spell(t, ss, s, p), ERR_CONST_LARGEHEX);
            *pp = p;
            return xba(n, lim);
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            p = *pp;
            c = 1;
            n = xiu(p[-1] - '0');
            if (*p >= '0' && *p <= '7') {
                n = xau(xsl(n, 3), xiu(*p++ - '0')), c++;
                if (*p >= '0' && *p <= '7')
                    n = xau(xsl(n, 3), xiu(*p++ - '0')), c++;
            }
            if (isdigit(*(unsigned char *)p)) {
                if (c < 3 && (*p == '8' || *p == '9'))
                    err_dpos(lmap_spell(t, ss, p, p+1), ERR_CONST_ESCOCT89);
                else if (c == 3)
                    err_dpos(lmap_spell(t, ss, s, p), ERR_CONST_ESCOCT3DIG);
            }
            if (xgu(n, lim))
                err_dpos(lmap_spell(t, ss, s, p), ERR_CONST_LARGEOCT);
            *pp = p;
            return xba(n, lim);
        default:
            m[1] = (*pp)[-1];
            if (isprint(((unsigned char *)(*pp))[-1]))
                err_dpos(lmap_spell(t, ss, s, *pp), ERR_PP_INVESC, m, w);
            else
                err_dpos(lmap_spell(t, ss, s, *pp), ERR_PP_INVESCNP, w);
            break;
    }

    return xiu((*pp)[-1]);
}


/*
 *  makes a token
 */
lex_t *(lex_make)(int id, const char *chn, int end)
{
    lex_t *p = ARENA_CALLOC(strg_line, sizeof(*p), 1);

    assert((void *)chn != strg_perm);

    p->id = id;
    p->spell = chn;
    p->f.clean = 1;
    p->f.end = end;
    p->next = p;

    return p;
}


/*
 *  gets a "clean" spelling of a token
 */
const char *(lex_spell)(const lex_t *t)
{
    int c;
    const char *s;
    char *u, *p;

    assert(t);
    assert(!t->f.clean);

    s = t->spell;
    p = u = ARENA_ALLOC(strg_line, strlen(s)+1);
    while ((c = *s++) != '\0') {
        if (c == '\n')
            continue;
        else if (c == '?' && s[0] == '?' && (main_opt()->trigraph & 1)) {
            switch(s[1]) {
                case '(':
                    c = '[';
                    break;
                case ')':
                    c = ']';
                    break;
                case '<':
                    c = '{';
                    break;
                case '>':
                    c = '}';
                    break;
                case '=':
                    c = '#';
                    break;
                case '/':
                    c = '\\';
                    break;
                case '\'':
                    c = '^';
                    break;
                case '!':
                    c = '|';
                    break;
                case '-':
                    c = '~';
                    break;
            }
            if (c != '?')
                s += 2;
        }
        *p++ = c;
    }
    *p = '\0';

    return u;
}


/*
 *  backs up or restores side effects from token recognization;
 *  cannot be nested
 */
void (lex_backup)(int restore, const lmap_t *pos)
{
    static const char *pcp;
    static const char *plimit;
    static const char *pline;
    static int bx;
    static sz_t by;

    if (!restore) {
        fromstr = 1;
        posstr = pos;
        pcp = in_cp;
        plimit = in_limit;
        pline = in_line;
        bx = wx, by = dy;
    } else {
        fromstr = 0;
        in_cp = pcp;
        in_limit = plimit;
        in_line = pline;
        wx = bx, dy = by;
    }
}

/* end of lex.c */
