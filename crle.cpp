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
#include <algorithm>
#include <regex>

#include "inc/crle.h"

using namespace std;
using namespace filesystem;
using namespace crle;

bool a_show_info = true;
bool a_show_extra = false;

string get_int_str(const string& s) {
    string result;
    copy_if(s.begin(), s.end(), back_inserter(result), ::isdigit);
    return result;
}

vector<unsigned char> str_split(const string& s, const char& delim = ',') 
{
    vector<unsigned char> result;
	
	// Replace string brackes on commas
    stringstream ss(regex_replace(s, regex("\n"), ","));
    string str;
	
	while (getline(ss, str, (char)delim)) {
		int val = stoi(get_int_str(str));
        result.push_back(val);
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

string print_data_splits(int i, size_t sz, int stps=16)
{
	string result = "";
	
	if (i != sz - 1) {
		result.append(",");
		
		if ((i + 1) % stps == 0) {
			result.append("\n");
		}
	}
	return result;
}


int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "ru_RU.UTF-8");
	
	s_prog_argument args[] = {
		{"hi", "hide information.", false}, 
		{"arr", "show array information.", false},
		
		{"h", "hex format.", false}, 
		{"b", "binary format.", false}, 
		{"p", "cpp header format.", false},
		{"n", "cpp header no rle compression.", false},
		
		{"sp", " [w] [h] split file.", false},
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
		e_mode mode = E_MODE_DEFAULT;
		
		
		// Fill arguments struct
		for (int i = 2; i < argc; i++)
		{
			string s = strtolower(argv[i]);
			for (auto& a : args) {
				if ("/" + a.s_name == s) {
					a.b_on = true;
					
					if (a.s_name == "hi") {
						a_show_info = false;
					}
					if (a.s_name == "arr") {
						a_show_extra = true;
					}
					if (a.s_name == "sp") {
						mode = E_MODE_SPLIT;
					}
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
		
		if (!fin.is_open()) {
			cerr << "ERROR: Could not open file " << s_file_name << endl;
			return 0;
		}
		
		buffer << fin.rdbuf();		
		fin.close();
		
		if (mode == E_MODE_SPLIT) {
			// Split files	
			int width = 16;  // 32
			int height = 16; // 30
			
			if (argc > 3) width = stoi(argv[3]);
			if (argc > 4) height = stoi(argv[4]);
			
			if (width < 4) width = 4;
			if (height < 1) height = 4;
			
			// Split data to lines
			vector<string> file_names;
			stringstream stream(buffer.str());
			string line;
			int count_lines = 0;
			int count_height_files = 0;
			while (getline(stream, line)) {
				vector<unsigned char> arr = str_split(line);
				
				if (arr.size() < width) {
					cerr << "ERROR: Data is shorter than the setting width" << endl;
					break;
				}
				
				string result;
				
				for(int i = 0; i < arr.size(); i++) {
					// Insert value
					result.append(to_string(arr.at(i)));
					
					// Write all values in File
					if ((i + 1) % width == 0) {
						// Make name
						s_out_name = s_name;
						s_out_name.append("_x");
						s_out_name.append(to_string(i / width));
						s_out_name.append("_y");
						s_out_name.append(to_string(count_height_files));
						s_out_name.append(".csv");
						file_names.push_back(s_out_name);
						
						// Write file
						ofstream fout(s_out_name.c_str(), ofstream::app);
						if (!fout.is_open()) {
							cerr << "ERROR: Could not create file " << s_out_name << endl;
							return 0;
						}
						
						fout << result << endl;
						fout.close();
						result = "";
						
					} else {
						result.append(",");
					}
				}
				
				// Increment counters
				count_lines++;
				if ((count_lines + 1) % (height + 1) == 0)  {
					count_height_files++;
					count_lines = 0;
				}								
			}

			
			// Show information
			if (a_show_info) {
				cout << "[ INFORMATION ]" << endl;
				
				cout << "File name: " << s_file_name << endl;
				cout << "Size: ";
					cout << dec << width;
					cout << "x";
					cout << dec << height;
				cout << endl;
					
				cout << "Create files: " << endl;
				
				// Remove non-unique names
				sort(file_names.begin(), file_names.end());
				file_names.erase(unique(file_names.begin(), file_names.end()), file_names.end());
				for(auto& n : file_names) {
					cout << n << endl;
				}
				
				cout << endl;
			}
		} else {
			// RLE Compression		
		
			vector<unsigned char> arr = str_split(buffer.str());
			vector<unsigned char> rle = compress_rle(arr);		
					
			for (auto& a : args) {
				// Mode set is one chars.
				if (a.b_on && a.s_name.length() == 1) {
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
						case 'n':
							mode = E_MODE_NO;
						break;	
					}
				}
			}
			
			if (mode == E_MODE_NO) {
				rle = arr;
				mode = E_MODE_CPP;
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
					s_result << print_data_splits(i, rle.size());
				}
			}	

			if (mode != E_MODE_BINARY) {
				s_result << endl;
			}
			
			if (mode == E_MODE_CPP) {
				s_result << "};" << endl;
				
			}

			// Show information
			if (a_show_info) {
				
				// Show extra information
				if (a_show_extra) {
					cout << "[ EXTRA INFORMATION ]" << endl;
					cout << "ORIGINAL: " << endl;
					for (size_t i = 0; i < arr.size(); i++) {
						cout << dec << (int)arr.at(i);
						cout << print_data_splits(i, arr.size());
					}
					cout << endl;
					cout << endl;
					cout << "RLE: " << endl;
					
					cout << s_result.str();
					
					cout << endl;
				}			
				

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
				
			s_out_name = s_name;
			s_out_name.append(".txt");
			ofstream fout(s_out_name.c_str());
			if (!fout.is_open()) {
				cerr << "ERROR: Could not create file " << s_out_name << endl;
				return 0;
			}
				
			fout << s_result.str();
			fout.close();
		}	
	
	}
		
}