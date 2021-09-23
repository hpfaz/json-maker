
/*
<https://github.com/rafagafe/tiny-json>

  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
  SPDX-License-Identifier: MIT
  Copyright (c) 2018 Rafa Garcia <rafagarcia77@gmail.com>.
  Permission is hereby  granted, free of charge, to any  person obtaining a copy
  of this software and associated  documentation files (the "Software"), to deal
  in the Software  without restriction, including without  limitation the rights
  to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
  copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
  IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
  FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
  AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
  LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <stddef.h> // For NULL
#include <string.h>
#include "include/json-maker/json-maker.h"
#ifndef NO_SPRINTF
#include <stdio.h>
#include <stdbool.h>

#endif // NO_SPRINTF

static char* primitivename( char* dest, char const* name );
static char* atoesc( char* dest, char const* src, int len );
static int escape( int ch );
static int nibbletoch( int nibble );
static char* strname( char* dest, char const* name );
static json_errcodes_t chtoa( json_buffer_t *info_p, const char ch );
static json_errcodes_t atoa( json_buffer_t* buff, char const* src );
static inline char *buff_seekCur( const json_buffer_t* info_p);
static inline bool buff_hasEnoughSpaceFor( const json_buffer_t*  info_p,
                                           size_t data_sz);
static inline char* buff_seekOffsetFromCur( const json_buffer_t* info_p,
                                            size_t offset);
static inline json_errcodes_t buff_increaseRemSize( json_buffer_t* info_p,
                                                    size_t sizeIncrease );
static inline json_errcodes_t buff_decreaseRemSize( json_buffer_t* info_p,
                                                    size_t sizeDecrease );
static inline json_errcodes_t openScopedObject( json_scoped_object_t objectType,
                                               json_buffer_t* jsonBuffer,
                                               char const* name);
static inline char closeScopedObject( json_scoped_object_t objectType,
                                      json_buffer_t* jsonBuffer);


/** TODO
*/
json_errcodes_t json_start(json_buffer_t *const jsonBuffer)
{
    json_errcodes_t errCode = JSON_NO_ERROR;

    if (NULL == jsonBuffer ||
        NULL == jsonBuffer->buffer ||
        3 > jsonBuffer->remaining_sz ||
        jsonBuffer->total_sz != jsonBuffer->remaining_sz)
    {
        errCode = JSON_GLOBAL_ERROR;
        if(3 > jsonBuffer->remaining_sz) {
            errCode = JSON_OUT_OF_MEMORY;
        }
    } else {
        memset(jsonBuffer->buffer, 0, jsonBuffer->total_sz);
        jsonBuffer->remaining_sz = jsonBuffer->total_sz - 1;
    }

    return errCode;
}

/* Used to finish the root JSON object. After call json_objClose(). */
void json_end( json_buffer_t* const jsonBuffer ) {
    char* const curr_char_p = buff_seekOffsetFromCur(jsonBuffer, -1);
    if ( ',' == *curr_char_p) {
        *curr_char_p = '\0';
    }
}

/* Open a JSON object in a JSON string. */
json_errcodes_t json_objOpen(json_buffer_t* const jsonBuffer, char const* name ) {
    return openScopedObject(JSON_OBJECT, jsonBuffer, name);
}

/* Close a JSON object in a JSON string. */
json_errcodes_t json_objClose( json_buffer_t* const jsonBuffer) {
   return closeScopedObject(JSON_OBJECT, jsonBuffer);
}

/* Open an array in a JSON string. */
json_errcodes_t json_arrOpen(json_buffer_t* const jsonBuffer, char const* name ) {
    return openScopedObject(JSON_ARRAY, jsonBuffer, name);
}

/* Close an array in a JSON string. */
json_errcodes_t json_arrClose( json_buffer_t* const jsonBuffer) {
    return closeScopedObject(JSON_ARRAY, jsonBuffer);
}

/* Add a text property in a JSON string. */
char* json_nstr( char* dest, char const* name, char const* value, int len ) {
    dest = strname( dest, name );
    dest = atoesc( dest, value, len );
    dest = atoa( dest, "\"," );
    return dest;
}

/*  Add a boolean property in a JSON string. */
char* json_bool( char* dest, char const* name, int value ) {
    dest = primitivename( dest, name );
    dest = atoa( dest, value ? "true," : "false," );
    return dest;
}

/* Add a null property in a JSON string. */
char* json_null( char* dest, char const* name ) {
    dest = primitivename( dest, name );
    dest = atoa( dest, "null," );
    return dest;
}

/** Add the name of a text property.
  * @param dest Destination memory.
  * @param name The name of the property.
  * @return Pointer to the next char. */
static char* strname( char* dest, char const* name ) {
    dest = chtoa( dest, '\"' );
    if ( NULL != name ) {
        dest = atoa( dest, name );
        dest = atoa( dest, "\":\"" );
    }
    return dest;
}

/** Get the hexadecimal digit of the least significant nibble of a integer. */
static int nibbletoch( int nibble ) {
    return "0123456789ABCDEF"[ nibble % 16u ];
}

