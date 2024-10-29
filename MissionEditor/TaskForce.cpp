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

// TaskForce.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "TaskForce.h"
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

CString GetFree(const char* section);

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CTaskForce 

IMPLEMENT_DYNCREATE(CTaskForce, CDialog)

CTaskForce::CTaskForce() : CDialog(CTaskForce::IDD)
{
	//{{AFX_DATA_INIT(CTaskForce)
	m_Group = _T("");
	m_Name = _T("");
	m_NumberOfUnits = 0;
	//}}AFX_DATA_INIT
}

CTaskForce::~CTaskForce()
{
}

BOOL CTaskForce::OnInitDialog()
{
	auto const ret = CDialog::OnInitDialog();
	translateUI();
	return ret;
}

void CTaskForce::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTaskForce)
	DDX_Control(pDX, IDC_UNITTYPE, m_UnitType);
	DDX_Control(pDX, IDC_UNITS, m_Units);
	DDX_Control(pDX, IDC_TASKFORCES, m_TaskForces);
	DDX_Text(pDX, IDC_GROUP, m_Group);
	DDX_Text(pDX, IDC_NAME, m_Name);
	DDX_Text(pDX, IDC_NUMBERUNITS, m_NumberOfUnits);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTaskForce, CDialog)
	//{{AFX_MSG_MAP(CTaskForce)
	ON_CBN_KILLFOCUS(IDC_TASKFORCES, OnEditchangeTaskforces)
	ON_CBN_SELCHANGE(IDC_TASKFORCES, OnSelchangeTaskforces)
	ON_LBN_SELCHANGE(IDC_UNITS, OnSelchangeUnits)
	ON_BN_CLICKED(IDC_DELETEUNIT, OnDeleteunit)
	ON_EN_KILLFOCUS(IDC_NUMBERUNITS, OnChangeNumberunits)
	ON_EN_KILLFOCUS(IDC_NAME, OnChangeName)
	ON_CBN_KILLFOCUS(IDC_UNITTYPE, OnEditchangeUnittype)
	ON_CBN_SELCHANGE(IDC_UNITTYPE, OnSelchangeUnittype)
	ON_BN_CLICKED(IDC_ADDUNIT, OnAddunit)
	ON_BN_CLICKED(IDC_DELETETASKFORCE, OnDeletetaskforce)
	ON_BN_CLICKED(IDC_ADDTASKFORCE, OnAddtaskforce)
	ON_EN_KILLFOCUS(IDC_GROUP, OnChangeGroup)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_COPYTASKFORCE, &CTaskForce::OnBnClickedCopytaskforce)
	ON_BN_CLICKED(IDC_COPYUNIT, &CTaskForce::OnBnClickedCopyunit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTaskForce 

CString CTaskForce::getCurrrentID()
{
	CString tf;
	tf = GetText(&m_TaskForces);
	TruncSpace(tf);
	return tf;
}

void CTaskForce::translateUI()
{
	TranslateWindowCaption(*this, "TaskforcesCaption");

	TranslateDlgItem(*this, IDC_TASKFORCE_T_TYPE, "TaskforcesType");
	TranslateDlgItem(*this, IDC_ADDTASKFORCE, "TaskforcesAdd");
	TranslateDlgItem(*this, IDC_COPYTASKFORCE, "TaskforcesCopy");
	TranslateDlgItem(*this, IDC_DELETETASKFORCE, "TaskforcesDelete");
	TranslateDlgItem(*this, IDC_TASKFORCE_G_SELECTED, "TaskforcesSelected");
	TranslateDlgItem(*this, IDC_TASKFORCE_T_NAME, "TaskforcesName");
	TranslateDlgItem(*this, IDC_TASKFORCE_T_GROUP, "TaskforcesGroup");
	TranslateDlgItem(*this, IDC_TASKFORCE_T_MEMBERS, "TaskforcesMembers");
	
	TranslateDlgItem(*this, IDC_ADDUNIT, "TaskforcesAddUnit");
	TranslateDlgItem(*this, IDC_COPYUNIT, "TaskforcesCopyUnit");
	TranslateDlgItem(*this, IDC_DELETEUNIT, "TaskforcesDeleteUnit");
	TranslateDlgItem(*this, IDC_TASKFORCE_T_U_NUM, "TaskforcesUnitNumber");
	TranslateDlgItem(*this, IDC_TASKFORCE_T_U_TYPE, "TaskforcesUnitType");
}

void CTaskForce::UpdateDialog()
{
	CIniFile& ini = Map->GetIniFile();

	int sel = m_TaskForces.GetCurSel();
	int sel2 = m_Units.GetCurSel();

	while (m_TaskForces.DeleteString(0) != CB_ERR);
	while (m_Units.DeleteString(0) != LB_ERR);
	while (m_UnitType.DeleteString(0) != CB_ERR);

	// MW 07/24/01: Clear
	m_Group = "";
	m_Name = "";
	m_NumberOfUnits = 0;
	UpdateData(FALSE);


	auto const& sec = ini["TaskForces"];
	for (auto const& [seq, id] : sec) {
		CString s;
		s = id;
		s += " (";
		s += ini[id].GetString("Name");
		s += ")";
		m_TaskForces.AddString(s);
	}

	auto addIntoUnitTypeByIni = [this](const CIniFile& ini, const CString& section) {
		for (auto const& [seq, type] : ini[section]) {
			CString s;
			s = type;
			s += " (";
			/*if(ini.sections.find((char*)(LPCTSTR)type)!=ini.sections.end() && ini.sections[(char*)(LPCTSTR)type].values.find("Name")!=ini.sections[(char*)(LPCTSTR)type].values.end())
				s+=ini.sections[(char*)(LPCTSTR)type].values["Name"];
			else
				s+=rules.sections[(char*)(LPCTSTR)type].values["Name"];
			*/
			s += Map->GetUnitName((char*)(LPCTSTR)type);

			s += ")";
			m_UnitType.AddString(s);
		}
	};

	auto addIntoUnitType = [&addIntoUnitTypeByIni, &ini](const CString& section) {
		addIntoUnitTypeByIni(rules, section);
		addIntoUnitTypeByIni(ini, section);
	};
	addIntoUnitType("InfantryTypes");
	addIntoUnitType("VehicleTypes");
	addIntoUnitType("AircraftTypes");

	if (sel < 0) {
		if (m_TaskForces.SetCurSel(0) != CB_ERR)
			OnSelchangeTaskforces();
	} else {
		if (m_TaskForces.SetCurSel(sel) != CB_ERR)
			OnSelchangeTaskforces();
	}





	if (!(sel2 < 0)) {
		if (m_Units.SetCurSel(sel2) != LB_ERR)
			OnSelchangeUnits();
	}


}

void CTaskForce::OnEditchangeTaskforces()
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

}

