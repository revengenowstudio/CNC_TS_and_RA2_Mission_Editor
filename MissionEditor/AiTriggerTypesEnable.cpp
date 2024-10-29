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

// AiTriggerTypesEnable.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "AiTriggerTypesEnable.h"
#include "mapdata.h"
#include "variables.h"
#include "inlines.h"
#include "aitriggeradddlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CAiTriggerTypesEnable 

IMPLEMENT_DYNCREATE(CAiTriggerTypesEnable, CDialog)

CAiTriggerTypesEnable::CAiTriggerTypesEnable() : CDialog(CAiTriggerTypesEnable::IDD)
{
	//{{AFX_DATA_INIT(CAiTriggerTypesEnable)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}

CAiTriggerTypesEnable::~CAiTriggerTypesEnable()
{
}

void CAiTriggerTypesEnable::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAiTriggerTypesEnable)
	DDX_Control(pDX, IDC_AITRIGGERTYPE, m_AITriggerType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAiTriggerTypesEnable, CDialog)
	//{{AFX_MSG_MAP(CAiTriggerTypesEnable)
	ON_BN_CLICKED(IDC_ENABLEALL, OnEnableall)
	ON_CBN_SELCHANGE(IDC_AITRIGGERTYPE, OnSelchangeAitriggertype)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CAiTriggerTypesEnable 
BOOL CAiTriggerTypesEnable::OnInitDialog()
{
	auto const ret = CDialog::OnInitDialog();
	translateUI();
	return ret;
}

void CAiTriggerTypesEnable::translateUI()
{
	TranslateWindowCaption(*this, "AITriggerEnableCaption");

	TranslateDlgItem(*this, IDC_AI_TRG_ENB_DESC, "AITriggerEnableDesc");
	TranslateDlgItem(*this, IDC_ENABLEALL, "AITriggerEnableAll");
	TranslateDlgItem(*this, IDC_AI_TRG_ENB_TYPE, "AITriggerEnableType");
	TranslateDlgItem(*this, IDC_ADD, "AITriggerEnableAdd");
	TranslateDlgItem(*this, IDC_DELETE, "AITriggerEnableDelete");
}

void CAiTriggerTypesEnable::UpdateDialog()
{
	int sel = m_AITriggerType.GetCurSel();
	if (sel < 0) {
		sel = 0;
	}

	while (m_AITriggerType.DeleteString(0) != CB_ERR);

	CIniFile& ini = Map->GetIniFile();

	int i;
	for (auto const [aitrigger, val] : ini["AITriggerTypesEnable"]) {
		CString str = aitrigger;
		str += " (";
		// parse from map definition first
		auto const& aiDef = ini.GetString("AITriggerTypes", aitrigger);
		if (!aiDef.IsEmpty()) {
			str += GetParam(aiDef, 0);
			str += " -> ";
			str += val;
		} else {
			// if not found, try retrieving from global AI ini
			auto const& aiDef = ai.GetString("AITriggerTypes", aitrigger);
			if (!aiDef.IsEmpty()) {
				// standard ai trigger
				str += GetParam(aiDef, 0);
				str += " -> ";
				str += val;
			}
		}

		str += ")";

		m_AITriggerType.AddString(str);
	}

	if (m_AITriggerType.SetCurSel(sel) == CB_ERR) {
		m_AITriggerType.SetCurSel(0);
	}

	OnSelchangeAitriggertype();

}

void CAiTriggerTypesEnable::OnEnableall()
{
	// enable all standard ai triggers
	CIniFile& ini = Map->GetIniFile();
	for (auto const& [id, def] : ai["AITriggerTypes"]) {
		ini.SetBool("AITriggerTypesEnable", id, true);
	}

	UpdateDialog();
}

void CAiTriggerTypesEnable::OnSelchangeAitriggertype()
{
	int sel = m_AITriggerType.GetCurSel();
	if (sel < 0) {
		return;
	}

}

void CAiTriggerTypesEnable::OnDelete()
{
	int sel = m_AITriggerType.GetCurSel();
	if (sel < 0) {
		return;
	}
	CString aitrigger;
	m_AITriggerType.GetLBText(sel, aitrigger);
	TruncSpace(aitrigger);

	CIniFile& ini = Map->GetIniFile();

	ini.RemoveValueByKey("AITriggerTypesEnable", aitrigger);
	UpdateDialog();
}

void CAiTriggerTypesEnable::OnAdd()
{
	//CString newTriggerId=InputBox("Please enter the ID of the AITriggerType (for a list of all AITriggerType-IDs use the All-Section)","Enable AITriggerType");
	CAITriggerAddDlg dlg;
	if (dlg.DoModal() == IDCANCEL) {
		return;
	}

	CString newTriggerId = dlg.m_AITrigger;
	TruncSpace(newTriggerId);
	if (newTriggerId.GetLength() == 0) {
		return;
	}

	CIniFile& ini = Map->GetIniFile();

	ini.SetBool("AITriggerTypesEnable", newTriggerId, true);
	UpdateDialog();
}
