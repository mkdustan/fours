// Hi, Thanks for using me :)
// vansave is a super simple saving system for variables in your c++ program.
// At the Moment there is practically no support for dynamic objects. If you intend to add and remove a lot of object at runtime, I recomend you looks somewhere else for a state saving system.
// You can find a writeup about the basics of this system here:
// Including adding elements, adding support for more datatypes, and the basics of how to load and save to files using this framework

#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>

#include <sstream>
#include <fstream>

namespace vansave
{
	/* * * * * * * * * * * * * * * * * * * * * * *
	* HOW TO USE:
	* 
	* > BASIC USE:
	* 
	* > ADDING SUPPORT FOR NEW DATATYPES:
	*	Saving and loading pushes the saved variable into strinstream:
	*   you can add support to a custom struct by adding inside the struct
	*----------------------------------------------------------------------------------
	* 	friend std::ostream& operator<<(std::ostream& os, const datatype& v)
	* 	{
	* 		// Push formating into os here
	* 		return os;
	* 	}
	*----------------------------------------------------------------------------------
	* 
	* > CHANGING KEY TYPE:
	*   The key datatype is defined as using key_t = uint32_t by default.
	*   If you wish to change this to another datatype, simply go to where key_t is defined and replace uint32_t with something else.
	*	Depending on what you change it to some errors may occur. Make sure you have read through the code before you 
	*   change it to anticipate any errors that may occur.
	*
	*   NOTE: I would like to point out that, if you have use for something that uses strings as the keys there 
	*		  are many formats that may be better suited for your application. Json, YAML, XML to mention a few. 
	*
	* * * * * * * * * * * * * * * * * * * * * * *
	* DOWNFALLS & WORKAROUNDS:
	* 
	* > PROBLEM:    Value elements only have one line to store their value. 
	*   WORKAROUND: If you need more lines for a single value you can make a list and save the values under there.
	* 
	* > PROBLEM:    Config file is difficult ot read for humans
	*   WORKAROUND: You can change the key to be a string with relative few problems by changing the key_t typedef
	*	NOTE:       If you do this there is very little reason to use this system over something like a modern json for c++
	* 
	* > PROBLEM:    Key cannot be retreived from the element itself
	*   WORKAROUND: Save the parent somewhere and use it to get the key.
	*   
	* > PROBLEM: Save and load functions are heavily string relient and very under-engineered, so saving and loading larger structs may be slow
	*	TODO:    Optimize it
	* 
	* > PROBLEM:    Dynamic lists in the form of std::vector and such aren't supported.
	*   NOTE:       There may be a solution to this problem; At the moment I need static refferences to work.
	*               If std::vector<element_list> support is added, it is likely to come with the sacrifice of static refferences working.
	*	WORKAROUND: Variables support std::vector, and any other datatype you may wish for if you add the support.
	*				It will most likely look very ugly on file, but it should work.
	*
	* * * * * * * * * * * * * * * * * * * * * * * */

	// Value entries will have the key on the left side of this symbol, and the value on the right side
	// NOTE: Make sure that this symbol doesn't appear in the key, as it will be looked for as an identifier for the start of the value and to seperate values from lists
	constexpr auto ENTRY_DEVIDER_SYMBOL = '=';
	constexpr auto LIST_SYMBOL = '�';
	constexpr size_t INDENT_AMOUNT = 3u;

	// The key that will be used by the map in element_list
	using key_t = uint32_t;

	// Elements can be values or lists (hold multiple values)
	enum struct e_element_type : bool
	{
		VALUE = false,
		LIST
	};

	// Return values for the save/load functions
	enum class e_save_returns : uint8_t
	{
		SUCCESS = 0,
		SAVEDIRECTORY_IS_NOT_DIRECTORY
	};
	enum class e_load_returns : uint8_t
	{
		SUCCESS = 0,
		LOADDIRECTORY_DOES_NOT_EXIST,
		LOADDIRECTORY_IS_NOT_DIRECTORY,
		FILE_DOES_NOT_EXIST
	};

	// Base element that both element_value and element_list inherits from.
	// Needed to put both elements in the same map
	struct base_element
	{
		virtual e_element_type type() const = 0;

