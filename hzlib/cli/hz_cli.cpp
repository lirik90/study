#include <iostream>

#include "hz_cli.h"

namespace hz {

bool Cli::parse(int argc, char* argv[])
{
	std::cout << "Argc: " << argc << std::endl;

	Cli_Key* key = nullptr;
	for (int i = 1; i < argc; ++i)
	{
		parse(argv[i], &key);
		std::cout << "I " << i << " KEY " << argv[i] << std::endl;
	}
	return false;
}

bool Cli::parse(const std::vector<std::string>& args)
{
	Cli_Key* key = nullptr;
	for (const std::string& arg: args)
		parse(arg.data(), &key);
	return false;
}

void Cli::parse(const char* data, Cli_Key** key)
{
	bool is_key = *data == '-';
	const char* begin = nullptr;
	while (true)
	{
		// Если конец строки
		if (!*data)
		{
			// Если начало текста было найдено
			if (begin)
				process(begin, data, is_key, key);
			break;
		}

		if (!begin)
		{
			if (*data != '-')
				begin = data;
		}
		else if (is_key && *data == '=')
		{
			process(begin, data, is_key, key);
			// Если не наёден ключ, то и значение после = не обрабатываем
			if (!*key)
				break;

			begin = data + 1;
			is_key = false;
		}

		++data;
	}
}

void Cli::process(const char* begin, const char* end, bool is_key, Cli_Key** key)
{
	std::string text{begin, end};

	// Если текст это ключ
	if (is_key)
	{
		*key = find_key(text);
		if (*key)
		{
			bool is_flag = (*key)->type() == Cli_Key::Type::FLAG;
			(*key)->set_value(true);
			if (is_flag)
				*key = nullptr;
		}
	}
	else
	{
		if (!*key)
			*key = get_next_empty_arg();

		if (*key)
		{
			// Устанавливаем значение
			(*key)->set_value(text);
			*key = nullptr;
		}
	}
}

Cli_Key* Cli::add(const Cli_Key_Id& id, const std::string& description, std::function<void()> callback)
{
	return &_keys.emplace_back(id, description, std::move(callback));
}

Cli_Key* Cli::add(const Cli_Key_Id& id, const std::string& description, const std::vector<Cli_Key_Variant>& key_variants, std::function<void(int)> callback)
{
	return &_keys.emplace_back(id, description, key_variants, std::move(callback));
}

bool Cli::has(const std::string& id) const
{
	const Cli_Key* key = find_key(id);
	return key ? key->has_value() : false;
}

bool Cli::has(char id) const
{
	const Cli_Key* key = find_key(id);
	return key ? key->has_value() : false;
}

const Cli_Key* Cli::find_key(const std::string& id) const
{
	auto it = std::find(_keys.cbegin(), _keys.cend(), id);
	return it == _keys.cend() ? nullptr : &*it;
}

const Cli_Key* Cli::find_key(char id) const
{
	auto it = std::find(_keys.cbegin(), _keys.cend(), id);
	return it == _keys.cend() ? nullptr : &*it;
}

Cli_Key* Cli::find_key(const std::string& id)
{
	auto it = std::find(_keys.begin(), _keys.end(), id);
	return it == _keys.end() ? nullptr : &*it;
}

Cli_Key* Cli::find_key(char id)
{
	auto it = std::find(_keys.begin(), _keys.end(), id);
	return it == _keys.end() ? nullptr : &*it;
}

void Cli::print_help() const
{
	std::cout << get_help_text() << std::endl;
}

std::string Cli::get_help_text() const
{
	return {};
}

void Cli::add_arg(const std::string& id, const std::string& description, char type)
{
	std::string err = add_arg_or_get_error(id, description, type);
	if (!err.empty())
		throw std::runtime_error("Cli: " + err);
}

std::string Cli::add_arg_or_get_error(const std::string& id, const std::string& description, char type)
{
	if (id.empty())
		return "Id can't be empty";

	if (!_args.empty())
	{
		// throw if last is multiple or if last is optional but current is not
		Cli_Key& last = _args.back();
		if (last.id()._small & Arg_Type::MULTIPLE)
			return "Attempt to add argument after multiple";
		if ((last.id()._small & Arg_Type::OPTIONAL) != 0 && (type & Arg_Type::OPTIONAL) == 0)
			return "Only optional argument can be after last optional";
	}

	_args.emplace_back(Cli_Key_Id{type, id}, description, nullptr, Cli_Key::NO_INITIALIZED{}, Cli_Key::Type::TEXT);
	return {};
}

std::string Cli::get_arg(const std::string& id) const
{
	return {};
}

std::vector<std::string> Cli::get_args(const std::string& id) const
{
	return {};
}

Cli_Key* Cli::get_next_empty_arg()
{
	for (Cli_Key& key: _args)
	{
		if (!key.has_value())
		{
			if (key.id()._small & Arg_Type::MULTIPLE)
				_args.push_back(key);
			return &key;
		}
	}
	return nullptr;
}

} // namespace hz

