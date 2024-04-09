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

// IniFile.h: Schnittstelle für die Klasse CIniFile.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INIFILE_H__96455620_6528_11D3_99E0_DB2A1EF71411__INCLUDED_)
#define AFX_INIFILE_H__96455620_6528_11D3_99E0_DB2A1EF71411__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <map>
#include <CString>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <ios>
#include "IniHelper.h"


using namespace std;

class SortDummy
{
public:
	bool operator() (const CString&, const CString&) const;
};



class CIniFileSection
{
public:

	static const CString EmptyValue;

	CIniFileSection();
	virtual ~CIniFileSection();

	[[deprecated("instead use GetString or TryGetString")]]
	const CString& AccessValueByName(const CString& name) const {
		return GetString(name);
	}

	auto const& Nth(size_t index) const {
		ASSERT(index < value_pairs.size());
		return this->value_pairs[index];
	}
	int FindIndex(const CString& key) const noexcept;
	int FindValue(CString val) const noexcept;

	const CString& operator[](const CString& key) const {
		return this->GetString(key);
	}

	const CString* TryGetString(const CString& key) const {
		auto const it = value_pos.find(key);
		if (it != value_pos.end()) {
			return &this->value_pairs[it->second].second;
		}
		return nullptr;
	}

	const std::pair<int, bool> TryGetInteger(const CString& key) const {
		auto const got = this->TryGetString(key);
		if (!got) {
			return {};
		}
		return { INIHelper::StringToInteger(*got, 0), true };
	}

	const CString& GetString(const CString& key) const {
		if (auto const ret = TryGetString(key)) {
			return *ret;
		}
		return EmptyValue;
	}
	CString GetStringOr(const CString& key, const CString& defaultValue) const {
		auto const it = value_pos.find(key);
		if (it != value_pos.end()) {
			return this->value_pairs[it->second].second;
		}
		return defaultValue;
	}

	int GetInteger(const CString& key, int def = 0) const {
		return INIHelper::StringToInteger(this->GetString(key), def);
	}

	float GetFloat(const CString& key, float def = 0.0) const {
		return INIHelper::StringToFloat(this->GetString(key), def);
	}

	bool GetBool(const CString& key, bool def = false) const {
		auto const& str = this->GetString(key);
		return INIHelper::StringToBool(str, def);
	}

	size_t Size() const { return value_pos.size(); }

	bool Exists(const CString& key) const {
		auto const it = value_pos.find(key);
		return it != value_pos.end();
	}
	bool HasValue(const CString& val) const {
		return this->FindValue(val) >= 0;
	}

	void SetString(const CString& key, const CString& value) {
		return this->SetString(key, CString(value));
	}

	void SetString(const CString& key, CString&& value) {
		auto const it = value_pos.find(key);
		// new, never had one
		if (it == value_pos.end()) {
			this->value_pairs.push_back({ key, std::move(value) });
			value_pos[key] = value_pairs.size();
			return;
		}
		value_pairs[it->second].second = std::move(value);
	}

	void SetInteger(const CString& key, const int val) {
		this->SetString(key, INIHelper::ToString(val));
	}

	void SetBool(const CString& key, const bool val) {
		this->SetString(key, INIHelper::ToString(val));
	}

	void Insert(const CString& key, const CString& value) {
		this->Insert(key, CString(value));
	}

	void Insert(const CString& key, CString&& value) {
		 value_pairs.push_back({ key, value });
		 value_pos.insert_or_assign(key, value_pairs.size() - 1);
	}

	// ==================== Delete

	void RemoveAt(size_t idx) {
		ASSERT(idx < value_pairs.size());
		for (auto affectedIdx = idx + 1; affectedIdx < value_pairs.size(); ++affectedIdx) {
			auto const& kvPair = value_pairs[affectedIdx];
			auto const it = value_pos.find(kvPair.first);
			ASSERT(it != value_pos.end());
			it->second--;
		}
		auto const itErased = value_pairs.erase(value_pairs.begin() + idx);
		ASSERT(value_pos.erase(itErased->first) == 1);
	}

	void RemoveByKey(const CString& key) {
		auto const idx = this->FindIndex(key);
		if (idx >= 0) {
			RemoveAt(idx);
		}
	}

	void RemoveValue(const CString& val) {
		auto const idx = this->FindValue(val);
		if (idx >= 0) {
			RemoveAt(idx);
		}
	}

	auto begin() const noexcept
	{
		return value_pairs.begin();
	}

	auto end() const noexcept
	{
		return value_pairs.end();
	}

	[[deprecated("instead use iterators or for_each")]]
	// get key
	const CString* GetValueName(std::size_t index) const noexcept {
		return &Nth(index).first;
	}

private:
	map<CString, int, SortDummy> value_pos{};
	vector<std::pair<CString, CString>> value_pairs{};// sequenced
	mutable bool isRegistry{false};
};

