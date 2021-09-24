
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

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "json-maker/json-maker.h"

// ----------------------------------------------------- Test "framework": ---

#define done() return 0
#define fail() return __LINE__
static int checkqty = 0;
#define check( x ) do { ++checkqty; if (!(x)) fail(); } while ( 0 )

struct test {
    int(*func)(void);
    char const* name;
};

static int test_suit( struct test const* tests, int numtests ) {
    printf( "%s", "\n\nTests:\n" );
    int failed = 0;
    for( int i = 0; i < numtests; ++i ) {
        printf( " %02d%s%-25s \n", i, ": ", tests[i].name );
        int linerr = tests[i].func();
        if ( 0 == linerr )
            printf( "%s", "OK\n" );
        else {
            printf( "%s%d\n", "Failed, line: ", linerr );
            ++failed;
        }
    }
    printf( "\n%s%d\n", "Total checks: ", checkqty );
    printf( "%s[ %d / %d ]\r\n\n\n", "Tests PASS: ", numtests - failed, numtests );
    return failed;
}


// ----------------------------------------------------------- Unit tests: ---
typedef struct json_errcodes_map {
    json_errcodes_t errCode;
    char funcName[64];
} json_errcodes_map_t;

static inline void printErrCodeMap(const json_errcodes_map_t map[],
                                   const size_t mapSz)
{
    for (size_t i = 0; i < mapSz; i++)
    {
        printf("%s returned with 0x%02X\n", map[i].funcName, map[i].errCode);
    }
}

static int escape( void ) {
    char buff[512];
    json_buffer_t jsonBuffer = {
            .buffer = buff,
            .total_sz = sizeof(buff),
            .remaining_sz = sizeof(buff)
    };
    json_errcodes_map_t errcodesMap[] = {
            {.funcName = "json_start"},
            {.funcName = "json_objOpen"},
            {.funcName = "json_str"},
            {.funcName = "json_objClose"},
            {.funcName = "json_end"}
    };

    errcodesMap[0].errCode = json_start(&jsonBuffer);
    errcodesMap[1].errCode = json_objOpen( &jsonBuffer, NULL );
    errcodesMap[2].errCode = json_str( &jsonBuffer, "name", "\tHello: \"man\"\n" );
    errcodesMap[3].errCode = json_objClose( &jsonBuffer );
    errcodesMap[4].errCode = json_end( &jsonBuffer );
    printErrCodeMap(errcodesMap, (sizeof(errcodesMap)/sizeof(errcodesMap[0])));
    printf( "\n\n%s\n\n", buff );

    static char const rslt[] = "{\"name\":\"\\tHello: \\\"man\\\"\\n\"}";
    check(strlen(buff) == sizeof rslt - 1 );
    check( 0 == strcmp( buff, rslt ) );
    done();
}

static int len( void ) {
    char buff[512];
    json_buffer_t jsonBuffer = {
            .buffer = buff,
            .total_sz = sizeof(buff),
            .remaining_sz = sizeof(buff)
    };
    json_errcodes_map_t errcodesMap[] = {
            {.funcName = "json_start"},
            {.funcName = "json_objOpen"},
            {.funcName = "json_nstr"},
            {.funcName = "json_objClose"},
            {.funcName = "json_end"}
    };
    errcodesMap[0].errCode = json_start(&jsonBuffer);
    errcodesMap[1].errCode = json_objOpen( &jsonBuffer, NULL );
    errcodesMap[2].errCode = json_nstr( &jsonBuffer, "name", "\tHello: \"man\"\n", 6 );
    errcodesMap[3].errCode = json_objClose( &jsonBuffer );
    errcodesMap[4].errCode = json_end( &jsonBuffer );
    printErrCodeMap(errcodesMap, sizeof(errcodesMap)/sizeof(errcodesMap[0]));
    printf( "\n\n%s\n\n", buff );

    static char const rslt[] = "{\"name\":\"\\tHello\"}";
    check(strlen(buff) == sizeof rslt - 1 );
    check( 0 == strcmp( buff, rslt ) );
    done();
}

