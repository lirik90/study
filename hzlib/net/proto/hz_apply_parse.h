#ifndef HZ_APPLY_PARSE_H
#define HZ_APPLY_PARSE_H

#include <tuple>

#include "hz_data_stream.h"
#include "hz_byte_array_device.h"

namespace hz {

struct Apply_Parse_Exception : std::runtime_error
{
	Apply_Parse_Exception(const std::string& what, const std::string& type_name, std::size_t type_size, std::size_t start_pos, std::shared_ptr<Data_Device> dev) :
		std::runtime_error{"Apply_Parse failed for type: \"" + type_name + "\"(" + std::to_string(type_size)
			+ ") pos: " + std::to_string(start_pos) + "(" + std::to_string(dev->pos()) + ") err: " + what},
		_start_pos{start_pos}, _dev{dev} {}

	std::size_t _start_pos;
	std::shared_ptr<Data_Device> _dev;
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
#ifndef DEBUG
		std::size_t start_pos = ds.pos();
#endif
		throw Apply_Parse_Exception{e.what(), typeid(T).name(), sizeof(T), start_pos, ds.device()};
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

template <typename RetType, typename _Tuple, typename _Fn, class T, std::size_t... _Idx, typename... Args>
RetType __apply_parse_impl(Data_Stream &ds, _Fn __f, T* obj, std::index_sequence<_Idx...>, Args&&... args)
{
	_Tuple tuple;
	parse<_Tuple, _Idx...>(ds, tuple);

#ifdef __cpp_lib_invoke
	return std::invoke(__f, obj, std::get<_Idx>(std::forward<_Tuple&&>(tuple))..., std::forward<Args&&>(args)...);
#else
#pragma GCC warning "Old invoke"
	auto func = std::bind(__f, obj, std::_Placeholder<_Idx + 1>{}...);
	return std::__invoke(func, std::get<_Idx>(std::forward<_Tuple>(tuple))..., args...);
#endif
}

template <typename RetType, typename _Fn, class T, typename... FArgs, typename... Args>
RetType apply_parse_impl(Data_Stream &ds, _Fn __f, T* obj, Args&&... args)
{
//	  auto tuple = std::make_tuple(typename std::decay<Args>::type()...);
//	  using Tuple = decltype(tuple);
//	  using Indices = std::make_index_sequence<std::tuple_size<Tuple>::value>;
	using Tuple = std::tuple<typename std::decay<FArgs>::type...>; // TODO: create tuple without Args, only {FArgs - Args}
	using Indices = std::make_index_sequence<sizeof...(FArgs) - sizeof...(Args)>;

	return __apply_parse_impl<RetType, Tuple>(ds, __f, obj, Indices{}, std::forward<Args&&>(args)...);
}

template<class FT, class T, typename RetType, typename... FArgs, typename... Args>
RetType apply_parse(Data_Stream &ds, RetType(FT::*__f)(FArgs...) const, T* obj, Args&&... args)
{
	return apply_parse_impl<RetType, decltype(__f), T, FArgs...>(ds, __f, obj, std::forward<Args&&>(args)...);
}

template<class FT, class T, typename RetType, typename... FArgs, typename... Args>
RetType apply_parse(Data_Stream &ds, RetType(FT::*__f)(FArgs...), T* obj, Args&&... args)
{
	return apply_parse_impl<RetType, decltype(__f), T, FArgs...>(ds, __f, obj, std::forward<Args&&>(args)...);
}

template<class FT, class T, typename RetType, typename... FArgs, typename... Args>
RetType apply_parse(std::shared_ptr<Data_Device> data_dev, RetType(FT::*__f)(FArgs...) const, T* obj, Args&&... args)
{
	Data_Stream ds{std::move(data_dev)};
	return apply_parse_impl<RetType, decltype(__f), T, FArgs...>(ds, __f, obj, std::forward<Args&&>(args)...);
}

template<class FT, class T, typename RetType, typename... FArgs, typename... Args>
RetType apply_parse(std::shared_ptr<Data_Device> data_dev, RetType(FT::*__f)(FArgs...), T* obj, Args&&... args)
{
	Data_Stream ds{std::move(data_dev)};
	return apply_parse_impl<RetType, decltype(__f), T, FArgs...>(ds, __f, obj, std::forward<Args&&>(args)...);
}

template<class FT, class T, typename RetType, typename... FArgs, typename... Args>
RetType apply_parse(const std::vector<uint8_t> &data, RetType(FT::*__f)(FArgs...) const, T* obj, Args&&... args)
{
	Data_Stream ds(std::make_shared<Byte_Array_Device>(data));
	return apply_parse_impl<RetType, decltype(__f), T, FArgs...>(ds, __f, obj, std::forward<Args&&>(args)...);
}

template<class FT, class T, typename RetType, typename... FArgs, typename... Args>
RetType apply_parse(const std::vector<uint8_t> &data, RetType(FT::*__f)(FArgs...), T* obj, Args&&... args)
{
	Data_Stream ds(std::make_shared<Byte_Array_Device>(data));
	return apply_parse_impl<RetType, decltype(__f), T, FArgs...>(ds, __f, obj, std::forward<Args&&>(args)...);
}

} // namespace hz

#endif // HZ_APPLY_PARSE_H
