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

// TeamTypes.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "TeamTypes.h"
#include "FinalSunDlg.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"
#include "inlines.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CTeamTypes 

IMPLEMENT_DYNCREATE(CTeamTypes, CDialog)

CTeamTypes::CTeamTypes() : CDialog(CTeamTypes::IDD)
{
	//{{AFX_DATA_INIT(CTeamTypes)
	m_Aggressive = FALSE;
	m_Annoyance = FALSE;
	m_AreTeamMembersRecruitable = FALSE;
	m_Autocreate = FALSE;
	m_AvoidThreats = FALSE;
	m_Droppod = FALSE;
	m_Full = FALSE;
	m_Group = _T("");
	m_GuardSlower = FALSE;
	m_House = _T("");
	m_IonImmune = FALSE;
	m_IsBaseDefense = FALSE;
	m_Loadable = FALSE;
	m_LooseRecruit = FALSE;
	m_Max = _T("");
	m_Name = _T("");
	m_OnlyTargetHouseEnemy = FALSE;
	m_OnTransOnly = FALSE;
	m_Prebuild = FALSE;
	m_Priority = _T("");
	m_Recruiter = FALSE;
	m_Reinforce = FALSE;
	m_Script = _T("");
	m_Suicide = FALSE;
	m_TaskForce = _T("");
	m_TechLevel = _T("");
	m_TransportReturnsOnUnload = FALSE;
	m_Waypoint = _T("");
	m_Whiner = FALSE;
	m_VeteranLevel = _T("");
	m_Tag = _T("");
	m_TransportWaypoint = _T("");
	//}}AFX_DATA_INIT
}

CTeamTypes::~CTeamTypes()
{
}

void CTeamTypes::translateUI()
{
	TranslateWindowCaption(*this, "TeamTypesCaption");

	TranslateDlgItem(*this, IDC_TEAMTYPE_COPY, "TeamTypesCopy");
	TranslateDlgItem(*this, IDC_TEAMTYPE_DESC, "TeamTypesDesc");
	TranslateDlgItem(*this, IDC_TEAMTYPE_T_TXT, "TeamTypesTemplate");
	TranslateDlgItem(*this, IDC_TEAMTYPE_TYPE, "TeamTypesType");
	TranslateDlgItem(*this, IDC_NEWTEAMTYPE, "TeamTypesNew");
	TranslateDlgItem(*this, IDC_DELETETEAMTYPE, "TeamTypesDelete");
	TranslateDlgItem(*this, IDC_TEAMTYPE_GBOX, "TeamTypesSelected");
	TranslateDlgItem(*this, IDC_TEAMTYPE_P_NAME, "TeamTypesName");
	TranslateDlgItem(*this, IDC_TEAMTYPE_P_GROUP, "TeamTypesGroup");
	TranslateDlgItem(*this, IDC_TEAMTYPE_P_VLEVEL, "TeamTypesVeteranLevel");
	TranslateDlgItem(*this, IDC_TEAMTYPE_P_WAYP, "TeamTypesWaypoint");
	TranslateDlgItem(*this, IDC_TEAMTYPE_P_HOUSE, "TeamTypesHouse");
	TranslateDlgItem(*this, IDC_LTRANSPORTWAYPOINT, "TeamTypesTransWaypoint");
	TranslateDlgItem(*this, IDC_TEAMTYPE_P_PRIORITY, "TeamTypesPriority");
	TranslateDlgItem(*this, IDC_TEAMTYPE_P_TECHLEVEL, "TeamTypesTechLevel");
	TranslateDlgItem(*this, IDC_TEAMTYPE_P_MAX, "TeamTypesMax");
	TranslateDlgItem(*this, IDC_MCD_L, "TeamTypesMCDecision");
	TranslateDlgItem(*this, IDC_TEAMTYPE_P_TAG, "TeamTypesTag");
	TranslateDlgItem(*this, IDC_TEAMTYPE_P_SCRIPT, "TeamTypesScript");
	TranslateDlgItem(*this, IDC_TEAMTYPE_P_TASKFORCE, "TeamTypesTaskforce");
	TranslateDlgItem(*this, IDC_LOADABLE, "TeamTypesLoadable");
	TranslateDlgItem(*this, IDC_FULL, "TeamTypesFull");
	TranslateDlgItem(*this, IDC_ANNOYANCE, "TeamTypesAnnoyance");
	TranslateDlgItem(*this, IDC_GUARDSLOWER, "TeamTypesGuardSlower");
	TranslateDlgItem(*this, IDC_RECRUITER, "TeamTypesRecruiter");
	TranslateDlgItem(*this, IDC_AUTOCREATE, "TeamTypesAutocreate");
	TranslateDlgItem(*this, IDC_PREBUILD, "TeamTypesPrebuild");
	TranslateDlgItem(*this, IDC_REINFORCE, "TeamTypesReinforce");
	TranslateDlgItem(*this, IDC_DROPPOD, "TeamTypesDropped");
	TranslateDlgItem(*this, IDC_WHINER, "TeamTypesWhiner");
	TranslateDlgItem(*this, IDC_LOOSERECRUIT, "TeamTypesLooseRecruit");
	TranslateDlgItem(*this, IDC_AGGRESSIVE, "TeamTypesAggressive");
	TranslateDlgItem(*this, IDC_SUICIDE, "TeamTypesSuicide");
	TranslateDlgItem(*this, IDC_ONTRANSONLY, "TeamTypesOnTransOnly");
	TranslateDlgItem(*this, IDC_AVOIDTHREATS, "TeamTypesAvoidThreats");
	TranslateDlgItem(*this, IDC_IONIMMUNE, "TeamTypesIonImmune");
	TranslateDlgItem(*this, IDC_TRANSPORTRETURNSONUNLOAD, "TeamTypesTransportsReturnOnUnload");
	TranslateDlgItem(*this, IDC_ARETEAMMEMBERSRECRUITABLE, "TeamTypesAreTeamMembersRecruitable");
	TranslateDlgItem(*this, IDC_ISBASEDEFENSE, "TeamTypesIsBaseDefense");
	TranslateDlgItem(*this, IDC_ONLYTARGETHOUSEENEMY, "TeamTypesOnlyTargetHouseEnemy");
}

