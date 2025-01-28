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

// ViewObjects.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "ViewObjects.h"
#include "FinalSunDlg.h"
#include "structs.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"
#include "inlines.h"
#include "rtpdlg.h"
#include "TubeTool.h"
#include "StringHelper.h"
#include "IniMega.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewObjects

const size_t valadded = 2 << 16;

IMPLEMENT_DYNCREATE(CViewObjects, CTreeView)

CViewObjects::CViewObjects()
{
}

CViewObjects::~CViewObjects()
{
}


BEGIN_MESSAGE_MAP(CViewObjects, CTreeView)
	//{{AFX_MSG_MAP(CViewObjects)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeydown)
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


extern int overlay_number[];
extern CString overlay_name[];
extern BOOL overlay_visible[];
extern BOOL overlay_trail[];


extern int overlay_count;


extern ACTIONDATA AD;

/////////////////////////////////////////////////////////////////////////////
// Zeichnung CViewObjects 

void CViewObjects::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// ZU ERLEDIGEN: Code zum Zeichnen hier einfügen
}

/////////////////////////////////////////////////////////////////////////////
// Diagnose CViewObjects

#ifdef _DEBUG
void CViewObjects::AssertValid() const
{
	CTreeView::AssertValid();
}

void CViewObjects::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

CString GetTheaterLanguageString(const CString& l10nTag)
{
	CString s = l10nTag;
	CString t = l10nTag;

#define TO_ADDR(x) reinterpret_cast<const size_t>(x)

	if (TO_ADDR(tiledata) == TO_ADDR(&t_tiledata)) {
		t += "TEM";
	}
	else if (TO_ADDR(tiledata) == TO_ADDR(&s_tiledata)) {
		t += "SNO";
	}
	else if (TO_ADDR(tiledata) == TO_ADDR(&u_tiledata)) {
		t += "URB";
	}
	else if (TO_ADDR(tiledata) == TO_ADDR(&un_tiledata)) {
		t += "UBN";
	}
	else if (TO_ADDR(tiledata) == TO_ADDR(&l_tiledata)) {
		t += "LUN";
	}
	else if (TO_ADDR(tiledata) == TO_ADDR(&d_tiledata)) {
		t += "DES";
	}
	CString res = GetLanguageStringACP(t);
	if (res.Left(l10nTag.GetLength()) == l10nTag || res.GetLength() == 0) {
		res = GetLanguageStringACP(s);
	}
	return res;
}

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CViewObjects 


