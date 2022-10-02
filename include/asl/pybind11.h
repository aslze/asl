/*
Copyright (c) 1999-2022 aslze
Transparent conversion for some ASL types to/from Python with pybind11
Based on code from pybind11:
Copyright (c) 2016 Wenzel Jakob <wenzel.jakob@epfl.ch>
All rights reserved. Use of this source code is governed by a BSD-style license,
found in the LICENSE file of pybind11
*/


// Currently supported classes:
// String, Array<>, Map<>, Dic<>, Array_<>, Vec2, Vec3, Vec4
// NOTE: String and Array are always supported. Bindings for the other types only added if they are included before this header!
// Include <pyind11/pybind11.h> or <pyind11/embed.h> before including this file

#pragma once

#include <asl/String.h>

#ifdef PYBIND11_NAMESPACE_BEGIN
#define PY11_NAMESPACE_BEGIN PYBIND11_NAMESPACE_BEGIN
#define PY11_NAMESPACE_END PYBIND11_NAMESPACE_END
#else
#define PY11_NAMESPACE_BEGIN NAMESPACE_BEGIN
#define PY11_NAMESPACE_END NAMESPACE_END
#endif

PY11_NAMESPACE_BEGIN(PYBIND11_NAMESPACE)
PY11_NAMESPACE_BEGIN(detail)

template <typename T, typename U>
using aforwarded_type = conditional_t<std::is_lvalue_reference<T>::value, remove_reference_t<U>&, remove_reference_t<U>&&>;

template <typename T, typename U>
aforwarded_type<T, U> aforward_like(U&& u) {
	return std::forward<detail::aforwarded_type<T, U>>(std::forward<U>(u));
}

template <typename Type, typename Key, typename Value>
struct asl_map_caster
{
	using key_conv   = make_caster<Key>;
	using value_conv = make_caster<Value>;

	bool load(handle src, bool convert) {
		if (!isinstance<dict>(src))
			return false;
		auto d = reinterpret_borrow<dict>(src);
		value.clear();
		for (auto it : d) {
			key_conv kconv;
			value_conv vconv;
			if (!kconv.load(it.first.ptr(), convert) || !vconv.load(it.second.ptr(), convert))
				return false;
			value[cast_op<Key&&>(std::move(kconv))] = cast_op<Value&&>(std::move(vconv));
		}
		return true;
	}

	template <typename T>
	static handle cast(T &&src, return_value_policy policy, handle parent) {
		dict d;
		return_value_policy policy_key = policy;
		return_value_policy policy_value = policy;
		if (!std::is_lvalue_reference<T>::value) {
			policy_key = return_value_policy_override<Key>::policy(policy_key);
			policy_value = return_value_policy_override<Value>::policy(policy_value);
		}
		for (auto && kv : src) {
			auto key = reinterpret_steal<object>(key_conv::cast(aforward_like<T>(kv.key), policy_key, parent));
			auto value = reinterpret_steal<object>(value_conv::cast(aforward_like<T>(kv.value), policy_value, parent));
			if (!key || !value)
				return handle();
			d[key] = value;
		}
		return d.release();
	}

	PYBIND11_TYPE_CASTER(Type, _("Dict[") + key_conv::name + _(", ") + value_conv::name + _("]"));
};

#ifdef ASL_MAP_H
template <typename Key, typename Value>
struct type_caster<asl::Map<Key, Value>> : asl_map_caster<asl::Map<Key, Value>, Key, Value> { };

template <typename Value>
struct type_caster<asl::Dic<Value>> : asl_map_caster<asl::Dic<Value>, asl::String, Value> { };
#endif


template <typename Type, typename Value> struct asl_list_caster
{
	using value_conv = make_caster<Value>;

	bool load(handle src, bool convert) {
		if (!isinstance<sequence>(src) || isinstance<str>(src))
			return false;
		auto s = reinterpret_borrow<sequence>(src);
		value.resize((int)s.size());
		int i = 0;
		for (auto it : s) {
			value_conv conv;
			if (!conv.load(it, convert))
				return false;
			value[i++] = cast_op<Value&&>(std::move(conv));
		}
		return true;
	}

	template <typename T>
	static handle cast(T &&src, return_value_policy policy, handle parent) {
		if (!std::is_lvalue_reference<T>::value)
			policy = return_value_policy_override<Value>::policy(policy);
		list l(src.length());
		size_t i = 0;
		for(auto && value : src) {
			auto value_ = reinterpret_steal<object>(value_conv::cast(aforward_like<T>(value), policy, parent));
			if (!value_)
				return handle();
			PyList_SET_ITEM(l.ptr(), (ssize_t) i++, value_.release().ptr());
		}
		return l.release();
	}

	PYBIND11_TYPE_CASTER(Type, _("List[") + value_conv::name + _("]"));
};