class CIniFile
{
	using StorageMap = map<CString, CIniFileSection>;

	static const CIniFileSection EmptySection;

public:
	CIniFile(CIniFile&& rhs) noexcept :
		m_filename(std::move(rhs.m_filename)),
		sections(std::move(rhs.sections))
	{}
	CIniFile(const CIniFile& rhs) noexcept :
		m_filename(rhs.m_filename),
		sections(rhs.sections)
	{}

	CIniFile& operator=(CIniFile&& rhs) noexcept {
		new (this)CIniFile(std::move(rhs));
		return *this;
	}
	CIniFile& operator=(const CIniFile& rhs) noexcept {
		new (this)CIniFile(rhs);
		return *this;
	}

	[[deprecated("instead use GetString")]]
	CString GetValueByName(const CString& sectionName, const CString& valueName, const CString& defaultValue) const;
	void Clear();
	WORD InsertFile(const CString& filename, const char* Section, BOOL bNoSpaces = FALSE);
	WORD InsertFile(const std::string& filename, const char* Section, BOOL bNoSpaces = FALSE);
	BOOL SaveFile(const CString& Filename) const;
	BOOL SaveFile(const std::string& Filename) const;
	WORD LoadFile(const CString& filename, BOOL bNoSpaces = FALSE);
	WORD LoadFile(const std::string& filename, BOOL bNoSpaces = FALSE);

	// ================ Section interfaces ================

	const CString* GetSectionName(std::size_t Index) const noexcept;
	const CIniFileSection* TryGetSection(const CString& section) const
	{
		auto pMutThis = const_cast<std::remove_cv_t<std::remove_pointer_t<decltype(this)>>*>(this);
		return pMutThis->TryGetSection(section);
	}

	CIniFileSection* TryGetSection(const CString& section)
	{
		auto it = sections.find(section);
		if (it != sections.end()) {
			return &it->second;

		}
		return nullptr;
	}

	const CIniFileSection& GetSection(const CString& section) const {
		if (auto found = this->TryGetSection(section)) {
			return *found;
		}
		return EmptySection;
	}

	const CIniFileSection& operator[](const CString& section) const {
		return this->GetSection(section);
	}

	bool DeleteSection(const CString& section) {
		return sections.erase(section) > 0;
	}

	typename StorageMap::iterator DeleteAt(const StorageMap::iterator pos) {
		return sections.erase(pos);
	}

	// ============= Reader and Helper converter ============================
	const CString& GetString(const CString& section, const CString& key) const {
		return GetSection(section).GetString(key);
	}
	const CString GetStringOr(const CString& section, const CString& key, const CString& def) const {
		return GetSection(section).GetStringOr(key, def);
	}
	const bool GetBool(const CString& section, const CString& key, bool def = false) const {
		return this->GetSection(section).GetBool(key, def);
	}
	const int GetInteger(const CString& section, const CString& key, int def = 0) const {
		return GetSection(section).GetInteger(key, def);
	}

	// ============== Writer and Helper converter ============================
	CIniFileSection& AddSection(CString&& sectionName) {
		auto const ret = this->sections.insert({ std::move(sectionName), {}});
		return ret.first->second;
	}
	CIniFileSection& AddSection(const CString& sectionName) {
		return this->AddSection(CString(sectionName));
	}

	void SetSection(const CString& sectionName, const CIniFileSection& sec) {
		sections.insert_or_assign(sectionName, sec);
	}

	void SetString(const CString& section, const CString& key, CString&& value) {
		auto const it = sections.find(section);
		if (it != sections.end()) {
			it->second.SetString(key, value);
			return;
		}
		auto&& newSec = CIniFileSection{};
		newSec.SetString(key, value);
		ASSERT(sections.insert({ key, std::move(newSec) }).second == true);
	}

	void SetString(const CString& section, const CString& key, const CString& value) {
		return this->SetString(section, key, CString(value));
	}

	void SetBool(const CString& section, const CString& key, const bool value) {
		 this->SetString(section, key, INIHelper::ToString(value));
	}
	void SetInteger(const CString& section, const CString& key, const int value) {
		this->SetString(section, key, INIHelper::ToString(value));
	}

	void RemoveValueByKey(const CString& section, const CString& key) {
		if (auto pSec = this->TryGetSection(section)) {
			pSec->RemoveByKey(key);
		}
	}

	// ================= Iterator Related =============================

	auto Size() const noexcept {
		return this->sections.size();
	}

	auto begin() noexcept
	{
		return sections.begin();
	}

	auto begin() const noexcept
	{
		return sections.begin();
	}

	auto end() noexcept
	{
		return sections.end();
	}

	auto end() const noexcept
	{
		return sections.end();
	}

	CIniFile();
	virtual ~CIniFile();

private:
	std::string m_filename;
	StorageMap sections;
};

#endif // !defined(AFX_INIFILE_H__96455620_6528_11D3_99E0_DB2A1EF71411__INCLUDED_)
