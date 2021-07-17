#ifndef HZ_CLI_H
#define HZ_CLI_H

#include "hz_cli_key.h"

namespace hz {

class Cli
{
public:
	enum { PAIR, POSSIBLE };
	enum Arg_Type : char {
		NO_SPECIFIC	= 0, // - cmd
		OPTIONAL	= 1, // - [cmd]
		MULTIPLE	= 2, // - cmd cmd cmd ...
		// OPTIONAL | MULTIPLE - [cmd [cmd [cmd ...]]]
		// specific args can be only in tail
	};

	Cli() = default;

	bool parse(int argc, char* argv[]);
	bool parse(const std::vector<std::string>& args);

	Cli_Key* add(const Cli_Key_Id& id, const std::string& description = {}, std::function<void()> callback = nullptr);
	Cli_Key* add(const Cli_Key_Id& id, const std::string& description, const std::vector<Cli_Key_Variant>& key_variants, std::function<void(int)> callback);

	bool has(const std::string& id) const;
	bool has(char id) const;

	const Cli_Key* find_key(const std::string& id) const;
	const Cli_Key* find_key(char id) const;
	Cli_Key* find_key(const std::string& id);
	Cli_Key* find_key(char id);

	void print_help() const;
	std::string get_help_text() const;

	template<typename... Args>
	void add_rule(Args&& ...args) {}

	void add_arg(const std::string& id, const std::string& description = {}, char type = NO_SPECIFIC);
	std::string add_arg_or_get_error(const std::string& id, const std::string& description = {}, char type = NO_SPECIFIC);

	std::string get_arg(const std::string& id) const;
	std::vector<std::string> get_args(const std::string& id) const; // for MULTIPLE
private:
	void parse(const char* data, Cli_Key** key);
	void process(const char* begin, const char* end, bool is_key, Cli_Key** key);

	Cli_Key* get_next_empty_arg();

	std::vector<Cli_Key> _keys, _args;
};

} // namespace hz

#endif // HZ_CLI_H