		// Push the save value onto 'file'
		virtual void save(std::ofstream& file, const key_t key, size_t indentation) const = 0;

		virtual void load(const std::string& value) {};
	};

#pragma region Base of the Value elements

	template <typename T>
	struct element_value : public base_element
	{
		// TYPEDEFS
		using value_type = T;

		// CONSTRUCTORS
		element_value(const value_type value) : m_value(std::move(value)), base_element()
		{
		}

		e_element_type type() const
		{
			return e_element_type::VALUE;
		}

		// OPERATORS
		T& operator()()
		{
			return m_value;
		}

		T& get()
		{
			return m_value;
		}

		void save(std::ofstream& file, const key_t key, size_t indentation) const
		{
			file << std::string(indentation, ' ') << key << ENTRY_DEVIDER_SYMBOL << this->m_value;
		}

		void load(const std::string& value) override
		{
			if (value.empty())
				return;

			m_value = stoi(value);
		}

	private:
		value_type m_value;
	};

#pragma endregion

#pragma region Base of the List elements

	struct element_list : public base_element, public std::unordered_map < key_t, base_element* >
	{
		friend class master_list;

		// CONSTRUCTORS
		explicit element_list() : base_element()
		{
		}

		e_element_type type() const
		{
			return e_element_type::LIST;
		}

		template <typename T>
		element_value<T>& add_value(T value, key_t key)
		{
			operator[](key) = new element_value<T>(value);
			m_entries_ordered.push_back(key);
			return *reinterpret_cast<element_value<T>*>(operator[](key));
		}

		// Template is for adding structs that inherit from element_list
		template <typename T = element_list>
		T* add_list(key_t key)
		{
			operator[](key) = new T();
			m_entries_ordered.push_back(key);
			return reinterpret_cast<T*>(operator[](key));
		}

		template <size_t Size, typename T = element_list>
		std::array<T*, Size> add_list_array(key_t key)
		{
			element_list* new_list = add_list<element_list>(key);

			std::array<T*, Size> lists;
			for (size_t i = 0u; i < Size; i++)
				lists.at(i) = new_list->add_list<T>(i);

			return lists;
		}

		void save(std::ofstream& file, const key_t key, size_t indentation) const
		{
			file << std::string(indentation, ' ') << LIST_SYMBOL << key << '\n';

			for (const key_t entry_key : m_entries_ordered)
			{
				at(entry_key)->save(file, entry_key, indentation + INDENT_AMOUNT);

				if (at(entry_key)->type() == e_element_type::VALUE)
					file << '\n';
			}
		}

		void json_save(std::ofstream& file, const key_t key, size_t indentation) const
		{
			file << std::string(indentation, ' ') << "\"" << key << "\": {\n";

			for (const key_t entry_key : m_entries_ordered)
			{
				at(entry_key)->save(file, entry_key, indentation + INDENT_AMOUNT);

				if (at(entry_key)->type() == e_element_type::VALUE)
					file << '\n';
			}

			file << std::string(indentation, ' ') << "}";
		}

		// TO FILE SAVES
		e_save_returns save_to_file(std::filesystem::path filepath) const
		{
			// Check if the directory exists already and create it if it doesn't
			if (!std::filesystem::exists(filepath.parent_path()))
				std::filesystem::create_directories(filepath.parent_path());

			if (!std::filesystem::is_directory(filepath.parent_path()))
				return e_save_returns::SAVEDIRECTORY_IS_NOT_DIRECTORY;

			std::ofstream file;
			file.open(filepath);

			this->save(file, 0u, 0u);

			file.close();

			return e_save_returns::SUCCESS;
		}