static int empty( void ) {
    char buff[512];
    json_buffer_t jsonBuffer = {
            .buffer = buff,
            .total_sz = sizeof(buff),
            .remaining_sz = sizeof(buff)
    };
    {
        json_errcodes_map_t errcodesMap[] = {
                {.funcName = "json_start"},
                {.funcName = "json_objOpen"},
                {.funcName = "json_objClose"},
                {.funcName = "json_end"}
        };
        errcodesMap[0].errCode = json_start(&jsonBuffer);
        errcodesMap[1].errCode = json_objOpen( &jsonBuffer, NULL );
        errcodesMap[2].errCode = json_objClose( &jsonBuffer );
        errcodesMap[3].errCode = json_end( &jsonBuffer );
        printErrCodeMap(errcodesMap, sizeof(errcodesMap)/sizeof(errcodesMap[0]));
        printf( "\n\n%s\n\n", buff );

        static char const rslt[] = "{}";
        check(strlen(buff) == sizeof rslt - 1 );
        check( 0 == strcmp( buff, rslt ) );
    }
    {
        json_errcodes_map_t errcodesMap[] = {
                {.funcName = "json_start"},
                {.funcName = "json_objOpen"},
                {.funcName = "json_arrOpen"},
                {.funcName = "json_arrClose"},
                {.funcName = "json_objClose"},
                {.funcName = "json_end"}
        };
        errcodesMap[0].errCode = json_start(&jsonBuffer);
        errcodesMap[1].errCode = json_objOpen( &jsonBuffer, NULL );
        errcodesMap[2].errCode = json_arrOpen( &jsonBuffer, "a" );
        errcodesMap[3].errCode = json_arrClose( &jsonBuffer );
        errcodesMap[4].errCode = json_objClose( &jsonBuffer );
        errcodesMap[5].errCode = json_end( &jsonBuffer );
        printErrCodeMap(errcodesMap, sizeof(errcodesMap)/sizeof(errcodesMap[0]));
        printf( "\n\n%s\n\n", buff );

        static char const rslt[] = "{\"a\":[]}";
        check(strlen(buff) == sizeof rslt - 1 );
        check( 0 == strcmp( buff, rslt ) );
    }
    {
        json_errcodes_map_t errcodesMap[] = {
                {.funcName = "json_start"},
                {.funcName = "json_objOpen"},
                {.funcName = "json_arrOpen"},
                {.funcName = "json_objOpen"},
                {.funcName = "json_objClose"},
                {.funcName = "json_objOpen"},
                {.funcName = "json_objClose"},
                {.funcName = "json_arrClose"},
                {.funcName = "json_objClose"},
                {.funcName = "json_end"}
        };
        errcodesMap[0].errCode = json_start(&jsonBuffer);
        errcodesMap[1].errCode = json_objOpen( &jsonBuffer, NULL );
        errcodesMap[2].errCode = json_arrOpen( &jsonBuffer, "a" );
        errcodesMap[3].errCode = json_objOpen( &jsonBuffer, NULL );
        errcodesMap[4].errCode = json_objClose( &jsonBuffer );
        errcodesMap[5].errCode = json_objOpen( &jsonBuffer, NULL );
        errcodesMap[6].errCode = json_objClose( &jsonBuffer );
        errcodesMap[7].errCode = json_arrClose( &jsonBuffer );
        errcodesMap[8].errCode = json_objClose( &jsonBuffer );
        errcodesMap[9].errCode = json_end( &jsonBuffer );
        printErrCodeMap(errcodesMap, sizeof(errcodesMap)/sizeof(errcodesMap[0]));
        printf( "\n\n%s\n\n", buff );

        static char const rslt[] = "{\"a\":[{},{}]}";
        check(strlen(buff) == sizeof rslt - 1 );
        check( 0 == strcmp( buff, rslt ) );
    }
    done();
}

#ifndef LONG_LONG_MAX
#define LONG_LONG_MAX LLONG_MAX
#define LONG_LONG_MIN LLONG_MIN
#endif