inline void TeamTemplate::assignInteger(int& val, const CString& str)
{
	val = INIHelper::StringToInteger(str, val);
}
inline void TeamTemplate::assignBool(bool& val, const CString& str)
{
	val = INIHelper::StringToBool(str, val);
}

TeamTemplate::TeamTemplate(std::vector<CString>&& input) noexcept :
	m_displayName(std::move(input[0]))
{
	m_params.Name = std::move(input[1]);
	assignInteger(m_params.VeteranLevel, input[2]);
	assignInteger(m_params.Group, input[5]);
	assignInteger(m_params.Priority,input[3]);
	assignInteger(m_params.TechLevel ,input[6]);
	assignInteger(m_params.Max, input[4]);
#ifdef RA2_MODE
	assignInteger(m_params.MindControlDecision, input[7]);
#endif
	assignBool(m_params.Loadable, input[8]);
	assignBool(m_params.Full, input[9]);
	assignBool(m_params.Annoyance, input[10]);
	assignBool(m_params.GuardSlower, input[11]);
	assignBool(m_params.Recruiter, input[12]);
	assignBool(m_params.Droppod, input[13]);
	assignBool(m_params.Whiner, input[14]);
	assignBool(m_params.LooseRecruit, input[15]);
	assignBool(m_params.Aggressive, input[16]);
	assignBool(m_params.Suicide, input[17]);
	assignBool(m_params.Autocreate, input[18]);
	assignBool(m_params.Prebuild, input[19]);
	assignBool(m_params.Reinforce, input[20]);
	assignBool(m_params.OnTransOnly, input[21]);
	assignBool(m_params.AvoidThreats, input[22]);
	assignBool(m_params.AreTeamMembersRecruitable, input[23]);
	assignBool(m_params.TransportsReturnOnUnload, input[24]);
	assignBool(m_params.IsBaseDefense, input[25]);

	assignBool(m_params.OnlyTargetHouseEnemy, input[26]);
#if defined(RA2_MODE) && 0
	assignBool(m_params.UseTransportOrigin, input[27]);
#endif
	
}

void CTeamTypes::reloadTemplates()
{
	auto const& sec = g_data["TeamTemplates"];
	auto const count = sec.GetInteger("Counts");
	auto&& defName = sec.GetStringOr("DefaultName", "Default");

	m_templates.push_back({ std::move(defName), { .Name = "New teamtype" } });

	auto parseTemplate = [this, &sec](const CString& id) -> bool {
		auto&& elements = INIHelper::Split(sec.GetString(id));
		if (elements.size() != 27) {
			errstream << "[team type template] content fragments less than 27" << std::endl;
			return false;
		}
		m_templates.push_back(std::move(elements));
		return true;
	};

	auto offset = 0;
	// try from 0. If no 0, try from 1
	if (!parseTemplate("0")) {
		errstream << "[team type template] could not parse content from index 0" << std::endl;
		offset = 1;
	}

	CString idStr;
	for (auto idx = 0; idx < count; ++idx) {
		idStr.Format("%d", idx + offset);
		if (!parseTemplate(idStr)) {
			errstream << "[team type template] could not parse content from index " << idStr << std::endl;
		}
	}

	// load template into list
	ASSERT(m_templates.size() > 0);
	auto idx = 0;
	for (auto const& templ : m_templates) {
		m_Template.InsertString(idx++, templ.Desc());
	}
	m_Template.SetCurSel(0);
}

BOOL CTeamTypes::OnInitDialog()
{
	auto const ret = CDialog::OnInitDialog();
	initMCDecisionComboBox();
	translateUI();
	return ret;
}

