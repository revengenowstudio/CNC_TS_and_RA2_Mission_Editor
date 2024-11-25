#pragma once

#include <afx.h>
#include "IniFile.h"
#include <vector>
#include <map>

using IndiceStorage = std::vector<CString>;
using SequencedSection = std::vector<CString>;
class IniSectionGroup;

/*
* @brief This class uses to simulation a combination of a few ini dictionaries, to
		 act as they are inside a same ini
*/
class IniFileGroup
{
	friend class IniMegaFile;
public:

	using StorageType = std::vector<const CIniFile*>;

	IniFileGroup() = default;


	const CString& GetString(const CString& section, const CString& key) const;
	CString GetStringOr(const CString& section, const CString& key, const CString& def) const;
	IniSectionGroup GetSection(const CString& section) const;

	bool GetBool(const CString& pSection, const CString& pKey, bool def = false) const
	{
		return INIHelper::StringToBool(GetString(pSection, pKey), def);
	}
	int GetInteger(const CString& pSection, const CString& pKey, int def = 0) const
	{
		return INIHelper::StringToInteger(GetString(pSection, pKey), def);
	}

	auto Size() const { return m_group.size(); }

	void Append(const CIniFile& INI);
	const CIniFile* Nth(int idx) const;

	auto begin() const { return m_group.begin(); }
	auto end() const { return m_group.end(); }
	bool empty() const { return m_group.empty(); }

private:
	StorageType m_group;
};

class IniSectionGroup
{
	using KvIter = CIniFileSection::Container::const_iterator;
	using GroupIter = IniFileGroup::StorageType::const_iterator;
public:
	class Iterator;
	friend class Iterator;
	class Iterator {
	public:
		Iterator(const CString& section,
			GroupIter groupIter,
			GroupIter groupIterEnd,
			KvIter kvIter,
			KvIter kvIterEnd
		) :
			m_section(section),
			m_groupIter(groupIter),
			m_groupIterEnd(groupIterEnd),
			m_kvIter(kvIter),
			m_kvIterEnd(kvIterEnd)
		{
		}
		Iterator& operator++() {
			// section content still working
			if (m_kvIter != std::prev(m_kvIterEnd)) {
				m_kvIter++;
				return *this;
			}
			m_groupIter++;
			if (m_groupIter != m_groupIterEnd) {
				auto [beg, end] = IniSectionGroup::acquireNextKvGroup(m_section, m_groupIter, m_groupIterEnd);
				m_kvIter = beg;
				m_kvIterEnd = end;
			} else {
				m_kvIter = m_kvIterEnd;// to make (==) works
			}
			return *this;
		}

		auto const& operator*() const {
			return *m_kvIter;
		}
		bool operator==(const Iterator& rhs) const {
			return m_groupIter == rhs.m_groupIter;
		}

	private:
		GroupIter m_groupIter;
		GroupIter m_groupIterEnd;
		KvIter m_kvIter;
		KvIter m_kvIterEnd;
		const CString& m_section;
	};

	IniSectionGroup(IniFileGroup&& source, const CString& secName) : 
		m_source(std::move(source)),
		m_section(secName)
	{
	}

	IniSectionGroup(const IniFileGroup& source, const CString& secName) :
		IniSectionGroup(IniFileGroup(source), secName)
	{
	}

	Iterator begin() const {
		auto groupBeg = m_source.begin();
		auto [secItBeg, secItEnd] = acquireNextKvGroup(groupBeg);
		return Iterator(m_section, groupBeg, m_source.end(), secItBeg, secItEnd);
	}

	Iterator end() const {
		auto secItEnd = KvIter{};
		if (!m_source.empty()) {
			auto const& sec = (*std::prev(m_source.end()))->GetSection(m_section);
			secItEnd = sec.end();
		}
		return Iterator(m_section, m_source.end(), m_source.end(), secItEnd, secItEnd);
	}

	bool HasValue(const CString& expected) const {
		for (auto const& [_, val] : *this) {
			if (val == expected) {
				return true;
			}
		}
		return false;
	}

private:
	// attention: beginning iter 
	static std::pair< KvIter, KvIter> acquireNextKvGroup(const CString& section, GroupIter& beg, const GroupIter end) {

		auto secItBeg = KvIter{};
		auto secItEnd = KvIter{};
		for (; beg != end; ++beg) {
			auto const& sec = (*beg)->GetSection(section);
			if (sec.Size() == 0) {
				continue;
			}
			secItBeg = sec.begin();
			secItEnd = sec.end();
			break;
		}
		return { secItBeg, secItEnd };
	}

	std::pair< KvIter, KvIter> acquireNextKvGroup(GroupIter& beg) const {
		return acquireNextKvGroup(m_section, beg, m_source.end());
	}

	const IniFileGroup m_source;
	CString m_section;
};
