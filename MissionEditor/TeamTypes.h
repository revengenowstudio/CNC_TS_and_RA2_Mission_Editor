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

//{{AFX_INCLUDES()

//}}AFX_INCLUDES
#if !defined(AFX_TEAMTYPES_H__EE440E20_929B_11D3_B63B_F2ECCFA7A541__INCLUDED_)
#define AFX_TEAMTYPES_H__EE440E20_929B_11D3_B63B_F2ECCFA7A541__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TeamTypes.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CTeamTypes 

struct TeamTypeParams
{
	CString Name;
	CString House;
	CString TaskForce;
	CString Script;
	CString Tag;
	int VeteranLevel{ 1 };
	int Group{ -1 };
	int Priority{ 5 };
	int TechLevel{};
	int Max{ 1 };
	CString Waypoint{};
	int TransportWaypoint{};
#ifdef RA2_MODE
	int MindControlDecision{};
#endif
	bool Aggressive{};
	bool Annoyance{};
	bool AreTeamMembersRecruitable{};
	bool Autocreate{};
	bool AvoidThreats{};
	bool Droppod{};
	bool Full{};
	bool GuardSlower{};
	bool IonImmune{};
	bool IsBaseDefense{};
	bool Loadable{};
	bool LooseRecruit{};
	bool OnlyTargetHouseEnemy{};
	bool OnTransOnly{};
	bool Prebuild{};
	bool Recruiter{};
	bool Reinforce{};
	bool Suicide{};
	bool TransportsReturnOnUnload{};
	bool Whiner{};
#ifdef RA2_MODE
	bool UseTransportOrigin{};
#endif
};

class TeamTemplate
{
public:
	TeamTemplate(std::vector<CString>&& input) noexcept;
	// used for default template
	TeamTemplate(CString&& name, TeamTypeParams&& params) noexcept : 
		m_displayName(std::move(name)),
		m_params(std::move(params))
	{}

	const CString& Desc() const { return m_displayName; }
	const TeamTypeParams& Params() const { return m_params; }

private:
	static void assignInteger(int& val, const CString& str);
	static void assignBool(bool& val, const CString& str);

	CString m_displayName;
	TeamTypeParams m_params;
};

class CTeamTypes : public CDialog
{
	DECLARE_DYNCREATE(CTeamTypes)

	// Konstruktion
public:
	void UpdateDialog();
	CTeamTypes();
	~CTeamTypes();

	// Dialogfelddaten
	enum { IDD = IDD_TEAMTYPES };

// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CTeamTypes)
protected:
	virtual BOOL OnInitDialog() override;
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL
	void addTeamtype(const TeamTypeParams& params);
	void translateUI();
	void initMCDecisionComboBox();
	void reloadTemplates();
	void setMCDection(const int decision);
	int getMCDecision();

	// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CTeamTypes)
	afx_msg void OnSelchangeTeamtypes();
	afx_msg void OnChangeName();
	afx_msg void OnDeleteteamtype();
	afx_msg void OnEditchangeVeteranlevel();
	afx_msg void OnEditchangeHouse();
	afx_msg void OnChangePriority();
	afx_msg void OnChangeMax();
	afx_msg void OnEditchangeTechlevel();
	afx_msg void OnEditchangeGroup();
	afx_msg void OnEditchangeWaypoint();
	afx_msg void OnEditchangeScript();
	afx_msg void OnEditchangeTaskforce();
	afx_msg void OnKillfocusVeteranlevel();
	afx_msg void OnKillfocusHouse();
	afx_msg void OnKillfocusTechlevel();
	afx_msg void OnKillfocusGroup();
	afx_msg void OnKillfocusWaypoint();
	afx_msg void OnKillfocusScript();
	afx_msg void OnKillfocusTaskforce();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnLoadable();
	afx_msg void OnFull();
	afx_msg void OnAnnoyance();
	afx_msg void OnGuardslower();
	afx_msg void OnRecruiter();
	afx_msg void OnDroppod();
	afx_msg void OnWhiner();
	afx_msg void OnLooserecruit();
	afx_msg void OnAggressive();
	afx_msg void OnSuicide();
	afx_msg void OnAutocreate();
	afx_msg void OnPrebuild();
	afx_msg void OnOntransonly();
	afx_msg void OnReinforce();
	afx_msg void OnAvoidthreats();
	afx_msg void OnIonimmune();
	afx_msg void OnTransportreturnsonunload();
	afx_msg void OnAreteammembersrecruitable();
	afx_msg void OnIsbasedefense();
	afx_msg void OnOnlytargethouseenemy();
	afx_msg void OnNewteamtype();
	afx_msg void OnEditchangeTag();
	afx_msg void OnKillfocusTag();
	afx_msg void OnEditchangeTransportwaypoint();
	afx_msg void OnKillfocusTransportwaypoint();
	afx_msg void OnEditchangeMindcontroldecision();
	afx_msg void OnKillfocusMindcontroldecision();
	afx_msg void OnBnClickedTeamtypeCopy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CComboBox	m_Template;
	CComboBox	m_TeamTypes;
	BOOL	m_Aggressive;
	BOOL	m_Annoyance;
	BOOL	m_AreTeamMembersRecruitable;
	BOOL	m_Autocreate;
	BOOL	m_AvoidThreats;
	BOOL	m_Droppod;
	BOOL	m_Full;
	CString	m_Group;
	BOOL	m_GuardSlower;
	CString	m_House;
	BOOL	m_IonImmune;
	BOOL	m_IsBaseDefense;
	BOOL	m_Loadable;
	BOOL	m_LooseRecruit;
	CString	m_Max;
	CString	m_Name;
	BOOL	m_OnlyTargetHouseEnemy;
	BOOL	m_OnTransOnly;
	BOOL	m_Prebuild;
	CString	m_Priority;
	BOOL	m_Recruiter;
	BOOL	m_Reinforce;
	CString	m_Script;
	BOOL	m_Suicide;
	CString	m_TaskForce;
	CString	m_TechLevel;
	BOOL	m_TransportReturnsOnUnload;
	CString	m_Waypoint;
	BOOL	m_Whiner;
	CString	m_VeteranLevel;
	CString	m_Tag;
	CString	m_TransportWaypoint;
	CComboBox	m_MindControlDecision;

	std::vector<TeamTemplate> m_templates;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_TEAMTYPES_H__EE440E20_929B_11D3_B63B_F2ECCFA7A541__INCLUDED_
