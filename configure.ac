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


AC_ARG_ENABLE([wifi],
        AS_HELP_STRING([--enable-wifi],[enable WiFi profile (default is no)]),
        [
          case "${enableval}" in
           yes) WIFI_PROFILE_ENABLE=true
                WIFI_PROFILE_FLAG="-DUSE_WIFI_PROFILE"
                #m4_syscmd([test -d src/hostif/profiles/wifi])
                m4_if(m4_sysval,[0],[AC_CONFIG_FILES([src/Makefile])])
                m4_if(m4_sysval,[0],[SUBDIRS_WIFI="src"])  ;;
           no) WIFI_PROFILE_ENABLE=false AC_MSG_ERROR([wifi profile is disabled]) ;;
          *) AC_MSG_ERROR([bad value ${enableval} for --enable-wifi ]) ;;
           esac
           ],
         [echo "WiFi profile is disabled"])
L2_SUPPORT_FLAG =""
AC_ARG_ENABLE([l2support],
  AS_HELP_STRING([--enable-l2support], [enable L2 support (default is no)]),
  [
    case "${enableval}" in
      yes)
        L2_SUPPORT_ENABLED=true
        L2_ENABLE_FLAG="-DL2_SUPPORT"
        AC_CONFIG_FILES([Makefile src/Makefile])
        ;;
      no)
        L2_SUPPORT_ENABLED=false
        ;;
      *)
        AC_MSG_ERROR([bad value ${enableval} for --enable-l2support])
        ;;
    esac
  ],
  [echo "L2 support is disabled"])
AC_SUBST(L2_ENABLE_FLAG)
AM_CONDITIONAL([WITH_L2_SUPPORT], [test x$L2_SUPPORT_ENABLED = xtrue])
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

# IARMBus Support
AC_ARG_ENABLE([iarmbusSupport],
[  --enable-iarmbusSupport    Turn on iarmbus support],
[case "${enableval}" in
  yes) iarmbusSupport=true ;;
  no)  iarmbusSupport=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-iarmbusSupport]) ;;
esac],[iarmbusSupport=false])
AM_CONDITIONAL([IARMBUS_ENABLE], [test x$iarmbusSupport = xtrue])
# L2 Support
#AC_ARG_ENABLE([L2Support],
#[  --enable-L2Support    Turn on L2 support],
#[case "${enableval}" in
#  yes) L2Support=true ;;
#  no)  L2Support=false ;;
#  *) AC_MSG_ERROR([bad value ${enableval} for --enable-L2Support]) ;;
#esac],[L2Support=false])
#AM_CONDITIONAL([L2_ENABLE], [test x$L2Support = xtrue])
AC_SUBST(L2_SUPPORT_ENABLED)
AC_SUBST(GTEST_ENABLE_FLAG)
AC_OUTPUT
