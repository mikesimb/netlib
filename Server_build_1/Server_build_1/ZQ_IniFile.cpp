#include "stdafx.h"
#include "ZQ_IniFile.h"
#include <fstream>
#include <sstream>
#include<algorithm>
using namespace std;

struct ci_char_traits : public char_traits<char>
{
	static bool eq(char c1, char c2)
	{
		return toupper(c1) == toupper(c2);
	}
	static bool ne(char c1, char c2)
	{
		return toupper(c1) != toupper(c2);
	}
	static bool lt(char c1, char c2)
	{
		return toupper(c1) < toupper(c2);
	}
	static bool compare(const char* s1, const char* s2, size_t n)
	{
#ifdef WIN32  
		return memicmp(s1, s2, n) != 0;  //ʵ�ֲ����ִ�Сд�Ĵ��Ƚ�  
#else  
		//linux��֧��memicmp���Զ���汾  
		char *tmps1 = new char[n];
		char *tmps2 = new char[n];
		for (size_t i = 0; i < n; ++i)
		{
			tmps1[i] = toupper(s1[i]);
			tmps2[i] = toupper(s2[i]);
		}
		return memcmp(tmps1, tmps2, n) != 0;
#endif  
	}
	static const char* find(const char*s, int n, char a)
	{
		while (n-- > 0 && toupper(*s) != toupper(a))
			++s;
		return s;
	}
};

CZQ_IniFile::CZQ_IniFile(const string &fileName)
	:_filename(fileName),
	_ModifyFlag(false)
{
	ReadFile();
}


CZQ_IniFile::~CZQ_IniFile()
{
	WriteFile();
}

bool CZQ_IniFile::ReadFile(void)
{
	ifstream in(_filename.c_str());
	if (false == in.is_open())
		return false;

	string line;
	while (getline(in, line))
	{
		_VectLine.push_back(line);
	}
	_ModifyFlag = false;
	return true;
}

std::string CZQ_IniFile::ReadString(const string &section, const string &key, const string &value)
{
	for (size_t i = 0; i < _VectLine.size(); ++i)
	{
		string section_line = LTrim(_VectLine[i]);
		size_t sec_begin_pos = section_line.find('[');
		if (sec_begin_pos == string::npos || sec_begin_pos != 0)
		{
			continue;
		}
		size_t sec_end_pos = section_line.find(']', sec_begin_pos);
		if (sec_end_pos == string::npos)
		{
			continue;
		}

		if ((section.c_str()) != Trim(section_line.substr(sec_begin_pos + 1, sec_end_pos - sec_begin_pos - 1)).c_str())
		{
			continue;
		}

		//find key  
		for (++i; i < _VectLine.size(); ++i)
		{
			string key_line = LTrim(_VectLine[i]);
			size_t sec_pos = key_line.find('[');
			if (sec_pos != string::npos && sec_pos == 0)
			{
				--i;  //reback a step,find again  
				break;//the line is section line  
			}

			if (key_line.find('#') != string::npos)
			{
				continue;//this is comment line  
			}
			size_t equal_pos = key_line.find('=');
			if (equal_pos == string::npos)
			{
				continue;
			}
			if (string(key.c_str()) != RTrim(key_line.substr(0, equal_pos)).c_str())
			{
				continue;
			}

			size_t comment_pos = key_line.find("#", equal_pos + 1);
			if (comment_pos != string::npos)
			{
				return Trim(key_line.substr(equal_pos + 1, comment_pos - equal_pos - 1));
			}

			return Trim(key_line.substr(equal_pos + 1));
		}
	}
	return value;
}

int CZQ_IniFile::ReadInt(const string &section, const string &key, const int &value)
{
	string str = ReadString(section, key, "");
	if ("" == str)
	{
		return value;
	}

	istringstream in(str.c_str());
	int ret = 0;
	in >> ret;
	return ret;
}

