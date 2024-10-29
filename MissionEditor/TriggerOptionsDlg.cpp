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

// TriggerOptionsDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "TriggerOptionsDlg.h"
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
// Dialogfeld CTriggerOptionsDlg 


CTriggerOptionsDlg::CTriggerOptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTriggerOptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTriggerOptionsDlg)
	//}}AFX_DATA_INIT
}


void CTriggerOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTriggerOptionsDlg)
	DDX_Control(pDX, IDC_MEDIUM, m_Medium);
	DDX_Control(pDX, IDC_HARD, m_Hard);
	DDX_Control(pDX, IDC_EASY, m_Easy);
	DDX_Control(pDX, IDC_DISABLED, m_Disabled);
	DDX_Control(pDX, IDC_TRIGGERTYPE, m_TriggerType);
	DDX_Control(pDX, IDC_NAME, m_Name);
	DDX_Control(pDX, IDC_HOUSE, m_House);
	DDX_Control(pDX, IDC_ATTACHEDTRIGGER, m_AttachedTrigger);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTriggerOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(CTriggerOptionsDlg)
	ON_CBN_EDITCHANGE(IDC_HOUSE, OnEditchangeHouse)
	ON_CBN_EDITCHANGE(IDC_ATTACHEDTRIGGER, OnEditchangeAttachedtrigger)
	ON_EN_KILLFOCUS(IDC_NAME, OnChangeName)
	ON_WM_KILLFOCUS()
	ON_CBN_EDITCHANGE(IDC_TRIGGERTYPE, OnEditchangeTriggertype)
	ON_BN_CLICKED(IDC_DISABLED, OnDisabled)
	ON_BN_CLICKED(IDC_EASY, OnEasy)
	ON_BN_CLICKED(IDC_MEDIUM, OnMedium)
	ON_BN_CLICKED(IDC_HARD, OnHard)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTriggerOptionsDlg 

void CTriggerOptionsDlg::UpdateDialog()
{
	// MW 07/20/01

	CIniFile& ini = Map->GetIniFile();
	if (m_currentTrigger.IsEmpty()) {
		return;
	}

	ListHouses(m_House, FALSE, TRUE, FALSE);
	ListTriggers(m_AttachedTrigger);
	m_AttachedTrigger.InsertString(0, "<none>");

	auto triggerCopy = ini["Triggers"][m_currentTrigger];
	if (RepairTrigger(triggerCopy)) {
		ini.SetString("Triggers", m_currentTrigger, triggerCopy);
	}

	m_Name.SetWindowText(GetParam(triggerCopy, 2));
	m_House.SetWindowText(TranslateHouse(GetParam(triggerCopy, 0), TRUE));
	CString attachedTrigger = GetParam(triggerCopy, 1);
	m_AttachedTrigger.SetWindowText(attachedTrigger);

	m_Disabled.SetCheck((atoi(GetParam(triggerCopy, 3))));
	m_Easy.SetCheck((atoi(GetParam(triggerCopy, 4))));
	m_Medium.SetCheck((atoi(GetParam(triggerCopy, 5))));
	m_Hard.SetCheck((atoi(GetParam(triggerCopy, 6))));

	for (auto i = 0; i < m_AttachedTrigger.GetCount(); i++) {
		CString tmp;
		m_AttachedTrigger.GetLBText(i, tmp);
		TruncSpace(tmp);
		if (tmp == attachedTrigger) {
			m_AttachedTrigger.SetCurSel(i);
		}
	}

	for (auto const& [type, def] : ini["Tags"]) {
		CString attTrigg = GetParam(def, 2);
		if (attTrigg == m_currentTrigger) {
			m_TriggerType.SetWindowText(GetParam(def, 0));
			break;
		}
	}
}

