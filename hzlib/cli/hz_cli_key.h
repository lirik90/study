#ifndef HZ_CLI_KEY_H
#define HZ_CLI_KEY_H

#include <vector>
#include <string>
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
	struct NO_INITIALIZED {};

	using Value_Type = std::variant<
		NO_INITIALIZED,
		bool, // flag
		std::string, // text
		int // key_variants
	>;

	var.index() == std::variant_npos ? // ; How about is'n flag
	Cli_Key(const Cli_Key_Id& id, const std::string& description = {}, std::function<void()> callback = nullptr, Value_Type default_value = NO_INITIALIZED{}) :
		_id{id}, _description{description}, _callback{std::move(callback)}, _value{std::move(default_value)}
	{}

	Cli_Key(const Cli_Key_Id& id, const std::string& description, const std::vector<Cli_Key_Variant>& key_variants, std::function<void(int)> callback = nullptr, Value_Type default_value = NO_INITIALIZED{}) :
		_id{id}, _description{description}, _key_variants{key_variants}, _callback{std::move(callback)}, _value{std::move(default_value)}
	{}

	bool is_flag() const {}
	void set_flag() {}
	void set_value(const std::string& value) {}
	void set_value(std::string&& value) {}
private:
	Cli_Key_Id _id;
	std::string _description;
	std::vector<Cli_Key_Variant> _key_variants;
	std::variant<
		std::function<void()>,
		std::function<void(int)>
	> _callback;

	Value_Type _value;
};

} // namespace

#endif // HZ_CLI_KEY_H