/** Get the escape character of a non-printable.
  * @param ch Character source.
  * @return The escape character or null character if error. */
static int escape( int ch ) {
    int i;
    static struct { char code; char ch; } const pair[] = {
        { '\"', '\"' }, { '\\', '\\' }, { '/',  '/'  }, { 'b',  '\b' },
        { 'f',  '\f' }, { 'n',  '\n' }, { 'r',  '\r' }, { 't',  '\t' },
    };
    for( i = 0; i < sizeof pair / sizeof *pair; ++i )
        if ( ch == pair[i].ch )
            return pair[i].code;
    return '\0';
}

/** Copy a null-terminated string inserting escape characters if needed.
  * @param dest Destination memory block.
  * @param src Source string.
  * @param len Max length of source. < 0 for unlimit.
  * @return Pointer to the null character of the destination string. */
static char* atoesc( char* dest, char const* src, int len ) {
    int i;
    for( i = 0; src[i] != '\0' && ( i < len || 0 > len ); ++dest, ++i ) {
        if ( src[i] >= ' ' && src[i] != '\"' && src[i] != '\\' && src[i] != '/' )
            *dest = src[i];
        else {
            *dest++ = '\\';
            int const esc = escape( src[i] );
            if ( esc )
                *dest = esc;
            else {
                *dest++ = 'u';
                *dest++ = '0';
                *dest++ = '0';
                *dest++ = nibbletoch( src[i] / 16 );
                *dest++ = nibbletoch( src[i] );
            }
        }
    }
    *dest = '\0';
    return dest;
}

/** Add the name of a primitive property.
  * @param dest Destination memory.
  * @param name The name of the property.
  * @return Pointer to the next char. */
static char* primitivename( char* dest, char const* name ) {
    if( NULL == name )
        return dest;
    dest = chtoa( dest, '\"' );
    dest = atoa( dest, name );
    dest = atoa( dest, "\":" );
    return dest;
}

/** Add a character at the end of a string.
  * @param info_p Pointer to the null character of the string
  * @param ch Value to be added.
  * @return Pointer to the null character of the destination string. */
static json_errcodes_t chtoa(json_buffer_t *info_p, const char ch ) {
    json_errcodes_t errCode = JSON_NO_ERROR;

    if ( NULL == info_p ||
         NULL == info_p->buffer ||
         !buff_hasEnoughSpaceFor(info_p, 1))
    {
        errCode = JSON_GLOBAL_ERROR;

        if(!buff_hasEnoughSpaceFor(info_p, 1))
        {
            errCode = JSON_OUT_OF_MEMORY;
        }
    }
    else {
        char * const dest = buff_seekCur(info_p);
        dest[0] = ch;
        dest[1] = '\0';
        buff_decreaseRemSize(info_p, 1);
    }

    return errCode;
}

/** Copy a null-terminated string.
  * @param dest Destination memory block.
  * @param src Source string.
  * @return Pointer to the null character of the destination string. */
static json_errcodes_t atoa( json_buffer_t* const buff, char const* src ) {
    json_errcodes_t errCode = JSON_NO_ERROR;
    size_t src_strlen = 0;

    if ( NULL == buff ||
         NULL == src ||
         NULL == buff->buffer ||
         !buff_hasEnoughSpaceFor(buff, (src_strlen = strlen(src))))
    {
        errCode = JSON_GLOBAL_ERROR;

        // Always need to have an extra byte for nullbyte
        if(!buff_hasEnoughSpaceFor(buff, src_strlen))
        {
            errCode = JSON_OUT_OF_MEMORY;
        }
    }
    else {
        char* const dest = buff_seekCur(buff);

        for (size_t i = 0; i < src_strlen; i++)
        {
            dest[i] = src[i];
        }

        dest[src_strlen] = '\0';
        buff_decreaseRemSize(buff, 1);
    }
    return errCode;
}

static inline json_errcodes_t openScopedObject(const json_scoped_object_t objectType,
                                               json_buffer_t* const jsonBuffer,
                                               char const* name) {
    const char openingChar[] = {
            [JSON_ARRAY]  = '[',
            [JSON_OBJECT] = '{'
    };
    const char openingStr[][4] = {
            [JSON_ARRAY]  = "\":[",
            [JSON_OBJECT] = "\":{"
    };

    json_errcodes_t errCode;

    if (NULL == jsonBuffer ) {
        errCode = JSON_GLOBAL_ERROR;
    } else {
        if ( NULL == name )
            errCode = chtoa(jsonBuffer, openingChar[objectType] );
        else {
            chtoa(jsonBuffer, '\"' );
            atoa(jsonBuffer, name );
            errCode = atoa(jsonBuffer, openingStr[objectType] );
        }
    }

    return errCode;
}