void CTriggerOptionsDlg::OnChangeName()
{
	if (m_currentTrigger.GetLength() == 0) {
		return;
	}

	CIniFile& ini = Map->GetIniFile();

	if (!ini["Triggers"].Exists(m_currentTrigger) || m_currentTrigger.IsEmpty()) {
		return;
	}

	CString newName;
	m_Name.GetWindowText(newName);

	if (newName.GetLength() == 0) {
		newName = " ";
	}

	if (newName.Find(",", 0) >= 0) {//newName.SetAt(newName.Find(",",0), 0);
		newName = newName.Left(newName.Find(",", 0));

		m_Name.SetWindowText(newName);
	}



	ini.SetString("Triggers", m_currentTrigger, SetParam(ini["Triggers"][m_currentTrigger], 2, newName));

	int i;
	int p = 0;
	for (auto const& [type, def] : ini["Tags"]) {
		CString attTrigg = GetParam(def, 2);
		if (attTrigg == m_currentTrigger) {
			p++;
			char c[50];
			itoa(p, c, 10);
			CString newVal = newName + " ";
			newVal += c;
			ini.SetString("Tags", type, SetParam(ini.GetString("Tags", type), 1, newVal));
		}
	}
	//MessageBox(ini.sections["Triggers"].values[m_currentTrigger],newName);
	auto triggerCopy = ini["Triggers"][m_currentTrigger];
	if (RepairTrigger(triggerCopy)) {
		ini.SetString("Triggers", m_currentTrigger, triggerCopy);
	}
	OnKillfocusName();
}

void CTriggerOptionsDlg::OnEditchangeHouse()
{
	CIniFile& ini = Map->GetIniFile();

	if (!ini["Triggers"].Exists(m_currentTrigger) || m_currentTrigger.IsEmpty()) {
		return;
	}

	CString newHouse;
	m_House.GetWindowText(newHouse);



	newHouse = TranslateHouse(newHouse);

	newHouse.TrimLeft();
	TruncSpace(newHouse);
	if (newHouse.Find(",", 0) >= 0) {
		newHouse.SetAt(newHouse.Find(",", 0), 0);
	}

	ini.SetString("Triggers", m_currentTrigger, SetParam(ini["Triggers"][m_currentTrigger], 0, newHouse));

	auto triggerCopy = ini["Triggers"][m_currentTrigger];
	if (RepairTrigger(triggerCopy)) {
		ini.SetString("Triggers", m_currentTrigger, triggerCopy);
	}
}

void CTriggerOptionsDlg::OnEditchangeAttachedtrigger()
{
	CIniFile& ini = Map->GetIniFile();

	if (!ini["Triggers"].Exists(m_currentTrigger) || m_currentTrigger.IsEmpty()) {
		return;
	}

	CString newTrigger;
	m_AttachedTrigger.GetWindowText(newTrigger);
	newTrigger.TrimLeft();
	TruncSpace(newTrigger);

	if (newTrigger.Find(",", 0) >= 0) {
		newTrigger.SetAt(newTrigger.Find(",", 0), 0);
	}


	ini.SetString("Triggers", m_currentTrigger, SetParam(ini["Triggers"][m_currentTrigger], 1, newTrigger));



}

void CTriggerOptionsDlg::OnKillfocusName()
{
	((CTriggerEditorDlg*)(this->GetOwner()->GetOwner()))->UpdateDialog();
}

void CTriggerOptionsDlg::OnKillFocus(CWnd* pNewWnd)
{
	CDialog::OnKillFocus(pNewWnd);

	((CTriggerEditorDlg*)(this->GetOwner()->GetOwner()))->UpdateDialog();
}

void CTriggerOptionsDlg::OnEditchangeTriggertype()
{
	CIniFile& ini = Map->GetIniFile();

	if (!ini["Triggers"].Exists(m_currentTrigger) || m_currentTrigger.IsEmpty()) {
		return;
	}


	CString newType;
	m_TriggerType.GetWindowText(newType);
	TruncSpace(newType);

	int i;
	for (auto const& [type, def] : ini["Tags"]) {
		CString attTrigg = GetParam(def, 2);
		if (attTrigg == m_currentTrigger) {
			ini.SetString("Tags", type, SetParam(ini["Tags"][type], 0, newType));
		}
	}

	auto trigger = ini["Triggers"][m_currentTrigger];
	if (RepairTrigger(trigger)) {
		ini.SetString("Triggers", m_currentTrigger, trigger);
	}
}