void CViewObjects::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	CIniFile& ini = Map->GetIniFile();

	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	int val = pNMTreeView->itemNew.lParam;
	if (val < 0) { // return;
		if (val == -2) {
			AD.reset();
			((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		return;
	}

	if (val < valadded) {
		// standard selection (maybe erasing etc)
		switch (val) {
		case 10: // erase field
		{
			AD.mode = ACTIONMODE_ERASEFIELD;
			break;
		}
		case 20: // waypoint stuff now
		{
			AD.mode = ACTIONMODE_WAYPOINT;
			AD.type = 0;
			break;
		}
		case 21:
		{
			AD.mode = 3;
			AD.type = 1;
			break;
		}
		case 22:
		{
			AD.mode = 3;
			AD.type = 2;
			break;
		}
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		{
			AD.mode = 3;
			AD.type = 3 + val - 23;
			break;
		}
		case 36: // celltag stuff
		{
			AD.mode = 4;
			AD.type = 0;
			break;
		}
		case 37:
		{
			AD.mode = 4;
			AD.type = 1;
			break;
		}
		case 38:
		{
			AD.mode = 4;
			AD.type = 2;
			break;
		}
		case 40: // node stuff
		{
			AD.mode = 5;
			AD.type = 0;
			break;
		}
		case 41:
		{
			AD.mode = 5;
			AD.type = 1;
			break;
		}
		case 42:
		{
			AD.mode = 5;
			AD.type = 2;
			break;
		}
		case 50:
		{
			AD.mode = ACTIONMODE_MAPTOOL;
			AD.tool.reset(new AddTubeTool(*Map, *((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview, true));
			break;
		}
		case 51:
		{
			AD.mode = ACTIONMODE_MAPTOOL;
			AD.tool.reset(new ModifyTubeTool(*Map, *((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview, true));
			break;
		}
		case 52:
		{
			AD.mode = ACTIONMODE_MAPTOOL;
			AD.tool.reset(new AddTubeTool(*Map, *((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview, false));
			break;
		}
		case 53:
		{
			AD.mode = ACTIONMODE_MAPTOOL;
			AD.tool.reset(new ModifyTubeTool(*Map, *((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview, false));
			break;
		}
		case 54:
		{
			AD.mode = ACTIONMODE_MAPTOOL;
			AD.tool.reset(new RemoveTubeTool(*Map, *((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview));
			break;
		}

		case 61:
			if (!tiledata_count) break;
			AD.type = 0;
			AD.mode = ACTIONMODE_SETTILE;
			AD.data = 0;
			AD.z_data = 0;
			HandleBrushSize(0);
			break;

		case 62:
			int i;
			if (!tiledata_count) {
				break;
			}
			for (i = 0; i < (*tiledata_count); i++) {
				if ((*tiledata)[i].wTileSet == tiles->GetInteger("General", "SandTile")) {
					break;
				}
			}
			AD.type = i;
			AD.mode = ACTIONMODE_SETTILE;
			AD.data = 0;
			AD.z_data = 0;
			HandleBrushSize(i);
			break;
		case 63:
			if (!tiledata_count) {
				break;
			}
			for (i = 0; i < (*tiledata_count); i++) {
				if ((*tiledata)[i].wTileSet == tiles->GetInteger("General", "RoughTile")) {
					break;
				}
			}
			AD.type = i;
			AD.mode = ACTIONMODE_SETTILE;
			AD.data = 0;
			AD.z_data = 0;
			HandleBrushSize(i);
			break;
		case 64:
			if (!tiledata_count) break;
			for (i = 0; i < (*tiledata_count); i++)
				if ((*tiledata)[i].wTileSet == waterset) break;

			if (((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_x < 2 ||
				((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_y < 2) {

				((CFinalSunDlg*)theApp.m_pMainWnd)->m_settingsbar.m_BrushSize = 1;
				((CFinalSunDlg*)theApp.m_pMainWnd)->m_settingsbar.UpdateData(FALSE);
				((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_x = 2;
				((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_y = 2;
			}

			AD.type = i;
			AD.mode = ACTIONMODE_SETTILE;
			AD.data = 1;	 // use water placement logic
			AD.z_data = 0;
			break;
		case 65:
			if (!tiledata_count) {
				break;
			}
			for (i = 0; i < (*tiledata_count); i++) {
				if ((*tiledata)[i].wTileSet == tiles->GetInteger("General", "GreenTile")) {
					break;
				}
			}
			AD.type = i;
			AD.mode = ACTIONMODE_SETTILE;
			AD.data = 0;
			AD.z_data = 0;
			HandleBrushSize(i);
			break;
		case 66:
			if (!tiledata_count) {
				break;
			}
			for (i = 0; i < (*tiledata_count); i++) {
				if ((*tiledata)[i].wTileSet == tiles->GetInteger("General", "PaveTile")) {
					break;
				}
			}
			AD.type = i;
			AD.mode = ACTIONMODE_SETTILE;
			AD.data = 0;
			AD.z_data = 0;
			HandleBrushSize(i);
			break;
		case 67:
			if (!tiledata_count) {
				break;
			}
			for (i = 0; i < (*tiledata_count); i++) {
				if ((*tiledata)[i].wTileSet == g_data.GetInteger("NewUrbanInfo", "Morphable2")) {
					break;
				}
			}
			AD.type = i;
			AD.mode = ACTIONMODE_SETTILE;
			AD.data = 0;
			AD.z_data = 0;
			HandleBrushSize(i);
			break;


		}
	} else {
		int subpos = val % valadded;
		int pos = val / valadded;

		AD.mode = 1;
		AD.type = pos;
		AD.data = subpos;

		if (pos == 1) {
			CString sec = "InfantryTypes";

			if (subpos < rules[sec].Size()) {
				// standard unit!

				AD.data_s = rules[sec].Nth(subpos).second;
			} else {

				AD.data_s = ini[sec].Nth(subpos - rules[sec].Size()).second;
			}
		} else if (pos == 2) {
			CString sec = "BuildingTypes";

			if (subpos < rules[sec].Size()) {
				// standard unit!
				AD.data_s = rules[sec].Nth(subpos).second;
			} else {
				AD.data_s = ini[sec].Nth(subpos - rules[sec].Size()).second;
			}
		} else if (pos == 3) {
			CString sec = "AircraftTypes";

			if (subpos < rules[sec].Size()) {
				// standard unit!

				AD.data_s = rules[sec].Nth(subpos).second;
			} else {
				AD.data_s = ini[sec].Nth(subpos - rules[sec].Size()).second;
			}
		} else if (pos == 4) {
			CString sec = "VehicleTypes";

			if (subpos < rules[sec].Size()) {
				// standard unit!
				AD.data_s = rules[sec].Nth(subpos).second;
			} else {
				AD.data_s = ini[sec].Nth(subpos - rules[sec].Size()).second;
			}
		} else if (pos == 5) {

			CString sec = "TerrainTypes";

			if (subpos == 999) {

				CRTPDlg dlg;
				if (dlg.DoModal() == IDOK) {
					AD.mode = ACTIONMODE_RANDOMTERRAIN;
				}
			} else {
				if (subpos < rules[sec].Size()) {
					// standard unit!
					AD.data_s = rules[sec].Nth(subpos).second;
				} else {
					AD.data_s = ini[sec].Nth(subpos - rules[sec].Size()).second;
				}
			}
		} else if (pos == 6) {
			if (subpos < 100) {
				// general overlay functions!
				if (subpos == 1) {
					AD.data = 31;
					AD.data2 = atoi(InputBox("Please enter the value (0-255) of the overlay. Don´t exceed this range.", "Set overlay manually"));

				} else if (subpos == 2) {
					AD.data = 32;
					AD.data2 = atoi(InputBox("Please enter the value (0-255) of the overlay-data. Don´t exceed this range.", "Set overlay manually"));

				}

			} else {
				AD.data2 = subpos % 100;
				AD.data = subpos / 100;
				if (AD.data >= 30) {
					AD.data = 30;
					AD.data2 = subpos % 1000;
				}
			}
		} else if (pos == 7) {
			// set owner
			//if(ini.find(MAPHOUSES)!=ini.end() && ini[MAPHOUSES].Size()>0)
			if (ini[MAPHOUSES].Size() > 0) {
				AD.data_s = ini[MAPHOUSES].Nth(subpos).second;
			} else {
				AD.data_s = rules[HOUSES].Nth(subpos).second;
			}

			currentOwner = AD.data_s;
		}
#ifdef SMUDGE_SUPP
		else if (pos == 8) {


			CString sec = "SmudgeTypes";

			if (subpos < rules[sec].Size()) {
				// standard unit!
				AD.data_s = rules[sec].Nth(subpos).second;
			} else {
				AD.data_s = ini[sec].Nth(subpos - rules[sec].Size()).second;
			}

		}
#endif


	}



	*pResult = 0;
}

__inline HTREEITEM TV_InsertItemW(HWND hWnd, WCHAR* lpString, int len, HTREEITEM hInsertAfter, HTREEITEM hParent, int param)
{
	if (!lpString) return NULL;

	TVINSERTSTRUCTW tvis;
	tvis.hInsertAfter = hInsertAfter;
	tvis.hParent = hParent;
	tvis.itemex.mask = TVIF_PARAM | TVIF_TEXT;
	tvis.itemex.cchTextMax = len;
	tvis.itemex.pszText = lpString;
	tvis.itemex.lParam = param;

	// MW 07/17/2001: Updated to use Ascii if Unicode fails:
	HTREEITEM res = (HTREEITEM)::SendMessage(hWnd, TVM_INSERTITEMW, 0, ((LPARAM)(&tvis)));

	if (!res) {
		// failed... Probably because of missing Unicode support

		// convert text to ascii, then add it
		BYTE* lpAscii = new(BYTE[len + 1]);
		BOOL bUsedDefault;
		memset(lpAscii, 0, len + 1);
		WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK,
			lpString, len + 1, (LPSTR)lpAscii, len + 1, NULL, &bUsedDefault);

		TVINSERTSTRUCT tvis;
		tvis.hInsertAfter = hInsertAfter;
		tvis.hParent = hParent;
		tvis.itemex.mask = TVIF_PARAM | TVIF_TEXT;
		tvis.itemex.cchTextMax = len;
		tvis.itemex.lParam = param;
		tvis.itemex.pszText = (char*)lpAscii;

		res = TreeView_InsertItem(hWnd, &tvis);

		delete[] lpAscii;
	}

	return res;
}

const IgnoreSet CollectIgnoreSet()
{
	IgnoreSet ret;
	auto addToIgnoreSet = [&ret](const CString& section) {
		for (auto& [seq, id] : g_data[section]) {
			ret.insert(std::string(id));
		}
	};

#if defined(RA2_MODE)
	addToIgnoreSet("IgnoreRA2");
	if (Map->GetTheater() == THEATER0) {
		addToIgnoreSet("IgnoreTemperateRA2");
	}
	if (Map->GetTheater() == THEATER1) {
		addToIgnoreSet("IgnoreSnowRA2");
	}
	if (Map->GetTheater() == THEATER2) {
		addToIgnoreSet("IgnoreUrbanRA2");
	}
	if (Map->GetTheater() == THEATER3) {
		addToIgnoreSet("IgnoreNewUrbanRA2");
	}
	if (Map->GetTheater() == THEATER4) {
		addToIgnoreSet("IgnoreLunarRA2");
	}
	if (Map->GetTheater() == THEATER5) {
		addToIgnoreSet("IgnoreDesertRA2");
	}
#else
	addToIgnoreSet("IgnoreTS");
	if (Map->GetTheater() == THEATER0) {
		addToIgnoreSet("IgnoreTemperateTS");
	}
	if (Map->GetTheater() == THEATER1) {
		addToIgnoreSet("IgnoreSnowTS");
	}
#endif
	return ret;
}

TreeViewBuilder::mapSideNodeInfo TreeViewBuilder::collectCategoryInfo()
{
	mapSideNodeInfo ret;

	auto toType = [](const CString& str) -> TechnoTypeMask {
		if (str.IsEmpty()) {
			return TechnoTypeMask(-1);
		}
		return TechnoTypeMask(atoi(str));
	};

	CString typeStr;
	auto count = 0;

	auto const otherCategoryName = GetLanguageStringACP("Other");

	for (auto& [seq, def] : g_data["Sides"]) {
		auto sideName = def;
		auto const commaPos = def.Find(',');
		//now parse real type
		if (commaPos >= 0) {
			sideName = def.Mid(0, commaPos);
			typeStr = def.Mid(commaPos + 1);
		}
		if (typeStr == otherCategoryName) {
			continue;
		}

		auto&& info = CatetoryDefinition{ TranslateStringACP(sideName), toType(typeStr) };
		ret.insert({ count++, info });
	}

	// now parse side definition in Ini if it has more than editor setting
	auto const& mmh = IniMegaFile::GetRules();
	auto const& sides = mmh.GetSection("Sides");
	count = 0;
	for (auto it = sides.begin(); it != sides.end(); ++it) {
		// skip already defined ones
		if (count++ < ret.size()) {
			continue;
		}
		ret.insert({ count - 1, CatetoryDefinition{ (*it).first, TechnoTypeMask(-1) } });
	}

	// insert other first
	ret.insert({ -1, CatetoryDefinition { otherCategoryName, TechnoTypeMask(-1) } });

	return ret;
}

const TreeViewBuilder::houseMap TreeViewBuilder::collectOwners()
{
	houseMap ret;
	auto const& mmh = IniMegaFile::GetRules();
	auto const& sides = mmh.GetSection("Sides");
	size_t i = 0;
	for (auto it = sides.begin(); it != sides.end(); ++it) {
		for (auto const& owner : utilities::split_string((*it).second)) {
			ret[(std::string)owner] = i;
		}
		++i;
	}
	return ret;
}

const CString& GuessSideHelper::GetSideName(const CString& regName, TreeViewTechnoType technoType, const CIniFile& inWhichIni)
{
	auto const sideIdx = GuessSide(regName, technoType, inWhichIni);
	auto const& [name, mask] = builder.sideInfo.at(sideIdx);
	if (mask & technoType) {
		return name;
	}
	// -1 means other
	return builder.sideInfo.at(-1).CategoryName;
}

int GuessSideHelper::GuessSide(const CString& regName, TreeViewTechnoType type, const CIniFile& inWhichIni)
{
	auto const& knownIterator = KnownItem.find(regName.operator LPCSTR());
	if (knownIterator != KnownItem.end())
		return knownIterator->second;

	int result = -1;
	switch (type) {
	case TreeViewTechnoType::Set_None:
	default:
		break;
	case TreeViewTechnoType::Building:
		result = guessBuildingSide(regName, inWhichIni);
		break;
	case TreeViewTechnoType::Infantry:
	case TreeViewTechnoType::Vehicle:
	case TreeViewTechnoType::Aircraft:
		result = guessGenericSide(regName);
		break;
	}
	KnownItem.insert_or_assign(std::string(regName), result);
	return result;
}

int GuessSideHelper::guessBuildingSide(const CString& typeId, const CIniFile& inWhichIni)
{
	int planning;
	planning = inWhichIni.GetInteger(typeId, "AIBasePlanningSide", -1);
	if (planning >= 0) {
		if (planning >= rules.GetSection("Sides").Size()) {
			planning = -1;
		}
		return planning;
	}
	auto const& cons = utilities::split_string(rules.GetString("AI", "BuildConst"));
	size_t i = 0;
	for (; i < cons.size(); ++i) {
		if (cons[i] == typeId) {
			return i;
		}
	}
	if (i >= rules.GetSection("Sides").Size()) {
		return -1;
	}
	return guessGenericSide(typeId);
}

int GuessSideHelper::guessGenericSide(const CString& regName)
{
	auto const& mmh = IniMegaFile::GetRules();
	auto const& owners = utilities::split_string(mmh.GetString(regName, "Owner"));
	if (owners.size() <= 0) {
		return -1;
	}

	auto const& itr = builder.m_owners.find((std::string)owners[0]);
	if (itr == builder.m_owners.end()) {
		return -1;
	}
	return itr->second;
}

void TreeViewBuilder::updateBuildingTypes(HTREEITEM parentNode) {
	TreeViewCategoryHandler structhouses(this->tree, parentNode);
	GuessSideHelper sideHelper(*this);
	auto baseOffset = valadded * 2;

	structhouses.Preallocate(sideInfo, TreeViewTechnoType::Building);

	auto const& bldTypeSec = rules["BuildingTypes"];
	for (auto i = 0; i < bldTypeSec.Size(); i++) {

		auto const& unitname = bldTypeSec.Nth(i).second;

		if (unitname.IsEmpty()) {
			continue;
		}

		if (m_ignoreSet.find((std::string)unitname) != m_ignoreSet.end()) {
			continue;
		}

		if (!g_data.GetBool("Debug", "ShowBuildingsWithToTile") && !rules[unitname]["ToTile"].IsEmpty()) {
			continue;
		}

		auto const unitDisplayName = Map->GetUnitDisplayName(unitname);
		if (!unitDisplayName) {
			continue;
		}

		int id = Map->GetBuildingID(unitname);
		if (id < 0 /*|| (buildinginfo[id].pic[0].bTerrain!=0 && buildinginfo[id].pic[0].bTerrain!=needed_terrain)*/) {
			continue;
		}

		if (theater == THEATER0 && !buildinginfo[id].bTemp) { /*MessageBox("Ignored", unitname,0);*/ continue; }
		if (theater == THEATER1 && !buildinginfo[id].bSnow) { /*MessageBox("Ignored", unitname,0);*/ continue; }
		if (theater == THEATER2 && !buildinginfo[id].bUrban) { /*MessageBox("Ignored", unitname,0);*/ continue; }

		
		CString addedString = unitname;
		addedString += ' ';
		addedString += unitDisplayName->cString;

		XCString addedStrW;
		addedStrW.SetString(addedString.GetString());

		auto const& name = sideHelper.GetSideName(unitname, TreeViewTechnoType::Building);
		TV_InsertItemW(tree.m_hWnd, 
			addedStrW.wString,
			addedStrW.len, TVI_LAST, structhouses.GetOrAdd(name), baseOffset + i);
	}

	// okay, now the user-defined types:
	baseOffset += rules["BuildingTypes"].Size();
	auto const& localbldTypeSec = ini["BuildingTypes"];
	for (auto i = 0; i < localbldTypeSec.Size(); i++) {
		auto const& typeId = localbldTypeSec.Nth(i).second;
		if (typeId.IsEmpty()) {
			continue;
		}
		int id = Map->GetBuildingID(typeId);
		if (id < 0 || (buildinginfo[id].pic[0].bTerrain != TheaterChar::None && buildinginfo[id].pic[0].bTerrain != needed_terrain)) {
			continue;
		}
		CString undefinedName;
		CString addedString;
		auto const& name = ini[typeId]["Name"];
		if (name.IsEmpty()) {
			undefinedName = typeId + " UNDEFINED";
			addedString = undefinedName;
		} else {
			addedString += typeId;
			addedString += ' ';
			addedString += name;
		}
		auto const& sideName = sideHelper.GetSideName(typeId, TreeViewTechnoType::Building);
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, addedString, 0, 0, 0, 0, baseOffset + i, structhouses.GetOrAdd(sideName), TVI_LAST);
	}
}

void TreeViewBuilder::updateUnitTypes(HTREEITEM parentNode, const char* typeListId, TreeViewTechnoType technoType, int multiple)
{
	TreeViewCategoryHandler structhouses(this->tree, parentNode);
	GuessSideHelper sideHelper(*this);
	auto baseOffset = valadded * multiple;

	structhouses.Preallocate(sideInfo, technoType);

	for (auto i = 0; i < rules[typeListId].Size(); i++) {
		auto const& typeId = rules[typeListId].Nth(i).second;
		if (typeId.IsEmpty()) {
			continue;
		}
		if (m_ignoreSet.find((std::string)typeId) != m_ignoreSet.end()) {
			continue;
		}
		WCHAR* addedString = Map->GetUnitName(typeId);
		if (!addedString) {
			continue;
		}
		auto const& name = sideHelper.GetSideName(typeId, technoType);
		TV_InsertItemW(tree.m_hWnd, addedString, wcslen(addedString), TVI_LAST, structhouses.GetOrAdd(name), baseOffset + i);
	}
	// okay, now the user-defined types:
	baseOffset += rules[typeListId].Size();
	auto const& infTypeSec = ini[typeListId];
	for (auto i = 0; i < infTypeSec.Size(); i++) {
		auto const& typeId = infTypeSec.Nth(i).second;
		if (typeId.IsEmpty()) {
			continue;
		}

		CString undefinedName;
		auto const& name = ini[typeId]["Name"];
		auto addedString = std::ref(name);
		auto const& sideName = sideHelper.GetSideName(typeId, technoType);
		if (name.IsEmpty()) {
			undefinedName = typeId + " UNDEFINED";
			addedString = undefinedName;
		}
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, addedString.get(), 0, 0, 0, 0, baseOffset + i, structhouses.GetOrAdd(sideName), TVI_LAST);
	}
}

void TreeViewBuilder::updateTechnoItems() {
	updateBuildingTypes(rootitems[TreeRoot::Building]);
	updateUnitTypes(rootitems[TreeRoot::Infantry], "InfantryTypes", TreeViewTechnoType::Infantry, 1);
	updateUnitTypes(rootitems[TreeRoot::Aircraft], "AircraftTypes", TreeViewTechnoType::Aircraft, 3);
	updateUnitTypes(rootitems[TreeRoot::Vehicle], "VehicleTypes", TreeViewTechnoType::Vehicle, 4);
}

void CViewObjects::UpdateDialog()
{
	OutputDebugString("Objectbrowser redrawn\n");

	CTreeCtrl& tree = GetTreeCtrl();
	CIniFile& ini = Map->GetIniFile();

	tree.Select(0, TVGN_CARET);
	tree.DeleteAllItems();

	CString sTreeRoots[15];
	sTreeRoots[TreeRoot::Infantry] = GetLanguageStringACP("InfantryObList");
	sTreeRoots[TreeRoot::Vehicle] = GetLanguageStringACP("VehiclesObList");
	sTreeRoots[TreeRoot::Aircraft] = GetLanguageStringACP("AircraftObList");
	sTreeRoots[TreeRoot::Building] = GetLanguageStringACP("StructuresObList");
	sTreeRoots[TreeRoot::Terrain] = GetLanguageStringACP("TerrainObList");
	sTreeRoots[TreeRoot::Overlay] = GetLanguageStringACP("OverlayObList");
	sTreeRoots[TreeRoot::Waypoint] = GetLanguageStringACP("WaypointsObList");
	sTreeRoots[TreeRoot::Celltag] = GetLanguageStringACP("CelltagsObList");
	sTreeRoots[TreeRoot::Basenode] = GetLanguageStringACP("BaseNodesObList");
	sTreeRoots[TreeRoot::Tunnel] = GetLanguageStringACP("TunnelObList");
	sTreeRoots[TreeRoot::Delete] = GetLanguageStringACP("DelObjObList");
	sTreeRoots[TreeRoot::Owner] = GetLanguageStringACP("ChangeOwnerObList");
	sTreeRoots[TreeRoot::PlayerLocation] = GetLanguageStringACP("StartpointsObList");
	sTreeRoots[TreeRoot::Ground] = GetLanguageStringACP("GroundObList");
	sTreeRoots[TreeRoot::Smudge] = GetLanguageStringACP("SmudgesObList");

	int i = 0;

	auto const translatedNoObj = GetLanguageStringACP("NothingObList");
	HTREEITEM first = tree.InsertItem(TVIF_PARAM | TVIF_TEXT,
		translatedNoObj, i, i, 0, 0, -2, TVI_ROOT, TVI_LAST);

	HTREEITEM rootitems[TreeRoot::Count];

	// we want the change owner at the top

	if (!Map->IsMultiplayer() || !theApp.m_Options.bEasy) {
		rootitems[TreeRoot::Owner] = tree.InsertItem(TVIF_PARAM | TVIF_TEXT,
			sTreeRoots[TreeRoot::Owner], i, i, 0, 0, i, TVI_ROOT, TVI_LAST);
	}


	for (int i = TreeRoot::Begin; i < TreeRoot::Delete; i++) {
		BOOL bAllow = TRUE;
		if (theApp.m_Options.bEasy) {
			if (i >= TreeRoot::Waypoint && i <= TreeRoot::Tunnel)
				bAllow = FALSE;
		}

		// no tunnels in ra2 mode
		if (editor_mode == ra2_mode && i == 9 && !g_data.GetBool("Debug", "AllowTunnels")) {
			bAllow = FALSE;
		}

		if (bAllow) {
			rootitems[i] = tree.InsertItem(TVIF_PARAM | TVIF_TEXT,
				sTreeRoots[i], i, i, 0, 0, i, TVI_ROOT, TVI_LAST);
		}
	}


	rootitems[TreeRoot::Ground] = tree.InsertItem(TVIF_PARAM | TVIF_TEXT, sTreeRoots[TreeRoot::Ground], 13, 13, 0, 0, 13, TVI_ROOT, first);

	rootitems[TreeRoot::PlayerLocation] = tree.InsertItem(TVIF_PARAM | TVIF_TEXT,
		sTreeRoots[TreeRoot::PlayerLocation], 12, 12, 0, 0, 12, TVI_ROOT, TVI_LAST);

	rootitems[TreeRoot::Delete] = tree.InsertItem(TVIF_PARAM | TVIF_TEXT,
		sTreeRoots[TreeRoot::Delete], 10, 10, 0, 0, 10, TVI_ROOT, TVI_LAST);

#ifdef SMUDGE_SUPP
	rootitems[TreeRoot::Smudge] = tree.InsertItem(TVIF_PARAM | TVIF_TEXT,
		sTreeRoots[TreeRoot::Smudge], 14, 14, 0, 0, 10, TVI_ROOT, rootitems[TreeRoot::Terrain]);
#endif





	if (!theApp.m_Options.bEasy) {
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("CreateWaypObList"), 0, 0, 0, 0, 20, rootitems[TreeRoot::Waypoint], TVI_LAST);
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("CreateSpecWaypObList"), 0, 0, 0, 0, 22, rootitems[TreeRoot::Waypoint], TVI_LAST);
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("DelWaypObList"), 0, 0, 0, 0, 21, rootitems[TreeRoot::Waypoint], TVI_LAST);
	}


	int e;
	int max = 8;
	//if(ini.find(HOUSES)!=ini.end() && ini.find(MAPHOUSES)!=ini.end())
	if (!Map->IsMultiplayer())
		max = 1;
	else {

	}
	for (e = 0; e < max; e++) {
		CString ins = GetLanguageStringACP("StartpointsPlayerObList");
		char c[50];
		itoa(e + 1, c, 10);
		ins = TranslateStringVariables(1, ins, c);
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, ins, 0, 0, 0, 0, 23 + e, rootitems[TreeRoot::PlayerLocation], TVI_LAST);
	}
	tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("StartpointsDelete"), 0, 0, 0, 0, 21, rootitems[TreeRoot::PlayerLocation], TVI_LAST);



	tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetTheaterLanguageString("GroundClearObList"), 0, 0, 0, 0, 61, rootitems[TreeRoot::Ground], TVI_LAST);
	tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetTheaterLanguageString("GroundSandObList"), 0, 0, 0, 0, 62, rootitems[TreeRoot::Ground], TVI_LAST);
	tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetTheaterLanguageString("GroundRoughObList"), 0, 0, 0, 0, 63, rootitems[TreeRoot::Ground], TVI_LAST);
	tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetTheaterLanguageString("GroundGreenObList"), 0, 0, 0, 0, 65, rootitems[TreeRoot::Ground], TVI_LAST);
	tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetTheaterLanguageString("GroundPaveObList"), 0, 0, 0, 0, 66, rootitems[TreeRoot::Ground], TVI_LAST);
	tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetTheaterLanguageString("GroundWaterObList"), 0, 0, 0, 0, 64, rootitems[TreeRoot::Ground], TVI_LAST);
#ifdef RA2_MODE
	if (Map->GetTheater() == THEATER3)
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetTheaterLanguageString("GroundPave2ObList"), 0, 0, 0, 0, 67, rootitems[TreeRoot::Ground], TVI_LAST);
#endif

	if (!theApp.m_Options.bEasy) {
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("CreateCelltagObList"), 0, 0, 0, 0, 36, rootitems[TreeRoot::Celltag], TVI_LAST);
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("DelCelltagObList"), 0, 0, 0, 0, 37, rootitems[TreeRoot::Celltag], TVI_LAST);
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("CelltagPropObList"), 0, 0, 0, 0, 38, rootitems[TreeRoot::Celltag], TVI_LAST);
	}

	if (!theApp.m_Options.bEasy) {
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("CreateNodeNoDelObList"), 0, 0, 0, 0, 40, rootitems[TreeRoot::Basenode], TVI_LAST);
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("CreateNodeDelObList"), 0, 0, 0, 0, 41, rootitems[TreeRoot::Basenode], TVI_LAST);
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("DelNodeObList"), 0, 0, 0, 0, 42, rootitems[TreeRoot::Basenode], TVI_LAST);
	}


	HTREEITEM deleteoverlay = tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("DelOvrlObList"), 0, 0, 0, 0, -1, rootitems[TreeRoot::Overlay], TVI_LAST);
	HTREEITEM tiberium = tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("GrTibObList"), 0, 0, 0, 0, -1, rootitems[TreeRoot::Overlay], TVI_LAST);
	//HTREEITEM bluetiberium=tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("BlTibObList"), 0,0,0,0, -1, rootitems[TreeRoot::Overlay], TVI_LAST );