void CTeamTypes::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTeamTypes)
	DDX_Control(pDX, IDC_TEAMTYPE_TEMPLATE, m_Template);
	DDX_Control(pDX, IDC_TEAMTYPES, m_TeamTypes);
	DDX_Check(pDX, IDC_AGGRESSIVE, m_Aggressive);
	DDX_Check(pDX, IDC_ANNOYANCE, m_Annoyance);
	DDX_Check(pDX, IDC_ARETEAMMEMBERSRECRUITABLE, m_AreTeamMembersRecruitable);
	DDX_Check(pDX, IDC_AUTOCREATE, m_Autocreate);
	DDX_Check(pDX, IDC_AVOIDTHREATS, m_AvoidThreats);
	DDX_Check(pDX, IDC_DROPPOD, m_Droppod);
	DDX_Check(pDX, IDC_FULL, m_Full);
	DDX_CBString(pDX, IDC_GROUP, m_Group);
	DDX_Check(pDX, IDC_GUARDSLOWER, m_GuardSlower);
	DDX_CBString(pDX, IDC_HOUSE, m_House);
	DDX_Check(pDX, IDC_IONIMMUNE, m_IonImmune);
	DDX_Check(pDX, IDC_ISBASEDEFENSE, m_IsBaseDefense);
	DDX_Check(pDX, IDC_LOADABLE, m_Loadable);
	DDX_Check(pDX, IDC_LOOSERECRUIT, m_LooseRecruit);
	DDX_Text(pDX, IDC_MAX, m_Max);
	DDX_Text(pDX, IDC_NAME, m_Name);
	DDX_Check(pDX, IDC_ONLYTARGETHOUSEENEMY, m_OnlyTargetHouseEnemy);
	DDX_Check(pDX, IDC_ONTRANSONLY, m_OnTransOnly);
	DDX_Check(pDX, IDC_PREBUILD, m_Prebuild);
	DDX_Text(pDX, IDC_PRIORITY, m_Priority);
	DDX_Check(pDX, IDC_RECRUITER, m_Recruiter);
	DDX_Check(pDX, IDC_REINFORCE, m_Reinforce);
	DDX_CBString(pDX, IDC_SCRIPT, m_Script);
	DDX_Check(pDX, IDC_SUICIDE, m_Suicide);
	DDX_CBString(pDX, IDC_TASKFORCE, m_TaskForce);
	DDX_CBString(pDX, IDC_TECHLEVEL, m_TechLevel);
	DDX_Check(pDX, IDC_TRANSPORTRETURNSONUNLOAD, m_TransportReturnsOnUnload);
	DDX_CBString(pDX, IDC_WAYPOINT, m_Waypoint);
	DDX_Check(pDX, IDC_WHINER, m_Whiner);
	DDX_CBString(pDX, IDC_VETERANLEVEL, m_VeteranLevel);
	DDX_CBString(pDX, IDC_TAG, m_Tag);
	DDX_CBString(pDX, IDC_TRANSPORTWAYPOINT, m_TransportWaypoint);
	DDX_Control(pDX, IDC_MINDCONTROLDECISION, m_MindControlDecision);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTeamTypes, CDialog)
	//{{AFX_MSG_MAP(CTeamTypes)
	ON_CBN_SELCHANGE(IDC_TEAMTYPES, OnSelchangeTeamtypes)
	ON_EN_KILLFOCUS(IDC_NAME, OnChangeName)
	ON_BN_CLICKED(IDC_DELETETEAMTYPE, OnDeleteteamtype)
	ON_EN_KILLFOCUS(IDC_PRIORITY, OnChangePriority)
	ON_EN_KILLFOCUS(IDC_MAX, OnChangeMax)
	ON_CBN_KILLFOCUS(IDC_VETERANLEVEL, OnKillfocusVeteranlevel)
	ON_CBN_KILLFOCUS(IDC_HOUSE, OnKillfocusHouse)
	ON_CBN_KILLFOCUS(IDC_TECHLEVEL, OnKillfocusTechlevel)
	ON_CBN_KILLFOCUS(IDC_GROUP, OnKillfocusGroup)
	ON_CBN_KILLFOCUS(IDC_WAYPOINT, OnKillfocusWaypoint)
	ON_CBN_KILLFOCUS(IDC_SCRIPT, OnKillfocusScript)
	ON_CBN_KILLFOCUS(IDC_TASKFORCE, OnKillfocusTaskforce)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_LOADABLE, OnLoadable)
	ON_BN_CLICKED(IDC_FULL, OnFull)
	ON_BN_CLICKED(IDC_ANNOYANCE, OnAnnoyance)
	ON_BN_CLICKED(IDC_GUARDSLOWER, OnGuardslower)
	ON_BN_CLICKED(IDC_RECRUITER, OnRecruiter)
	ON_BN_CLICKED(IDC_DROPPOD, OnDroppod)
	ON_BN_CLICKED(IDC_WHINER, OnWhiner)
	ON_BN_CLICKED(IDC_LOOSERECRUIT, OnLooserecruit)
	ON_BN_CLICKED(IDC_AGGRESSIVE, OnAggressive)
	ON_BN_CLICKED(IDC_SUICIDE, OnSuicide)
	ON_BN_CLICKED(IDC_AUTOCREATE, OnAutocreate)
	ON_BN_CLICKED(IDC_PREBUILD, OnPrebuild)
	ON_BN_CLICKED(IDC_ONTRANSONLY, OnOntransonly)
	ON_BN_CLICKED(IDC_REINFORCE, OnReinforce)
	ON_BN_CLICKED(IDC_AVOIDTHREATS, OnAvoidthreats)
	ON_BN_CLICKED(IDC_IONIMMUNE, OnIonimmune)
	ON_BN_CLICKED(IDC_TRANSPORTRETURNSONUNLOAD, OnTransportreturnsonunload)
	ON_BN_CLICKED(IDC_ARETEAMMEMBERSRECRUITABLE, OnAreteammembersrecruitable)
	ON_BN_CLICKED(IDC_ISBASEDEFENSE, OnIsbasedefense)
	ON_BN_CLICKED(IDC_ONLYTARGETHOUSEENEMY, OnOnlytargethouseenemy)
	ON_BN_CLICKED(IDC_NEWTEAMTYPE, OnNewteamtype)
	ON_CBN_EDITCHANGE(IDC_TAG, OnEditchangeTag)
	ON_CBN_KILLFOCUS(IDC_TAG, OnKillfocusTag)
	ON_CBN_EDITCHANGE(IDC_TRANSPORTWAYPOINT, OnEditchangeTransportwaypoint)
	ON_CBN_KILLFOCUS(IDC_TRANSPORTWAYPOINT, OnKillfocusTransportwaypoint)
	ON_CBN_EDITCHANGE(IDC_MINDCONTROLDECISION, OnEditchangeMindcontroldecision)
	ON_CBN_KILLFOCUS(IDC_MINDCONTROLDECISION, OnKillfocusMindcontroldecision)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_TEAMTYPE_COPY, &CTeamTypes::OnBnClickedTeamtypeCopy)
