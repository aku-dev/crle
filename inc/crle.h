/*
    MIT License

    Copyright (c) 2016-2025, Alexandr Kuznetsov

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#pragma once

#include <string>

namespace crle {
	
	using std::string;
		
	typedef enum
	{
		E_MODE_DEFAULT  = 0x01,
		E_MODE_BINARY  = 0x02,
		E_MODE_HEX  = 0x03,
		E_MODE_CPP  = 0x04,
		E_MODE_NO  = 0x05,
		E_MODE_SPLIT  = 0x06,
	} e_mode;
	
	struct s_prog_argument
	{
		string s_name;
		string s_notice;
		bool b_on;
	};
	
	string strtolower(const string& str) 
	{ 
		string result = ""; 
	  
		for (char ch : str) { 
			result += tolower(ch); 
		}
	  
		return result; 
	}
}