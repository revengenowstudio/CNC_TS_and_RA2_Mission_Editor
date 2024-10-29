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

// TSOptions.cpp: implementation
//

#include "stdafx.h"
#include "FinalSun.h"
#include "TSOptions.h"
#include "resource.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"

extern CFinalSunApp theApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CTSOptions 


CTSOptions::CTSOptions(CWnd* pParent /*=NULL*/)
	: CDialog(CTSOptions::IDD, pParent)
	, m_PreferLocalTheaterFiles(FALSE)
{
	//{{AFX_DATA_INIT(CTSOptions)
	m_LikeTS = -1;
	//}}AFX_DATA_INIT
}


void CTSOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTSOptions)
	DDX_Control(pDX, IDC_LANGUAGE, m_Language);
	DDX_Control(pDX, IDC_EDIT1, m_TSExe);
	DDX_Radio(pDX, IDC_RULESLIKETS, m_LikeTS);
	DDX_Check(pDX, IDC_PREFER_LOCAL_THEATER_FILES, m_PreferLocalTheaterFiles);
	//}}AFX_DATA_MAP	
}


BEGIN_MESSAGE_MAP(CTSOptions, CDialog)
	ON_BN_CLICKED(IDC_CHOOSE, OnChoose)
	ON_CBN_SELCHANGE(IDC_LANGUAGE, &CTSOptions::OnCbnSelchangeLanguage)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTSOptions 

void CTSOptions::OnChoose()
{
	const char* pFileName = "game.exe";
	const char* pFileSearchPattern = "C&C EXE|game.exe|";
#if defined(RA2_MODE)
	pFileName = yuri_mode ? "ra2md.exe" : "ra2.exe";
	pFileSearchPattern = yuri_mode ? "Yuri's Revenge EXE|ra2md.exe|" : "Red Alert 2 EXE|ra2.exe|";
#else
	pFileName = "Sun.exe";
	pFileSearchPattern = "Tiberian Sun EXE|Sun.exe|";
#endif

	CFileDialog fd(TRUE, NULL, pFileName, OFN_FILEMUSTEXIST, pFileSearchPattern);
	fd.DoModal();

	this->GetDlgItem(IDC_EDIT1)->SetWindowText((LPCTSTR)fd.GetPathName());

	delete fd;
}

void CTSOptions::OnOK()
{
	this->GetDlgItem(IDC_EDIT1)->GetWindowText(m_TSEXE);
	m_LanguageName = getLanguageSelected();

	CDialog::OnOK();
}

BOOL CTSOptions::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_TSExe.SetWindowText((LPCTSTR)theApp.m_Options.TSExe);

	m_LikeTS = !theApp.m_Options.bSearchLikeTS;

	m_PreferLocalTheaterFiles = theApp.m_Options.bPreferLocalTheaterFiles;


	int englishIdx = 0;
	int selectedLanIdx = -1;
	auto const& languageSec = language["Languages"];
	for (auto i = 0; i < languageSec.Size(); i++) {
		auto const& def = languageSec.Nth(i).second;
		auto const& lang = language.GetString(def + "Header", "Name");
		m_Language.SetItemData(m_Language.AddString(lang), i);
		if (lang == "English") {
			englishIdx = i;
		}
		if (def == theApp.m_Options.LanguageName) {
			selectedLanIdx = i;
		}
	}
	if (selectedLanIdx < 0) {
		selectedLanIdx = englishIdx;
	}
	m_Language.SetCurSel(selectedLanIdx);

	updateUI();
	UpdateData(FALSE);


	return TRUE;
}


void CTSOptions::OnCbnSelchangeLanguage()
{
	theApp.m_Options.LanguageName = getLanguageSelected();
	updateUI();
}

void CTSOptions::updateUI()
{
	TranslateWindowCaption(*this, "OptionsCaption");

	TranslateDlgItem(*this, IDC_DESC, "OptionsDesc");
	TranslateDlgItem(*this, IDC_CHOOSE, "OptionsBrowse");
	TranslateDlgItem(*this, IDC_OPTIONS_LAN_TXT, "OptionsLanguage");
	TranslateDlgItem(*this, IDC_OPTIONS_SUPPORT_TXT, "OptionsSupportSettings");
	TranslateDlgItem(*this, IDC_RULESLIKETS, "OptionsSupportMissionsAndMods");
	TranslateDlgItem(*this, IDC_ONLYORIGINAL, "OptionsSupportOriginalRA2Only");
	TranslateDlgItem(*this, IDC_PREFER_LOCAL_THEATER_FILES, "OptionsPreferFA2TheaterSettings");
}

CString CTSOptions::getLanguageSelected()
{
	if (!language["Languages"].Size()) {
		return "English";
	}
	int n = m_Language.GetItemData(m_Language.GetCurSel());
	return language["Languages"].Nth(n).second;
}