#ifndef RA2_MODE
	HTREEITEM veinhole = tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("VeinholeObList"), 0, 0, 0, 0, -1, rootitems[TreeRoot::Overlay], TVI_LAST);
#endif
	HTREEITEM bridges = tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("BridgesObList"), 0, 0, 0, 0, -1, rootitems[TreeRoot::Overlay], TVI_LAST);
	HTREEITEM alloverlay = tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("OthObList"), 0, 0, 0, 0, -1, rootitems[TreeRoot::Overlay], TVI_LAST);
	HTREEITEM everyoverlay = NULL;

	if (!theApp.m_Options.bEasy) {
		everyoverlay = tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("AllObList"), 0, 0, 0, 0, -1, rootitems[TreeRoot::Overlay], TVI_LAST);
	}


	if (!theApp.m_Options.bEasy) {
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("OvrlManuallyObList"), 0, 0, 0, 0, valadded * 6 + 1, rootitems[TreeRoot::Overlay], TVI_LAST);
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("OvrlDataManuallyObList"), 0, 0, 0, 0, valadded * 6 + 2, rootitems[TreeRoot::Overlay], TVI_LAST);
	}

	tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("DelOvrl0ObList"), 0, 0, 0, 0, valadded * 6 + 100 + 0, deleteoverlay, TVI_LAST);
	tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("DelOvrl1ObList"), 0, 0, 0, 0, valadded * 6 + 100 + 1, deleteoverlay, TVI_LAST);
	tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("DelOvrl2ObList"), 0, 0, 0, 0, valadded * 6 + 100 + 2, deleteoverlay, TVI_LAST);
	tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("DelOvrl3ObList"), 0, 0, 0, 0, valadded * 6 + 100 + 3, deleteoverlay, TVI_LAST);

	//tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("DrawRanTibObList"), 0,0,0,0, valadded*6+200+0, tiberium, TVI_LAST );
	tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("DrawTibObList"), 0, 0, 0, 0, valadded * 6 + 200 + 10, tiberium, TVI_LAST);
	//tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("IncTibSizeObList"), 0,0,0,0, valadded*6+200+20, tiberium, TVI_LAST );
	//tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("DecTibSizeObList"), 0,0,0,0, valadded*6+200+21, tiberium, TVI_LAST );

	//tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("DrawRanTibObList"), 0,0,0,0, valadded*6+300+0, bluetiberium, TVI_LAST );
	tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("DrawTib2ObList"), 0, 0, 0, 0, valadded * 6 + 300 + 10, tiberium, TVI_LAST);
	//tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("IncTibSizeObList"), 0,0,0,0, valadded*6+300+20, bluetiberium, TVI_LAST );
	//tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("DecTibSizeObList"), 0,0,0,0, valadded*6+300+21, bluetiberium, TVI_LAST );
