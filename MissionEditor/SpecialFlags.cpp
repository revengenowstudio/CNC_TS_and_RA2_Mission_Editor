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

// SpecialFlags.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "SpecialFlags.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSpecialFlags 


CSpecialFlags::CSpecialFlags(CWnd* pParent /*=NULL*/)
	: CDialog(CSpecialFlags::IDD, 0)
{
	//{{AFX_DATA_INIT(CSpecialFlags)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}


void CSpecialFlags::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpecialFlags)
	DDX_Control(pDX, IDC_VISCEROIDS, m_Visceroids);
	DDX_Control(pDX, IDC_TIBERIUMSPREADS, m_TiberiumSpreads);
	DDX_Control(pDX, IDC_TIBERIUMGROWS, m_TiberiumGrows);
	DDX_Control(pDX, IDC_TIBERIUMEXPLOSIVE, m_TiberiumExplosive);
	DDX_Control(pDX, IDC_METEORITES, m_Meteorites);
	DDX_Control(pDX, IDC_MCVDEPLOY, m_MCVDeploy);
	DDX_Control(pDX, IDC_IONSTORMS, m_IonStorms);
	DDX_Control(pDX, IDC_INITIALVETERAN, m_InitialVeteran);
	DDX_Control(pDX, IDC_INERT, m_Inert);
	DDX_Control(pDX, IDC_HARVESTERIMMUNE, m_HarvesterImmune);
	DDX_Control(pDX, IDC_FOGOFWAR, m_FogOfWar);
	DDX_Control(pDX, IDC_FIXEDALLIANCE, m_FixedAlliance);
	DDX_Control(pDX, IDC_DESTROYABLEBRIDGES, m_DestroyableBridges);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpecialFlags, CDialog)
	//{{AFX_MSG_MAP(CSpecialFlags)
	ON_CBN_EDITCHANGE(IDC_TIBERIUMGROWS, OnEditchangeTiberiumgrows)
	ON_CBN_EDITCHANGE(IDC_TIBERIUMSPREADS, OnEditchangeTiberiumspreads)
	ON_CBN_EDITCHANGE(IDC_TIBERIUMEXPLOSIVE, OnEditchangeTiberiumexplosive)
	ON_CBN_EDITCHANGE(IDC_DESTROYABLEBRIDGES, OnEditchangeDestroyablebridges)
	ON_CBN_EDITCHANGE(IDC_MCVDEPLOY, OnEditchangeMcvdeploy)
	ON_CBN_EDITCHANGE(IDC_INITIALVETERAN, OnEditchangeInitialveteran)
	ON_CBN_EDITCHANGE(IDC_FIXEDALLIANCE, OnEditchangeFixedalliance)
	ON_CBN_EDITCHANGE(IDC_HARVESTERIMMUNE, OnEditchangeHarvesterimmune)
	ON_CBN_EDITCHANGE(IDC_FOGOFWAR, OnEditchangeFogofwar)
	ON_CBN_EDITCHANGE(IDC_INERT, OnEditchangeInert)
	ON_CBN_EDITCHANGE(IDC_IONSTORMS, OnEditchangeIonstorms)
	ON_CBN_EDITCHANGE(IDC_METEORITES, OnEditchangeMeteorites)
	ON_CBN_EDITCHANGE(IDC_VISCEROIDS, OnEditchangeVisceroids)

	ON_CBN_SELCHANGE(IDC_TIBERIUMGROWS, OnEditchangeTiberiumgrows)
	ON_CBN_SELCHANGE(IDC_TIBERIUMSPREADS, OnEditchangeTiberiumspreads)
	ON_CBN_SELCHANGE(IDC_TIBERIUMEXPLOSIVE, OnEditchangeTiberiumexplosive)
	ON_CBN_SELCHANGE(IDC_DESTROYABLEBRIDGES, OnEditchangeDestroyablebridges)
	ON_CBN_SELCHANGE(IDC_MCVDEPLOY, OnEditchangeMcvdeploy)
	ON_CBN_SELCHANGE(IDC_INITIALVETERAN, OnEditchangeInitialveteran)
	ON_CBN_SELCHANGE(IDC_FIXEDALLIANCE, OnEditchangeFixedalliance)
	ON_CBN_SELCHANGE(IDC_HARVESTERIMMUNE, OnEditchangeHarvesterimmune)
	ON_CBN_SELCHANGE(IDC_FOGOFWAR, OnEditchangeFogofwar)
	ON_CBN_SELCHANGE(IDC_INERT, OnEditchangeInert)
	ON_CBN_SELCHANGE(IDC_IONSTORMS, OnEditchangeIonstorms)
	ON_CBN_SELCHANGE(IDC_METEORITES, OnEditchangeMeteorites)
	ON_CBN_SELCHANGE(IDC_VISCEROIDS, OnEditchangeVisceroids)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CSpecialFlags::UpdateDialog()
{
	CIniFile& ini = Map->GetIniFile();

	auto const& sec = ini["SpecialFlags"];
	m_DestroyableBridges.SetWindowText(sec.GetString("DestroyableBridges"));
	m_FixedAlliance.SetWindowText(sec.GetString("FixedAlliance"));
	m_FogOfWar.SetWindowText(sec.GetString("FogOfWar"));
	m_HarvesterImmune.SetWindowText(sec.GetString("HarvesterImmune"));
	m_Inert.SetWindowText(sec.GetString("Inert"));
	m_InitialVeteran.SetWindowText(sec.GetString("InitialVeteran"));
	m_IonStorms.SetWindowText(sec.GetString("IonStorms"));
	m_MCVDeploy.SetWindowText(sec.GetString("MCVDeploy"));
	m_Meteorites.SetWindowText(sec.GetString("Meteorites"));
	m_TiberiumExplosive.SetWindowText(sec.GetString("TiberiumExplosive"));
	m_TiberiumGrows.SetWindowText(sec.GetString("TiberiumGrows"));
	m_TiberiumSpreads.SetWindowText(sec.GetString("TiberiumSpreads"));
	m_Visceroids.SetWindowText(sec.GetString("Visceroids"));

#ifdef RA2_MODE
	GetDlgItem(IDC_LTIBERIUMEXPLOSIVE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_TIBERIUMEXPLOSIVE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_HARVESTERIMMUNE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LHARVESTERIMMUNE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_METEORITES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LMETEORITES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_VISCEROIDS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LVISCEROIDS)->ShowWindow(SW_HIDE);
#endif

};
/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CSpecialFlags 

