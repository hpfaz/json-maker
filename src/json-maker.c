
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
#include "json-maker/json-maker.h"
#include <string.h>
#include <stdio.h>

static json_errcodes_t primitivename( json_buffer_t* jsonBuffer, char const* name );
static char escape( int ch );
static char nibbletoch( int nibble );
static json_errcodes_t atoesc( json_buffer_t* jsonBuffer, char const* src, size_t len );
static json_errcodes_t strname( json_buffer_t* jsonBuffer,
                                char const* name );
static json_errcodes_t chtoa( json_buffer_t *info_p, const char ch );
static json_errcodes_t atoa( json_buffer_t* buff, char const* src );
static inline char *buff_seekCur( const json_buffer_t* jsonBuffer);
static inline bool buff_hasEnoughSpaceFor( const json_buffer_t*  info_p,
                                           size_t data_sz);
static inline char* buff_seekOffsetFromCur( const json_buffer_t* jsonBuffer,
                                            int offset);
static inline json_errcodes_t buff_increaseRemSize( json_buffer_t* jsonBuffer,
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
        3 > jsonBuffer->total_sz )
    {
        errCode = JSON_GLOBAL_ERROR;
        if(3 > jsonBuffer->total_sz) {
            errCode = JSON_OUT_OF_MEMORY;
        }
    } else {
        memset(jsonBuffer->buffer, 0, jsonBuffer->total_sz);
        jsonBuffer->remaining_sz = jsonBuffer->total_sz - 1;
    }

    return errCode;
}