static int primitive( void ) {
    char buff[512];
    json_buffer_t jsonBuffer = {
            .buffer = buff,
            .total_sz = sizeof(buff),
            .remaining_sz = sizeof(buff)
    };
    json_errcodes_map_t errcodesMap[] = {
            {.funcName = "json_start"},
            {.funcName = "json_objOpen"},
            {.funcName = "json_verylong"},
            {.funcName = "json_verylong"},
            {.funcName = "json_bool"},
            {.funcName = "json_bool"},
            {.funcName = "json_null"},
            {.funcName = "json_objClose"},
            {.funcName = "json_end"}
    };
    errcodesMap[0].errCode = json_start(&jsonBuffer);
    errcodesMap[1].errCode = json_objOpen( &jsonBuffer, NULL );
    errcodesMap[2].errCode = json_verylong( &jsonBuffer, "max", LONG_LONG_MAX);
    errcodesMap[3].errCode = json_verylong( &jsonBuffer, "min", LONG_LONG_MIN);
    errcodesMap[4].errCode = json_bool( &jsonBuffer, "boolvar0", false );
    errcodesMap[5].errCode = json_bool( &jsonBuffer, "boolvar1", true);
    errcodesMap[6].errCode = json_null( &jsonBuffer, "nullvar");
    errcodesMap[7].errCode = json_objClose( &jsonBuffer );
    errcodesMap[8].errCode = json_end( &jsonBuffer );
    printErrCodeMap(errcodesMap, sizeof(errcodesMap)/sizeof(errcodesMap[0]));
    printf( "\n\n%s\n\n", buff );

    static char const rslt[] =  "{"
                                    "\"max\":9223372036854775807,"
                                    "\"min\":-9223372036854775808,"
                                    "\"boolvar0\":false,"
                                    "\"boolvar1\":true,"
                                    "\"nullvar\":null"
                                "}";
    check(strlen(buff) == sizeof rslt - 1 );
    check( 0 == strcmp( buff, rslt ) );
    done();
}


