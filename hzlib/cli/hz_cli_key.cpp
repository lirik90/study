
#include "hz_cli_key.h"

namespace hz {
Cli_Key::Cli_Key(const Cli_Key_Id& id, const std::string& description, std::function<void()> callback, Value_Type default_value, Type type) :
	_id{id}, _type{type}, _description{description}, _callback{std::move(callback)}, _value{std::move(default_value)} {}

Cli_Key::Cli_Key(const Cli_Key_Id& id, const std::string& description, const std::vector<Cli_Key_Variant>& key_variants, std::function<void(int)> callback, const Value_Type& default_value) :
	_id{id}, _type{Type::KEY_VARIANT}, _description{description}, _key_variants{key_variants}, _callback{std::move(callback)}
{
	std::visit([this](auto&& arg)
	{
		using T = decltype(arg);
		if constexpr (std::is_same_v<T, int> || std::is_same_v<T, std::string>)
			set_value(arg);
	}, default_value);
}

bool Cli_Key::operator==(const std::string& id) const { return _id == id; }
bool Cli_Key::operator==(char id) const { return _id == id; }

const Cli_Key_Id& Cli_Key::id() const { return _id; }
Cli_Key::Type Cli_Key::type() const { return _type; }
bool Cli_Key::has_value() const { return _value.index(); }

void Cli_Key::clear()
{
	_value = NO_INITIALIZED{};
}

bool Cli_Key::set_value(bool flag)
{
	if (_type == Type::FLAG || _type == Type::AUTO)
	{
		_value = flag;
		_type = Type::FLAG;
		return true;
	}
	return false;
}

bool Cli_Key::set_value(const std::string& text_or_key_variant)
{
	if (_type == Type::TEXT || _type == Type::AUTO)
	{
		_value = text_or_key_variant;
		_type = Type::TEXT;
		return true;
	}
	return set_key_variant(text_or_key_variant);
}

bool Cli_Key::set_value(std::string&& text_or_key_variant)
{
	if (_type == Type::TEXT || _type == Type::AUTO)
	{
		_value = std::move(text_or_key_variant);
		_type = Type::TEXT;
		return true;
	}
	return set_key_variant(text_or_key_variant);
}

bool Cli_Key::set_value(int key_variant)
{
	if (_type == Type::KEY_VARIANT)
	{
		for (const Cli_Key_Variant& key: _key_variants)
		{
			if (key._value == key_variant)
			{
				_value = key_variant;
				return true;
			}
		}
	}
	return false;
}

bool Cli_Key::set_key_variant(const std::string& text)
{
	if (_type == Type::KEY_VARIANT || _type == Type::AUTO)
	{
		for (const Cli_Key_Variant& key: _key_variants)
		{
			if (key._text == text)
			{
				_value = key._value;
				_type = Type::KEY_VARIANT;
				return true;
			}
		}
	}
	return false;
}

bool Cli_Key::get_flag() const { return value<bool>(); }
std::string Cli_Key::get_text() const { return value<std::string>(); }
int Cli_Key::get_key_variant() const { return value<int>(); }

} // namespace hz

