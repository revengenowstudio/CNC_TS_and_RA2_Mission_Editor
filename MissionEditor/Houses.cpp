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

// Houses.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "Houses.h"
#include "resource.h"
#include "finalsundlg.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"
#include "newra2housedlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CHouses 

IMPLEMENT_DYNCREATE(CHouses, CDialog)

CHouses::CHouses() : CDialog(CHouses::IDD)
{
	//{{AFX_DATA_INIT(CHouses)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}

CHouses::~CHouses()
{
}

void CHouses::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHouses)
	DDX_Control(pDX, IDC_HUMANPLAYER, m_HumanPlayer);
	DDX_Control(pDX, IDC_TECHLEVEL, m_TechLevel);
	DDX_Control(pDX, IDC_SIDE, m_Side);
	DDX_Control(pDX, IDC_PLAYERCONTROL, m_PlayerControl);
	DDX_Control(pDX, IDC_PERCENTBUILT, m_PercentBuilt);
	DDX_Control(pDX, IDC_NODECOUNT, m_Nodecount);
	DDX_Control(pDX, IDC_IQ, m_IQ);
	DDX_Control(pDX, IDC_EDGE, m_Edge);
	DDX_Control(pDX, IDC_CREDITS, m_Credits);
	DDX_Control(pDX, IDC_COLOR, m_Color);
	DDX_Control(pDX, IDC_ALLIES, m_Allies);
	DDX_Control(pDX, IDC_ACTSLIKE, m_ActsLike);
	DDX_Control(pDX, IDC_HOUSES, m_houses);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHouses, CDialog)
	//{{AFX_MSG_MAP(CHouses)
	ON_CBN_SELCHANGE(IDC_HOUSES, OnSelchangeHouses)
	ON_BN_CLICKED(IDC_PREPAREHOUSES, OnPreparehouses)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_ADDHOUSE, OnAddhouse)
	ON_BN_CLICKED(IDC_DELETEHOUSE, OnDeletehouse)
	ON_CBN_KILLFOCUS(IDC_IQ, OnKillfocusIq)
	ON_CBN_KILLFOCUS(IDC_EDGE, OnKillfocusEdge)
	ON_CBN_KILLFOCUS(IDC_SIDE, OnKillfocusSide)
	ON_CBN_KILLFOCUS(IDC_COLOR, OnKillfocusColor)
	ON_EN_KILLFOCUS(IDC_CREDITS, OnKillfocusCredits)
	ON_CBN_EDITCHANGE(IDC_ACTSLIKE, OnEditchangeActslike)
	ON_CBN_KILLFOCUS(IDC_NODECOUNT, OnKillfocusNodecount)
	ON_CBN_KILLFOCUS(IDC_TECHLEVEL, OnKillfocusTechlevel)
	ON_CBN_KILLFOCUS(IDC_PERCENTBUILT, OnKillfocusPercentbuilt)
	ON_CBN_KILLFOCUS(IDC_PLAYERCONTROL, OnKillfocusPlayercontrol)
	ON_CBN_SELCHANGE(IDC_HUMANPLAYER, OnSelchangeHumanplayer)
	ON_CBN_SELCHANGE(IDC_ACTSLIKE, OnSelchangeActslike)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_HOUSES_CHANGE_ALLIES, &CHouses::OnBnClickedHousesChangeAllies)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CHouses 

