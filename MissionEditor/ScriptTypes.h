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

#if !defined(AFX_SCRIPTTYPES_H__981EF700_951E_11D3_B63B_BCCF9C98B341__INCLUDED_)
#define AFX_SCRIPTTYPES_H__981EF700_951E_11D3_B63B_BCCF9C98B341__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScriptTypes.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CScriptTypes 

enum /*class*/ ActionType
{
	ACT_None = 0,
	ACT_DoMission = 11,
};

enum /*class*/ ParameterType
{
	PRM_None = 0,
	PRM_BuildingType = 16,
	PRM_Custom = -1,
};

enum class ExtraParameterType
{
	None = -1,
	ScanType,
	Counter,
};

class ScriptTemplate {
public:
	ScriptTemplate(const CString& desc, const CString& name, const std::vector<CString>& content) :
		ScriptTemplate(desc, name, parse(content))
	{
	}
	ScriptTemplate(const CString& desc, const CString& name, const CString& content) :
		ScriptTemplate(desc, name, parse(content))
	{
	}

	auto const& Desc() const { return m_desc; }
	auto const& Name() const { return m_name; }
	auto const& Content() const { return m_content; }

private:
	ScriptTemplate(const CString& desc, const CString& name, std::vector<CString>&& content) :
		m_desc(desc),
		m_name(name),
		m_content(std::move(content))
	{
	}

	static std::vector<CString> parse(const CString& content);
	static std::vector<CString> parse(const std::vector<CString>& content, size_t count, size_t offset = 0);
	static inline std::vector<CString> parse(const std::vector<CString>& content) {
		return parse(content, content.size() / 2);
	}

	CString m_desc;
	CString m_name;
	std::vector<CString> m_content;
};

class CScriptTypes : public CDialog
{

	struct CScriptTypeAction {
		CString Name{};
		CString Description{};
		int ParamTypeIndex{};//!< index linked to specific CScriptTypeParam, can be user defined
		bool Hide{};
		bool Editable{};
	};

	struct CScriptTypeParam {
		ParameterType Type{};//!< internal predefined paramter type
		CString Label{};//!< the string displayed for such parameter type
		CString ParseFromSection{};
		ExtraParameterType ExtraValueType{ ExtraParameterType::None };
	};


	DECLARE_DYNCREATE(CScriptTypes)
	// Konstruktion
public:
	using ActionDefinitionMap = std::map<int, CScriptTypeAction>;

	void UpdateDialog();
	CScriptTypes();
	~CScriptTypes();

	// Dialogfelddaten
		//{{AFX_DATA(CScriptTypes)
	enum { IDD = IDD_SCRIPTTYPES };
	//}}AFX_DATA
// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CScriptTypes)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL
	void UpdateStrings();

	void updateExtraParamComboBox(ExtraParameterType type, int value);
	const CScriptTypeAction& getActionData(int actionCbIndex) const;
	const CScriptTypeParam& getParamData(int paramIndex) const;
	ParameterType getParameterType(int actionCbIndex) const;
	ExtraParameterType getExtraParamType(ParameterType paramType) const;
	void updateExtraValue(const CScriptTypes::CScriptTypeParam& paramDef, CString* paramNumStr);
	void updateDefaultParams(const ParameterType type);
	void UpdateParams(int actionIndex, CString* paramNumStr = nullptr);

	// Implementierung
protected:
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog() override;
	BOOL PreTranslateMessage(MSG* pMsg) override;

	void ListBehaviours(CComboBox& cb);
	// Generierte Nachrichtenzuordnungsfunktionen
	afx_msg void OnSelchangeScripttype();
	afx_msg void OnSelchangeActionList();
	afx_msg void OnChangeName();
	afx_msg void OnEditchangeActionType();
	afx_msg void OnSelchangeActionType();
	afx_msg void OnEditchangeParam();
	afx_msg void OnSelchangeParam();
	afx_msg void OnAddaction();
	afx_msg void OnDeleteaction();
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnCbnSelchangeScriptExtra();
	afx_msg void OnBnClickedScriptCopy();
	afx_msg void OnBnClickedCopyaction();


	void reloadTemplates();
	void insertScriptType(const CString& name, const std::vector<CString>& items);
	int getExtraValue();
	CString getCurrentTypeID();
	void insertAction(int curSel, const CString& scriptTypeId, const CString& value);
	BOOL onMessageKeyDown(MSG* pMsg);

	CEdit	m_Description;
	CComboBox	m_Template;
	CComboBox	m_ActionType;
	CComboBox	m_ScriptType;
	CComboBox	m_Param;
	CComboBox	m_ParamExt;
	CListBox	m_Actions;
	CString	m_Name;

	ActionDefinitionMap m_actionDefinitions;
	std::map<int, CScriptTypeParam> m_paramDefinitions;
	std::vector<std::vector<CString>> m_extraValueDefinitions;
	std::vector<ScriptTemplate> m_scriptTemplates;


};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_SCRIPTTYPES_H__981EF700_951E_11D3_B63B_BCCF9C98B341__INCLUDED_
