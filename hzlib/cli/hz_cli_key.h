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

	bool operator==(const std::string& id) const { return id == _full; }
	bool operator==(char id) const { return id == _small; }

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
	enum class Type : uint8_t { AUTO, FLAG, TEXT, KEY_VARIANT };
	struct NO_INITIALIZED {};

	using Value_Type = std::variant<
		NO_INITIALIZED,
		bool, // flag
		std::string, // text
		int // key_variant
	>;

	Cli_Key(const Cli_Key_Id& id, const std::string& description = {}, std::function<void()> callback = nullptr, Value_Type default_value = NO_INITIALIZED{}, Type type = Type::AUTO);

	Cli_Key(const Cli_Key_Id& id, const std::string& description, const std::vector<Cli_Key_Variant>& key_variants, std::function<void(int)> callback = nullptr, const Value_Type& default_value = NO_INITIALIZED{});

	bool operator==(const std::string& id) const;
	bool operator==(char id) const;

	const Cli_Key_Id& id() const;

	Type type() const;
	bool has_value() const;

	void clear();
	bool set_value(bool flag);
	bool set_value(const std::string& text_or_key_variant);
	bool set_value(std::string&& text_or_key_variant);
	bool set_value(int key_variant);
	bool set_key_variant(const std::string& text);

	bool get_flag() const;
	std::string get_text() const;
	int get_key_variant() const;

	template<typename T,
		std::enable_if_t<
			std::is_same_v<T, bool> || std::is_same_v<T, std::string> || std::is_same_v<T, int>,
		bool> = true
	>
	T value() const
	{
		const T* v = std::get_if<T>(&_value);
		return v ? *v : T{};
	}
private:
	Cli_Key_Id _id;
	Type _type;
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