static int integers( void ) {
    char buff[64];
    json_buffer_t jsonBuffer = {
            .buffer = buff,
            .total_sz = sizeof(buff),
            .remaining_sz = sizeof(buff)
    };
    {
        json_errcodes_map_t errcodesMap[] = {
                {.funcName = "json_start"},
                {.funcName = "json_objOpen"},
                {.funcName = "json_int"},
                {.funcName = "json_int"},
                {.funcName = "json_objClose"},
                {.funcName = "json_end"}
        };
        errcodesMap[0].errCode = json_start(&jsonBuffer);
        errcodesMap[1].errCode = json_objOpen( &jsonBuffer, NULL );
        errcodesMap[2].errCode = json_int( &jsonBuffer, "a", 0 );
        errcodesMap[3].errCode = json_int( &jsonBuffer, "b", 1 );
        errcodesMap[4].errCode = json_objClose( &jsonBuffer );
        errcodesMap[5].errCode = json_end( &jsonBuffer );
        printErrCodeMap(errcodesMap, sizeof(errcodesMap)/sizeof(errcodesMap[0]));
        printf( "\n\n%s\n\n", buff );

        static char const rslt[] = "{\"a\":0,\"b\":1}";
        check( strlen(buff) == sizeof rslt - 1 );
        check( 0 == strcmp( buff, rslt ) );
    }
    {
        json_errcodes_map_t errcodesMap[] = {
                {.funcName = "json_start"},
                {.funcName = "json_objOpen"},
                {.funcName = "json_int"},
                {.funcName = "json_int"},
                {.funcName = "json_objClose"},
                {.funcName = "json_end"}
        };
        errcodesMap[0].errCode = json_start(&jsonBuffer);
        errcodesMap[1].errCode = json_objOpen( &jsonBuffer, NULL );
        errcodesMap[2].errCode = json_int( &jsonBuffer, "max", INT_MAX );
        errcodesMap[3].errCode = json_int( &jsonBuffer, "min", INT_MIN );
        errcodesMap[4].errCode = json_objClose( &jsonBuffer );
        errcodesMap[5].errCode = json_end( &jsonBuffer );
        printErrCodeMap(errcodesMap, sizeof(errcodesMap)/sizeof(errcodesMap[0]));
        printf( "\n\n%s\n\n", buff );

        char rslt[ sizeof buff ];
        int len = sprintf( rslt, "{\"max\":%d,\"min\":%d}", INT_MAX, INT_MIN );
        check( len < sizeof buff );
        check(strlen(buff) == len );
        check( 0 == strcmp( buff, rslt ) );
    }
    {
        json_errcodes_map_t errcodesMap[] = {
                {.funcName = "json_start"},
                {.funcName = "json_objOpen"},
                {.funcName = "json_uint"},
                {.funcName = "json_objClose"},
                {.funcName = "json_end"}
        };
        errcodesMap[0].errCode = json_start(&jsonBuffer);
        errcodesMap[1].errCode = json_objOpen( &jsonBuffer, NULL );
        errcodesMap[2].errCode = json_uint( &jsonBuffer, "max", UINT_MAX );
        errcodesMap[3].errCode = json_objClose( &jsonBuffer );
        errcodesMap[4].errCode = json_end( &jsonBuffer );
        printErrCodeMap(errcodesMap, sizeof(errcodesMap)/sizeof(errcodesMap[0]));
        printf( "\n\n%s\n\n", buff );

        char rslt[ sizeof buff ];
        int len = sprintf( rslt, "{\"max\":%u}", UINT_MAX );
        check( len < sizeof buff );
        check(strlen(buff) == len );
        check( 0 == strcmp( buff, rslt ) );
    }
    {
        json_errcodes_map_t errcodesMap[] = {
                {.funcName = "json_start"},
                {.funcName = "json_objOpen"},
                {.funcName = "json_long"},
                {.funcName = "json_long"},
                {.funcName = "json_objClose"},
                {.funcName = "json_end"}
        };
        errcodesMap[0].errCode = json_start(&jsonBuffer);
        errcodesMap[1].errCode = json_objOpen( &jsonBuffer, NULL );
        errcodesMap[2].errCode = json_long( &jsonBuffer, "max", LONG_MAX);
        errcodesMap[3].errCode = json_long( &jsonBuffer, "min", LONG_MIN);
        errcodesMap[4].errCode = json_objClose( &jsonBuffer );
        errcodesMap[5].errCode = json_end( &jsonBuffer );
        printErrCodeMap(errcodesMap, sizeof(errcodesMap)/sizeof(errcodesMap[0]));
        printf( "\n\n%s\n\n", buff );

        char rslt[ sizeof buff ];
        int len = sprintf( rslt, "{\"max\":%ld,\"min\":%ld}", LONG_MAX, LONG_MIN );
        check( len < sizeof buff );
        check( strlen(buff) == len );
        check( 0 == strcmp( buff, rslt ) );
    }
    {
        json_errcodes_map_t errcodesMap[] = {
                {.funcName = "json_start"},
                {.funcName = "json_objOpen"},
                {.funcName = "json_ulong"},
                {.funcName = "json_objClose"},
                {.funcName = "json_end"}
        };
        errcodesMap[0].errCode = json_start(&jsonBuffer);
        errcodesMap[1].errCode = json_objOpen( &jsonBuffer, NULL );
        errcodesMap[2].errCode = json_ulong( &jsonBuffer, "max", ULONG_MAX);
        errcodesMap[3].errCode = json_objClose( &jsonBuffer );
        errcodesMap[4].errCode = json_end( &jsonBuffer );
        printErrCodeMap(errcodesMap, sizeof(errcodesMap)/sizeof(errcodesMap[0]));
        printf( "\n\n%s\n\n", buff );

        char rslt[ sizeof buff ];
        int len = sprintf( rslt, "{\"max\":%lu}", ULONG_MAX );
        check( len < sizeof buff );
        check( strlen(buff) == len );
        check( 0 == strcmp( buff, rslt ) );
    }
    {
        json_errcodes_map_t errcodesMap[] = {
                {.funcName = "json_start"},
                {.funcName = "json_objOpen"},
                {.funcName = "json_verylong"},
                {.funcName = "json_verylong"},
                {.funcName = "json_objClose"},
                {.funcName = "json_end"}
        };
        errcodesMap[0].errCode = json_start(&jsonBuffer);
        errcodesMap[1].errCode = json_objOpen( &jsonBuffer, NULL );
        errcodesMap[2].errCode = json_verylong( &jsonBuffer, "max", LONG_LONG_MAX);
        errcodesMap[3].errCode = json_verylong( &jsonBuffer, "min", LONG_LONG_MIN);
        errcodesMap[4].errCode = json_objClose( &jsonBuffer );
        errcodesMap[5].errCode = json_end( &jsonBuffer );
        printErrCodeMap(errcodesMap, sizeof(errcodesMap)/sizeof(errcodesMap[0]));
        printf( "\n\n%s\n\n", buff );

        char rslt[ sizeof buff ];
        int len = sprintf( rslt, "{\"max\":%lld,\"min\":%lld}", LONG_LONG_MAX, LONG_LONG_MIN );
        check( len < sizeof buff );
        check( strlen(buff) == len );
        check( 0 == strcmp( buff, rslt ) );
    }
    done();
}