bool CZQ_IniFile::WriteString(const string &section, const string &key, const string &value)
{

	for (size_t i = 0; i < _VectLine.size(); ++i)
	{
		string section_line = LTrim(_VectLine[i]);
		size_t sec_begin_pos = section_line.find('[');
		if (sec_begin_pos == string::npos || sec_begin_pos != 0)
		{
			continue;
		}
		size_t sec_end_pos = section_line.find(']', sec_begin_pos);
		if (sec_end_pos == string::npos)
		{
			continue;
		}
		if (string(section.c_str()) != RTrim(section_line.substr(sec_begin_pos + 1, sec_end_pos - sec_begin_pos - 1)).c_str())
		{
			continue;
		}

		//find key  
		for (++i; i < _VectLine.size(); ++i)
		{
			string key_line = LTrim(_VectLine[i]);
			size_t sec_pos = key_line.find('[');
			if (sec_pos != string::npos && sec_pos == 0)
			{
				--i;  //reback a step,find again  
				break;//the line is section line  
			}

			if (key_line.find('#') != string::npos)
			{
				continue;//this is comment line  
			}
			size_t equal_pos = key_line.find('=');
			if (equal_pos == string::npos)
			{
				continue;
			}
			if (string(key.c_str()) != RTrim(key_line.substr(0, equal_pos)).c_str())
			{
				continue;
			}

			size_t comment_pos = key_line.find("#", equal_pos + 1);
			string new_line = key_line.substr(0, equal_pos + 1) + value;
			if (comment_pos != string::npos)
			{
				new_line += key_line.substr(comment_pos);
			}
			key_line = new_line;
			_ModifyFlag = true;
			return true;
		}

		//add a new key  
		_VectLine.insert(_VectLine.begin() + i, key + "=" + value);
		_ModifyFlag = true;
		return true;
	}

	//add a new section and a new key  
	_VectLine.insert(_VectLine.end(), "");
	_VectLine.insert(_VectLine.end(), "[" + section + "]");
	_VectLine.insert(_VectLine.end(), key + "=" + value);
	_ModifyFlag = true;
	return true;
}

bool CZQ_IniFile::WriteInt(const string &section, const string &key, const int &value)
{
	ostringstream out;
	out << value;
	return WriteString(section, key, out.str());
}

bool CZQ_IniFile::RemoveSection(const string &section)
{
	for (size_t i = 0; i < _VectLine.size(); ++i)
	{
		string section_line = LTrim(_VectLine[i]);
		size_t sec_begin_pos = section_line.find('[');
		if (sec_begin_pos == string::npos || sec_begin_pos != 0)
		{
			continue;
		}
		size_t sec_end_pos = section_line.find(']', sec_begin_pos);
		if (sec_end_pos == string::npos)
		{
			continue;
		}
		if (string(section.c_str()) != RTrim(section_line.substr(sec_begin_pos + 1, sec_end_pos - sec_begin_pos - 1)).c_str())
		{
			continue;
		}

		//  
		size_t del_begin = i;
		for (++i; i < _VectLine.size(); ++i)
		{
			string next_section = LTrim(_VectLine[i]);
			size_t next_pos = next_section.find('[');
			if (next_pos == string::npos || next_pos != 0)
			{
				continue;
			}

			break;
		}
		_VectLine.erase(_VectLine.begin() + del_begin, _VectLine.begin() + i);
		return true;
	}
	return false;
}

bool CZQ_IniFile::RemoveKey(const string &section, const string &key)
{
	for (size_t i = 0; i < _VectLine.size(); ++i)
	{
		string section_line = _VectLine[i];
		size_t sec_begin_pos = section_line.find('[');
		if (sec_begin_pos == string::npos || sec_begin_pos != 0)
		{
			continue;
		}
		size_t sec_end_pos = section_line.find(']', sec_begin_pos);
		if (sec_end_pos == string::npos)
		{
			continue;
		}
		if (string(section.c_str()) != Trim(section_line.substr(sec_begin_pos + 1, sec_end_pos - sec_begin_pos - 1)).c_str())
		{
			continue;
		}

		//find key  
		for (++i; i < _VectLine.size(); ++i)
		{
			string key_line = _VectLine[i];
			key_line = Trim(key_line);
			if (key_line.find('#') == 0)
			{
				continue;
			}

			size_t key_pos = key_line.find('=');
			if (key_pos == string::npos)
			{
				continue;
			}

			if (string(key.c_str()) == Trim(key_line.substr(0, key_pos)).c_str())
			{
				_VectLine.erase(_VectLine.begin() + i);
				return true;
			}
		}
	}
	return false;
}

bool CZQ_IniFile::WriteFile(void)
{
	//check if is need to save  
	if (false == _ModifyFlag)
	{
		return true;
	}
	ofstream out(_filename.c_str());
	for (size_t i = 0; i < _VectLine.size(); ++i)
	{
		out << _VectLine[i] << endl;
	}
	_ModifyFlag = false;
	return true;

}

std::string CZQ_IniFile::Trim(const string &str)
{
	return LTrim(RTrim(str));
}

std::string CZQ_IniFile::LTrim(const string &str)
{
	size_t pos = 0;
	while (pos != str.size())
	{
		if (' ' == str[pos])
		{
			++pos;
		}
		else
		{
			break;
		}
	}

	return str.substr(pos);
}

std::string CZQ_IniFile::RTrim(const string &str)
{
	size_t pos = str.size() - 1;
	while (pos >= 0)
	{
		if (' ' == str[pos])
		{
			--pos;
		}
		else
		{
			break;
		}
	}

	return str.substr(0, pos + 1);
}
