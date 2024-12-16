#pragma once
#include "StdAfx.h"

class IniTestHelper
{
	std::string m_fileName;

	void writeDownContent(const char* pContent) {
		std::ofstream iniFile(m_fileName.c_str());
		ASSERT(iniFile.is_open() == true);
		iniFile << pContent;
		iniFile.flush();
		iniFile.close();
	}

public:
	IniTestHelper(std::string&& name, const char* pContent) :
		m_fileName(std::move(name))
	{
		ASSERT(!m_fileName.empty());
		ASSERT(pContent != nullptr);
		writeDownContent(pContent);
	}
	~IniTestHelper() {
		remove(m_fileName.c_str());
	}

};