void CTaskForce::OnSelchangeTaskforces()
{
	CIniFile& ini = Map->GetIniFile();

	auto const tf = getCurrrentID();
	auto const& sec = ini[tf];
	m_Name = sec.GetString("Name");
	m_Group = sec.GetString("Group");

	int i;
	while (m_Units.DeleteString(0) != LB_ERR);

	if (sec.Size()) {
		CString p;
		for (i = 0; i < sec.Size() - 2; i++) {
			p.Format("%d", i);
			auto const& data = sec.GetString(p);
			CString type = GetParam(data, 1);
			CString s = GetParam(data, 0);
			s += " ";
			s += Map->GetUnitName(type);
			m_Units.SetItemData(m_Units.AddString(s), i);
		}
	}

	UpdateData(FALSE);

	if (m_Units.SetCurSel(0) != LB_ERR) {
		OnSelchangeUnits();
	}
}

void CTaskForce::OnSelchangeUnits()
{
	CIniFile& ini = Map->GetIniFile();

	int sel = m_Units.GetCurSel();
	int u = m_Units.GetItemData(sel);
	auto const tf = getCurrrentID();
	CString indexStr;
	indexStr.Format("%d", u);

	auto const& sec = ini[tf];
	auto const& data = sec.GetString(indexStr);
	CString countStr = GetParam(data, 0);

	CString type = GetParam(data, 1);
	CString unitID = Map->GetUnitName(type);

	m_UnitType.SetWindowText((type + (CString)" (" + unitID + (CString)")"));
	m_NumberOfUnits = atoi(countStr);

	UpdateData(FALSE);
}