#ifndef RA2_MODE
	tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("VeinholeObList"), 0, 0, 0, 0, valadded * 6 + 400 + 0, veinhole, TVI_LAST);
	tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("VeinsObList"), 0, 0, 0, 0, valadded * 6 + 400 + 1, veinhole, TVI_LAST);
#endif

	if (Map->GetTheater() != THEATER4 && Map->GetTheater() != THEATER5) {
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("SmallBridgeObList"), 0, 0, 0, 0, valadded * 6 + 500 + 1, bridges, TVI_LAST);
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("BigBridgeObList"), 0, 0, 0, 0, valadded * 6 + 500 + 0, bridges, TVI_LAST);
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("BigTrackBridgeObList"), 0, 0, 0, 0, valadded * 6 + 500 + 2, bridges, TVI_LAST);
#ifdef RA2_MODE
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("SmallConcreteBridgeObList"), 0, 0, 0, 0, valadded * 6 + 500 + 3, bridges, TVI_LAST);
#endif
	} else {
		if (Map->GetTheater() == THEATER5) {
			tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("SmallBridgeObList"), 0, 0, 0, 0, valadded * 6 + 500 + 1, bridges, TVI_LAST);
#ifdef RA2_MODE
			tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("SmallConcreteBridgeObList"), 0, 0, 0, 0, valadded * 6 + 500 + 3, bridges, TVI_LAST);
