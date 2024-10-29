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

// All1.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "All.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

extern CFinalSunApp theApp;
#include "ImportIni.h"

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CAll 


CAll::CAll(CWnd* pParent /*=NULL*/)
	: CDialog(CAll::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAll)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}


void CAll::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAll)
	DDX_Control(pDX, IDC_VALUE, m_Value);
	DDX_Control(pDX, IDC_KEYS, m_Keys);
	DDX_Control(pDX, IDC_INISECTION, m_IniSection);
	DDX_Control(pDX, IDC_DELETESECTION, m_DeleteSection);
	DDX_Control(pDX, IDC_DELETEKEY, m_DeleteKey);
	DDX_Control(pDX, IDC_ADDSECTION, m_AddSection);
	DDX_Control(pDX, IDC_ADDKEY, m_AddKey);
	DDX_Control(pDX, IDC_SECTIONS, m_Sections);
	//}}AFX_DATA_MAP
}

BOOL CAll::OnInitDialog()
{
	auto const ret = CDialog::OnInitDialog();

	translateUI();

	return ret;
}

void CAll::translateUI()
{
	TranslateWindowCaption(*this, "IniEditorCaption");

	TranslateDlgItem(*this, IDC_INI_EDITOR_DESC, "IniEditorDesc");
	TranslateDlgItem(*this, IDC_INI_EDITOR_SECTIONS, "IniEditorSections");
	TranslateDlgItem(*this, IDC_INI_EDITOR_CONTENT, "IniEditorSectionContent");
	TranslateDlgItem(*this, IDC_INI_EDITOR_KEYS, "IniEditorSectionKeys");
	TranslateDlgItem(*this, IDC_INI_EDITOR_VAL, "IniEditorSectionValue");
	
	TranslateDlgItem(*this, IDC_ADDSECTION, "IniEditorAdd");
	TranslateDlgItem(*this, IDC_DELETESECTION, "IniEditorDelete");
	TranslateDlgItem(*this, IDC_INISECTION, "IniEditorInsert");

	TranslateDlgItem(*this, IDC_ADDKEY, "IniEditorAddKey");
	TranslateDlgItem(*this, IDC_DELETEKEY, "IniEditorDeleteKey");
	
}

BEGIN_MESSAGE_MAP(CAll, CDialog)
	//{{AFX_MSG_MAP(CAll)
	ON_CBN_SELCHANGE(IDC_SECTIONS, OnSelchangeSections)
	ON_EN_CHANGE(IDC_VALUE, OnChangeValue)
	ON_LBN_SELCHANGE(IDC_KEYS, OnSelchangeKeys)
	ON_EN_UPDATE(IDC_VALUE, OnUpdateValue)
	ON_BN_CLICKED(IDC_ADDSECTION, OnAddsection)
	ON_BN_CLICKED(IDC_DELETESECTION, OnDeletesection)
	ON_BN_CLICKED(IDC_DELETEKEY, OnDeletekey)
	ON_BN_CLICKED(IDC_ADDKEY, OnAddkey)
	ON_BN_CLICKED(IDC_INISECTION, OnInisection)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CAll 

void CAll::UpdateDialog()
{
	//m_Sections.Clear();

	while (m_Sections.DeleteString(0) != -1);
	while (m_Keys.DeleteString(0) != -1);
	CIniFile& ini = Map->GetIniFile();


	m_Value.SetWindowText("");

	int i;
	for (auto const& [name, sec] : ini) {
		if (!Map->IsMapSection(name)) {
			m_Sections.InsertString(-1, name);
		}
	}

	m_Sections.SetCurSel(1);
	OnSelchangeSections();
}

void CAll::OnSelchangeSections()
{
	while (m_Keys.DeleteString(0) != CB_ERR);
	CIniFile& ini = Map->GetIniFile();

	CString cuSection;
	m_Sections.GetWindowText(cuSection);

	if (cuSection.GetLength()) {
		int i;
		m_Keys.SetRedraw(FALSE);
		SetCursor(LoadCursor(0, IDC_WAIT));
		for (auto const& [key, val] : ini[cuSection]) {
			m_Keys.InsertString(-1, key);

		}
		SetCursor(m_hArrowCursor);
		m_Keys.SetRedraw(TRUE);
		m_Keys.RedrawWindow();
	}
}


