// Example Program.cpp : 
//

#include <array>
#include <iostream>

#include "vansave.hpp"

#pragma region EXAMPLE OF ADDING DATATYPE COMPATIBILITY

struct rgba_colour
{
	uint8_t r, g, b;

	std::string get_hex() const
	{
		char hexcol[8];
		snprintf(hexcol, sizeof hexcol, "%02x%02x%02x", r, g, b);

		return "#" + std::string(hexcol);
	}

	// For saving
	friend std::ostream& operator<<(std::ostream& o, const rgba_colour& j)
	{
		return o << j.get_hex();
	}

	void load_hex(const std::string& hex)
	{

	}

	static rgba_colour cyan()
	{
		return rgba_colour{ 0,255,255 };
	}
};

namespace vansave
{
	void element_value<std::string>::load(const std::string& value)
	{
		m_value = value;
	}
	void element_value<rgba_colour>::load(const std::string& value)
	{
		m_value.load_hex(value);
	}
}

#pragma endregion

#pragma region EXAMPLE OF BUILT UP LIST

M_MASTER_LIST(main_list_t)
{
	M_NEW_LIST(optimizations_list)
	{
		M_DECLARE_VAR(bool, cap_fps, false);
		M_DECLARE_VAR(int, fps_cap, 60);
	};
	M_ADD_SUBLIST(optimizations_list, n_optimization);
	M_ADD_SUBLIST(optimizations_list, m_optimization);

	M_NEW_LIST(game_settings_list)
	{
		M_DECLARE_VAR(rgba_colour, crosshair_colour, rgba_colour::cyan());
		M_DECLARE_VAR(int, max_ping, 100);
		M_DECLARE_VAR(float, crosshair_size, 100);
		M_DECLARE_VAR(float, field_of_view, 75.f);
	};
	M_ADD_SUBLIST(game_settings_list, game_settings);

	M_NEW_LIST(account_list)
	{
		M_DECLARE_VAR(std::string, username, "FooBarDestroyer");
		M_DECLARE_VAR(std::string, profile_picture, "https://i.imgur.com/GjlI2N4.jpeg");
		M_DECLARE_VAR(std::string, profile_art, "https://www.youtube.com/watch?v=dQw4w9WgXcQ");
	};
	// 
	M_ADD_SUBLIST(account_list, account_settings);
	M_ADD_SUBLIST_ARR(account_list, accounts, 4);

	M_DECLARE_VAR(int, game_version, 60.f);
	M_DECLARE_VAR(float, field_of_view, 10.f);
};

main_list_t* main_list = new main_list_t();

#pragma endregion TODO: Add array saving and input them here (multiple account_settings)

int main()
{
	main_list->save_to_file("Example Save.vsave");
	//main_list->load_from_file("Example Save.vsave");
	for (int i = 0; i < main_list->accounts.size(); i++)
	std::cout << main_list->accounts.at(i)->username;

	return 1;
}