BOOL CSpecialFlags::OnInitDialog()
{
	auto const ret = CDialog::OnInitDialog();

#if defined(RA2_MODE)
	SetDlgItemText(IDC_LTIBERIUMGROWS, "Ore grows:");
	SetDlgItemText(IDC_LTIBERIUMSPREADS, "Ore spreads:");
	SetDlgItemText(IDC_LFOGOFWAR, "Shroud:");
	SetDlgItemText(IDC_LIONSTORMS, "Weather Storms:");
#endif

	TranslateUI();
	return ret;
}

void CSpecialFlags::TranslateUI()
{
	TranslateWindowCaption(*this, "SpecialFlagsCaption");
	TranslateDlgItem(*this, IDC_DESC, "SpecialFlagsDesc");
	TranslateDlgItem(*this, IDC_LTIBERIUMGROWS, "SpecialFlagsTiberiumGrows");
	TranslateDlgItem(*this, IDC_LTIBERIUMSPREADS, "SpecialFlagsTiberiumSpread");
	TranslateDlgItem(*this, IDC_LTIBERIUMEXPLOSIVE, "SpecialFlagsTiberiumExplosive");
	TranslateDlgItem(*this, IDC_LDESTROYABLEBRIDGES, "SpecialFlagsTiberiumBridgeDestroyable");
	TranslateDlgItem(*this, IDC_LMCVDEPLOY, "SpecialFlagsMCVRedeploy");
	TranslateDlgItem(*this, IDC_LINITIALVETERAN, "SpecialFlagsInitialVeteran");
	TranslateDlgItem(*this, IDC_LFIXEDALLIANCE, "SpecialFlagsFixedAlliance");
	TranslateDlgItem(*this, IDC_LHARVESTERIMMUNE, "SpecialFlagsHarvesterImmune");
	TranslateDlgItem(*this, IDC_LINERT, "SpecialFlagsInert");
	TranslateDlgItem(*this, IDC_LFOGOFWAR, "SpecialFlagsFogOfWar");
	TranslateDlgItem(*this, IDC_LIONSTORMS, "SpecialFlagsIonStoms");
	TranslateDlgItem(*this, IDC_LMETEORITES, "SpecialFlagsMeteorites");
	TranslateDlgItem(*this, IDC_LVISCEROIDS, "SpecialFlagsVisceroids");
}