template <typename Type>
struct type_caster<asl::Array<Type>> : asl_list_caster<asl::Array<Type>, Type> { };

template <typename ArrayType, typename Value, size_t Size>
struct asl_array_caster
{
	using value_conv = make_caster<Value>;
	bool load(handle src, bool convert) {
		if (!isinstance<sequence>(src))
			return false;
		auto l = reinterpret_borrow<sequence>(src);
		if (l.size() != Size)
			return false;
		Value* ptr = (Value*)(const Value*)value;
		for (int i = 0; i < Size; i++) {
			value_conv conv;
			if (!conv.load(l[i], convert))
				return false;
			ptr[i] = cast_op<Value&&>(std::move(conv));
		}
		return true;
	}

	template <typename T>
	static handle cast(T &&src, return_value_policy policy, handle parent) {
		list l(Size);
		const Value* ptr = (const Value*)src;
		for(int i = 0; i < Size; i++) {
			auto value_ = reinterpret_steal<object>(value_conv::cast(aforward_like<T>(ptr[i]), policy, parent));
			if (!value_)
				return handle();
			PyList_SET_ITEM(l.ptr(), (ssize_t) i, value_.release().ptr());
		}
		return l.release();
	}

	PYBIND11_TYPE_CASTER(ArrayType, _("List[") + value_conv::name + _("[") + _<Size>() + _("]"));
};

#ifdef ASL_ARRAYX_H
template <typename Type, size_t Size>
struct type_caster<asl::Array_<Type, Size>> : asl_array_caster<asl::Array_<Type, Size>, Type, Size> { };
#endif

#ifdef ASL_VECTOR4_H
template <typename Type>
struct type_caster<asl::Vec4_<Type>> : asl_array_caster<asl::Vec2_<Type>, Type, 4> { };
#endif

#ifdef ASL_VECTOR3_H
template <typename Type>
struct type_caster<asl::Vec3_<Type>> : asl_array_caster<asl::Vec3_<Type>, Type, 3> { };
#endif

#ifdef ASL_VECTOR2_H
template <typename Type>
struct type_caster<asl::Vec2_<Type>> : asl_array_caster<asl::Vec2_<Type>, Type, 2> { };
#endif

struct asl_string_caster
{
	bool load(handle src, bool) {
		handle load_src = src;
		if (!src) {
			return false;
		} else if (!PyUnicode_Check(load_src.ptr())) {
			return load_bytes(load_src);
		}

		object utfNbytes = reinterpret_steal<object>(PyUnicode_AsEncodedString(load_src.ptr(), "utf-8", nullptr));
		if (!utfNbytes) {
			PyErr_Clear();
			return false;
		}

		const char* buffer = reinterpret_cast<const char*>(PYBIND11_BYTES_AS_STRING(utfNbytes.ptr()));
		size_t length = (size_t) PYBIND11_BYTES_SIZE(utfNbytes.ptr()) / sizeof(char);
		value.assign(buffer, (int)length);

		return true;
	}

	static handle cast(const asl::String&src, return_value_policy /* policy */, handle /* parent */) {
		const char *buffer = reinterpret_cast<const char *>(*src);
		ssize_t nbytes = ssize_t(src.length() * sizeof(char));
		handle s =
#ifndef PYPY_VERSION
		PyUnicode_DecodeUTF8(buffer, nbytes, nullptr);
#else
		PyUnicode_Decode(buffer, nbytes, "utf-8", nullptr);
#endif
		if (!s) throw error_already_set();
		return s;
	}

	PYBIND11_TYPE_CASTER(asl::String, _(PYBIND11_STRING_NAME));

private:

	bool load_bytes(handle src) {
		if (PYBIND11_BYTES_CHECK(src.ptr())) {
			const char *bytes = PYBIND11_BYTES_AS_STRING(src.ptr());
			if (bytes) {
				value.assign(bytes, (int) PYBIND11_BYTES_SIZE(src.ptr()));
				return true;
			}
		}

		return false;
	}
};

template<>
struct type_caster<asl::String> : asl_string_caster {};

PY11_NAMESPACE_END(detail)
PY11_NAMESPACE_END(PYBIND11_NAMESPACE)
