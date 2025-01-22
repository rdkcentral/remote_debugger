/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>

#ifndef __SECURE_H
#  define __SECURE_H
#  ifdef __cplusplus
extern "C" {
#  endif

__attribute__((nonnull))
__attribute__((format(printf,1,2)))
int v_secure_system(const char *command, ...);

__attribute__((nonnull))
__attribute__((format(printf,2,3)))
FILE *v_secure_popen(const char *direction, const char *command, ...);

__attribute__((nonnull))
int v_secure_pclose(FILE *);

/* OBSOLETE CONSTANTS */
#define _SPIPE "|"
#define _SOR "||"
#define _SAND "&&"
#define _SBG "&"
#define _STHEN ";"

/* The following is just some gcc magic to make sure
 * 1) the format string isn't a variable
 * 2) the number of arguments matches the format
 * 3) popen's direction arg is "r" or "w"
 */

#if (defined (__GNUC__) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))) || defined(__clang__)
#define PRAGMA_PUSH                \
    _Pragma ("GCC diagnostic push")                       \
    _Pragma ("GCC diagnostic error \"-Wformat\"")       \
    _Pragma ("GCC diagnostic error \"-Wformat-security\"")
#define PRAGMA_POP             \
    _Pragma ("GCC diagnostic pop")
#else
#define PRAGMA_PUSH
#define PRAGMA_POP
#endif

#define v_secure_system(fmt, args...) \
	({ \
		int ret; \
                PRAGMA_PUSH; \
		if (!__builtin_constant_p(fmt)) { \
		extern void format_error() __attribute__((error("command argument cannot be a variable\nreplace \"sprintf(buffer, command, args); v_secure_system(buffer);\" with \"v_secure_system(command, args);\""))); \
			format_error(); \
		} \
		ret = v_secure_system(fmt, ##args); \
                PRAGMA_POP; \
		ret; \
	})

#define v_secure_popen(direction, fmt, args...) \
	({ \
		FILE *ret; \
                PRAGMA_PUSH; \
		if ( \
			__builtin_constant_p(*direction) && \
			((direction[0] != 'r' && direction[0] != 'w') || direction[1] != '\0') \
		) { \
			extern void popen_check() __attribute__((error("v_secure_popen(direction, command, ...) direction must be \"r\" or \"w\""))); \
			popen_check(); \
		} \
		if (!__builtin_constant_p(fmt)) { \
			extern void format_error() __attribute__((error("command argument cannot be a variable"))); \
			format_error(); \
		} \
		ret = v_secure_popen(direction, fmt, ##args); \
		PRAGMA_POP; \
                ret; \
	})

extern int system(const char *command) __attribute__((warning("please replace system() with v_secure_system()")));

__attribute__((deprecated))
__attribute__((warning("contains_secure_separator is obsolete")))
static inline int contains_secure_separator(__attribute__((unused)) char *str) {
	return 0;
}

extern int secure_system_call_p(const char *cmd, char *argv[]);
extern int secure_system_call_vp(const char *cmd, ...);

#  ifdef __cplusplus
}
#  endif
#endif