#endif
		}

	}

#ifndef RA2_MODE
	if (!theApp.m_Options.bEasy && isTrue(g_data["Debug"]["AllowTunnels"])) {
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("NewTunnelObList"), 0, 0, 0, 0, 50, rootitems[TreeRoot::Tunnel], TVI_LAST);
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("ModifyTunnelObList"), 0, 0, 0, 0, 51, rootitems[TreeRoot::Tunnel], TVI_LAST);
		if (isTrue(g_data["Debug"]["AllowUnidirectionalTunnels"])) {
			tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("NewTunnelSingleObList"), 0, 0, 0, 0, 52, rootitems[TreeRoot::Tunnel], TVI_LAST);
			tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("ModifyTunnelSingleObList"), 0, 0, 0, 0, 53, rootitems[TreeRoot::Tunnel], TVI_LAST);
		}
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("DelTunnelObList"), 0, 0, 0, 0, 54, rootitems[TreeRoot::Tunnel], TVI_LAST);
	}
#else
	if (!theApp.m_Options.bEasy && g_data.GetBool("Debug", "AllowTunnels")) {
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("NewTunnelObList"), 0, 0, 0, 0, 50, rootitems[TreeRoot::Tunnel], TVI_LAST);
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("ModifyTunnelObList"), 0, 0, 0, 0, 51, rootitems[TreeRoot::Tunnel], TVI_LAST);
		if (g_data.GetBool("Debug", "AllowUnidirectionalTunnels")) {
			tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("NewTunnelSingleObList"), 0, 0, 0, 0, 52, rootitems[TreeRoot::Tunnel], TVI_LAST);
			tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("ModifyTunnelSingleObList"), 0, 0, 0, 0, 53, rootitems[TreeRoot::Tunnel], TVI_LAST);
		}
		tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("DelTunnelObList"), 0, 0, 0, 0, 54, rootitems[TreeRoot::Tunnel], TVI_LAST);
	}
