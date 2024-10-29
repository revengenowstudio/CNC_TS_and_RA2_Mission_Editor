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

// Lighting.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "Lighting.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CLighting 

IMPLEMENT_DYNCREATE(CLighting, CDialog)

CLighting::CLighting() : CDialog(CLighting::IDD)
{
	//{{AFX_DATA_INIT(CLighting)
	//}}AFX_DATA_INIT
}

CLighting::~CLighting()
{
}

void CLighting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLighting)
	DDX_Control(pDX, IDC_RED2, m_Red2);
	DDX_Control(pDX, IDC_RED, m_Red);
	DDX_Control(pDX, IDC_LEVEL2, m_Level2);
	DDX_Control(pDX, IDC_LEVEL, m_Level);
	DDX_Control(pDX, IDC_GREEN2, m_Green2);
	DDX_Control(pDX, IDC_GREEN, m_Green);
	DDX_Control(pDX, IDC_BLUE2, m_Blue2);
	DDX_Control(pDX, IDC_BLUE, m_Blue);
	DDX_Control(pDX, IDC_AMBIENT2, m_Ambient2);
	DDX_Control(pDX, IDC_AMBIENT, m_Ambient);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLighting, CDialog)
	//{{AFX_MSG_MAP(CLighting)
	ON_EN_CHANGE(IDC_AMBIENT, OnChangeAmbient)
	ON_EN_CHANGE(IDC_LEVEL, OnChangeLevel)
	ON_EN_KILLFOCUS(IDC_AMBIENT, OnKillfocusAmbient)
	ON_EN_CHANGE(IDC_RED, OnChangeRed)
	ON_EN_CHANGE(IDC_GREEN, OnChangeGreen)
	ON_EN_CHANGE(IDC_BLUE, OnChangeBlue)
	ON_EN_CHANGE(IDC_AMBIENT2, OnChangeAmbient2)
	ON_EN_CHANGE(IDC_LEVEL2, OnChangeLevel2)
	ON_EN_CHANGE(IDC_RED2, OnChangeRed2)
	ON_EN_CHANGE(IDC_GREEN2, OnChangeGreen2)
	ON_EN_CHANGE(IDC_BLUE2, OnChangeBlue2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CLighting 

void CLighting::UpdateDialog()
{
	CIniFile& ini = Map->GetIniFile();

	m_Ambient.SetWindowText(ini.GetString("Lighting", "Ambient"));
	m_Ambient2.SetWindowText(ini.GetString("Lighting", "IonAmbient"));
	m_Level.SetWindowText(ini.GetString("Lighting", "Level"));
	m_Level2.SetWindowText(ini.GetString("Lighting", "IonLevel"));
	m_Red.SetWindowText(ini.GetString("Lighting", "Red"));
	m_Red2.SetWindowText(ini.GetString("Lighting", "IonRed"));
	m_Green.SetWindowText(ini.GetString("Lighting", "Green"));
	m_Green2.SetWindowText(ini.GetString("Lighting", "IonGreen"));
	m_Blue.SetWindowText(ini.GetString("Lighting", "Blue"));
	m_Blue2.SetWindowText(ini.GetString("Lighting", "IonBlue"));
	//MessageBox(ini.GetString("Lightning", "Ambient"));
}

void CLighting::translateUI()
{
	TranslateDlgItem(*this, IDC_TASKFORCE_T_TYPE, "TaskforcesType");
	TranslateWindowCaption(*this, "Lighting");
	TranslateDlgItem(*this, IDD_LIGHTING, "Lighting");
	TranslateDlgItem(*this, IDC_DESC, "LightingDesc");
	TranslateDlgItem(*this, IDC_LNORMAL, "LightingNormal");
	TranslateDlgItem(*this, IDC_LAMBIENT1, "LightingNormalAmbient");
	TranslateDlgItem(*this, IDC_LLEVEL1, "LightingNormalLevel");
	TranslateDlgItem(*this, IDC_LRED1, "LightingNormalRed");
	TranslateDlgItem(*this, IDC_LGREEN1, "LightingNormalGreen");
	TranslateDlgItem(*this, IDC_LBLUE1, "LightingNormalBlue");
	TranslateDlgItem(*this, IDC_LIONSTORM, "LightingIonStorm");
	TranslateDlgItem(*this, IDC_LAMBIENT2, "LightingWeatherStormAmbient");
	TranslateDlgItem(*this, IDC_LLEVEL2, "LightingWeatherStormLevel");
	TranslateDlgItem(*this, IDC_LRED2, "LightingWeatherStormRed");
	TranslateDlgItem(*this, IDC_LGREEN2, "LightingWeatherStormGreen");
	TranslateDlgItem(*this, IDC_LBLUE2, "LightingWeatherStormBlue");

#ifdef RA2_MODE
	TranslateDlgItem(*this, IDC_LIONSTORM, "LightingWeatherStorm");
#endif
}

BOOL CLighting::OnInitDialog()
{
	auto const ret = CDialog::OnInitDialog();
	translateUI();
	return ret;
}

void CLighting::OnChangeAmbient()
{
	CIniFile& ini = Map->GetIniFile();

	CString ctext;
	m_Ambient.GetWindowText(ctext);
	CString text = (char*)(LPCTSTR)ctext;
	ini.SetString("Lighting", "Ambient", text);
}

void CLighting::OnChangeLevel()
{
	CIniFile& ini = Map->GetIniFile();

	CString ctext;
	m_Level.GetWindowText(ctext);
	CString text = (char*)(LPCTSTR)ctext;
	ini.SetString("Lighting", "Level", text);
}

void CLighting::OnKillfocusAmbient()
{

}

void CLighting::OnChangeRed()
{
	CIniFile& ini = Map->GetIniFile();

	CString ctext;
	m_Red.GetWindowText(ctext);
	CString text = (char*)(LPCTSTR)ctext;
	ini.SetString("Lighting", "Red", text);
}

void CLighting::OnChangeGreen()
{
	CIniFile& ini = Map->GetIniFile();

	CString ctext;
	m_Green.GetWindowText(ctext);
	CString text = (char*)(LPCTSTR)ctext;
	ini.SetString("Lighting", "Green", text);
}

void CLighting::OnChangeBlue()
{
	CIniFile& ini = Map->GetIniFile();

	CString ctext;
	m_Blue.GetWindowText(ctext);
	CString text = (char*)(LPCTSTR)ctext;
	ini.SetString("Lighting", "Blue", text);
}

void CLighting::OnChangeAmbient2()
{
	CIniFile& ini = Map->GetIniFile();

	CString ctext;
	m_Ambient2.GetWindowText(ctext);
	CString text = (char*)(LPCTSTR)ctext;
	ini.SetString("Lighting", "IonAmbient", text);
}

void CLighting::OnChangeLevel2()
{
	CIniFile& ini = Map->GetIniFile();

	CString ctext;
	m_Level2.GetWindowText(ctext);
	CString text = (char*)(LPCTSTR)ctext;
	ini.SetString("Lighting", "IonLevel", text);
}

void CLighting::OnChangeRed2()
{
	CIniFile& ini = Map->GetIniFile();

	CString ctext;
	m_Red2.GetWindowText(ctext);
	CString text = (char*)(LPCTSTR)ctext;
	ini.SetString("Lighting", "IonRed", text);
}

void CLighting::OnChangeGreen2()
{
	CIniFile& ini = Map->GetIniFile();

	CString ctext;
	m_Green2.GetWindowText(ctext);
	CString text = (char*)(LPCTSTR)ctext;
	ini.SetString("Lighting", "IonGreen", text);
}

void CLighting::OnChangeBlue2()
{
	CIniFile& ini = Map->GetIniFile();

	CString ctext;
	m_Blue2.GetWindowText(ctext);
	CString text = (char*)(LPCTSTR)ctext;
	ini.SetString("Lighting", "IonBlue", text);
}