void CSpecialFlags::OnEditchangeTiberiumgrows()
{
	CIniFile& ini = Map->GetIniFile();
	CString str;
	str = GetText(&m_TiberiumGrows);
	ini.SetString("SpecialFlags", "TiberiumGrows", str);
}

void CSpecialFlags::OnEditchangeTiberiumspreads()
{
	CIniFile& ini = Map->GetIniFile();
	CString str;
	str = GetText(&m_TiberiumSpreads);
	ini.SetString("SpecialFlags", "TiberiumSpreads", str);
}

void CSpecialFlags::OnEditchangeTiberiumexplosive()
{
	CIniFile& ini = Map->GetIniFile();
	CString str;
	str = GetText(&m_TiberiumExplosive);
	ini.SetString("SpecialFlags", "TiberiumExplosive", str);
}

void CSpecialFlags::OnEditchangeDestroyablebridges()
{
	CIniFile& ini = Map->GetIniFile();
	CString str;
	str = GetText(&m_DestroyableBridges);
	ini.SetString("SpecialFlags", "DestroyableBridges", str);
}

void CSpecialFlags::OnEditchangeMcvdeploy()
{
	CIniFile& ini = Map->GetIniFile();
	CString str;
	str = GetText(&m_MCVDeploy);
	ini.SetString("SpecialFlags", "MCVDeploy", str);
}

void CSpecialFlags::OnEditchangeInitialveteran()
{
	CIniFile& ini = Map->GetIniFile();
	CString str;
	str = GetText(&m_InitialVeteran);
	ini.SetString("SpecialFlags", "InitialVeteran", str);
}

void CSpecialFlags::OnEditchangeFixedalliance()
{
	CIniFile& ini = Map->GetIniFile();
	CString str;
	str = GetText(&m_FixedAlliance);
	ini.SetString("SpecialFlags", "FixedAlliance", str);
}

void CSpecialFlags::OnEditchangeHarvesterimmune()
{
	CIniFile& ini = Map->GetIniFile();
	CString str;
	str = GetText(&m_HarvesterImmune);
	ini.SetString("SpecialFlags", "HarvesterImmune", str);
}

void CSpecialFlags::OnEditchangeFogofwar()
{
	CIniFile& ini = Map->GetIniFile();
	CString str;
	str = GetText(&m_FogOfWar);
	ini.SetString("SpecialFlags", "FogOfWar", str);
}

void CSpecialFlags::OnEditchangeInert()
{
	CIniFile& ini = Map->GetIniFile();
	CString str;
	str = GetText(&m_Inert);
	ini.SetString("SpecialFlags", "Inert", str);
}

void CSpecialFlags::OnEditchangeIonstorms()
{
	CIniFile& ini = Map->GetIniFile();
	CString str;
	str = GetText(&m_IonStorms);
	ini.SetString("SpecialFlags", "IonStorms", str);
}

void CSpecialFlags::OnEditchangeMeteorites()
{
	CIniFile& ini = Map->GetIniFile();
	CString str;
	str = GetText(&m_Meteorites);
	ini.SetString("SpecialFlags", "Meteorites", str);
}

void CSpecialFlags::OnEditchangeVisceroids()
{
	CIniFile& ini = Map->GetIniFile();
	CString str;
	str = GetText(&m_Visceroids);
	ini.SetString("SpecialFlags", "Visceroids", str);
}