#endif

	if (!theApp.m_Options.bEasy || !Map->IsMultiplayer()) {
		if (ini[MAPHOUSES].Size() > 0) {
			for (i = 0; i < ini[MAPHOUSES].Size(); i++) {
#ifdef RA2_MODE
				auto const& name = ini[MAPHOUSES].Nth(i).second;
				if (!name.CompareNoCase("nod") || !name.CompareNoCase("gdi")) {
					continue;
				}
#endif	

				tree.InsertItem(TVIF_PARAM | TVIF_TEXT, TranslateHouse(name, TRUE), 0, 0, 0, 0, valadded * 7 + i, rootitems[TreeRoot::Owner], TVI_LAST);
			}

		} else {
			for (i = 0; i < rules[HOUSES].Size(); i++) {
				//tree.InsertItem(TVIF_PARAM | TVIF_TEXT, CCStrings[*rules[HOUSES].Nth(i)].cString, 
				//0,0,0,0, valadded*7+i, rootitems[TreeRoot::Owner], TVI_LAST );
#ifdef RA2_MODE
				auto const& name = rules[HOUSES].Nth(i).second;
				if (!name.CompareNoCase("nod") || !name.CompareNoCase("gdi")) {
					continue;
				}
#endif	
				TV_InsertItemW(tree.m_hWnd, CCStrings[name].wString, CCStrings[name].len, TVI_LAST, rootitems[TreeRoot::Owner], valadded * 7 + i);
			}
		}
	} else {
		// change owner to neutral
		if (ini[MAPHOUSES].Size() > 0) {
			if (ini[MAPHOUSES].FindValue("Neutral") >= 0)
				currentOwner = "Neutral";
			else
				currentOwner = ini[MAPHOUSES].Nth(0).second;
		} else
			currentOwner = "Neutral";

	}


	for (i = 0; i < overlay_count; i++) {
		if (overlay_visible[i] && (!yr_only[i] || yuri_mode)) {
			if (!overlay_trdebug[i] || g_data.GetBool("Debug", "EnableTrackLogic"))
				tree.InsertItem(TVIF_PARAM | TVIF_TEXT, TranslateStringACP(overlay_name[i]), 0, 0, 0, 0, valadded * 6 + 3000 + overlay_number[i], alloverlay, TVI_LAST);
		}
	}

	auto const ignoreSet = CollectIgnoreSet();

	e = 0;
	if (!theApp.m_Options.bEasy) {
		for (i = 0; i < rules["OverlayTypes"].Size(); i++) {
			// it seems there is somewhere a bug that lists empty overlay ids... though they are not in the rules.ini
			// so this here is the workaround:
			auto const& unitname = rules["OverlayTypes"].Nth(i).second;
			auto id = unitname;
			//if(strchr(id,' ')!=NULL){ id[strchr(id,' ')-id;};		
			if (id.Find(' ') >= 0) {
				id = id.Left(id.Find(' '));
			}
			if (id.IsEmpty()) {
				continue;
			}
			if (ignoreSet.contains(std::string(unitname))) {
				continue;
			}
#ifdef RA2_MODE
			if ((i >= 39 && i <= 60) || (i >= 180 && i <= 201) || i == 239 || i == 178 || i == 167 || i == 126
				|| (i >= 122 && i <= 125) || i == 1 || (i >= 0x03 && i <= 0x17) || (i >= 0x3d && i <= 0x43)
				|| (i >= 0x4a && i <= 0x65) || (i >= 0xcd && i <= 0xec)) {
				if (!g_data.GetBool("Debug", "DisplayAllOverlay")) {
					e++;
					continue;
				}
			}
#endif

			CString val = unitname;
#ifdef RA2_MODE
			val.Replace("TIB", "ORE");
#endif
			tree.InsertItem(TVIF_PARAM | TVIF_TEXT, val, 0, 0, 0, 0, valadded * 6 + 3000 + e, everyoverlay, TVI_LAST);
			e++;
		}
	}

	CString theater = Map->GetTheater();


	auto needed_terrain = TheaterChar::None;
	if (tiledata == &s_tiledata) {
		needed_terrain = TheaterChar::A;
	} else if (tiledata == &t_tiledata) {
		needed_terrain = TheaterChar::T;
	}

	TreeViewBuilder b(tree, ini, ignoreSet, theater, needed_terrain, rootitems);