static inline char closeScopedObject( const json_scoped_object_t objectType,
                                      json_buffer_t* const jsonBuffer) {
    const char closingStr[][3] = {
            [JSON_ARRAY]  = "],",
            [JSON_OBJECT] = "},"
    };

    json_errcodes_t errCode = JSON_NO_ERROR;

    if ( ',' == buff_seekOffsetFromCur(jsonBuffer, -1)[0]) {
        errCode = buff_increaseRemSize(jsonBuffer, 1);
    }
    if(errCode == JSON_NO_ERROR)
    {
        errCode = atoa(jsonBuffer, closingStr[objectType]);
    }

    return errCode;
}

static inline char* buff_seekCur( const json_buffer_t* const info_p) {
    return info_p->buffer + (info_p->total_sz - info_p->remaining_sz);
}

static inline char* buff_seekOffsetFromCur( const json_buffer_t* const info_p,
                                            const size_t offset) {
    char* const buffer_end = info_p->buffer + info_p->total_sz;

    char *pos = info_p->buffer + (info_p->total_sz - info_p->remaining_sz + offset);

    if ( pos > buffer_end ) {
        pos = buffer_end;
    } else if ( pos < info_p->buffer) {
        pos = info_p->buffer;
    }

    return pos;
}

static inline json_errcodes_t buff_increaseRemSize( json_buffer_t* const info_p,
                                          const size_t sizeIncrease ) {
    json_errcodes_t errCode = JSON_NO_ERROR;
    const size_t newSize = info_p->remaining_sz + sizeIncrease;

    // Always need one extra byte for trailing \0
    if ( newSize > info_p->total_sz - 1 ) {
        errCode = JSON_OUT_OF_MEMORY;
    } else {
        info_p->remaining_sz = newSize;
    }

    return errCode;
}

static inline json_errcodes_t buff_decreaseRemSize( json_buffer_t* const info_p,
                                                    const size_t sizeDecrease ) {
    json_errcodes_t errCode = JSON_NO_ERROR;
    const size_t newSize = info_p->remaining_sz - sizeDecrease;

    // Can't go below than minimal size JSON "{}\0"
    if ( newSize < 3 ) {
        errCode = JSON_OUT_OF_MEMORY;
    } else {
        info_p->remaining_sz = newSize;
    }

    return errCode;
}

static inline bool buff_hasEnoughSpaceFor(const json_buffer_t* const info_p,
                                          const size_t data_sz) {
    // account for extra byte required by nullbyte
    return data_sz < info_p->remaining_sz;
}
#ifdef NO_SPRINTF

static char* format( char* dest, int len, int isnegative ) {
    if ( isnegative )
        dest[ len++ ] = '-';
    dest[ len ] = '\0';
    int head = 0;
    int tail = len - 1;
    while( head < tail ) {
        char tmp = dest[ head ];
        dest[ head ] = dest[ tail ];
        dest[ tail ] = tmp;
        ++head;
        --tail;
    }
    return dest + len;
}

#define numtoa( func, type, utype )         \
static char* func( char* dest, type val ) { \
    enum { base = 10 };                     \
    if ( 0 == val )                         \
        return chtoa( dest, '0' );          \
    int const isnegative = 0 > val;         \
    utype num = isnegative ? -val : val;    \
    int len = 0;                            \
    while( 0 != num ) {                     \
        int rem = num % base;               \
        dest[ len++ ] = rem + '0';          \
        num = num / base;                   \
    }                                       \
    return format( dest, len, isnegative ); \
}                                           \

#define json_num( func, func2, type )                       \
char* func( char* dest, char const* name, type value ) {    \
    dest = primitivename( dest, name );                     \
    dest = func2( dest, value );                            \
    dest = chtoa( dest, ',' );                              \
    return dest;                                            \
}                                                           \

#define ALL_TYPES \
    X( int,      int,          unsigned int        ) \
    X( long,     long,         unsigned long       ) \
    X( uint,     unsigned int, unsigned int        ) \
    X( ulong,    unsigned      long, unsigned long ) \
    X( verylong, long long,    unsigned long long  ) \

#define X( name, type, utype ) numtoa( name##toa, type, utype )
ALL_TYPES
#undef X

#define X( name, type, utype ) json_num( json_##name, name##toa, type )
ALL_TYPES
#undef X

char* json_double( char* dest, char const* name, double value ) {
    return json_verylong( dest, name, value );
}

#else

#define ALL_TYPES \
    X( json_int,      int,           "%d"   ) \
    X( json_long,     long,          "%ld"  ) \
    X( json_uint,     unsigned int,  "%u"   ) \
    X( json_ulong,    unsigned long, "%lu"  ) \
    X( json_verylong, long long,     "%lld" ) \
    X( json_double,   double,        "%g"   ) \


#define json_num( funcname, type, fmt )                         \
char* funcname( char* dest, char const* name, type value ) {    \
    dest = primitivename( dest, name );                         \
    dest += sprintf( dest, fmt, value );                        \
    dest = chtoa( dest, ',' );                                  \
    return dest;                                                \
}

#define X( name, type, fmt ) json_num( name, type, fmt )
ALL_TYPES
#undef X

#endif // NO_SPRINTF
