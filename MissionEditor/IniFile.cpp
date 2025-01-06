/*
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
#include "Helpers.h"
#include <string>
#include <algorithm>
#include <stdexcept>
#include <set>
#include <unordered_set>


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

bool isSectionRegistry(const CString& secName)
{
	static std::unordered_set<std::string_view> registryNames = {
		"BuildingTypes",
		"InfantryTypes",
		"AircraftTypes",
		"VehicleTypes",
		"Animations",
	};

	auto const nameView = std::string_view(secName.GetString(), secName.GetLength());
	return registryNames.find(nameView) != registryNames.end();
}

WORD CIniFile::InsertFile(const CString& filename, const char* Section, BOOL bNoSpaces)
{
	return InsertFile(std::string(filename.GetString()), Section, bNoSpaces);
}

WORD CIniFile::InsertFile(const std::string& filename, const char* pSectionSpecified, BOOL bNoSpaces)
{
	if (filename.size() == 0) {
		return 1;
	}

	fstream file;

	file.open(filename, ios::in);
	if (!file.good()) {
		return 2;
	}

	//char cSec[256];
	//char cLine[4096];

	//memset(cSec, 0, 256);
	//memset(cLine, 0, 4096);
	CString curSecParsed;
	std::string curLineParsed;

	const auto npos = std::string::npos;

#if 0
	std::map<CString, std::list<std::pair<CString, CString>>> registryMap;
#endif
	std::set<CString> registryValues;

	while (!file.eof()) {
		std::getline(file, curLineParsed);

		// strip to left side of newline or comment
		curLineParsed.erase(std::find_if(curLineParsed.begin(), curLineParsed.end(), [](const char c) { return c == '\r' || c == '\n' || c == ';'; }), curLineParsed.end());

		const auto openBracketPos = curLineParsed.find('[');
		const auto closeBracketPos = curLineParsed.find(']');
		const auto equalPos = curLineParsed.find('=');

		if (openBracketPos != npos && closeBracketPos != npos && openBracketPos < closeBracketPos && (equalPos == npos || equalPos > openBracketPos)) {
			if ((pSectionSpecified != nullptr) && curSecParsed == pSectionSpecified) {
				break; // the section we want to insert is finished
			}

			curSecParsed = curLineParsed.substr(openBracketPos + 1, closeBracketPos - openBracketPos - 1).c_str();
			registryValues.clear();
			continue;
		}
		
		if (equalPos != npos && !curSecParsed.IsEmpty()) {
			if (pSectionSpecified == NULL || curSecParsed == pSectionSpecified) {
				// a value is set and we have a valid current section!
				CString keyName = curLineParsed.substr(0, equalPos).c_str();
				CString value = curLineParsed.substr(equalPos + 1, curLineParsed.size() - equalPos - 1).c_str();
				auto& curSectionMap = sections[curSecParsed];

				if (bNoSpaces) {
					keyName.Trim();
					value.Trim();
				}

				if (isSectionRegistry(curSecParsed) && IsNumeric(keyName)) {
					auto const [_, inserted] = registryValues.insert(value);
					// WW's duplicated record, skip
					if (!inserted) {
						continue;
					}
				}

				// this is a duplicated number, and already added
				// so we will append in the end of the list later
#if 0
				if (IsNumeric(keyName) && curSectionMap.Exists(keyName)) {
					registryMap[curSecParsed].push_back({ keyName, value });
				}
#endif

				// special handling for +=
				if (keyName == '+') {
					keyName += value;
				}

				curSectionMap.SetString(keyName, value);
			}
		}

	}

	// now we append all items to the list
#if 0
	for (auto const& [secName, registry] : registryMap) {
		auto& secMap = sections[secName];
		for (auto const& [key, value] : registry) {
			secMap.SetString("auto" + key, value);
		}
	}
#endif


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
			auto keyName = pair.first;
			// restore +=
			if (keyName[0] == '+') {
				keyName = '+';
			}
			file << keyName << "=" << pair.second << endl;
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