END_MESSAGE_MAP()

BOOL stob(const char* s)
{
	if (_stricmp(s, "no") == NULL) {
		return FALSE;
	}
	return TRUE;
}

CString btos(BOOL b)
{
	CString s = "no";
	if (b == TRUE) {
		s = "yes";
	}
	return s;
}

int letter2number(char let) {
	int reply = let - 'A';
	return reply;

}

char number2letter(int let) {
	int reply = let + 'A';
	return reply;

}

int GetWaypoint(const char* c)
{
	if (strlen(c) == 0) {
		return -1;
	}
	int i;
	int res = 0;
	for (i = 0; i < strlen(c); i++) {
		int addval = letter2number(c[i]);
		res += addval + (res + 1) * (i * 25) + i;
	}
	return res;
}

// Serialize waypoint, will be renamed later
CString GetWaypoint(int n)
{
	if (n == -1) {
		return "";
	}
	for (int i = -1; i < 26; i++) {
		for (int e = 0; e < 26; e++) {
			char c[50];
			CString p;
			if (i == -1) {
				c[0] = number2letter(e);
				c[1] = 0;
				if (GetWaypoint(c) == n) {
					return c;
				}
			} else {
				c[0] = number2letter(i);
				c[1] = number2letter(e);
				c[2] = 0;
				if (GetWaypoint(c) == n) {
					return c;
				}
			}

		}
	}
	return "";
}


/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTeamTypes 
void CTeamTypes::UpdateDialog()
{
	if (m_templates.empty()) {
		reloadTemplates();
	}

	if (!yuri_mode) {
		GetDlgItem(IDC_MINDCONTROLDECISION)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MCD_L)->ShowWindow(SW_HIDE);
	}

	CIniFile& ini = Map->GetIniFile();

	CComboBox& taskforces = *(CComboBox*)GetDlgItem(IDC_TASKFORCE);
	CComboBox& scripts = *(CComboBox*)GetDlgItem(IDC_SCRIPT);
	CComboBox& houses = *(CComboBox*)GetDlgItem(IDC_HOUSE);

	int sel = m_TeamTypes.GetCurSel();

	while (m_TeamTypes.DeleteString(0) != CB_ERR);
	while (taskforces.DeleteString(0) != CB_ERR);
	while (scripts.DeleteString(0) != CB_ERR);

	// MW 07/24/01: Clear:
	m_Aggressive = 0;
	m_Annoyance = 0;
	m_AreTeamMembersRecruitable = 0;
	m_Autocreate = 0;
	m_AvoidThreats = 0;
	m_Droppod = 0;
	m_Full = 0;
	m_Group = "";
	m_GuardSlower = 0;
	m_House = "";
	m_IonImmune = 0;
	m_IsBaseDefense = 0;
	m_Loadable = 0;
	m_LooseRecruit = 0;
	m_Max = "";
	m_Name = "";
	m_OnlyTargetHouseEnemy = 0;
	m_OnTransOnly = 0;
	m_Prebuild = 0;
	m_Priority = "";
	m_Recruiter = 0;
	m_Reinforce = 0;
	m_Script = "";
	m_Suicide = 0;
	m_Tag = "";
	m_TaskForce = "";
	m_TechLevel = "";
	m_TransportReturnsOnUnload = 0;
	m_TransportWaypoint = "";
	m_VeteranLevel = "";

	UpdateData(FALSE);

	int i;

	auto updateStringForCBox = [&ini](CComboBox& box, const CString& sec) {
		for (auto const& [seq, id] : ini[sec]) {

			CString str;
			str = id;
			str += " (";
			str += ini.GetString(id, "Name");
			str += ")";

			box.AddString(str);
		}
	};

	updateStringForCBox(m_TeamTypes, "TeamTypes");
	updateStringForCBox(taskforces, "TaskForces");
	updateStringForCBox(scripts, "ScriptTypes");

	// now list all tags, but not "None", because we want the correct language,
	// and ListTags uses (for compatibility) always the english one
	ListTags(*(CComboBox*)GetDlgItem(IDC_TAG), FALSE);
	(*(CComboBox*)GetDlgItem(IDC_TAG)).InsertString(0, GetLanguageStringACP("None"));


	CComboBox* house;
	house = (CComboBox*)GetDlgItem(IDC_HOUSE);

	/*while(house->DeleteString(0)!=CB_ERR);
	// houses:  rules.ini + map definitions!
	if(ini.sections.find("Houses")!=ini.sections.end())
	{
		if(ini.sections["Houses"].values.size()==0) goto wasnohouse;
		// we use the map definitions!
		for(i=0;i<ini.sections["Houses"].values.size();i++)
		{
			house->AddString(*ini.sections["Houses"].GetValue(i));
		}
	}
	else
	{
		wasnohouse:
		for(i=0;i<rules.sections["Houses"].values.size();i++)
		{
			house->AddString(*rules.sections["Houses"].GetValue(i));
		}
	}*/
	ListHouses(*house, FALSE, TRUE, TRUE);

	CComboBox* wayp;
	wayp = (CComboBox*)GetDlgItem(IDC_WAYPOINT);

	while (wayp->DeleteString(0) != CB_ERR);
	// houses:  rules.ini + map definitions!
	for (auto const& [num, _] : ini["Waypoints"]) {
		wayp->AddString(num);
	}

	wayp = (CComboBox*)GetDlgItem(IDC_TRANSPORTWAYPOINT);

	while (wayp->DeleteString(0) != CB_ERR);
	// houses:  rules.ini + map definitions!
	wayp->SetItemData(wayp->InsertString(0, TranslateStringACP("None")), 0);

	for (auto const& [num, _] : ini["Waypoints"]) {
		wayp->SetItemData(wayp->AddString(num), 1);
	}