void CHouses::UpdateDialog()
{
	while (this->m_houses.DeleteString(0) != CB_ERR);
	while (this->m_HumanPlayer.DeleteString(0) != CB_ERR);
	while (this->m_Color.DeleteString(0) != CB_ERR);
	while (this->m_ActsLike.DeleteString(0) != CB_ERR);

	ListHouses(m_Side, FALSE, TRUE);

	int i;
	auto const& houseSec = rules[HOUSES];
	for (i = 0; i < houseSec.Size(); i++) {
#ifdef RA2_MODE
		auto const& name = houseSec.Nth(i).second;
		if (!name.CompareNoCase("nod") || !name.CompareNoCase("gdi")) {
			continue;
		}
#endif
		char house_id[5];
		itoa(i, house_id, 10);
		CString houseCString = house_id;
		houseCString += " ";
		houseCString += TranslateHouse(name, TRUE);
		m_ActsLike.AddString(houseCString);
	}

	CIniFile& ini = Map->GetIniFile();

	if (ini[MAPHOUSES].Size() <= 0) {
		// MessageBox("No houses do exist, if you want to use houses, you should use ""Prepare houses"" before doing anything else.");
	} else {
		m_HumanPlayer.AddString("None");
		m_HumanPlayer.SetCurSel(0);
		for (auto const& [seq, id] : ini[MAPHOUSES]) {
#ifdef RA2_MODE
			if (!id.CompareNoCase("nod") || !id.CompareNoCase("gdi")) {
				continue;
			}
#endif
			m_houses.AddString(TranslateHouse(id, TRUE));
			m_HumanPlayer.AddString(TranslateHouse(id, TRUE));
		}

		auto const& playerHouseStr = ini.GetString("Basic", "Player");
		if (!playerHouseStr.IsEmpty()) {
			m_HumanPlayer.SetCurSel(m_HumanPlayer.FindStringExact(0, TranslateHouse(playerHouseStr, TRUE)));
		}

		m_houses.SetCurSel(0);

		m_ActsLike.SetWindowText("");
		m_Allies.SetWindowText("");
		m_Color.SetWindowText("");
		m_Credits.SetWindowText("");
		m_Edge.SetWindowText("");
		m_IQ.SetWindowText("");
		m_Nodecount.SetWindowText("");
		m_PercentBuilt.SetWindowText("");
		m_PlayerControl.SetWindowText("");
		m_Side.SetWindowText("");
		m_TechLevel.SetWindowText("");



		OnSelchangeHouses();
	}

	// houses list done


	// ok now color list
	const auto& rulesColors = rules["Colors"];
	for (auto const& [id, def] : rulesColors) {
		m_Color.AddString(id);
	}
	for (auto const& [id, def] : ini["Colors"]) {
		if (!rulesColors.Exists(id)) {
			m_Color.AddString(id);
		}
	}
}

