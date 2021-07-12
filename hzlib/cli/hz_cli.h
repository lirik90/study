#ifndef HZ_CLI_H
#define HZ_CLI_H

#include "hz_cli_key.h"

namespace hz {

class Cli
{
public:
	enum { PAIR, POSSIBLE };
	enum { OPTIONAL };

	Cli() = default;

	bool process(int argc, char* argv[]) const
	{
		return false;
	}

	bool process(const std::vector<std::string>& args) const
	{
		return false;
	}

	void add(const Cli_Key_Id& id, const std::string& description = {}, std::function<void()> callback = nullptr)
	{
		_keys.emplace_back(id, description, std::move(callback));
	}

	void add(const Cli_Key_Id& id, const std::string& description, const std::vector<Cli_Key_Variant>& key_variants, std::function<void(int)> callback)
	{
		_keys.emplace_back(id, description, key_variants, std::move(callback));
	}

	bool has(char id)
	{
		return false;
	}

	void print_help() const
	{
	}

	std::string get_help_text() const
	{
		return {};
	}

	template<typename... Args>
	void add_arg(Args&& ...args) {}

	template<typename... Args>
	void add_rule(Args&& ...args) {}

	std::string get_arg(const std::string& id) const
	{
		return {};
	}
private:
	std::vector<Cli_Key> _keys;
};

} // namespace hz

#endif // HZ_CLI_H
