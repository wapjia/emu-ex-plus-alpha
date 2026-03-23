// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

export namespace std {
#ifndef __ANDROID__
  using std::isalnum _LIBCPP_USING_IF_EXISTS;
  using std::isalpha _LIBCPP_USING_IF_EXISTS;
  using std::isblank _LIBCPP_USING_IF_EXISTS;
  using std::iscntrl _LIBCPP_USING_IF_EXISTS;
  using std::isdigit _LIBCPP_USING_IF_EXISTS;
  using std::isgraph _LIBCPP_USING_IF_EXISTS;
  using std::islower _LIBCPP_USING_IF_EXISTS;
  using std::isprint _LIBCPP_USING_IF_EXISTS;
  using std::ispunct _LIBCPP_USING_IF_EXISTS;
  using std::isspace _LIBCPP_USING_IF_EXISTS;
  using std::isupper _LIBCPP_USING_IF_EXISTS;
  using std::isxdigit _LIBCPP_USING_IF_EXISTS;
  using std::tolower _LIBCPP_USING_IF_EXISTS;
  using std::toupper _LIBCPP_USING_IF_EXISTS;
#else
  inline int isalnum(int c) { return ::isalnum(c); }
  inline int isalpha(int c) { return ::isalpha(c); }
  inline int isblank(int c) { return ::isblank(c); }
  inline int iscntrl(int c) { return ::iscntrl(c); }
  inline int isdigit(int c) { return ::isdigit(c); }
  inline int isgraph(int c) { return ::isgraph(c); }
  inline int islower(int c) { return ::islower(c); }
  inline int isprint(int c) { return ::isprint(c); }
  inline int ispunct(int c) { return ::ispunct(c); }
  inline int isspace(int c) { return ::isspace(c); }
  inline int isupper(int c) { return ::isupper(c); }
  inline int isxdigit(int c) { return ::isxdigit(c); }
  inline int tolower(int c) { return ::tolower(c); }
  inline int toupper(int c) { return ::toupper(c); }
#endif
} // namespace std