void CAll::OnChangeValue()
{
	CIniFile& ini = Map->GetIniFile();

	CString t;
	m_Value.GetWindowText(t);

	CString cuSection;
	m_Sections.GetWindowText(cuSection);


	CString cuKey;
	if (m_Keys.GetCurSel() >= 0) {
		m_Keys.GetText(m_Keys.GetCurSel(), cuKey);
	}

	ini.SetString(cuSection, cuKey, t);

}

void CAll::OnSelchangeKeys()
{
	CIniFile& ini = Map->GetIniFile();

	CString cuSection;
	m_Sections.GetWindowText(cuSection);

	CString cuKey;
	m_Keys.GetText(m_Keys.GetCurSel(), cuKey);

	m_Value.SetWindowText(ini.GetString(cuSection, cuKey));
}

void CAll::OnUpdateValue()
{

}

void CAll::OnAddsection()
{
	CString name = InputBox("Please set the name of the new section (the section may already exist)", "Insert Section");

	CIniFile& ini = Map->GetIniFile();

	ini.AddSection(name);

	UpdateDialog();
}

void CAll::OnDeletesection()
{
	CIniFile& ini = Map->GetIniFile();

	const int cusection = m_Sections.GetCurSel();

	if (cusection == -1) {
		auto const msg = TranslateStringACP("IniEditorItemUnselected");
		auto const cap = TranslateStringACP("Error");
		MessageBox(msg, cap);
		return;
	}

	CString str;
	m_Sections.GetLBText(cusection, str);

	auto const msgBefore = TranslateStringACP("IniEditorSelectionDeletePrefix");
	auto const msgAfter = TranslateStringACP("IniEditorSelectionDeleteSUffix");
	auto const cap = TranslateStringACP("IniEditorDeleteSelectionCap");

	if (MessageBox(msgBefore + str + msgAfter, cap, MB_YESNO) == IDNO) {
		return;
	}

	ini.DeleteSection(str);

	UpdateDialog();
}

void CAll::OnDeletekey()
{
	CIniFile& ini = Map->GetIniFile();

	int cukey;
	if (m_Sections.GetCurSel() < 0) return;
	cukey = m_Keys.GetCurSel();
	if (cukey == -1) {
		MessageBox("You cannot delete a key without choosing one.");
		return;
	}

	CString str;
	CString sec;
	int cuSection = m_Sections.GetCurSel();
	m_Sections.GetLBText(cuSection, sec);
	m_Keys.GetText(cukey, str);

	if (MessageBox(CString((CString)"Are you sure you want to delete " + str + "? You should be really careful, you may not be able to use the map afterwards."), "Delete key", MB_YESNO) == IDNO) {
		return;
	}

	ini.RemoveValueByKey(sec, str);

	UpdateDialog();

	m_Sections.SetCurSel(cuSection);
	OnSelchangeSections();
}

void CAll::OnAddkey()
{
	CIniFile& ini = Map->GetIniFile();
	int cusection;
	cusection = m_Sections.GetCurSel();
	if (cusection == -1) {
		MessageBox("You need to specify a section first.");
		return;
	}

	CString sec;
	m_Sections.GetLBText(cusection, sec);

	auto const msg = TranslateStringACP("IniEditorAddKeyDesc");
	auto const cap = TranslateStringACP("IniEditorAddKeyCap");
	auto key = InputBox(msg, cap);

	if (key.IsEmpty()) {
		return;
	}

	CString value;
	if (key.Find("=") != -1) {
		// value specified
		// MW BUGFIX
		value = key.Right(key.GetLength() - key.Find("=") - 1);
		key = key.Left(key.Find("="));
	}
	key.Trim();
	value.Trim();

	ini.SetString(sec, key, value);

	UpdateDialog();
	m_Sections.SetCurSel(cusection);
	OnSelchangeSections();
}

void CAll::OnInisection()
{
	CFileDialog dlg(FALSE, ".ini", "*.ini", OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, "INI files|*.ini|");

	char cuPath[MAX_PATH];
	BOOL hidePreview = FALSE;
	BOOL previewPrinted = FALSE;
	GetCurrentDirectory(MAX_PATH, cuPath);
	dlg.m_ofn.lpstrInitialDir = cuPath;

	if (theApp.m_Options.TSExe.GetLength()) dlg.m_ofn.lpstrInitialDir = (char*)(LPCTSTR)theApp.m_Options.TSExe;

	if (dlg.DoModal() != IDCANCEL) {
		CImportINI impini;
		impini.m_FileName = dlg.GetPathName();
		if (impini.DoModal() != IDCANCEL) {
			UpdateDialog();
		}
	}
}