#ifdef RA2_MODE
	HTREEITEM hTrees = tree.InsertItem(GetLanguageStringACP("TreesObList"), rootitems[TreeRoot::Terrain], TVI_LAST);
	HTREEITEM hTL = tree.InsertItem(GetLanguageStringACP("TrafficLightsObList"), rootitems[TreeRoot::Terrain], TVI_LAST);
	HTREEITEM hSigns = tree.InsertItem(GetLanguageStringACP("SignsObList"), rootitems[TreeRoot::Terrain], TVI_LAST);
	HTREEITEM hLightPosts = tree.InsertItem(GetLanguageStringACP("LightPostsObList"), rootitems[TreeRoot::Terrain], TVI_LAST);
#endif

	// random tree placer
#ifdef RA2_MODE
	tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("RndTreeObList"), 0, 0, 0, 0, valadded * 5 + 999, hTrees, TVI_LAST);
#else
	tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP("RndTreeObList"), 0, 0, 0, 0, valadded * 5 + 999, rootitems[TreeRoot::Terrain], TVI_LAST);
#endif

	auto const& terrainTypeSec = rules["TerrainTypes"];
	for (i = 0; i < terrainTypeSec.Size(); i++) {
		auto const& unitname = terrainTypeSec.Nth(i).second;
		CString addedString = Map->GetUnitName(unitname);

		if (ignoreSet.contains(std::string(unitname))) {
			continue;
		}
#ifdef RA2_MODE
		if (g_data["IgnoreTerrainRA2"].FindValue(unitname) >= 0) continue;
#else
		if (g_data["IgnoreTerrainTS"].FindValue(unitname) >= 0) continue;
#endif

		addedString = TranslateStringACP(addedString);

		UINT flags = MF_STRING;

		HTREEITEM howner = rootitems[TreeRoot::Terrain];

#ifdef RA2_MODE
		if (unitname.Find("SIGN") >= 0) howner = hSigns;
		if (unitname.Find("TRFF") >= 0) howner = hTL;
		if (unitname.Find("TREE") >= 0) howner = hTrees;
		if (unitname.Find("LT") >= 0) howner = hLightPosts;
#endif

#ifdef RA2_MODE
		if (howner == hTrees) {
			int TreeMin = atoi(g_data[Map->GetTheater() + "Limits"]["TreeMin"]);
			int TreeMax = atoi(g_data[Map->GetTheater() + "Limits"]["TreeMax"]);

			CString id = unitname;
			id.Delete(0, 4);
			int n = atoi(id);

			if (n<TreeMin || n>TreeMax) continue;
		}
#endif

		if (unitname.GetLength() > 0 && unitname != "VEINTREE" && unitname.Find("ICE") < 0 && unitname.Find("BOXES") < 0 && unitname.Find("SPKR") < 0) // out with it :-)
			tree.InsertItem(TVIF_PARAM | TVIF_TEXT, (addedString + " (" + unitname + ")"), 0, 0, 0, 0, valadded * 5 + i, howner, TVI_LAST);
	}

