#ifndef HZ_APPLY_PARSE_H
#define HZ_APPLY_PARSE_H

#include <tuple>
#include <type_traits>

#include "hz_data_stream.h"
#include "hz_byte_array_device.h"

namespace hz {

template<typename Ret, typename Obj, typename... Args>
struct __fn_traits
{
	using args = std::tuple<typename std::decay<Args>::type...>;
	using ret = Ret;
	using obj = Obj;
};

template<typename F> struct fn_traits;
template<typename R, typename... Args> struct fn_traits<R (*)(Args...)> : __fn_traits<R, void, Args...> {};
template<typename R, typename O, typename... Args> struct fn_traits<R (O::*)(Args...)> : __fn_traits<R, O, Args...> {};
template<typename R, typename O, typename... Args> struct fn_traits<R (O::*)(Args...) const> : __fn_traits<R, O, Args...> {};

// ----

struct Apply_Parse_Exception : std::runtime_error
{
	Apply_Parse_Exception(const std::string& what, const std::string& type_name, std::size_t type_size, std::size_t start_pos, std::size_t pos) :
		std::runtime_error{"Apply_Parse failed for type: \"" + type_name + "\"(" + std::to_string(type_size)
			+ ") pos: " + std::to_string(start_pos) + "(" + std::to_string(pos) + ") err: " + what}{}
};

template<typename T = void>
void parse_out(Data_Stream &) {}

template<typename T, typename... Args>
void parse_out(Data_Stream &ds, T& out, Args&... args)
{
#ifdef DEBUG
	std::size_t start_pos = ds.pos();
#endif
	try {
		ds >> out;
	}
	catch (const std::exception& e) {
		std::string text = e.what();
#ifdef DEBUG
		// TODO: add hex of data to text
#else
		std::size_t start_pos = ds.pos();
#endif
		throw Apply_Parse_Exception{text, typeid(T).name(), sizeof(T), start_pos, ds.pos()};
	}
	parse_out(ds, args...);
}

template<typename... Args>
void parse_out(const std::vector<uint8_t> &data, Args&... args)
{
	Data_Stream ds(std::make_shared<Byte_Array_Device>(data));
	parse_out(ds, args...);
}

template<typename... Args>
void parse_out(std::shared_ptr<Data_Device> data_dev, Args&... args)
{
	Data_Stream ds{std::move(data_dev)};
	parse_out(ds, args...);
}

template<typename T>
T parse(std::shared_ptr<Data_Device> data_dev)
{
	T obj; parse_out(std::move(data_dev), obj); return obj;
}

template<typename T>
T parse(Data_Stream &ds)
{
	T obj; parse_out(ds, obj); return obj;
}

template<typename _Tuple>
void parse(Data_Stream &, _Tuple&) {}

template<typename _Tuple, std::size_t x, std::size_t... _Idx>
void parse(Data_Stream &ds, _Tuple& __t)
{
	parse_out(ds, std::get<x>(__t)); // typename std::tuple_element<x, _Tuple>::type
	parse<_Tuple, _Idx...>(ds, __t);
}

template <typename RetType, typename Tuple, typename Fn, class T, std::size_t... Idx, typename... Args>
RetType __apply_parse(Data_Stream &ds, Fn f, T* obj, std::index_sequence<Idx...>, Args&&... args)
{
	Tuple tuple;
	parse<Tuple, Idx...>(ds, tuple);

#ifdef __cpp_lib_invoke
	return std::invoke(f, obj, std::get<Idx>(std::forward<Tuple&&>(tuple))..., std::forward<Args&&>(args)...);
#else
#pragma GCC warning "Old invoke"
	auto func = std::bind(f, obj, std::_Placeholder<Idx + 1>{}...);
	return std::__invoke(func, std::get<Idx>(std::forward<Tuple&&>(tuple))..., std::forward<Args&&>(args)...);
#endif
}

template <typename Fn, class T, typename... Args>
typename fn_traits<Fn>::ret apply_parse(Data_Stream &ds, Fn f, T* obj, Args&&... args)
{
//	  auto tuple = std::make_tuple(typename std::decay<Args>::type()...);
//	  using Tuple = decltype(tuple);
//	  using Indices = std::make_index_sequence<std::tuple_size<Tuple>::value>;
	// using Tuple = std::tuple<typename std::decay<FArgs>::type...>; // TODO: create tuple without Args, only {FArgs - Args}
	// using Indices = std::make_index_sequence<sizeof...(FArgs) - sizeof...(Args)>;
	using Tuple = typename fn_traits<Fn>::args; // TODO: create tuple without Args, only {FArgs - Args}
	using Indices = std::make_index_sequence<std::tuple_size<Tuple>::value - sizeof...(Args)>;

	return __apply_parse<typename fn_traits<Fn>::ret, Tuple>(ds, f, obj, Indices{}, std::forward<Args&&>(args)...);
}

template <typename Data_Device_T, typename Fn, class T, typename... Args>
typename fn_traits<Fn>::ret apply_parse(Data_Device_T&& dev, Fn f, T* obj, Args&&... args)
{
	Data_Stream ds{std::forward<Data_Device_T&&>(dev)};
	return apply_parse(ds, f, obj, std::forward<Args&&>(args)...);
}

template <typename Fn, class T, typename... Args>
typename fn_traits<Fn>::ret apply_parse(const std::vector<uint8_t> &data, Fn f, T* obj, Args&&... args)
{
	Byte_Array_Device dev{data};
	return apply_parse(dev, f, obj, std::forward<Args&&>(args)...);
}

template <typename Fn, class T, typename... Args>
typename fn_traits<Fn>::ret apply_parse(const uint8_t* data, std::size_t size, Fn f, T* obj, Args&&... args)
{
	Byte_Array_Device dev{data, size};
	return apply_parse(dev, f, obj, std::forward<Args&&>(args)...);
}

} // namespace hz

#endif // HZ_APPLY_PARSE_H
