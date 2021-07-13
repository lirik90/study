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
		if (*key && (*key)->is_flag())
		{
			(*key)->set_flag();
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

void Cli::add(const Cli_Key_Id& id, const std::string& description, std::function<void()> callback)
{
	_keys.emplace_back(id, description, std::move(callback));
}

void Cli::add(const Cli_Key_Id& id, const std::string& description, const std::vector<Cli_Key_Variant>& key_variants, std::function<void(int)> callback)
{
	_keys.emplace_back(id, description, key_variants, std::move(callback));
}

bool Cli::has(char id) const
{
	return false;
}

void Cli::print_help() const
{
}

std::string Cli::get_help_text() const
{
	return {};
}

std::string Cli::get_arg(const std::string& id) const
{
	return {};
}

} // namespace hz