/* Used to finish the root JSON object. After call json_objClose(). */
json_errcodes_t json_end( json_buffer_t* const jsonBuffer ) {
    json_errcodes_t errCode = JSON_NO_ERROR;

    if(NULL == jsonBuffer)
    {
        errCode = JSON_GLOBAL_ERROR;
    } else {
        char *const lastBuffChar = buff_seekOffsetFromCur(jsonBuffer, -1);
        if (',' == *lastBuffChar) {
            *lastBuffChar = '\0';
        }
    }

    return errCode;
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
json_errcodes_t json_nstr( json_buffer_t* jsonBuffer,
                           char const* name,
                           char const* value,
                           const size_t len ) {
    const size_t nameLen = NULL == name ? 0 : strlen(name);
    const size_t valueStrlen = NULL == value ? 0 : strlen(value);
    const size_t valueRealLen = valueStrlen < len ? valueStrlen : len;

    json_errcodes_t errCode;

    if(NULL == jsonBuffer || NULL == value)
    {
        errCode = JSON_GLOBAL_ERROR;
    } else if(!buff_hasEnoughSpaceFor(jsonBuffer, valueRealLen + nameLen))
    {
        errCode = JSON_OUT_OF_MEMORY;
    } else {
        strname( jsonBuffer, name );
        atoesc( jsonBuffer, value, len );
        errCode = atoa( jsonBuffer, "\"," );
    }

    return errCode;
}

/*  Add a boolean property in a JSON string. */
json_errcodes_t json_bool(json_buffer_t* const jsonBuffer, char const* const name, const bool value ) {
    json_errcodes_t errCode = JSON_NO_ERROR;

    if(NULL == jsonBuffer) {
        errCode = JSON_GLOBAL_ERROR;
    } else {
        if(JSON_NO_ERROR == primitivename(jsonBuffer, name)) {
            errCode = atoa(jsonBuffer, value ? "true," : "false,");
        }
    }

    return errCode;
}

/* Add a null property in a JSON string. */
json_errcodes_t json_null( json_buffer_t* const jsonBuffer, char const* const name ) {
    json_errcodes_t errCode = JSON_NO_ERROR;

    if(NULL == jsonBuffer) {
        errCode = JSON_GLOBAL_ERROR;
    } else {
        if(JSON_NO_ERROR == primitivename(jsonBuffer, name)) {
            errCode = atoa(jsonBuffer, "null,");
        }
    }

    return errCode;
}

/** Add the name of a text property.
  * @param dest Destination memory.
  * @param name The name of the property.
  * @return Pointer to the next char. */
static json_errcodes_t strname( json_buffer_t* const jsonBuffer,
                                char const* const name ) {
    const size_t nameLen = NULL == name ? 0 : strlen(name);
    json_errcodes_t errCode;

    if( NULL == jsonBuffer ) {
        errCode = JSON_GLOBAL_ERROR;
    } else if(!buff_hasEnoughSpaceFor(jsonBuffer, nameLen)) {
        errCode = JSON_OUT_OF_MEMORY;
    } else {
        errCode = chtoa(jsonBuffer, '\"' );
        if ( NULL != name ) {
            atoa( jsonBuffer, name );
            errCode = atoa( jsonBuffer, "\":\"" );
        }
    }

    return errCode;
}

/** Get the hexadecimal digit of the least significant nibble of a integer. */
static char nibbletoch( int nibble ) {
    return "0123456789ABCDEF"[ nibble % 16u ];
}

/** Get the escape character of a non-printable.
  * @param ch Character source.
  * @return The escape character or null character if error. */
static char escape( int ch ) {
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
  * @param len Max length of source.
  * @return Pointer to the null character of the destination string. */
static json_errcodes_t atoesc( json_buffer_t* const jsonBuffer, char const* const src, const size_t len ) {
    const size_t srcStrlen  = NULL == src ? 0 : strlen(src);
    const size_t srcRealLen = srcStrlen < len ? srcStrlen : len;
    json_errcodes_t errCode = JSON_NO_ERROR;

    if(NULL == jsonBuffer)
    {
        errCode = JSON_GLOBAL_ERROR;
    } else if (!buff_hasEnoughSpaceFor(jsonBuffer, srcRealLen)) {
        errCode = JSON_OUT_OF_MEMORY;
    } else {
        char * const curBuffPos = buff_seekCur(jsonBuffer);
        size_t additionalCharsLen = 0;
        size_t offsetFromSrc = 0;

        for (size_t i = 0; i < srcRealLen && errCode == JSON_NO_ERROR; ++i) {
            const size_t destIdx = i + offsetFromSrc;
            size_t tmpAdditionalCharsLen = 0;

            if (src[i] >= ' ' && src[i] != '\"' && src[i] != '\\' && src[i] != '/') {
                curBuffPos[destIdx] = src[i];
                buff_decreaseRemSize(jsonBuffer, 1);
            } else {
                tmpAdditionalCharsLen += 2;
                if (buff_hasEnoughSpaceFor(jsonBuffer, srcRealLen +
                                            additionalCharsLen +
                                            tmpAdditionalCharsLen)) {
                    curBuffPos[destIdx] = '\\';
                    char const esc = escape(src[i]);
                    if (esc) {
                        curBuffPos[destIdx + 1] = esc;
                    } else {
                        tmpAdditionalCharsLen += 4;
                        if (buff_hasEnoughSpaceFor(jsonBuffer,
                                                   srcRealLen +
                                                   additionalCharsLen +
                                                   tmpAdditionalCharsLen)) {
                            curBuffPos[destIdx + 1] = 'u';
                            curBuffPos[destIdx + 2] = '0';
                            curBuffPos[destIdx + 3] = '0';
                            curBuffPos[destIdx + 4] = nibbletoch(src[i] / 16);
                            curBuffPos[destIdx + 5] = nibbletoch(src[i]);
                        } else {
                            errCode = JSON_OUT_OF_MEMORY;
                        }
                    }
                } else {
                    errCode = JSON_OUT_OF_MEMORY;
                }
                buff_decreaseRemSize(jsonBuffer, tmpAdditionalCharsLen);
                additionalCharsLen += tmpAdditionalCharsLen;
                offsetFromSrc += tmpAdditionalCharsLen - 1;
            }
        }
        buff_seekCur(jsonBuffer)[0] = '\0';
    }

    return errCode;
}

/** Add the name of a primitive property.
  * @param dest Destination memory.
  * @param name The name of the property.
  * @return Pointer to the next char. */
static json_errcodes_t primitivename( json_buffer_t* const jsonBuffer, char const* const name ) {
    json_errcodes_t errCode = JSON_NO_ERROR;

    if(NULL == jsonBuffer)
    {
        errCode = JSON_GLOBAL_ERROR;
    }
    else if( NULL != name) {
        if(!buff_hasEnoughSpaceFor(jsonBuffer, strlen(name))) {
            errCode = JSON_OUT_OF_MEMORY;
        } else {
             chtoa(jsonBuffer, '\"');
             atoa(jsonBuffer, name);
             errCode = atoa(jsonBuffer, "\":");
        }
    }

    return errCode;
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
    size_t src_strlen = NULL == src ? 0 : strlen(src);

    if ( NULL == buff ||
         NULL == src ||
         NULL == buff->buffer)
    {
        errCode = JSON_GLOBAL_ERROR;
    }
    else if(!buff_hasEnoughSpaceFor(buff, src_strlen))
    {
        errCode = JSON_OUT_OF_MEMORY;
    } else {
        char* const dest = buff_seekCur(buff);

        for (size_t i = 0; i < src_strlen; i++)
        {
            dest[i] = src[i];
        }

        dest[src_strlen] = '\0';
        buff_decreaseRemSize(buff, src_strlen);
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

static inline char* buff_seekCur( const json_buffer_t* const jsonBuffer) {
    return jsonBuffer->buffer + (jsonBuffer->total_sz - jsonBuffer->remaining_sz - 1);
}

static inline char* buff_seekOffsetFromCur( const json_buffer_t* const jsonBuffer,
                                            const int offset) {
    char* const buffer_end = jsonBuffer->buffer + jsonBuffer->total_sz;

    char *pos = buff_seekCur(jsonBuffer) + offset;

    if ( pos > buffer_end ) {
        pos = buffer_end;
    } else if (pos < jsonBuffer->buffer) {
        pos = jsonBuffer->buffer;
    }

    return pos;
}

static inline json_errcodes_t buff_increaseRemSize( json_buffer_t* const jsonBuffer,
                                          const size_t sizeIncrease ) {
    json_errcodes_t errCode = JSON_NO_ERROR;
    const size_t newSize = jsonBuffer->remaining_sz + sizeIncrease;

    // Always need one extra byte for trailing \0
    if (newSize > jsonBuffer->total_sz - 1 ) {
        errCode = JSON_OUT_OF_MEMORY;
    } else {
        jsonBuffer->remaining_sz = newSize;
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

// ----
// int is type,
json_errcodes_t funcname( json_buffer_t* const jsonBuffer, char const* const name, const int value ) {
json_errcodes_t errCode = JSON_NO_ERROR;

    if(NULL == jsonBuffer) {
        errCode = JSON_GLOBAL_ERROR;
    } else if( JSON_NO_ERROR == (errCode = primitivename( jsonBuffer, name )) ) {
        // Approximation of log10 + nullbyte
        char tmpBuff[(241 * sizeof(int) / 100 + 1)] = {0};
        const size_t nbrSz = sprintf( tmpBuff, "%d", value );

        if(!buff_hasEnoughSpaceFor(jsonBuffer, nbrSz)) {
            errCode = JSON_OUT_OF_MEMORY;
        } else {
            if ( JSON_NO_ERROR == (errCode = atoa(jsonBuffer, tmpBuff))) {
                errCode = chtoa( jsonBuffer, ',' );}
        }
    }
        return errCode;
}

#define ALL_TYPES \
    X( json_int,      int,           "%d"   ) \
    X( json_long,     long,          "%ld"  ) \
    X( json_uint,     unsigned int,  "%u"   ) \
    X( json_ulong,    unsigned long, "%lu"  ) \
    X( json_verylong, long long,     "%lld" ) \
    X( json_double,   double,        "%g"   ) \


#define json_num( funcname, type, fmt ) \
json_errcodes_t funcname( json_buffer_t* const jsonBuffer,                    \
                          char const* const name,                             \
                          const type value ) {                                \
    json_errcodes_t errCode = JSON_NO_ERROR;                                  \
                                                                              \
    if(NULL == jsonBuffer) {                                                  \
        errCode = JSON_GLOBAL_ERROR;                                          \
    } else if( JSON_NO_ERROR ==                                               \
                  (errCode = primitivename( jsonBuffer, name )) ) {           \
        /* Approximation of log10 + nullbyte */                               \
        char tmpBuff[(241 * sizeof(type) / 100 + 1)] = {0};                   \
        const size_t nbrSz = sprintf( tmpBuff, fmt, value );                  \
                                                                              \
        if(!buff_hasEnoughSpaceFor(jsonBuffer, nbrSz)) {                      \
            errCode = JSON_OUT_OF_MEMORY;                                     \
        } else if ( JSON_NO_ERROR == (errCode = atoa(jsonBuffer, tmpBuff))) { \
            errCode = chtoa( jsonBuffer, ',' );                               \
        }                                                                     \
    }                                                                         \
    return errCode;                                                           \
}


#define X( name, type, fmt ) json_num( name, type, fmt )
ALL_TYPES
#undef X