		e_load_returns load_from_file(std::filesystem::path filepath)
		{
			// Lambda will return the indentation level of an inputted line
			auto get_indentation = [&](const std::string& line) -> size_t
			{
				size_t i = 0;
				while (line.at(i) == ' ' && i < line.size()) ++i;
				return i / INDENT_AMOUNT;
			};

			// Check if the directory exists already and create it if it doesn't
			if (!std::filesystem::exists(filepath.parent_path()))
				return e_load_returns::LOADDIRECTORY_DOES_NOT_EXIST;

			if (!std::filesystem::is_directory(filepath.parent_path()))
				return e_load_returns::LOADDIRECTORY_IS_NOT_DIRECTORY;

			std::ifstream file(filepath);
			if (!file.good())
				return e_load_returns::FILE_DOES_NOT_EXIST;

			// current_list will be used to keep track of which list we are currently in
			std::vector<element_list*> list_hierarchy = { this };

			// Looping the lines in the file
			std::string line;
			while (getline(file, line))
			{
				if (line.empty())
					continue;

				// The current indentation level will be based on how deel we are in the list hierarchy
				size_t indentation_level = list_hierarchy.size();

				// If the indentation on the line is smaller than the current indentation level a list has ended and should be backed out of
				while (get_indentation(line) < indentation_level)
				{
					list_hierarchy.pop_back();
					indentation_level = list_hierarchy.size();
				}

				// If the line contains the symbol LIST_SYMBOL it is a list start 
				// CONSIDER: `if (line.at(indentation_level+1u) == LIST_SYMBOL)` <would decrease missinterpretation>
				const size_t list_symbol_pos = line.find(LIST_SYMBOL);
				const bool is_list = list_symbol_pos != std::string::npos;

				// The currently browsing list will be at the back of the list_hierarchy
				element_list* current_list = list_hierarchy.back();

				// If the entry is a new list update the current_list array 
				if (is_list)
				{
					key_t key;
					std::istringstream(line.substr(list_symbol_pos + 1u, line.size() - 1u)) >> key;

					// Make sure that the key exists
					auto it = std::find(current_list->m_entries_ordered.begin(), current_list->m_entries_ordered.end(), key);
					if (it == current_list->m_entries_ordered.end())
						continue;

					list_hierarchy.push_back(reinterpret_cast<element_list*>(current_list->at(key)));
				}
				// If it isn't a list, it's a value
				else
				{
					// Get the value as a string
					size_t value_start = line.find(ENTRY_DEVIDER_SYMBOL) + 1u;
					std::string value = line.substr(value_start, line.size());

					// Get the element key
					key_t key;
					std::istringstream(line.substr(0u, value_start)) >> key;

					// Make sure that the key exists
					auto it = std::find(current_list->m_entries_ordered.begin(), current_list->m_entries_ordered.end(), key);
					if (it == current_list->m_entries_ordered.end())
						continue;

					current_list->operator[](key)->load(value);
				}
			}

			file.close();

			return e_load_returns::SUCCESS;
		}

	protected:
		std::vector<key_t> m_entries_ordered;
	};

#pragma endregion List elements are meant as containers for the value elements

#pragma region Base of the Master list elements

	struct master_list : public element_list
	{
		// CONSTRUCTORS
		master_list() : element_list()
		{
		}

		// Override as our master_element won't need to be Mentioned directly, as it is the object we are loading the file from/to anyways
		void save(std::ofstream& file, const key_t key, size_t indentation) const override
		{
			for (const key_t entry_key : m_entries_ordered)
			{
				at(entry_key)->save(file, entry_key, indentation + INDENT_AMOUNT);

				if (at(entry_key)->type() == e_element_type::VALUE)
					file << '\n';
			}
		}
	};

#pragma endregion The master list elements are similar to the list elements, however they can save their content to disk or load from disk

}

#ifndef VANSAVE_NO_HELPER_MACROS

// c+p string hasher found here: https://stackoverflow.com/a/8317622
constexpr unsigned int hash_str(const char* s) { unsigned h = 37; for (; *s; s++) h = (h * 54059) ^ (s[0] * 76963); return h % 86969; }
#define CT_HASH(str) hash_str(str)

#define M_DECLARE_VAR(datatype, name, default_value) datatype& name = add_value<datatype>(default_value, CT_HASH(#name))()
#define M_ADD_SUBLIST(listtype, name) listtype* name = add_list<listtype>(CT_HASH(#name)); 
#define M_ADD_SUBLIST_ARR(listtype, name, size)  std::array<listtype*, size> name = add_list_array<size, listtype>(CT_HASH(#name)); 
#define M_NEW_LIST(name) struct name : public vansave::element_list
#define M_MASTER_LIST(name) struct name : public vansave::master_list

#endif