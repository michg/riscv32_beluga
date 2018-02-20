/*
 *  error handling
 */

#ifndef ERR_H
#define ERR_H

#include <stdio.h>         /* stderr, putc */
#include <cbl/except.h>    /* except_t */

#include "lmap.h"
#include "sym.h"
#include "tree.h"


/* error codes */
enum {
#define xx(a, b, c, d) ERR_##a,
#define yy(a, b, c, d) ERR_##a,
#include "xerror.h"
    ERR_LAST
};


extern int err_level;                /* diagnostic level; mute when > 9 */
extern int err_lim;                  /* # of allowed errors before stop */
extern const except_t err_except;    /* exception for too many errors */


void err_init(void);
int err_count(void);
void err_setwarn(int, int);
int err_chkwarn(int);
int err_experr(void);
void err_cleareff(void);

int err_dpos(const lmap_t *, int, ...);
int err_dmpos(const lmap_t *, int, ...);
int err_dline(const char *, int, int, ...);
int err_dtpos(tree_pos_t *, const tree_t *, const tree_t *, int, ...);

const sym_t *err_idsym(const char *);

#ifndef NDEBUG
void err_print(const lmap_t *);
void err_tprint(tree_pos_t *);
#endif    /* !NDEBUG */


/* turns off diagnostics except fatal ones in a nestable way */
#define err_mute()   ((void)(err_level += 10))
#define err_unmute() (err_level -= 10, assert(err_level >= 0))

/* finalizes JSON diagnostics */
#ifdef JSON_DIAG
#define err_close() ((void)putc(']', stderr))
#else    /* !JSON_DIAG */
#define err_close() ((void)0)
#endif    /* JSON_DIAG */


#endif    /* ERR_H */

/* end of err.h */