#ifdef TS_MODE
	wayp->ShowWindow(SW_HIDE);
#endif

	m_TeamTypes.SetCurSel(0);
	if (sel >= 0) {
		m_TeamTypes.SetCurSel(sel);
	}
	OnSelchangeTeamtypes();


}

void CTeamTypes::initMCDecisionComboBox()
{
	ComboBoxHelper::Clear(m_MindControlDecision);
	m_MindControlDecision.AddString(TranslateStringACP("0 - <Don't care>"));
	m_MindControlDecision.AddString(TranslateStringACP("1 - Add To Team"));
	m_MindControlDecision.AddString(TranslateStringACP("2 - Put in Grinder"));
	m_MindControlDecision.AddString(TranslateStringACP("3 - Put in Bio Reactor"));
	m_MindControlDecision.AddString(TranslateStringACP("4 - Go to Hunt"));
	m_MindControlDecision.AddString(TranslateStringACP("5 - Do Nothing"));
}

int CTeamTypes::getMCDecision()
{
	return std::max(m_MindControlDecision.GetCurSel(), 0);
}

void CTeamTypes::setMCDection(const int decision)
{
	m_MindControlDecision.SetCurSel(decision);
}

void CTeamTypes::OnSelchangeTeamtypes()
{
	CIniFile& ini = Map->GetIniFile();

	if (m_TeamTypes.GetCurSel() < 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	auto const& sec = ini[str];

	m_Aggressive = sec.GetBool("Aggressive");
	m_Annoyance = sec.GetBool("Annoyance");
	m_AreTeamMembersRecruitable = sec.GetBool("AreTeamMembersRecruitable");
	m_Autocreate = sec.GetBool("Autocreate");
	m_AvoidThreats = sec.GetBool("AvoidThreats");
	m_Droppod = sec.GetBool("Droppod");
	m_Full = sec.GetBool("Full");
	m_Group = sec.GetString("Group");
	m_GuardSlower = sec.GetBool("GuardSlower");
	m_House = TranslateHouse(sec.GetString("House"), TRUE);
	m_IonImmune = sec.GetBool("IonImmune");
	m_IsBaseDefense = sec.GetBool("IsBaseDefense");
	m_Loadable = sec.GetBool("Loadable");
	m_LooseRecruit = sec.GetBool("LooseRecruit");
	m_Max = sec.GetString("Max");
	m_Name = sec.GetString("Name");
	m_OnlyTargetHouseEnemy = sec.GetBool("OnlyTargetHouseEnemy");
	m_OnTransOnly = sec.GetBool("OnTransOnly");
	m_Prebuild = sec.GetBool("Prebuild");
	m_Priority = sec.GetString("Priority");
	m_Recruiter = sec.GetBool("Recruiter");
	m_Reinforce = sec.GetBool("Reinforce");
	m_Script = (sec.GetString("Script"));
	m_Script += " (" + ini.GetString(sec.GetString("Script"), "Name") + ")";
	m_Suicide = sec.GetBool("Suicide");

	auto const& tagId = sec.GetString("Tag");
	if (!tagId.IsEmpty()) {
		m_Tag = tagId;
		auto const& tagDef = ini.GetString("Tags", tagId);
		if (!tagDef.IsEmpty()) {
			CString tag = m_Tag;
			m_Tag += " ";
			m_Tag += GetParam(tagDef, 1);
		}
	} else {
		m_Tag = GetLanguageStringACP("None");
	}
	m_TaskForce = sec["TaskForce"];
	auto const& taskForceName = ini.GetString(m_TaskForce, "Name");
	if (!taskForceName.IsEmpty()) {
		m_TaskForce += " (" + taskForceName + ")";
	}
	m_TechLevel = sec.GetString("TechLevel");
	m_TransportReturnsOnUnload = sec.GetBool("TransportsReturnOnUnload");
	m_VeteranLevel = sec.GetString("VeteranLevel");

	if (yuri_mode) {
		setMCDection(sec.GetInteger("MindControlDecision"));
	}


	int w = GetWaypoint(sec["Waypoint"]);
	char c[50];
	itoa(w, c, 10);
	if (w != -1)
		m_Waypoint = c;
	else
		m_Waypoint = "";

#ifdef RA2_MODE
	if (sec.GetBool("UseTransportOrigin")) {
		int w = GetWaypoint(sec["TransportWaypoint"]);
		char c[50];
		itoa(w, c, 10);
		if (w != -1)
			m_TransportWaypoint = c;
		else
			m_TransportWaypoint = "";

	} else {
		m_TransportWaypoint = TranslateStringACP("None");
	}
#endif

	m_Whiner = sec.GetBool("Whiner");

	UpdateData(FALSE);
}

void CTeamTypes::OnChangeName()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) return;

	CEdit& n = *(CEdit*)GetDlgItem(IDC_NAME);
	DWORD pos = n.GetSel();

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetString(str, "Name", m_Name);

	UpdateDialog();

	n.SetSel(pos);
}

