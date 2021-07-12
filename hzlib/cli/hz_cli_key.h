#ifndef HZ_CLI_KEY_H
#define HZ_CLI_KEY_H

#include <variant>
#include <functional>

namespace hz {

struct Cli_Key_Id
{
	Cli_Key_Id(char small, const std::string& full = {}) :
		_small{small}, _full{full} {}
	Cli_Key_Id(const std::string& full) :
		_small{0}, _full{full} {}

	char _small;
	std::string _full;
};

struct Cli_Key_Variant
{
	Cli_Key_Variant(int value, const std::string& text, const std::string& description = {}) :
		_value{value}, _text{text}, _description{description} {}

	int _value;
	std::string _text, _description;
};

class Cli_Key
{
public:
	Cli_Key(const Cli_Key_Id& id, const std::string& description = {}, std::function<void()> callback = nullptr) :
		_id{id}, _description{description}, _callback{std::move(callback)}
	{}

	Cli_Key(const Cli_Key_Id& id, const std::string& description, const std::vector<Cli_Key_Variant>& key_variants, std::function<void(int)> callback) :
		_id{id}, _description{description}, _key_variants{key_variants}, _callback{std::move(callback)}
	{}

private:
	Cli_Key_Id _id;
	std::string _description;
	std::vector<Cli_Key_Variant> _key_variants;
	std::variant<
		std::function<void()>,
		std::function<void(int)>
	> _callback;
};

} // namespace

#endif // HZ_CLI_KEY_H
