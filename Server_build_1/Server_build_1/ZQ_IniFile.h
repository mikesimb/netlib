#pragma once
#include <string>
#include <vector>
using namespace  std;
class CZQ_IniFile
{
public:
	explicit CZQ_IniFile(const string &fileName);
	~CZQ_IniFile();
	bool ReadFile(void);
	string ReadString(const string  &section, const string &key, const string &value);
	int ReadInt(const string &section, const string &key, const int &value);
	bool WriteString(const string &section, const string &key, const string &value);
	bool WriteInt(const string &section, const string &key, const int &value);
	bool RemoveSection(const string &section);
	bool RemoveKey(const string &section, const string &key);
	bool WriteFile(void);
private:
	static string Trim(const string &str);
	static string LTrim(const string &str);
	static string RTrim(const string &str);

private:
	string  _filename;
	vector<string> _VectLine;
	bool _ModifyFlag;
};