BOOL CHouses::OnInitDialog()
{
	CDialog::OnInitDialog();

	UpdateStrings();
	UpdateDialog();

#ifdef RA2_MODE
	::ShowWindow(GetDlgItem(IDC_LACTSLIKE)->m_hWnd, SW_HIDE);
	m_ActsLike.ShowWindow(SW_HIDE);
	//m_Edge.ShowWindow(SW_HIDE);
#endif

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CHouses::OnSelchangeHouses()
{
	CIniFile& ini = Map->GetIniFile();

	int cusel;
	cusel = m_houses.GetCurSel();
	if (cusel == -1) return;

	CString name;
	m_houses.GetLBText(cusel, name);

	name = TranslateHouse(name);

	auto const& s = ini[name];

	// ListHouses(m_ActsLike, TRUE);

#ifndef RA2_MODE
	m_ActsLike.SetWindowText(s.values["ActsLike"]);
#endif
	m_Allies.SetWindowText(TranslateHouse(s.GetString("Allies"), TRUE));
	m_Color.SetWindowText(s.GetString("Color"));
	m_Credits.SetWindowText(s.GetString("Credits"));
	m_Edge.SetWindowText(s.GetString("Edge"));
	m_IQ.SetWindowText(s.GetString("IQ"));
	m_Nodecount.SetWindowText(s.GetString("NodeCount"));
	m_PercentBuilt.SetWindowText(s.GetString("PercentBuilt"));
	m_PlayerControl.SetWindowText(s.GetString("PlayerControl"));
#ifndef RA2_MODE
	m_Side.SetWindowText(s.GetString("Side"));
#else
	m_Side.SetWindowText(TranslateHouse(s.GetString("Country"), TRUE));
#endif
	m_TechLevel.SetWindowText(s.GetString("TechLevel"));

}

void CHouses::OnPreparehouses()
{
	CIniFile& ini = Map->GetIniFile();

#ifdef RA2_MODE
	if (Map->IsMultiplayer()) {
		ini.SetInteger("Basic", "MultiplayerOnly", 1);

		auto const rulesHouseSec = rules[HOUSES];
		for (auto i = 0; i < rulesHouseSec.Size(); ++i) {
			char c[50];
			int k = i;
			itoa(k, c, 10);
			auto const& country = rulesHouseSec.Nth(i).second;
			// we now create a MAPHOUSE with the same name as the current rules house
			ini.SetString(MAPHOUSES, c, country);

			ini.SetString(country, "IQ", "0");
			ini.SetString(country, "Edge", "North");
			ini.SetString(country, "Color", rules.GetString(country, "Color"));
			ini.SetString(country, "Allies", country);
			ini.SetString(country, "Country", country);
			ini.SetString(country, "Credits", "0");
			ini.SetString(country, "NodeCount", "0");
			ini.SetString(country, "TechLevel", "1");
			ini.SetString(country, "PercentBuilt", "0");
			ini.SetString(country, "PlayerControl", "no");

		}

		UpdateDialog();
		return;
	}
#endif

	// import the rules.ini houses
	if (ini[MAPHOUSES].Size() > 0) {
		auto const title = TranslateStringACP("HouseDuplicatedCreatingCaption");
		auto const content = TranslateStringACP("HouseDuplicatedCreatingTip");
		MessageBox(content, title);
		return;
	}

	for (auto const& [seq, id] : rules[HOUSES]) {
		AddHouse(GetHouseSectionName(id));
	}
}

void CHouses::AddHouse(const CString& name)
{
	CIniFile& ini = Map->GetIniFile();

	if (ini.TryGetSection(name)) {
		MessageBox("Sorry this name is not available. " + name + " is already used in the map file. You need to use another name.");
		return;
	}
	if (ini.TryGetSection(TranslateHouse(name))) {
		MessageBox("Sorry this name is not available. " + name + " is already used in the map file. You need to use another name.");
		return;
	}
#ifdef RA2_MODE
	CNewRA2HouseDlg dlg;
	if (dlg.DoModal() == IDCANCEL) {
		return;
	}
#endif

	int c;

	//okay, get a free slot
	int pos = -1;
#ifdef RA2_MODE
	int pos2 = -1;
#endif
	for (c = 0; c > -1; c++) {
		char k[50];
		itoa(c, k, 10);
		if (!ini[MAPHOUSES].Exists(k)) {
			pos = c;
		}
		if (pos != -1) {
			break;
		}
	}
#ifdef RA2_MODE
	for (c = 0; c > -1; c++) {
		char k[50];
		itoa(c, k, 10);
		if (!ini[HOUSES].Exists(k)) {
			pos2 = c;
		}
		if (pos2 != -1) break;
	}
#endif

	char k[50];
	itoa(pos, k, 10);

	auto const realHouseID = TranslateHouse(name);
	ini.SetString(MAPHOUSES, k, realHouseID);

	CString country;
	country = name;
	country.Replace(" House", "");
	country.Replace("House", "");
	if (country.Find(" ") >= 0) {
		country.Replace(" ", "_"); //=country.Left(country.Find(" "));
	}

#ifdef RA2_MODE
	_itoa(pos2, k, 10);
	ini.SetString(HOUSES, k, country);
#endif
	ini.SetInteger(realHouseID, "IQ", 0);
	ini.SetString(realHouseID, "Edge", "West");
	ini.SetString(realHouseID, "Allies", realHouseID);

	CString side = name;
	auto const parentCountryID = TranslateHouse(dlg.m_Country);
#ifdef RA2_MODE
	side = rules.GetString(parentCountryID, "Side");
#endif

	if (side.Find("Nod") >= 0) {
#ifndef RA2_MODE
		ini.sections[realHouseID].values["Side"] = "Nod";
		if (name != "Nod") {
			ini.SetString(name, "Allies", ini.GetString(name, "Allies") + ",Nod");
		}
#endif
		ini.SetString(realHouseID, "Color", "DarkRed");
	} else if (side.Find("GDI") >= 0) {
#if defined(RA2_MODE)
		ini.SetString(realHouseID, "Color", "DarkBlue");
#else
		if (name != "GDI") {
			ini.SetString(realHouseID, "Allies", ini.GetString(realHouseID, "Allies") + ",GDI");
			ini.SetString(realHouseID, "Color", "Gold");
		}
#endif
	}
	ini.SetInteger(realHouseID, "Credits", 0);
#ifndef RA2_MODE
	ini.SetInteger(realHouseID, "ActsLike", 0);
#else
	ini.SetString(realHouseID, "Country", TranslateHouse(country));
#endif
	ini.SetInteger(realHouseID, "NodeCount", 0);
	ini.SetInteger(realHouseID, "TechLevel", 10);
	ini.SetInteger(realHouseID, "PercentBuilt", 100);
	ini.SetBool(realHouseID, "PlayerControl", false);

#ifdef RA2_MODE
	country = TranslateHouse(country);
	ini.SetString(country, "ParentCountry", parentCountryID);
	ini.SetString(country, "Name", country);
	ini.SetString(country, "Suffix", rules.GetString(parentCountryID, "Suffix"));
	ini.SetString(country, "Prefix", rules.GetString(parentCountryID, "Prefix"));
	ini.SetString(country, "Color", rules.GetString(parentCountryID, "Color"));
	ini.SetString(country, "Side", rules.GetString(parentCountryID, "Side"));
	ini.SetString(country, "SmartAI", rules.GetString(parentCountryID, "SmartAI"));
	ini.SetInteger(country, "CostUnitsMult", 1);
#endif

	int cusel = m_houses.GetCurSel();
	UpdateDialog();
	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs();
	if (cusel != -1) {
		m_houses.SetCurSel(cusel);
	}
}

void CHouses::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	CIniFile& ini = Map->GetIniFile();

	if (bShow) {
		if (!ini.TryGetSection(MAPHOUSES) && ini.Size() > 0) {
			auto const caption = TranslateStringACP("HouseNoneExistenseCap");
#ifndef RA2_MODE
			MessageBox("No houses do exist, if you want to use houses, you should use ""Prepare houses"" before doing anything else. Note that in a multiplayer map independent computer players cannot be created by using the names GDI and Nod for the house. Just use something like GDI_AI.");
#else
			MessageBox(TranslateStringACP("HouseNoneExistense"), caption);

#endif
		}
	} else {
		// call all KillFocus !
		OnKillfocusIq();
		OnEditchangeActslike();
		OnKillfocusColor();
		OnKillfocusCredits();
		OnKillfocusEdge();
		OnKillfocusNodecount();
		OnKillfocusPercentbuilt();
		OnKillfocusPlayercontrol();
		OnKillfocusSide();
		OnKillfocusTechlevel();
	}
}