#ifdef SMUDGE_SUPP
	auto const& smudgeTypeSec = rules["SmudgeTypes"];
	for (i = 0; i < smudgeTypeSec.Size(); i++) {
		auto const& unitname = smudgeTypeSec.Nth(i).second;
		CString addedString = unitname;

		if (ignoreSet.contains(std::string(unitname))) {
			continue;
		}

		addedString = TranslateStringACP(addedString);

		UINT flags = MF_STRING;

		HTREEITEM howner = rootitems[TreeRoot::Smudge];


		if (unitname.GetLength() > 0) {
			tree.InsertItem(TVIF_PARAM | TVIF_TEXT, unitname, 0, 0, 0, 0, valadded * 8 + i, howner, TVI_LAST);
		}
	}
#endif

	OutputDebugString("Objectbrowser redraw finished\n");
}

int CViewObjects::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	lpCreateStruct->style |= TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS;
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;


	return 0;
}

void CViewObjects::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();
}

void CViewObjects::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult)
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	*pResult = 0;
}

void CViewObjects::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen

	// CTreeView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CViewObjects::HandleBrushSize(int iTile)
{
	if (iTile >= *tiledata_count) {
		return;
	}

	for (auto const& [n, val] : g_data["StdBrushSize"]) {
		if ((*tiles)["General"].FindIndex(n) >= 0) {
			int tset = tiles->GetInteger("General", n);
			if (tset == (*tiledata)[iTile].wTileSet) {
				int bs = atoi(val);
				((CFinalSunDlg*)theApp.m_pMainWnd)->m_settingsbar.m_BrushSize = bs - 1;
				((CFinalSunDlg*)theApp.m_pMainWnd)->m_settingsbar.UpdateData(FALSE);
				((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_x = bs;
				((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_y = bs;
			}
		}
	}

}

void TreeViewCategoryHandler::Preallocate(const TreeViewBuilder::mapSideNodeInfo& sideInfo, TreeViewTechnoType type)
{
	for (auto const& [idx, def] : sideInfo) {
		if (idx < 0) {
			continue;
		}
		if (!(def.CategoryMask & type)) {
			continue;
		}
		GetOrAdd(def.CategoryName);
	}
}

HTREEITEM TreeViewCategoryHandler::GetOrAdd(const CString& name)
{
	auto&& nameStd = std::string(name);
	auto const it = structhouses.find(nameStd);
	if (it != structhouses.end()) {
		return it->second;
	}
	auto newItem = tree.InsertItem(TVIF_PARAM | TVIF_TEXT, GetLanguageStringACP(name), 0, 0, 0, 0, -1, parentNode, TVI_LAST);
	structhouses.insert({ std::move(nameStd), newItem });
	return newItem;
}