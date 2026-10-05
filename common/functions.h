#if !defined FUNCTIONS_H
#define FUNCTIONS_H

// === include ===
#include "types.h"

// === declaration ===
void		fct_throbber(void);

void		fct_print_local_time(void);

void		fct_parse_numeric_argument(const TYPE_BYTE *, const TYPE_UINT, const TYPE_UINT, TYPE_UINT *);

TYPE_BYTE	fct_fprint_log(const TYPE_BYTE *);

#endif