void CTaskForce::OnDeleteunit()
{
	CIniFile& ini = Map->GetIniFile();

	int sel = m_Units.GetCurSel();
	if (sel < 0) {
		return;
	}
	int u = m_Units.GetItemData(sel);
	auto const tf = getCurrrentID();

	auto sec = ini.TryGetSection(tf);
	ASSERT(sec != nullptr);
	CString numberStrToDelete;
	numberStrToDelete.Format("%d", u);

	if (!sec) {
		return;
	}

	if (sec->Size() < 4) {
		sec->RemoveByKey(numberStrToDelete);
		m_Units.DeleteString(sel);
		m_UnitType.SetWindowText("");
		m_NumberOfUnits = atoi("0");
		UpdateDialog();
		return;
	}

	int lastpos = sec->Size() - 3;

	CString lastPosStr;
	lastPosStr.Format("%d", lastpos);

	sec->SetString(numberStrToDelete, sec->GetString(lastPosStr));
	sec->RemoveByKey(lastPosStr);
	m_Units.DeleteString(sel);


	UpdateDialog();
}

void CTaskForce::OnChangeNumberunits()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData();

	int sel = m_Units.GetCurSel();
	if (sel < 0) {
		return;
	}
	int u = m_Units.GetItemData(sel);
	auto const tf = getCurrrentID();
	auto sec = ini.TryGetSection(tf);

	CString k, n;
	k.Format("%d", u);
	n.Format("%d", m_NumberOfUnits);

	auto const& data = sec->GetString(k);
	CString c = GetParam(data, 1);
	sec->SetString(k, n + (CString)"," + c);
	UpdateDialog();
}

void CTaskForce::OnChangeName()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData();

	CEdit& n = *(CEdit*)GetDlgItem(IDC_NAME);
	DWORD pos = n.GetSel();

	if (m_TaskForces.GetCurSel() < 0) {
		return;
	}
	auto const tf = getCurrrentID();
	ini.SetString(tf, "Name", m_Name);

	UpdateDialog();
	n.SetSel(pos);
}

void CTaskForce::OnEditchangeUnittype()
{
	CIniFile& ini = Map->GetIniFile();

	CString type = GetText(&m_UnitType);
	TruncSpace(type);

	if (type.IsEmpty()) {
		return;
	}

	int sel = m_Units.GetCurSel();
	if (sel < 0) {
		return;
	}
	int u = m_Units.GetItemData(sel);
	auto const tf = getCurrrentID();
	auto sec = ini.TryGetSection(tf);
	ASSERT(sec != nullptr);

	if (sec == nullptr) {
		return;
	}

	CString k;
	k.Format("%d", u);

	CString count = GetParam(sec->GetString(k), 0);


	sec->SetString(k, count + "," + type);

	CString ut;
	m_UnitType.GetWindowText(ut);
	UpdateDialog();
	m_UnitType.SetWindowText(ut);


}

void CTaskForce::OnSelchangeUnittype()
{
	CIniFile& ini = Map->GetIniFile();

	int sel = m_Units.GetCurSel();
	if (sel < 0) {
		return;
	}
	int u = m_Units.GetItemData(sel);
	auto const tf = getCurrrentID();
	auto sec = ini.TryGetSection(tf);
	ASSERT(sec != nullptr);

	if (sec == nullptr) {
		return;
	}

	CString k;
	k.Format("%d", u);

	CString count = GetParam(sec->GetString(k), 0);
	CString type = GetText(&m_UnitType);

	TruncSpace(type);

	sec->SetString(k, count + "," + type);

	UpdateDialog();
	//m_UnitType.SetWindowText("H");
}

void CTaskForce::OnAddunit()
{
	CIniFile& ini = Map->GetIniFile();

	if (m_TaskForces.GetCurSel() < 0) {
		return;
	}
	auto const tf = getCurrrentID();
	auto sec = ini.TryGetSection(tf);
	ASSERT(sec != nullptr);

	char k[50];
	int c = m_Units.GetCount();
	if (c == LB_ERR) {
		c = 0;
	}
	itoa(c, k, 10);

	sec->SetString(k, "1" + (CString)"," + rules["InfantryTypes"].Nth(0).second);

	UpdateDialog();
}

