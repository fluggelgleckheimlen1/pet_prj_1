#if !defined TYPES_H
#define TYPES_H

// === include ===
#include <stdint.h>

/*              0..4 294 967 295 */	typedef uint32_t	TYPE_UDINT;
/* -2 147 483 648..2 147 483 647 */	typedef int32_t		TYPE_DINT;
/*              0..       65 535 */	typedef uint16_t	TYPE_UINT;
/*        -32 768.........32 767 */	typedef int16_t		TYPE_INT;
/*              0............255 */	typedef uint8_t		TYPE_USINT;
/*           -128............127 */	typedef int8_t		TYPE_SINT;

typedef uint32_t	TYPE_DWORD;
typedef uint16_t	TYPE_WORD;
typedef uint8_t		TYPE_BYTE;
typedef uint8_t		TYPE_BOOL;

#endif