void CHouses::OnAddhouse()
{
	CString name = InputBox(GetLanguageStringACP("AddHouse"), GetLanguageStringACP("AddHouseCap"));
	if (name.GetLength() == 0) return;

	name = GetHouseSectionName(name);
	//name=TranslateHouse(name);
	AddHouse(name);
}

void CHouses::OnDeletehouse()
{
	CIniFile& ini = Map->GetIniFile();

	int cusel;
	cusel = m_houses.GetCurSel();
	if (cusel == -1) {
		return;
	}

	CString name;
	CString uiname;
	m_houses.GetLBText(cusel, name);

	uiname = name;
	name = TranslateHouse(name);

	CString str = GetLanguageStringACP("DeleteHouse");
	str = TranslateStringVariables(1, str, uiname);
	if (MessageBox(str, GetLanguageStringACP("DeleteHouseCap"), MB_YESNO) == IDNO) {
		return;
	}

	ini.DeleteSection(name);

	ini.RemoveValue(MAPHOUSES, name);

	if (ini[MAPHOUSES].Size() == 0) {
		ini.DeleteSection(MAPHOUSES);
	}

	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs();
	UpdateDialog();
}

void CHouses::OnKillfocusIq()
{
	CIniFile& ini = Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel = m_houses.GetCurSel();
	if (cusel == -1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name = TranslateHouse(name);

	CString t;
	m_IQ.GetWindowText(t);
	ini.SetString(name, "IQ", t);
}

void CHouses::OnKillfocusEdge()
{
	CIniFile& ini = Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel = m_houses.GetCurSel();
	if (cusel == -1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name = TranslateHouse(name);

	CString t;
	m_Edge.GetWindowText(t);
	ini.SetString(name, "Edge", t);
}

void CHouses::OnKillfocusSide()
{
	CIniFile& ini = Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel = m_houses.GetCurSel();
	if (cusel == -1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name = TranslateHouse(name);

	CString t;
	m_Side.GetWindowText(t);
	t = TranslateHouse(t);
#ifndef RA2_MODE
	ini.SetString(name, "Side", t);
#else
	ini.SetString(name, "Country", t);
#endif
}

void CHouses::OnKillfocusColor()
{
	CIniFile& ini = Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel = m_houses.GetCurSel();
	if (cusel == -1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name = TranslateHouse(name);

	CString t;
	m_Color.GetWindowText(t);
	ini.SetString(name, "Color", t);

	//Map->UpdateIniFile(MAPDATA_UPDATE_FROM_INI);
	// MW fix: Only update structures
	// this recalculates the colors
	Map->UpdateStructures(FALSE);
	// and minimap
	Map->RedrawMinimap();
	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->RedrawWindow();
	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_minimap.RedrawWindow();
}

void CHouses::OnKillfocusCredits()
{
	CIniFile& ini = Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel = m_houses.GetCurSel();
	if (cusel == -1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name = TranslateHouse(name);

	CString t;
	m_Credits.GetWindowText(t);
	ini.SetString(name, "Credits", t);
}

void CHouses::OnEditchangeActslike()
{
	CIniFile& ini = Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel = m_houses.GetCurSel();
	if (cusel == -1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name = TranslateHouse(name);

	CString t;
	m_ActsLike.GetWindowText(t);
	TruncSpace(t);
	t = TranslateHouse(t);
	ini.SetString(name, "ActsLike", t);
}

void CHouses::OnKillfocusNodecount()
{
	CIniFile& ini = Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel = m_houses.GetCurSel();
	if (cusel == -1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name = TranslateHouse(name);

	CString t;
	m_Nodecount.GetWindowText(t);
	ini.SetString(name, "NodeCount", t);
}

void CHouses::OnKillfocusTechlevel()
{
	CIniFile& ini = Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel = m_houses.GetCurSel();
	if (cusel == -1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name = TranslateHouse(name);

	CString t;
	m_TechLevel.GetWindowText(t);
	ini.SetString(name, "TechLevel", t);
}

void CHouses::OnKillfocusPercentbuilt()
{
	CIniFile& ini = Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel = m_houses.GetCurSel();
	if (cusel == -1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name = TranslateHouse(name);

	CString t;
	m_PercentBuilt.GetWindowText(t);
	ini.SetString(name, "PercentBuilt", t);
}

void CHouses::OnKillfocusPlayercontrol()
{
	CIniFile& ini = Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel = m_houses.GetCurSel();
	if (cusel == -1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name = TranslateHouse(name);

	CString t;
	m_PlayerControl.GetWindowText(t);
	ini.SetString(name, "PlayerControl", t);
}

void CHouses::OnSelchangeHumanplayer()
{
	CIniFile& ini = Map->GetIniFile();

	CString pl;
	m_HumanPlayer.GetLBText(m_HumanPlayer.GetCurSel(), pl);
	pl = TranslateHouse(pl);

	if (pl.GetLength() == 0 || pl == "None") {
		ini.RemoveValueByKey("Basic", "Player");
		return;
	}

	ini.SetString("Basic", "Player", pl);
}

void CHouses::OnSelchangeActslike()
{
	CIniFile& ini = Map->GetIniFile();

	int cusel;
	cusel = m_houses.GetCurSel();
	if (cusel == -1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name = TranslateHouse(name);

	CString t;
	m_ActsLike.GetLBText(m_ActsLike.GetCurSel(), t);
	TruncSpace(t);
	t = TranslateHouse(t);
	ini.SetString(name, "ActsLike", t);
}

void CHouses::OnBnClickedHousesChangeAllies()
{
	auto const curSel = m_houses.GetCurSel();
	if (curSel == -1) {
		return;
	}

	CString name;
	m_houses.GetLBText(curSel, name);
	name = TranslateHouse(name);

	CIniFile& ini = Map->GetIniFile();

	std::vector<CString> allHouses;
	for (auto idx = 0; idx < m_houses.GetCount(); ++idx) {
		CString item;
		m_houses.GetLBText(idx, item);
		allHouses.emplace_back(std::move(item));
	}

	CHouseAllies dlg(std::move(allHouses), ini.GetString(name, "Allies"));

	if (dlg.DoModal() == IDCANCEL) {
		return;
	}
	auto&& allies = dlg.GetAllies();
	ini.SetString(name, "Allies", INIHelper::Join(allies));

	for (auto& house : allies) {
		house = TranslateHouse(house, true);
	}
	m_Allies.SetWindowText(INIHelper::Join(allies));
	SetMainStatusBarReady();
}

void CHouses::UpdateStrings()
{
	SetDlgItemText(IDC_DESC, GetLanguageStringACP("HousesDesc"));
	SetDlgItemText(IDC_LPLAYER, GetLanguageStringACP("HousesPlayerHouse"));
	SetDlgItemText(IDC_LHOUSE, GetLanguageStringACP("HousesHouse"));
	SetDlgItemText(IDC_LIQ, GetLanguageStringACP("HousesIQ"));
	SetDlgItemText(IDC_LEDGE, GetLanguageStringACP("HousesEdge"));
	SetDlgItemText(IDC_LSIDE, GetLanguageStringACP("HousesSide"));
	SetDlgItemText(IDC_LCOLOR, GetLanguageStringACP("HousesColor"));
	SetDlgItemText(IDC_LALLIES, GetLanguageStringACP("HousesAllies"));
	SetDlgItemText(IDC_LCREDITS, GetLanguageStringACP("HousesCredits"));
#ifndef RA2_MODE
	SetDlgItemText(IDC_LACTSLIKE, GetLanguageStringACP("HousesActsLike"));
#endif
	SetDlgItemText(IDC_LNODECOUNT, GetLanguageStringACP("HousesNodeCount"));
	SetDlgItemText(IDC_LTECHLEVEL, GetLanguageStringACP("HousesTechlevel"));
	SetDlgItemText(IDC_LBUILDACTIVITY, GetLanguageStringACP("HousesBuildActivity"));
	SetDlgItemText(IDC_LPLAYERCONTROL, GetLanguageStringACP("HousesPlayerControl"));

	SetDlgItemText(IDC_PREPAREHOUSES, GetLanguageStringACP("HousesPrepareHouses"));
	SetDlgItemText(IDC_ADDHOUSE, GetLanguageStringACP("HousesAddHouse"));
	SetDlgItemText(IDC_DELETEHOUSE, GetLanguageStringACP("HousesDeleteHouse"));

	SetWindowText(TranslateStringACP(HOUSES));
}

void CHouses::PostNcDestroy()
{
	CDialog::PostNcDestroy();
}

// CHouseAllies

CHouseAllies::CHouseAllies(std::vector<CString>&& houses, const CString& allies, CWnd* pParent) :
	CDialog(CHouseAllies::IDD, pParent),
	m_allHouses(std::move(houses))
{
	auto const allyStrings = INIHelper::Split(allies);
	for (auto& str : allyStrings) {
		m_allies.insert(TranslateHouse(str, true));
	}

}

CHouseAllies::~CHouseAllies()
{
}

std::vector<CString> CHouseAllies::GetAllies() const
{
	std::vector<CString> ret;
	for (auto const& str : m_allies) {
		ret.emplace_back(TranslateHouse(str));
	}
	return ret;
}

BOOL CHouseAllies::OnInitDialog()
{
	auto const ret = CDialog::OnInitDialog();
	translateUI();

	for (auto const& str : m_allies) {
		m_allyList.AddString(str);
	}
	for (auto const& house : m_allHouses) {
		if (m_allies.find(house) == m_allies.end()) {
			m_enemyList.AddString(house);
		}
	}
	updateAllyTexts();
	return ret;
}

void CHouseAllies::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_HOUSE_LIST_ALLIES, m_allyList);
	DDX_Control(pDX, IDC_HOUSE_LIST_ENEMIES, m_enemyList);
	DDX_Control(pDX, IDC_HOUSE_ALLIES_VALUES, m_allianceText);
}

BEGIN_MESSAGE_MAP(CHouseAllies, CDialog)
	ON_BN_CLICKED(IDC_HOUSE_ALLIES_ADD, &CHouseAllies::OnBnClickedHouseAlliesAdd)
	ON_BN_CLICKED(IDC_HOUSE_ALLIES_REMOVE, &CHouseAllies::OnBnClickedHouseAlliesRemove)
END_MESSAGE_MAP()


void CHouseAllies::translateUI()
{
	TranslateWindowCaption(*this, "HouseAlliesCaption");

	TranslateDlgItem(*this, IDC_HOUSE_ALLIES_T_ENEMIES, "HouseAlliesEnemies");
	TranslateDlgItem(*this, IDC_HOUSE_ALLIES_T_ALLIES, "HouseAlliesAllies");

	SetDlgItemText(IDOK, GetLanguageStringACP("OK"));
	SetDlgItemText(IDCANCEL, GetLanguageStringACP("Cancel"));
}

void CHouseAllies::updateAllyTexts()
{
	m_allianceText.SetWindowText(INIHelper::Join(m_allies));
}

void CHouseAllies::OnBnClickedHouseAlliesAdd()
{
	auto const selected = m_enemyList.GetCurSel();
	if (selected < 0) {
		return;
	}
	CString house;
	m_enemyList.GetText(selected, house);
	m_enemyList.DeleteString(selected);
	m_allies.insert(house);
	m_allyList.AddString(house);
	updateAllyTexts();
}


void CHouseAllies::OnBnClickedHouseAlliesRemove()
{
	auto const selected = m_allyList.GetCurSel();
	if (selected < 0) {
		return;
	}
	CString house;
	m_allyList.GetText(selected, house);
	m_allyList.DeleteString(selected);
	m_allies.erase(house);
	m_enemyList.AddString(house);
	updateAllyTexts();
}