void CTaskForce::OnBnClickedCopyunit()
{
	auto& ini = Map->GetIniFile();

	auto const tf = getCurrrentID();
	if (tf.IsEmpty()) {
		return;
	}
	auto const selected = m_Units.GetCurSel();

	if (selected < 0) {
		return;
	}
	auto const curCount = m_Units.GetCount();
	auto const& content = ini.GetString(tf, INIHelper::ToString(selected));
	ini.SetString(tf, INIHelper::ToString(curCount), content);
	UpdateDialog();
	m_Units.SetCurSel(curCount);
}

void CTaskForce::OnDeletetaskforce()
{
	CIniFile& ini = Map->GetIniFile();
	if (m_TaskForces.GetCurSel() < 0) {
		return;
	}
	auto const tf = getCurrrentID();
	int res;
	auto const title = TranslateStringACP("Delete task force");
	auto const content = TranslateStringACP("TaskforceDeleteMsgDesc");
	res = MessageBox(content, title, MB_YESNO);
	if (res == IDNO) {
		return;
	}

	ini.RemoveValue("TaskForces", tf);
	ini.DeleteSection(tf);
	while (m_Units.DeleteString(0) != LB_ERR);
	//UpdateDialog();
	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs(TRUE);
}

void CTaskForce::addTaskforce(CString&& name, int group, std::vector<CString>&& members)
{
	CIniFile& ini = Map->GetIniFile();

	CString ID = GetFreeID();
	CString tf = GetFree("TaskForces");
	ini.SetString("TaskForces", tf, ID);

	auto& sec = ini.AddSection(ID);
	sec.SetString("Name", std::move(name));
	sec.SetInteger("Group", group);

	int idx = 0;
	CString idxStr;
	for (auto&& member : members) {
		idxStr.Format("%d", idx++);
		sec.SetString(idxStr, std::move(member));
	}

	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs(TRUE);

	CString tf2;
	for (auto i = 0; i < m_TaskForces.GetCount(); i++) {
		m_TaskForces.GetLBText(i, tf2);
		TruncSpace(tf2);
		if (strcmp(ID, tf2) == NULL) {
			m_TaskForces.SetCurSel(i);
			OnSelchangeTaskforces(); // MW bugfix
			break;
		}
	}
}


void CTaskForce::OnAddtaskforce()
{
	addTaskforce("New task force", -1, {});
}

void CTaskForce::OnBnClickedCopytaskforce()
{
	auto const id = getCurrrentID();
	if (id.IsEmpty()) {
		return;
	}
	std::vector<CString> content;
	auto const& ini = Map->GetIniFile();
	auto const& sec = ini[id];
	for (auto const& [key, val] : sec) {
		if (IsNumeric(key)) {
			content.push_back(val);
		}
	}
	addTaskforce(
		sec.GetString("Name") + " Clone",
		sec.GetInteger("Group"),
		std::move(content)
	);
}


void CTaskForce::OnChangeGroup()
{
	CIniFile& ini = Map->GetIniFile();

	UpdateData();

	if (m_TaskForces.GetCurSel() < 0) {
		return;
	}
	auto const tf = getCurrrentID();
	ini.SetString(tf, "Group", m_Group);

	UpdateDialog();

}

BOOL CTaskForce::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN) {
		switch (pMsg->wParam) {
			case VK_RETURN:
			{
				auto pEdit = this->m_UnitType.GetWindow(GW_CHILD);
				if (pMsg->hwnd == pEdit->m_hWnd) {
					this->OnEditchangeUnittype();
				}

				switch (::GetDlgCtrlID(pMsg->hwnd)) {
					case IDC_NAME:
						this->OnChangeName();
						break;
					case IDC_NUMBERUNITS:
						this->OnChangeNumberunits();
						break;
					case IDC_GROUP:
						this->OnChangeGroup();
						break;
					default:
						break;
				}
			}
			//do not exit dialog when enter key pressed
			return TRUE;
			default:
				break;
		}
	}

	return this->CDialog::PreTranslateMessage(pMsg);
}