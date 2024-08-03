﻿/*
	FinalSun/FinalAlert 2 Mission Editor

	Copyright (C) 1999-2024 Electronic Arts, Inc.
	Authored by Matthias Wagner

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// IniFile.cpp: Implementierung der Klasse CIniFile.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IniFile.h"
#include <string>
#include <algorithm>
#include <stdexcept>



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

using namespace std;

const CIniFileSection CIniFile::EmptySection;
const CString CIniFileSection::EmptyValue;

typedef map<CString, CIniFileSection>::iterator CIniI;


//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CIniFile::CIniFile()
{
	Clear();
}

CIniFile::~CIniFile()
{
	sections.clear();
}

WORD CIniFile::LoadFile(const CString& filename, BOOL bNoSpaces)
{
	return LoadFile(std::string(filename.GetString()), bNoSpaces);
}

WORD CIniFile::LoadFile(const std::string& filename, BOOL bNoSpaces)
{
	Clear();

	if (filename.size() == NULL) {
		return 1;
	}
	m_filename = filename;

	return(InsertFile(filename, NULL, bNoSpaces));

}


void CIniFile::Clear()
{
	sections.clear();
}

CIniFileSection::CIniFileSection()
{
};

CIniFileSection::~CIniFileSection()
{
	value_pos.clear();
	value_pairs.clear();
};

WORD CIniFile::InsertFile(const CString& filename, const char* Section, BOOL bNoSpaces)
{
	return InsertFile(std::string(filename.GetString()), Section, bNoSpaces);
}

WORD CIniFile::InsertFile(const std::string& filename, const char* Section, BOOL bNoSpaces)
{
	if (filename.size() == 0)
		return 1;

	fstream file;

	file.open(filename, ios::in);
	if (!file.good())
		return 2;


	//char cSec[256];
	//char cLine[4096];

	//memset(cSec, 0, 256);
	//memset(cLine, 0, 4096);
	CString cSec;
	std::string cLine;

	const auto npos = std::string::npos;

	while (!file.eof()) {
		std::getline(file, cLine);

		// strip to left side of newline or comment
		cLine.erase(std::find_if(cLine.begin(), cLine.end(), [](const char c) { return c == '\r' || c == '\n' || c == ';'; }), cLine.end());

		const auto openBracket = cLine.find('[');
		const auto closeBracket = cLine.find(']');
		const auto equals = cLine.find('=');

		if (openBracket != npos && closeBracket != npos && openBracket < closeBracket && (equals == npos || equals > openBracket)) {
			if ((Section != nullptr) && cSec == Section)
				return 0; // the section we want to insert is finished

			cSec = cLine.substr(openBracket + 1, closeBracket - openBracket - 1).c_str();
		} else if (equals != npos && !cSec.IsEmpty()) {
			if (Section == NULL || cSec == Section) {
				// a value is set and we have a valid current section!
				CString name = cLine.substr(0, equals).c_str();
				CString value = cLine.substr(equals + 1, cLine.size() - equals - 1).c_str();

				if (bNoSpaces) {
					name.Trim();
					value.Trim();
				}
				sections[cSec].SetString(name, value);
			}
		}

	}



	file.close();

	return 0;
}

const CString* CIniFile::GetSectionName(std::size_t index) const noexcept
{
	if (index > sections.size() - 1)
		return NULL;

	auto i = sections.cbegin();
	for (auto e = 0; e < index; ++e)
		i++;

	return &(i->first);
}

BOOL CIniFile::SaveFile(const CString& filename) const
{
	return SaveFile(std::string(filename.GetString()));
}

BOOL CIniFile::SaveFile(const std::string& Filename) const
{
	fstream file;

	file.open(Filename, ios::out | ios::trunc);

	for (auto const& sec : sections) {
		file << "[" << sec.first << "]" << endl;
		for (auto const& pair : sec.second) {
			file << pair.first << "=" << pair.second << endl;
		}
		file << endl;
	}

	file << endl;

	return TRUE;
}


int64_t CIniFileSection::FindValue(CString val) const noexcept
{
	for (size_t idx = 0;
		idx < this->value_pairs.size();
		++idx) {
		if (this->value_pairs[idx].second == val) {
			return idx;
		}
	}
	return -1;
}

int64_t CIniFileSection::FindIndex(const CString& key) const noexcept
{
	auto const it = this->value_pos.find(key);
	if (it != this->value_pos.end()) {
		return it->second;
	}
	return -1;
}

CString CIniFile::GetValueByName(const CString& sectionName, const CString& valueName, const CString& defaultValue) const
{
	auto section = TryGetSection(sectionName);
	if (!section) {
		return defaultValue;
	}
	return section->GetStringOr(valueName, defaultValue);
}