void CTeamTypes::OnDeleteteamtype()
{
	CIniFile& ini = Map->GetIniFile();

	if (m_TeamTypes.GetCurSel() != -1) {
		int res = MessageBox("Are you sure that you want to delete the selected team-type? If you delete it, don´t forget to delete any reference to the team-type.", "Delete team-type", MB_YESNO);
		if (res == IDNO) {
			return;
		}

		CString str;
		str = GetText(&m_TeamTypes);
		TruncSpace(str);

		CIniFile& ini = Map->GetIniFile();

		if (auto pSec = ini.TryGetSection("TeamTypes")) {
			pSec->RemoveValue(str);
		}
		ini.DeleteSection(str);
	}
	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs(TRUE);
	//UpdateDialog();
}

void CTeamTypes::OnEditchangeVeteranlevel()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetString(str, "VeteranLevel", m_VeteranLevel);
}

void CTeamTypes::OnEditchangeHouse()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetString(str, "House", TranslateHouse(m_House));
}

void CTeamTypes::OnChangePriority()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetString(str, "Priority", m_Priority);
}

void CTeamTypes::OnChangeMax()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetString(str, "Max", m_Max);
}

void CTeamTypes::OnEditchangeTechlevel()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetString(str, "TechLevel", m_TechLevel);
}

void CTeamTypes::OnEditchangeGroup()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetString(str, "Group", m_Group);
}

void CTeamTypes::OnEditchangeWaypoint()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);
	auto sec = ini.TryGetSection(str);
	ASSERT(sec != nullptr);
	if (sec == nullptr) {
		return;
	}

	if (strlen(m_Waypoint) == 0) {
		sec->SetString("Waypoint", "");
	} else {
		sec->SetString("Waypoint", GetWaypoint(atoi(m_Waypoint)));
	}
}

void CTeamTypes::OnEditchangeScript()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	CString tmp = m_Script;
	TruncSpace(tmp);
	ini.SetString(str, "Script", tmp);
}

void CTeamTypes::OnEditchangeTaskforce()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	CString tmp = m_TaskForce;
	TruncSpace(tmp);
	ini.SetString(str, "TaskForce", tmp);
}



void CTeamTypes::OnKillfocusVeteranlevel()
{
	OnEditchangeVeteranlevel();
}

void CTeamTypes::OnKillfocusHouse()
{
	OnEditchangeHouse();
}

void CTeamTypes::OnKillfocusTechlevel()
{
	OnEditchangeTechlevel();
}

void CTeamTypes::OnKillfocusGroup()
{
	OnEditchangeGroup();
}

void CTeamTypes::OnKillfocusWaypoint()
{
	OnEditchangeWaypoint();
}

void CTeamTypes::OnKillfocusScript()
{
	OnEditchangeScript();
}

void CTeamTypes::OnKillfocusTaskforce()
{
	OnEditchangeTaskforce();
}

void CTeamTypes::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	if (!bShow) {
		OnKillfocusGroup();
		OnKillfocusHouse();
		OnKillfocusScript();
		OnKillfocusTaskforce();
		OnKillfocusTechlevel();
		OnKillfocusVeteranlevel();
		OnKillfocusWaypoint();
		OnKillfocusTag();
#ifdef RA2_MODE
		OnKillfocusTransportwaypoint();
		if (yuri_mode) {
			OnKillfocusMindcontroldecision();
		}
#endif
	}
}



void CTeamTypes::OnLoadable()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "Loadable", m_Loadable);
}

void CTeamTypes::OnFull()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "Full", m_Full);
}

void CTeamTypes::OnAnnoyance()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "Annoyance", m_Annoyance);
}

void CTeamTypes::OnGuardslower()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "GuardSlower", m_GuardSlower);
}

void CTeamTypes::OnRecruiter()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "Recruiter", m_Recruiter);
}

