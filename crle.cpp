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

#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>
#include <vector>
#include <cmath>
#include <bitset>

#include "inc/crle.h"

using namespace std;
using namespace filesystem;
using namespace crle;

bool a_show_info = true;

vector<unsigned char> str_split_iarr(const string& s) 
{
    vector<unsigned char> result;
    stringstream ss(s);
    string str;
	
	while (getline(ss, str, ',')) {
        // Remove spaces from str if has
        str.erase(0, str.find_first_not_of(' '));
        str.erase(str.find_last_not_of(' ') + 1);
		
		// Convert str to char
        try {
            unsigned char value = stoi(str);
            result.push_back(value);
        } catch (const invalid_argument& e) {
            cerr << "ERR: '" << str << "' not integer." << endl;
        } catch (const out_of_range& e) {
            cerr << "ERR: '" << str << "' integer is out of range." << endl;
        }		
	}	
	
	return result;
}

vector<unsigned char> compress_rle(vector<unsigned char> in)
{
    vector<unsigned char> result; 
	vector<int> stat(256, 0);
	int i;
	int tag =- 1;	
	int size = in.size();

	// Search TAG, count all bytes
	for(i = 0; i < size; i++) {
		stat.at((int)in.at(i))++;
	}
	
	for(i = 0; i < 256; i++) {
		if(!stat[i]) {
			tag = i;
			break;
		}
	}
	
	if(tag < 0) return result;
		
	// Make RLE Array
	int len = 1;
	unsigned char ch, ch_prev;
		
	result.push_back(tag);
	
	for(i = 0; i < size; i++) {	
		ch = in[i];
		if(ch_prev != ch || len >= 255 || i == size - 1) {
			if(len > 1) {
				if(len == 2) {
					result.push_back(ch_prev);
				} else {
					result.push_back(tag);
					result.push_back(len - 1);
				}
			}
			
			result.push_back(ch);
			ch_prev = ch;
			len = 1;
		} else {
			len++;
		}
	}
	
	// End of file marked with zero length
	result.push_back(tag);
	result.push_back(0);
	
	return result;
}

void print_data_size(double sz)
{
	string prefix;
	
	if (sz > 1024) {
		sz = sz / 1024;
		prefix = "K";
		if (sz > 1024) { 
			sz = sz / 1024;
			prefix = "M";
		}
		
		prefix += "b.";		
		cout << fixed << setprecision(2) << sz << " "<< prefix;
	} else {
		cout << sz << " b.";
	}
	
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "ru_RU.UTF-8");
	
	s_prog_argument args[] = {
		{"i", "hide information.", false}, 
		{"h", "hex format.", false}, 
		{"b", "binary format.", false}, 
		{"p", "cpp header format.", false}, 
	};
		
    if (argc < 2) {
		// No arguments - Usage help
		cout << "Usage: crle.exe *.csv";
		for (auto& a : args) {
			cout << " [/" << a.s_name << "]";
		}
		cout << endl << endl;

		// Notices
		for (auto& a : args) {
			cout << "/" << a.s_name << " " << a.s_notice << endl;
		}

		cout << endl;
    } else {	
		string s_file_name = argv[1];
		
		// Fill arguments struct
		for (int i = 2; i < argc; i++)
		{
			string s = strtolower(argv[i]);
			for (auto& a : args) {
				if ("/" + a.s_name == s) {
					a.b_on = true;
					
					if (a.s_name == "i")
						a_show_info = false;
				}
			}
		}		
		
		stringstream buffer;
		stringstream s_result;
		string s_name;
		string s_out_name;
		
		
		// Open files
		ifstream fin(s_file_name.c_str());
		unsigned int pos = s_file_name.find_last_of(".");
		
		s_name = s_file_name.substr(0, pos);
		s_out_name = s_name;
		s_out_name.append(".txt");
		ofstream fout(s_out_name.c_str());
		
		if (!fin.is_open()) {
			cerr << "ERROR: Could not open file " << s_file_name << endl;
			return 0;
		}												
		
		if (!fout.is_open()) {
			cerr << "ERROR: Could not create file " << s_out_name << endl;
			return 0;
		}				
		
		buffer << fin.rdbuf();		
		fin.close();
		
		vector<unsigned char> arr = str_split_iarr(buffer.str());		
		vector<unsigned char> rle = compress_rle(arr);

		e_mode mode = E_MODE_DEFAULT;
		for (auto& a : args) {
			if (a.b_on) {
				switch ((const char)a.s_name[0]) {
					case 'h':
						mode = E_MODE_HEX;
					break;
					case 'b':
						mode = E_MODE_BINARY;
					break;
					case 'p':
						mode = E_MODE_CPP;
					break;					
				}
			}
		}		
		
		if (mode == E_MODE_CPP) {
			s_result << "const unsigned char " << s_name << "[" << rle.size() << "] = {" << endl;
		}
		
		// Format file data
		for (size_t i = 0; i < rle.size(); i++) {
			if (mode == E_MODE_DEFAULT) {
				s_result << dec << (int)rle.at(i);
			}
			if (mode == E_MODE_HEX || mode == E_MODE_CPP) {
				s_result << "0x" << uppercase << hex << setw(2) << setfill('0') << (int)rle.at(i);
			}
			if (mode == E_MODE_BINARY) {
				s_result << rle.at(i);	
			} else {			
				if (i != rle.size() - 1) {
					s_result << ",";
					
					if ((i + 1) % 16 == 0) {
						s_result << endl;
					}
				}
			}
		}	

		if (mode != E_MODE_BINARY) {
			s_result << endl;
		}
		
		if (mode == E_MODE_CPP) {
		s_result << "};" << endl;
			
		}

		
		if (a_show_info) {

			cout << "[ INFORMATION ]" << endl;
			
			cout << "File name: " << s_file_name << endl;
			cout << "Write to file: " << s_out_name << endl;

			cout << "Original data size: ";			
			print_data_size((double)arr.size());		
			cout << endl;
			
			cout << "RLE data size: ";			
			print_data_size((double)rle.size());		
			cout << endl;	
			cout << endl;
			
			cout << "RLE tag: ";
			cout << "0x" << uppercase << hex << setw(2) << setfill('0') << (int)rle.at(0);	
			cout << endl;				
			
			double percentage = 100 - ((double)rle.size() / (double)arr.size()) * 100;
			
			cout << "Compress: ";			
			cout << fixed << setprecision(2) << percentage << "% " << endl;			
			cout << endl;
	
		}
					
		fout << s_result.str();
		fout.close();
	}	
	
		
}