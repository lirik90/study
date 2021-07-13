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

	bool parse(int argc, char* argv[]);
	bool parse(const std::vector<std::string>& args);

	void add(const Cli_Key_Id& id, const std::string& description = {}, std::function<void()> callback = nullptr);
	void add(const Cli_Key_Id& id, const std::string& description, const std::vector<Cli_Key_Variant>& key_variants, std::function<void(int)> callback);

	bool has(char id) const;

	Cli_Key* find_key(const std::string& id) const {}
	Cli_Key* find_key(char id) const {}

	void print_help() const;
	std::string get_help_text() const;

	template<typename... Args>
	void add_arg(Args&& ...args) {}

	template<typename... Args>
	void add_rule(Args&& ...args) {}

	std::string get_arg(const std::string& id) const;
private:
	void parse(const char* data, Cli_Key** key);
	void process(const char* begin, const char* end, bool is_key, Cli_Key** key);

	Cli_Key* get_next_empty_arg() const {}

	std::vector<Cli_Key> _keys;
};

} // namespace hz

#endif // HZ_CLI_H