BOOL CTriggerOptionsDlg::PreTranslateMessage(MSG* pMsg)
{
	{
		m_tooltip.RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CTriggerOptionsDlg::OnInitDialog()
{
	auto const ret = CDialog::OnInitDialog();
	{
		TranslateDlgItem(*this, IDC_TRIGGER_OPTION_TYPE_STR, "TriggerOptionType");
		TranslateDlgItem(*this, IDC_TRIGGER_OPTION_NAME, "TriggerOptionName");
		TranslateDlgItem(*this, IDC_TRIGGER_OPTION_HOUSE, "TriggerOptionHouse");
		TranslateDlgItem(*this, IDC_TRIGGER_OPTION_ATTACHED_TRIGGER, "TriggerOptionAttachedTrigger");
		TranslateDlgItem(*this, IDC_TRIGGER_OPTION_TRIGGER_DIS_TIP, "TriggerOptionDisableTip");
		TranslateDlgItem(*this, IDC_DISABLED, "TriggerOptionDisabled");
		TranslateDlgItem(*this, IDC_EASY, "TriggerOptionEasy");
		TranslateDlgItem(*this, IDC_MEDIUM, "TriggerOptionMedium");
		TranslateDlgItem(*this, IDC_HARD, "TriggerOptionHard");

		m_TriggerType.InsertString(0, TranslateStringACP("0 - Standard"));
		m_TriggerType.InsertString(1, TranslateStringACP("1 - All Attached"));
		m_TriggerType.InsertString(2, TranslateStringACP("2 - Repeating"));

		m_tooltip.Create(this);
		m_tooltip.Activate(TRUE);
		m_tooltip.AddTool(GetDlgItem(IDC_HOUSE), GetLanguageStringACP("TT_TriggerHouse"));
	}
	return ret;
}

void CTriggerOptionsDlg::OnDisabled()
{
	CIniFile& ini = Map->GetIniFile();

	if (!ini["Triggers"].Exists(m_currentTrigger) || m_currentTrigger.IsEmpty()) {
		return;
	}

	BOOL bDisabled = FALSE;
	if (m_Disabled.GetCheck() == 0) {
		bDisabled = FALSE;
	} else {
		bDisabled = TRUE;
	}

	auto const param = bDisabled ? "1" : "0";
	ini.SetString("Triggers", m_currentTrigger, SetParam(ini["Triggers"][m_currentTrigger], 3, param));
}

void CTriggerOptionsDlg::OnEasy()
{
	CIniFile& ini = Map->GetIniFile();

	if (!ini["Triggers"].Exists(m_currentTrigger) || m_currentTrigger.IsEmpty()) {
		return;
	}

	BOOL bEasy = FALSE;
	if (m_Easy.GetCheck() == 0) {
		bEasy = FALSE;
	} else {
		bEasy = TRUE;
	}

	auto const param = bEasy ? "1" : "0";
	ini.SetString("Triggers", m_currentTrigger, SetParam(ini["Triggers"][m_currentTrigger], 4, param));
}

void CTriggerOptionsDlg::OnMedium()
{
	CIniFile& ini = Map->GetIniFile();

	if (!ini["Triggers"].Exists(m_currentTrigger) || m_currentTrigger.IsEmpty()) {
		return;
	}

	BOOL bMedium = FALSE;
	if (m_Medium.GetCheck() == 0) {
		bMedium = FALSE;
	} else {
		bMedium = TRUE;
	}

	auto const param = bMedium ? "1" : "0";
	ini.SetString("Triggers", m_currentTrigger, SetParam(ini["Triggers"][m_currentTrigger], 5, param));
}

void CTriggerOptionsDlg::OnHard()
{
	CIniFile& ini = Map->GetIniFile();

	if (!ini["Triggers"].Exists(m_currentTrigger) || m_currentTrigger.IsEmpty()) {
		return;
	}

	BOOL bHard = FALSE;
	if (m_Hard.GetCheck() == 0) {
		bHard = FALSE;
	} else {
		bHard = TRUE;
	}

	auto const param = bHard ? "1" : "0";
	ini.SetString("Triggers", m_currentTrigger, SetParam(ini["Triggers"][m_currentTrigger], 6, param));
}

//MW 07/20/01
void CTriggerOptionsDlg::Clear()
{

}
