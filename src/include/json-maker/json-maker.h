
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

#ifndef JSON_MAKER_H
#define	JSON_MAKER_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum json_scoped_object{
    JSON_ARRAY,
    JSON_OBJECT
} json_scoped_object_t;

typedef enum json_errcodes{
    JSON_NO_ERROR,
    JSON_GLOBAL_ERROR,
    JSON_OUT_OF_MEMORY
} json_errcodes_t;

typedef struct json_document{
    char* buffer;
    const size_t total_sz;
    size_t remaining_sz;
} json_buffer_t;
/** @defgroup makejoson Make JSON.
  * @{ */


json_errcodes_t json_start(json_buffer_t *const jsonBuffer);

/** Open a JSON object in a JSON string.
  * @param jsonBuffer Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @return Pointer to the new end of JSON under construction. */
json_errcodes_t json_objOpen(json_buffer_t* jsonBuffer, char const* name );

/** Close a JSON object in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @return Pointer to the new end of JSON under construction. */
json_errcodes_t json_objClose( json_buffer_t* jsonBuffer);

/** Used to finish the root JSON object. After call json_objClose().
  * @param dest Pointer to the end of JSON under construction.
  * @return Pointer to the new end of JSON under construction. */
void json_end( json_buffer_t* jsonBuffer );

/** Open an array in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @return Pointer to the new end of JSON under construction. */
json_errcodes_t json_arrOpen(json_buffer_t* jsonBuffer, char const* name );

/** Close an array in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @return Pointer to the new end of JSON under construction. */
json_errcodes_t json_arrClose( json_buffer_t* const jsonBuffer);

/** Add a text property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param value A valid null-terminated string with the value.
  *              Backslash escapes will be added for special characters.
  * @param len Max length of value. < 0 for unlimit.  
  * @return Pointer to the new end of JSON under construction. */
char* json_nstr( char* dest, char const* name, char const* value, int len );

/** Add a text property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param value A valid null-terminated string with the value.
  *              Backslash escapes will be added for special characters.
  * @return Pointer to the new end of JSON under construction. */
static inline char* json_str( char* dest, char const* name, char const* value ) {
    return json_nstr( dest, name, value, -1 );
}

/** Add a boolean property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param value Zero for false. Non zero for true.
  * @return Pointer to the new end of JSON under construction. */
char* json_bool( char* dest, char const* name, int value );

/** Add a null property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @return Pointer to the new end of JSON under construction. */
char* json_null( char* dest, char const* name );

/** Add an integer property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param value Value of the property.
  * @return Pointer to the new end of JSON under construction. */
char* json_int( char* dest, char const* name, int value );

/** Add an unsigned integer property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param value Value of the property.
  * @return Pointer to the new end of JSON under construction. */
char* json_uint( char* dest, char const* name, unsigned int value );

/** Add a long integer property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param value Value of the property.
  * @return Pointer to the new end of JSON under construction. */
char* json_long( char* dest, char const* name, long int value );

/** Add an unsigned long integer property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param value Value of the property.
  * @return Pointer to the new end of JSON under construction. */
char* json_ulong( char* dest, char const* name, unsigned long int value );

/** Add a long long integer property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param value Value of the property.
  * @return Pointer to the new end of JSON under construction. */
char* json_verylong( char* dest, char const* name, long long int value );

/** Add a double precision number property in a JSON string.
  * @param dest Pointer to the end of JSON under construction.
  * @param name Pointer to null-terminated string or null for unnamed.
  * @param value Value of the property.
  * @return Pointer to the new end of JSON under construction. */
char* json_double( char* dest, char const* name, double value );

/** @ } */

#ifdef	__cplusplus
}
#endif

#endif    /* JSON_MAKER_H */

