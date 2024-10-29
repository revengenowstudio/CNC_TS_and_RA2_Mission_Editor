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

// WaypointID.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "WaypointID.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CWaypointID 


CWaypointID::CWaypointID(CWnd* pParent /*=NULL*/)
	: CDialog(CWaypointID::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWaypointID)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}


void CWaypointID::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWaypointID)
	DDX_Control(pDX, IDC_ID, m_id);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWaypointID, CDialog)
	//{{AFX_MSG_MAP(CWaypointID)
	ON_BN_CLICKED(IDC_FREE, OnFree)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CWaypointID 

void CWaypointID::OnFree()
{
	CIniFile& ini = Map->GetIniFile();

	CString freen;

	for (auto i = 0; ; i++) {
		char d[50];
		itoa(i, d, 10);
		if (ini.GetString("Waypoints", d).IsEmpty()) {
			freen = d;
			break;
		}
	}

	m_id.SetWindowText(freen);
}

void CWaypointID::OnOK()
{
	CString h;
	m_id.GetWindowText(h);
	if (h.GetLength() < 1) {
		return;
	}

	m_value = atoi(h);

	CDialog::OnOK();
}

BOOL CWaypointID::OnInitDialog()
{
	CDialog::OnInitDialog();

	translateUI();

	OnFree();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CWaypointID::translateUI()
{
	TranslateWindowCaption(*this, "CreateWaypointDlgCaption");

	TranslateDlgItem(*this, IDC_DESC, "CreateWaypointDlgDesc");
	TranslateDlgItem(*this, IDC_LID, "CreateWaypointDlgID");
	TranslateDlgItem(*this, IDC_FREE, "CreateWaypointDlgFree");
	TranslateDlgItem(*this, IDOK, "CreateWaypointDlgOk");
	TranslateDlgItem(*this, IDCANCEL, "CreateWaypointDlgCancel");
}