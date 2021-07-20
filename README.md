# MKSave
A Simple Static State Saving System. wow, that's a lot of words with S.

## Right, you probabily want to know what this is, and how to use it.

### What?
Well, the basic concept is definently familiar to you. It's another alternative to things like json, and ini parsers.
The main goal of this data to file saver is that I am trying to keep the data feel as normal as possible to work with in code. 

### Why?
Although this library isn't very extensive and relies on user to add support for custom types It has some clear upsides.
* 1. Whenever I would use other libraries for storing data, I found myself writing wrappers for them, and it made me think that I might as well make something that I would not have to write a wrapper for my own uses. 
* 2. The format of other file savers can be a bit weird some times. Like how most json libraries want you to use string keys like `core_obj["some_key"]["some_other_key]["yet_another_key].some_getter_function<some_datatype>()`. This constant string map search isn't ideal for performance. And in many cases refferences to these values may break after the file is reloaded. (`bool& simple_var = core_obj["you_get_the_path_point"]["by_now"].get<bool>()`)
* 3. You can ofc use other file savers to save values for custom structs like rgb colours, but in most cases implementation isn't quite as clean and "wrapper free" as I would like it. Which is why MKSave focuses on support of custom datatypes (without using raw data copying, so that you can actually understand the format in the file).
 
### OK, so what do I need to know?

The only thing you need to know to get started is how to set up a basic file.
You can see an example of how a file is setup, aswell as how support for custom structs can be added by looking at the example.cpp file.


# Here is a more in depth guide, for those of you that like reading:
  
## BASIC USE:
	
#### ADDING SUPPORT FOR NEW DATATYPES:
Saving and loading pushes the saved variable into a stream:
you can add support to a custom struct by adding stream support. This can be done by including these two friendly operators inside the struct :)
I would highly recommend taking a look at example.cpp to see how this has been used there.
```cpp
	friend std::ostream& operator<<(std::ostream& os, const datatype& v)
	{
		// Push formating into os here
		return os;
	}
	friend std::istream& operator>>(std::istream& is, data_struct& v)
	{
		// Load the values from is into v here
		return is;
	}
```
TIP: Keep in mind that spaces seperate inputs, so `os << v.v1 << " " << v.v2;` can be loaded as `is >> v.v1 >> v.v2`
	
#### CHANGING KEY TYPE:
The key is what is used to identify the variable on file. By default this is set to be `key_t = uint32_t`
if you wish to change this to another datatype, simply go to where key_t is defined and replace uint32_t with something else.
Depending on what you change it to some errors may occur. String should be supported, but make sure you change the macros as a hashing function is used there.

##### Why in the everloving *** would you use integers as keys? 
Well, first off, manners. 
Secondly, It is for the get functions. As I mentioned above, getting a matching string in a list is slower than getting an integer. 
I have included a lovely string compiletime hash function that you are free to use untill you lose your mind :)

## Massive list of DOWNFALLS & WORKAROUNDS:
	
 > PROBLEM:	Value elements only have one line to store their value.
   WORKAROUND:	If you need more lines for a single value you can make a list and save the values under there.
	
 > PROBLEM:	Config file is difficult ot read for humans
   WORKAROUND:	You can change the key to be a string with relative few problems by changing the key_t typedef
	   
 > PROBLEM:	Key cannot be retreived from the element itself
   WORKAROUND:	Save the parent somewhere and use it to get the key.
	
 > PROBLEM:	Save and load functions are heavily string relient and very under-engineered, so saving and loading larger structs may be slow
 > TODO:	Optimize it

 > PROBLEM:	Dynamic lists in the form of std::vector and such aren't supported.
   WORKAROUND:	In the same as you add custom variable support, you can add support for std::vector<your_datatype>. Would look a little ugly on file, but it should work.