void CTeamTypes::OnDroppod()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "Droppod", m_Droppod);
}

void CTeamTypes::OnWhiner()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "Whiner", m_Whiner);
}

void CTeamTypes::OnLooserecruit()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "LooseRecruit", m_LooseRecruit);
}

void CTeamTypes::OnAggressive()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "Aggressive", m_Aggressive);
}

void CTeamTypes::OnSuicide()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "Suicide", m_Suicide);
}

void CTeamTypes::OnAutocreate()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "Autocreate", m_Autocreate);
}

void CTeamTypes::OnPrebuild()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "Prebuild", m_Prebuild);
}

void CTeamTypes::OnOntransonly()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "OnTransOnly", m_OnTransOnly);
}

void CTeamTypes::OnReinforce()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "Reinforce", m_Reinforce);
}

void CTeamTypes::OnAvoidthreats()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "AvoidThreats", m_AvoidThreats);
}

void CTeamTypes::OnIonimmune()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "IonImmune", m_IonImmune);
}

void CTeamTypes::OnTransportreturnsonunload()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "TransportsReturnOnUnload", m_TransportReturnsOnUnload);
}

void CTeamTypes::OnAreteammembersrecruitable()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "AreTeamMembersRecruitable", m_AreTeamMembersRecruitable);
}

void CTeamTypes::OnIsbasedefense()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "IsBaseDefense", m_IsBaseDefense);
}

void CTeamTypes::OnOnlytargethouseenemy()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);

	ini.SetBool(str, "OnlyTargetHouseEnemy", m_OnlyTargetHouseEnemy);
}

CString GetFree(const char* section);

void CTeamTypes::addTeamtype(const TeamTypeParams& params)
{
	CIniFile& ini = Map->GetIniFile();

	CString id = GetFreeID();
	CString p;
	p = GetFree("TeamTypes");

	// TODO: change default value
	ini.SetString("TeamTypes", p, id);
	CIniFileSection& s = ini.AddSection(id);

	s.SetString("Name", params.Name);
	s.SetInteger("VeteranLevel", params.VeteranLevel);
	s.SetBool("Loadable", params.Loadable);
	s.SetBool("Full", params.Full);
	s.SetBool("Annoyance", params.Annoyance);
	s.SetBool("GuardSlower", params.GuardSlower);
	s.SetBool("Recruiter", params.Recruiter);
	s.SetBool("Autocreate", params.Autocreate);
	s.SetBool("Prebuild", params.Prebuild);
	s.SetBool("Reinforce", params.Reinforce);
	s.SetBool("Droppod", params.Droppod);
	s.SetBool("Whiner", params.Whiner);
	s.SetBool("LooseRecruit", params.LooseRecruit);
	s.SetBool("Aggressive", params.Aggressive);
	s.SetBool("Suicide", params.Suicide);
	s.SetInteger("Priority", params.Priority);
	s.SetInteger("Max", params.Max);
	s.SetInteger("TechLevel", params.TechLevel);
	s.SetInteger("Group", params.Group);
	s.SetString("Waypoint", params.Waypoint);
	s.SetInteger("TransportWaypoint", params.TransportWaypoint);
	s.SetBool("OnTransOnly", params.OnTransOnly);
	s.SetBool("AvoidThreats", params.AvoidThreats);
	s.SetBool("IonImmune", params.IonImmune);
	s.SetBool("TransportsReturnOnUnload", params.TransportsReturnOnUnload);
	s.SetBool("AreTeamMembersRecruitable", params.AreTeamMembersRecruitable);
	s.SetBool("IsBaseDefense", params.IsBaseDefense);
	s.SetBool("OnlyTargetHouseEnemy", params.OnlyTargetHouseEnemy);

	bool anythingCopied = false;
	if (!params.House.IsEmpty()) {
		s.SetString("House", params.House);
		anythingCopied = true;
	}
	if (!params.TaskForce.IsEmpty()) {
		s.SetString("TaskForce", params.TaskForce);
		anythingCopied = true;
	}
	if (!params.Script.IsEmpty()) {
		s.SetString("Script", params.Script);
		anythingCopied = true;
	}
	if (!params.Tag.IsEmpty()) {
		s.SetString("Tag", params.Tag);
		anythingCopied = true;
	}

#ifdef RA2_MODE
	s.SetBool("UseTransportOrigin", params.UseTransportOrigin);
	if (yuri_mode) {
		s.SetInteger("MindControlDecision", params.MindControlDecision);
	}
#endif

	//UpdateDialog();
	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs(TRUE);

	int i;
	for (i = 0; i < m_TeamTypes.GetCount(); i++) {
		CString k;
		m_TeamTypes.GetLBText(i, k);
		TruncSpace(k);
		if (strcmp(k, id) == NULL) {
			m_TeamTypes.SetCurSel(i);
			OnSelchangeTeamtypes();
		}
	}

	if (!anythingCopied) {
		CComboBox& houses = *(CComboBox*)GetDlgItem(IDC_HOUSE);
		houses.SetCurSel(0);
		OnEditchangeHouse();
		CComboBox& waypoints = *(CComboBox*)GetDlgItem(IDC_WAYPOINT);
		waypoints.SetCurSel(0);
		CComboBox& script = *(CComboBox*)GetDlgItem(IDC_SCRIPT);
		script.SetCurSel(0);
		CComboBox& taskforce = *(CComboBox*)GetDlgItem(IDC_TASKFORCE);
		taskforce.SetCurSel(0);

		OnKillfocusHouse();
		OnKillfocusWaypoint();
		OnKillfocusScript();
		OnKillfocusTaskforce();
	}
}

