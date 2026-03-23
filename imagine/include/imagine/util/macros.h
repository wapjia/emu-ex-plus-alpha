#pragma once

/*  This file is part of Imagine.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Imagine.  If not, see <http://www.gnu.org/licenses/> */

#ifdef __cplusplus
	#define CLINK extern "C"
	#define BEGIN_C_DECLS extern "C" {
	#define END_C_DECLS }
	#define IG_forward(var) std::forward<decltype(var)>(var)
	#define ConditionalProperty [[no_unique_address]] ConditionalPropertyImpl
	#define ConditionalMemberOr [[no_unique_address]] IG::UseIfOrConstantTagInjector<__LINE__>::Type
	#define ConditionalMember [[no_unique_address]] IG::UseIfTagInjector<__LINE__>::Type

	#define IG_MemberTypeReflection(member) \
	template <class T, class Fallback, class U = void> struct member##Reflection \
	{ using Type = Fallback; }; \
	\
	template <class T, class Fallback> \
	struct member##Reflection<T, Fallback, std::void_t<decltype(std::declval<T>().member)>> \
	{ using Type = decltype(std::declval<T>().member); }; \
	\
	template <class T, class Fallback> \
	using member##ReflectionType = typename member##Reflection<T, Fallback>::Type
#else
	#define CLINK
	#define BEGIN_C_DECLS
	#define END_C_DECLS
#endif

// Make symbol remain visible after linking
#define LVISIBLE __attribute__((visibility("default")))

#define PP_STRINGIFY(A) #A
#define PP_STRINGIFY_EXP(A) PP_STRINGIFY(A)
