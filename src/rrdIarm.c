##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2018 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Initialize component
AC_INIT([remote-debugger], [1.0], [naveenkumar_hanasi@comcast.com])
AM_INIT_AUTOMAKE([foreign])
LT_INIT
GTEST_ENABLE_FLAG = ""

AC_ARG_ENABLE([gtestapp],
             AS_HELP_STRING([--enable-gtestapp],[enable Gtest support (default is no)]),
             [
               case "${enableval}" in
                yes) GTEST_SUPPORT_ENABLED=true
                     GTEST_ENABLE_FLAG="-DGTEST_ENABLE"
                     m4_if(m4_sysval,[0],[AC_CONFIG_FILES([src/unittest/Makefile])]);;
                no) GTEST_SUPPORT_ENABLED=false AC_MSG_ERROR([Gtest support is disabled]);;
                 *) AC_MSG_ERROR([bad value ${enableval} for --enable-gtestapp ]);;
               esac
             ],
             [echo "Gtestapp is disabled"])
AM_CONDITIONAL([WITH_GTEST_SUPPORT], [test x$GTEST_SUPPORT_ENABLED = xtrue])

AC_ARG_ENABLE([iarmbus],
	      AS_HELP_STRING([--enable-iarmbus], [enable IARMBus support (default is no)]),
	      [
                case "${enableval}" in
                  yes) USE_IARMBUS=true;;
                  no) AC_MSG_ERROR([IARMBus support is disabled]);;
                  *) AC_MSG_ERROR([bad value ${enableval} for --enable-iarmbus]);;
                esac
              ],
	      [echo "IARMBus support is disabled"])
AM_CONDITIONAL([USE_IARMBUS], [test x$USE_IARMBUS = xtrue])

# Config Source files.
AC_CONFIG_SRCDIR([src/rrdMain.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AM_PROG_LIBTOOL(libtool)

# Checks for libraries.
PKG_CHECK_MODULES([CJSON],[libcjson >= 1.7.0])

# Checks for header files.
AC_CHECK_HEADERS([stdio.h string.h unistd.h stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

AC_CONFIG_FILES([Makefile
                 src/Makefile])

AC_SUBST(GTEST_ENABLE_FLAG)
AC_OUTPUT