void CTeamTypes::OnNewteamtype()
{
	//errstream << "Add Script" << std::endl;
	int curTemplateIndex = m_Template.GetCurSel();
	ASSERT(curTemplateIndex >= 0);
	auto const& curTemplate = m_templates[curTemplateIndex];
	//errstream << "Now using Script Template: " << curTemplate.Name() << std::endl;
	addTeamtype(curTemplate.Params());
}

void CTeamTypes::OnBnClickedTeamtypeCopy()
{
	auto const& ini = Map->GetIniFile();
	auto curTeamType = GetText(&m_TeamTypes);
	TruncSpace(curTeamType);

	if (curTeamType.IsEmpty()) {
		return;
	}

	auto const& sec = ini.GetSection(curTeamType);
	addTeamtype({
	.Name = sec.GetString("Name") + " Clone",
	.House = sec.GetString("House"),
	.TaskForce = sec.GetString("TaskForce"),
	.Script = sec.GetString("Script"),
	.Tag = sec.GetString("Tag"),
	.VeteranLevel = sec.GetInteger("VeteranLevel"),
	.Group = sec.GetInteger("Group"),
	.Priority = sec.GetInteger("Priority"),
	.TechLevel = sec.GetInteger("TechLevel"),
	.Max = sec.GetInteger("Max"),
	.Waypoint = sec.GetString("Waypoint"),
	.TransportWaypoint = sec.GetInteger("TransportWaypoint"),
#ifdef RA2_MODE
	.MindControlDecision = sec.GetInteger("MindControlDecision"),
#endif
	.Aggressive = sec.GetBool("Aggressive"),
	.Annoyance = sec.GetBool("Annoyance"),
	.AreTeamMembersRecruitable = sec.GetBool("AreTeamMembersRecruitable"),
	.Autocreate = sec.GetBool("Autocreate"),
	.AvoidThreats = sec.GetBool("AvoidThreats"),
	.Droppod = sec.GetBool("Droppod"),
	.Full = sec.GetBool("Full"),
	.GuardSlower = sec.GetBool("GuardSlower"),
	.IonImmune = sec.GetBool("IonImmune"),
	.IsBaseDefense = sec.GetBool("IsBaseDefense"),
	.Loadable = sec.GetBool("Loadable"),
	.LooseRecruit = sec.GetBool("LooseRecruit"),
	.OnlyTargetHouseEnemy = sec.GetBool("OnlyTargetHouseEnemy"),
	.OnTransOnly = sec.GetBool("OnTransOnly"),
	.Prebuild = sec.GetBool("Prebuild"),
	.Recruiter = sec.GetBool("Recruiter"),
	.Reinforce = sec.GetBool("Reinforce"),
	.Suicide = sec.GetBool("Suicide"),
	.TransportsReturnOnUnload = sec.GetBool("TransportsReturnOnUnload"),
	.Whiner = sec.GetBool("Whiner"),
#ifdef RA2_MODE
	.UseTransportOrigin = sec.GetBool("UseTransportOrigin"),
#endif
		});
}

void CTeamTypes::OnEditchangeTag()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);

	TruncSpace(str);
	auto sec = ini.TryGetSection(str);
	ASSERT(sec != nullptr);
	TruncSpace(m_Tag);

	if (m_Tag == GetLanguageStringACP("None") || m_Tag.IsEmpty()) {
		sec->RemoveByKey("Tag");
	} else {
		sec->SetString("Tag", m_Tag);
	}
}

void CTeamTypes::OnKillfocusTag()
{
	OnEditchangeTag();
}

void CTeamTypes::OnEditchangeTransportwaypoint()
{
#ifndef RA2_MODE
	return;
#endif

	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);
	auto sec = ini.TryGetSection(str);
	ASSERT(sec != nullptr);
	if (strlen(m_TransportWaypoint) == 0 || isSame(m_TransportWaypoint, TranslateStringACP("None"))) {
		sec->RemoveByKey("TransportWaypoint");
		sec->SetBool("UseTransportOrigin", false);
		return;
	}
	sec->SetString("TransportWaypoint", GetWaypoint(atoi(m_TransportWaypoint)));
	sec->SetBool("UseTransportOrigin", true);
}

void CTeamTypes::OnKillfocusTransportwaypoint()
{
	OnEditchangeTransportwaypoint();
}

void CTeamTypes::OnEditchangeMindcontroldecision()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData(TRUE);
	if (m_TeamTypes.GetCount() == 0) {
		return;
	}

	CString str;
	str = GetText(&m_TeamTypes);
	TruncSpace(str);
	auto sec = ini.TryGetSection(str);
	ASSERT(sec != nullptr);
	sec->SetInteger("MindControlDecision", getMCDecision());
}

void CTeamTypes::OnKillfocusMindcontroldecision()
{
	OnEditchangeMindcontroldecision();
}

