# Copyright 2023 Comcast Cable Communications Management, LLC
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
#
# SPDX-License-Identifier: Apache-2.0
#

# Initialize Autoconf
AC_INIT([remotedebugger_gtest],[1.0])
# Initialize Automake
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
# Check for necessary headers
AC_CHECK_HEADERS([jsoncpp/json/json.h gtest/gtest.h gmock/gmock.h])
# Check for necessary libraries
AC_CHECK_LIB([jsoncpp], [main])
AC_CHECK_LIB([gtest], [main])
AC_CHECK_LIB([gmock], [main])
# Checks for programs.
AC_PROG_CXX
# Checks for libraries.
AC_CHECK_LIB([stdc++], [main])
# Checks for header files.
m4_warn([obsolete],
[The preprocessor macro `STDC_HEADERS' is obsolete.
  Except in unusual embedded environments, you can safely include all
  ISO C90 headers unconditionally.])dnl
# Autoupdate added the next two lines to ensure that your configure
# script's behavior did not change.  They are probably safe to remove.
AC_CHECK_INCLUDES_DEFAULT
AC_PROG_EGREP

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
# Generate the Makefile
AC_CONFIG_FILES([Makefile])
# Generate the configure script
AC_OUTPUT