static int array( void ) {
    char buff[64];
    json_buffer_t jsonBuffer = {
            .buffer = buff,
            .total_sz = sizeof(buff),
            .remaining_sz = sizeof(buff)
    };
    json_errcodes_map_t errcodesMap[] = {
            {.funcName = "json_start"},
            {.funcName = "json_objOpen"},
            {.funcName = "json_arrOpen"},
            {.funcName = "json_int"},
            {.funcName = "json_arrClose"},
            {.funcName = "json_objClose"},
            {.funcName = "json_end"}
    };
    errcodesMap[0].errCode = json_start(&jsonBuffer);
    errcodesMap[1].errCode = json_objOpen( &jsonBuffer, NULL );
    errcodesMap[2].errCode = json_arrOpen( &jsonBuffer, "a");
    for( int i = 0; i < 4; ++i ) {
        errcodesMap[3].errCode = json_int(&jsonBuffer, NULL, i);
    }
    errcodesMap[4].errCode = json_arrClose( &jsonBuffer );
    errcodesMap[5].errCode = json_objClose( &jsonBuffer );
    errcodesMap[6].errCode = json_end( &jsonBuffer );
    printErrCodeMap(errcodesMap, sizeof(errcodesMap)/sizeof(errcodesMap[0]));
    printf( "\n\n%s\n\n", buff );

    static char const rslt[] = "{\"a\":[0,1,2,3]}";
    check( strlen(buff) == sizeof rslt - 1 );
    check( 0 == strcmp( buff, rslt ) );
    done();
}

static int real( void ) {
    char buff[64];
    json_buffer_t jsonBuffer = {
            .buffer = buff,
            .total_sz = sizeof(buff),
            .remaining_sz = sizeof(buff)
    };
    json_errcodes_map_t errcodesMap[] = {
            {.funcName = "json_start"},
            {.funcName = "json_objOpen"},
            {.funcName = "json_arrOpen"},
            {.funcName = "json_double"},
            {.funcName = "json_arrClose"},
            {.funcName = "json_objClose"},
            {.funcName = "json_end"}
    };
    errcodesMap[0].errCode = json_start(&jsonBuffer);
    errcodesMap[1].errCode = json_objOpen( &jsonBuffer, NULL );
    errcodesMap[2].errCode = json_arrOpen( &jsonBuffer, "data");
    static double const lut[] = { 0.2, 2e-6, 5e6 };
    for( int i = 0; i < sizeof lut / sizeof lut[0]; ++i ) {
        errcodesMap[3].errCode = json_double(&jsonBuffer, NULL, lut[i]);
    }
    errcodesMap[4].errCode = json_arrClose( &jsonBuffer );
    errcodesMap[5].errCode = json_objClose( &jsonBuffer );
    errcodesMap[6].errCode = json_end( &jsonBuffer );
    printErrCodeMap(errcodesMap, sizeof(errcodesMap)/sizeof(errcodesMap[0]));
    printf( "\n\n%s\n\n", buff );

    static char const rslt1[] = "{\"data\":[0.2,2e-006,5e+006]}";
    static char const rslt2[] = "{\"data\":[0.2,2e-06,5e+06]}";
    check( strlen(buff) == sizeof rslt1 - 1 || strlen(buff) == sizeof rslt2 - 1 );
    check( 0 == strcmp( buff, rslt1 ) ||  0 == strcmp( buff, rslt2 ) );
    done();
}

// --------------------------------------------------------- Execute tests: ---

int main( void ) {
    static struct test const tests[] = {
        { escape,    "Escape characters"        },
        { len,       "Non-null-terminated"      },
        { empty,     "Empty objects and arrays" },
        { primitive, "Primitives values"        },
        { integers,  "Integers values"          },
        { array,     "Array"                    },
        { real,      "Real"                     }
    };
    return test_suit( tests, sizeof tests / sizeof *tests );
}
