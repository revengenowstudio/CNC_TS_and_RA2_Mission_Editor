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

// standard functions

#include "stdafx.h"
#include "FinalSunDlg.h"
#include "TSoptions.h"
#include "variables.h"
#include "functions.h"
#include "inlines.h"
#include "mmsystem.h"
#include "IniMega.h"

#include <algorithm>

#define DBG
#undef DBG

#define DBG2
#undef DBG2

bool isValidUtf8(const char* utf8)
{
	// wstring_convert and codecvt_utf8_utf16 are deprecated in C++17, fallback to Win32
	auto utf8Count = strlen(utf8);
	if (utf8Count == 0)
		return true;

	// unterminatedCountWChars will be the count of WChars NOT including the terminating zero (due to passing in utf8.size() instead of -1)
	auto unterminatedCountWChars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, utf8Count, nullptr, 0);
	return unterminatedCountWChars > 0;
}

std::wstring utf8ToUtf16(const std::string& utf8)
{
	// wstring_convert and codecvt_utf8_utf16 are deprecated in C++17, fallback to Win32
	if (utf8.size() == 0)
		// MultiByteToWideChar does not support passing in cbMultiByte == 0
		return L"";

	// unterminatedCountWChars will be the count of WChars NOT including the terminating zero (due to passing in utf8.size() instead of -1)
	auto utf8Count = utf8.size();
	auto unterminatedCountWChars = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, utf8.data(), utf8Count, nullptr, 0);
	if (unterminatedCountWChars == 0) {
		throw std::runtime_error("UTF8 -> UTF16 conversion failed");
	}

	std::wstring utf16;
	utf16.resize(unterminatedCountWChars);
	if (MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, utf8.data(), utf8Count, utf16.data(), unterminatedCountWChars) == 0) {
		throw std::runtime_error("UTF8 -> UTF16 conversion failed");
	}
	return utf16;
}

std::wstring utf8ToUtf16(const char* utf8)
{
	// wstring_convert and codecvt_utf8_utf16 are deprecated in C++17, fallback to Win32
	auto utf8Count = strlen(utf8);
	if (utf8Count == 0)
		// MultiByteToWideChar does not support passing in cbMultiByte == 0
		return L"";

	// unterminatedCountWChars will be the count of WChars NOT including the terminating zero (due to passing in utf8.size() instead of -1)
	auto unterminatedCountWChars = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, utf8, utf8Count, nullptr, 0);
	if (unterminatedCountWChars == 0) {
		throw std::runtime_error("UTF8 -> UTF16 conversion failed");
	}

	std::wstring utf16;
	utf16.resize(unterminatedCountWChars);
	if (MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, utf8, utf8Count, utf16.data(), unterminatedCountWChars) == 0) {
		throw std::runtime_error("UTF8 -> UTF16 conversion failed");
	}
	return utf16;
}

std::string utf16ToCP(const std::wstring& utf16, int CP)
{
	// wstring_convert and codecvt_utf8_utf16 are deprecated in C++17, fallback to Win32
	if (utf16.size() == 0)
		// WideCharToMultiByte does not support passing in cbMultiByte == 0
		return "";

	// unterminatedCountWChars will be the count of WChars NOT including the terminating zero (due to passing in utf8.size() instead of -1)
	auto utf16Count = utf16.size();
	auto unterminatedCountChars = WideCharToMultiByte(CP, CP == CP_UTF8 ? WC_ERR_INVALID_CHARS : 0, utf16.data(), utf16Count, nullptr, 0, nullptr, nullptr);
	if (unterminatedCountChars == 0) {
		throw std::runtime_error(CP == CP_UTF8 ? "UTF16 -> UTF8 conversion failed" : "UTF16 -> MultiByte conversion failed");
	}

	std::string cps;
	cps.resize(unterminatedCountChars);
	if (WideCharToMultiByte(CP, CP == CP_UTF8 ? WC_ERR_INVALID_CHARS : 0, utf16.data(), utf16Count, cps.data(), unterminatedCountChars, nullptr, nullptr) == 0) {
		throw std::runtime_error(CP == CP_UTF8 ? "UTF16 -> UTF8 conversion failed" : "UTF16 -> MultiByte conversion failed");
	}
	return cps;
}

std::string utf16ToUtf8(const std::wstring& utf16)
{
	return utf16ToCP(utf16, CP_UTF8);
}

std::string utf16ToACP(const std::wstring& utf16)
{
	return utf16ToCP(utf16, CP_ACP);
}

// strcpy for overlapping strings
char* strcpy_safe(char* strDestination, const char* strSource)
{
	/*char* buffer=new(char[strlen(strSource)+1]);
	strcpy(buffer, strSource);

	strcpy(strDestination, buffer);*/

	int len = strlen(strSource) + 1;
	memmove(strDestination, strSource, len);

	return strDestination;
}

CString TranslateHouse(CString original, BOOL bToUI)
{
#ifdef RA2_MODE
	if (bToUI) {
		// CCStrings[*rules.sections[HOUSES].GetValue(i)].wString
		for (auto const& pair : rules.GetSection(HOUSES)) {
			original.Replace(pair.second, CCStrings[pair.second].cString);
		}
	} else {
		for (auto const& pair : rules.GetSection(HOUSES)) {
			original.Replace(CCStrings[pair.second].cString, pair.second);
		}
	}
#endif
	return original;
}


bool deleteFile(const std::string& u8FilePath)
{
	return DeleteFileW(utf8ToUtf16(u8FilePath).c_str()) ? true : false;
}

// set the status bar text in the main dialog
void SetMainStatusBar(const char* text)
{
	CFinalSunDlg* dlg = (CFinalSunDlg*)theApp.GetMainWnd();
	dlg->SetText(text);
}

// set the status bar text in the main dialog to ready
void SetMainStatusBarReady()
{
	CFinalSunDlg* dlg = (CFinalSunDlg*)theApp.GetMainWnd();
	dlg->SetReady();;
}

// Should not be required anymore
bool RepairTrigger(CString& triggerdata)
{
	if (GetParam(triggerdata, 3).IsEmpty()) {
		triggerdata = SetParam(triggerdata, 3, "0");
		return true;
	}
	if (GetParam(triggerdata, 4).IsEmpty()) {
		triggerdata = SetParam(triggerdata, 4, "1");
		return true;
	}
	if (GetParam(triggerdata, 5).IsEmpty()) {
		triggerdata = SetParam(triggerdata, 5, "1");
		return true;
	}
	if (GetParam(triggerdata, 6).IsEmpty()) {
		triggerdata = SetParam(triggerdata, 6, "1");
		return true;
	}
	if (GetParam(triggerdata, 7).IsEmpty()) {
		triggerdata = SetParam(triggerdata, 7, "0");
		return true;
	}
	return false;
}

// make some UI noise
void Sound(int ID)
{
	if (theApp.m_Options.bNoSounds) return;

	if (ID == SOUND_NONE) return;

	LPCSTR lpSound = NULL;

	if (ID == SOUND_POSITIVE) {
		lpSound = MAKEINTRESOURCE(IDR_WAVE1);
	} else if (ID == SOUND_NEGATIVE)
		lpSound = MAKEINTRESOURCE(IDR_WAVE2);
	else if (ID == SOUND_LAYDOWNTILE)
		lpSound = MAKEINTRESOURCE(IDR_WAVE3);


	if (lpSound) {
		PlaySound(lpSound, GetModuleHandle(NULL), SND_ASYNC | SND_RESOURCE);
	}
}

void HandleParamList(CComboBox& cb, int type)
{
	CString oldText;
	cb.GetWindowText(oldText);

	switch (type) {
		case PARAMTYPE_NOTHING:
		{
			while (cb.DeleteString(0) != CB_ERR);
			cb.SetWindowText(oldText);


			//cb.AddString("0");
		}
		break;
		case PARAMTYPE_HOUSES:
			ListHouses(cb, TRUE, TRUE, TRUE);
			break;
		case PARAMTYPE_WAYPOINTS:
			ListWaypoints(cb);
			break;
		case PARAMTYPE_TEAMTYPES:
			ListTeamTypes(cb, FALSE);
			break;
		case PARAMTYPE_UNITTYPES:
			ListUnits(cb);
			break;
		case PARAMTYPE_INFANTRYTYPES:
			ListInfantry(cb);
			break;
		case PARAMTYPE_AIRCRAFTTYPES:
			ListAircraft(cb);
			break;
		case PARAMTYPE_BUILDINGTYPES:
			ListBuildings(cb);
			break;
		case PARAMTYPE_VIDEOS:
			ListMovies(cb, FALSE, TRUE);
			break;
		case PARAMTYPE_TUTORIALTEXTS:
			ListTutorial(cb);
			break;
		case PARAMTYPE_TRIGGERS:
			ListTriggers(cb);
			break;
		case PARAMTYPE_YESNO:
			ListYesNo(cb);
			break;
		case PARAMTYPE_SOUNDS:
			ListSounds(cb);
			break;
		case PARAMTYPE_THEMES:
			ListThemes(cb);
			break;
		case PARAMTYPE_SPEECHES:
			ListSpeeches(cb);
			break;
		case PARAMTYPE_SPECIALWEAPONS:
			ListSpecialWeapons(cb);
			break;
		case PARAMTYPE_ANIMATIONS:
			ListAnimations(cb);
			break;
		case PARAMTYPE_PARTICLES:
			ListParticles(cb);
			break;
		case PARAMTYPE_CRATETYPES:
			ListCrateTypes(cb);
			break;
		case PARAMTYPE_SPEECHBUBBLETYPES:
			ListSpeechBubbleTypes(cb);
			break;
		case PARAMTYPE_TAGS:
			ListTags(cb, false);
			break;
		case PARAMTYPE_GLOBALS:
			ListMapVariables(cb);
			break;
		case PARAMTYPE_RULESGLOBALS:
			ListRulesGlobals(cb);
			break;
		case PARAMTYPE_BUILDINGTYPESINI:
			ListBuildings(cb, true);
			break;
		case PARAMTYPE_TECHTYPES:
			ListTechtypes(cb);
			break;
		case PRAMTYPE_TECHNOTYPES_NUM:
			ListTechtypes(cb, true);
	}
}

void ShowOptionsDialog(CIniFile& optIni)
{
	// show the options dialog, and save the options.

#ifdef RA2_MODE
	CString game = "RA2";
	CString app = "FinalAlert";
#else
	CString game = "TS";
	CString app = "FinalSun";
#endif

	std::string iniFile = "";
	iniFile = u8AppDataPath;
#ifndef RA2_MODE
	iniFile += "\\FinalSun.ini";
#else
	iniFile += "\\FinalAlert.ini";
#endif
	optIni.LoadFile(iniFile);
	CTSOptions opt;
	opt.m_TSEXE = theApp.m_Options.TSExe;
	if (opt.DoModal() == IDCANCEL) {
		return;
	}
	theApp.m_Options.TSExe = opt.m_TSEXE;
	optIni.SetString(game, "Exe", theApp.m_Options.TSExe);
	optIni.SetString(app, "Language", opt.m_LanguageName);

	BOOL bOldSearch = theApp.m_Options.bSearchLikeTS;
	if (!(opt.m_LikeTS == 1)) {
		optIni.SetString(app, "FileSearchLikeGame", "yes");
		theApp.m_Options.bSearchLikeTS = TRUE;
	} else {
		theApp.m_Options.bSearchLikeTS = FALSE;
		optIni.SetString(app, "FileSearchLikeGame", "no");
	}

	auto bOldPreferLocalTheaterFiles = theApp.m_Options.bPreferLocalTheaterFiles;
	theApp.m_Options.bPreferLocalTheaterFiles = opt.m_PreferLocalTheaterFiles ? true : false;
	optIni.SetString(app, "PreferLocalTheaterFiles", theApp.m_Options.bPreferLocalTheaterFiles ? "1" : "0");


	if (
		(
			(bOldPreferLocalTheaterFiles != theApp.m_Options.bPreferLocalTheaterFiles) ||
			(bOldSearch != theApp.m_Options.bSearchLikeTS)
			) && bOptionsStartup == FALSE) {
		MessageBox(0, GetLanguageStringACP("RestartNeeded"), "Restart", 0);
	}


	CString oldLang = theApp.m_Options.LanguageName;
	theApp.m_Options.LanguageName = opt.m_LanguageName;
	if (oldLang != theApp.m_Options.LanguageName && theApp.m_pMainWnd != NULL && theApp.m_pMainWnd->m_hWnd != NULL) {
		((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateStrings();
	}
	optIni.SaveFile(iniFile);
}


BOOL DoesFileExist(LPCSTR szFile)
{
	std::wstring file = utf8ToUtf16(szFile);
	HANDLE hFound = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, 0, NULL);

	if (hFound != INVALID_HANDLE_VALUE) {
		CloseHandle(hFound);
		return TRUE;
	}
	return FALSE;
}

CString ToACP(const CString& utf8)
{
	// convert to process codepage (should be UTF8 on newer systems)
	auto cp = GetACP();
	return (cp == CP_UTF8) ? utf8 : CString(utf16ToACP(utf8ToUtf16(utf8)).c_str());
}

// change all %n (where n is an int) in an string, to another specified string
CString TranslateStringVariables(int n, const char* originaltext, const char* inserttext)
{
	char c[50];
	_itoa_s(n, c, 10);

	char seekedstring[50];
	seekedstring[0] = '%';
	seekedstring[1] = 0;
	strcat_s(seekedstring, c);

	CString orig = originaltext;
	if (orig.Find(seekedstring) < 0) {
		return orig;
	}

	orig.Replace(seekedstring, inserttext);

	return orig;
}

const CString* getLanguageString(const CString& key)
{
#ifdef RA2_MODE
	auto const strRA2Sec = theApp.m_Options.LanguageName + "-StringsRA2";
	if (auto const& translated = language.GetSection(strRA2Sec).TryGetString(key)) {
		return translated;
	}
	// fallback to english
	if (auto const& def = language.GetSection("English-StringsRA2").TryGetString(key)) {
		return def;
	}
#endif
	// last try
	auto const defSec = theApp.m_Options.LanguageName + "-Strings";
	if (auto const& def = language.GetSection(defSec).TryGetString(key)) {
		return def;
	}
	// last try
	return language.GetSection("English-Strings").TryGetString(key);
}

CString EscapeString(const CString& input)
{
	auto updated = input;
	if (updated.Find("\\n")) {
		updated.Replace("\\n", "\n");
	}
	return updated;
}

// retrieve the string name in the correct language (name is an ID).
CString GetLanguageStringACP(const CString name)
{
#ifndef RA2_MODE
	auto const pStrToInsert = "FinalSun";
#elif YR_MODE
	auto const pStrToInsert = "FinalAlert 2: Yuri's Revenge";
#else
	auto const pStrToInsert = "FinalAlert 2";
#endif

	auto translated = getLanguageString(name);
	if (!translated) {
		CString encoded = name;
		encoded = TranslateStringVariables(9, encoded, pStrToInsert);
		return ToACP(encoded);
	}

	CString encoded;
	encoded = *translated;

	if (encoded.Find("%9") >= 0) {
		encoded = TranslateStringVariables(9, encoded, pStrToInsert);
	}
	return ToACP(encoded);
}

CString TranslateStringACP(WCHAR* u16EnglishString)
{
	return TranslateStringACP(utf16ToUtf8(u16EnglishString).c_str());
}

// tranlate a string/word by using the table from english to the current language
CString TranslateStringACP(CString u8EnglishString)
{
	if (!isValidUtf8(u8EnglishString)) {
		errstream << "TranslateStringACP(\"" << u8EnglishString << "\") called with an invalid UTF-8 string" << std::endl;
		return u8EnglishString;
	}
	return GetLanguageStringACP(u8EnglishString);
}

void TranslateDlgItem(CWnd& cwnd, int controlID, const CString& label)
{
	auto const translated = getLanguageString(label);
	if (translated) {
		cwnd.SetDlgItemText(controlID, EscapeString(*translated));
	}
}

void TranslateWindowCaption(CWnd& cwnd, const CString& label)
{
	auto const translated = getLanguageString(label);
	if (translated) {
		cwnd.SetWindowText(EscapeString(*translated));
	}
}

void TruncSpace(string& str)
{
	CString cstr = str.data();
	TruncSpace(cstr);
	str = cstr;
}
void TruncSpace(CString& str)
{
	str.TrimLeft();
	str.TrimRight();
	auto const spacePos = str.Find(" ");
	if (spacePos >= 0) {
		str.Delete(spacePos, str.GetLength() - spacePos);
	}
}

CString GetText(CWnd* wnd) {
	CString str;
	wnd->GetWindowText(str);
	return str;
}

CString GetText(CSliderCtrl* wnd)
{
	int v = wnd->GetPos();
	char c[150];
	_itoa_s(v, c, 10);
	return(c);
}

CString GetText(CComboBox* wnd) {

	CString str;
	if (wnd->GetCurSel() != -1) {
		wnd->GetLBText(wnd->GetCurSel(), str);
		return str;
	}

	wnd->GetWindowText(str);

	return(str);
}



void Info(const char* data, string& house, string& type, int& strength, int& x, int& y, string& other)
{
	other = "";
	house = GetParam(data, 0);
	type = GetParam(data, 1);
	strength = atoi(GetParam(data, 2));
	y = atoi(GetParam(data, 3));
	x = atoi(GetParam(data, 4));

	CString tmp;
	BOOL takeABreak = FALSE;
	int i = 1;
	do {

		tmp = GetParam(data, 4 + i);
		//MessageBox(0,tmp.data(),"",0);
		if (tmp != "") {
			other += ",";
			other += tmp;
		} else {
			takeABreak = TRUE;
			break;
		}
		i++;
	} while (takeABreak == FALSE);
};

void UnitInfo(const char* data, string& house, string& type, int& strength, int& x, int& y, int& direction, string& action, string& actiontrigger, int& u1, int& u2, int& u3, int& u4, int& u5, int& u6)
{
	house = GetParam(data, 0);
	type = GetParam(data, 1);
	strength = atoi(GetParam(data, 2));
	y = atoi(GetParam(data, 3));
	x = atoi(GetParam(data, 4));
	direction = atoi(GetParam(data, 5));
	action = GetParam(data, 6);
	actiontrigger = GetParam(data, 7);
	u1 = atoi(GetParam(data, 8));
	u2 = atoi(GetParam(data, 9));
	u3 = atoi(GetParam(data, 10));
	u4 = atoi(GetParam(data, 11));
	u5 = atoi(GetParam(data, 12));
	u6 = atoi(GetParam(data, 13));
}

void AirInfo(const char* data, string& house, string& type, int& strength, int& x, int& y, int& direction, string& action, string& actiontrigger, int& u1, int& u2, int& u3, int& u4)
{
	house = GetParam(data, 0);
	type = GetParam(data, 1);
	strength = atoi(GetParam(data, 2));
	y = atoi(GetParam(data, 3));
	x = atoi(GetParam(data, 4));
	direction = atoi(GetParam(data, 5));
	action = GetParam(data, 6);
	actiontrigger = GetParam(data, 7);
	u1 = atoi(GetParam(data, 8));
	u2 = atoi(GetParam(data, 9));
	u3 = atoi(GetParam(data, 10));
	u4 = atoi(GetParam(data, 11));
}



void InfanteryInfo(const char* data, string& house, string& type, int& strength, int& x, int& y, int& pos, string& action, int& direction, string& actiontrigger, int& u1, int& u2, int& u3, int& u4, int& u5)
{
	house = GetParam(data, 0);
	type = GetParam(data, 1);
	strength = atoi(GetParam(data, 2));
	y = atoi(GetParam(data, 3));
	x = atoi(GetParam(data, 4));
	pos = atoi(GetParam(data, 5));
	action = GetParam(data, 6);
	direction = atoi(GetParam(data, 7));
	actiontrigger = GetParam(data, 8);
	u1 = atoi(GetParam(data, 9));
	u2 = atoi(GetParam(data, 10));
	u3 = atoi(GetParam(data, 11));
	u4 = atoi(GetParam(data, 12));
	u5 = atoi(GetParam(data, 12));
}

void StructureInfo(const char* data, string& house, string& type, int& strength, int& x, int& y, int& direction, string& action, int& u1, int& u2, int& energy, int& upgrades, int& u5, string& upgrade1, string& upgrade2, string& upgrade3, int& u9, int& u10)
{
	house = GetParam(data, 0);
	type = GetParam(data, 1);
	strength = atoi(GetParam(data, 2));
	y = atoi(GetParam(data, 3));
	x = atoi(GetParam(data, 4));
	direction = atoi(GetParam(data, 5));
	action = GetParam(data, 6);
	u1 = atoi(GetParam(data, 7));
	u2 = atoi(GetParam(data, 8));
	energy = atoi(GetParam(data, 9));
	upgrades = atoi(GetParam(data, 10));
	u5 = atoi(GetParam(data, 11));
	upgrade1 = GetParam(data, 12);
	upgrade2 = GetParam(data, 13);
	upgrade3 = GetParam(data, 14);
	u9 = atoi(GetParam(data, 15));
	u10 = atoi(GetParam(data, 16));
}


void listSpecifcTechnoTypes(CComboBox& cb, const CString& sectionName, bool clear = true, bool useIniName = false)
{
	if (clear) {
		while (cb.DeleteString(0) != CB_ERR);
	}
	auto const& sec = rules.GetSection(sectionName);
	for (auto idx = 0; idx < sec.Size(); ++idx) {
		char idxNum[50];
		_itoa_s(idx, idxNum, 10);
		CString record = idxNum;

		auto const& kvPair = sec.Nth(idx);

		if (useIniName) {
			record = kvPair.second;
		}
		else {
			record += ' ';
			record += kvPair.second;
		}
		record += " ";

		CString translated = Map->GetUnitName(kvPair.second);
		//if(t!="MISSING")
		{
			record += translated;
			cb.AddString(record);
		}
	}
}

void listSpecifcTypesWithSequence(CComboBox& cb, const CString& sectionName, bool clear = true) {
	if (clear) {
		while (cb.DeleteString(0) != CB_ERR);
	}
	auto const& sec = rules.GetSection(sectionName);
	for (auto idx = 0; idx < sec.Size(); ++idx) {
		char idxNum[50];
		_itoa_s(idx, idxNum, 10);
		auto const& kvPair = sec.Nth(idx);
		CString record = idxNum;
		record += " ";
		record += kvPair.second;
		cb.AddString(record);
	}
}

void ListBuildings(CComboBox& cb, bool bININame)
{
	listSpecifcTechnoTypes(cb, "BuildingTypes", true, bININame);
}

void ListInfantry(CComboBox& cb)
{
	listSpecifcTechnoTypes(cb, "InfantryTypes");
}

void ListUnits(CComboBox& cb)
{
	listSpecifcTechnoTypes(cb, "VehicleTypes");
}

void ListAircraft(CComboBox& cb)
{
	listSpecifcTechnoTypes(cb, "AircraftTypes");
}

void ListTechtypes(CComboBox& cb, bool numberic)
{
	auto const useId = !numberic;
	while (cb.DeleteString(0) != CB_ERR);
	listSpecifcTechnoTypes(cb, "AircraftTypes", false, useId);
	listSpecifcTechnoTypes(cb, "InfantryTypes", false, useId);
	listSpecifcTechnoTypes(cb, "VehicleTypes", false, useId);
	listSpecifcTechnoTypes(cb, "BuildingTypes", false, useId);
}

void listLocalVariables(CComboBox& cb, const CIniFile& ini)
{
	while (cb.DeleteString(0) != CB_ERR);

	for (auto const& kvPair : ini.GetSection("VariableNames")) {
		auto const desc = kvPair.first + " " + kvPair.second;
		cb.AddString(desc);
	}
}

// should be ListLocals()
void ListMapVariables(CComboBox& cb)
{
	listLocalVariables(cb, Map->GetIniFile());
}

void ListRulesGlobals(CComboBox& cb)
{
	listLocalVariables(cb, rules);
}

extern TranslationMap AllStrings;
void ListTutorial(CComboBox& cb)
{
	while (cb.DeleteString(0) != CB_ERR);

#ifndef RA2_MODE
	int i;
	for (i = 0; i < tutorial.sections["Tutorial"].values.size(); i++) {
		CString s;
		s = *tutorial.sections["Tutorial"].GetValueName(i);

		s += " ";

		s += *tutorial.sections["Tutorial"].GetValue(i);

		cb.AddString(s);
	}
#else

	typedef map<CString, XCString>::iterator it;
	it _it = AllStrings.begin();
	/*it begin;
	it end;
	begin=CCStrings.begin();
	end=CCStrings.end();*/

	int i;
	for (i = 0; i < CCStrings.size(); i++) {

		CString s;
		s = _it->first;
		s += " : ";
		s += _it->second.cString;

		cb.AddString(s);
		_it++;
	}

#endif
}

void ListTriggers(CComboBox& cb)
{
	while (cb.DeleteString(0) != CB_ERR);
	CIniFile& ini = Map->GetIniFile();

	for (auto const& kvPair : ini.GetSection("Triggers")) {
		auto s = kvPair.first;
		s += " (";
		s += GetParam(kvPair.second, 2);
		s += ")";

		cb.AddString(s);
	}

}

void ListYesNo(CComboBox& cb)
{
	while (cb.DeleteString(0) != CB_ERR);
	cb.AddString("1 " + GetLanguageStringACP("Yes"));
	cb.AddString("0 " + GetLanguageStringACP("No"));
}

void ListSounds(CComboBox& cb)
{
	while (cb.DeleteString(0) != CB_ERR);
#ifdef RA2_MODE
	for (auto const& kvPair : sound.GetSection("SoundList")) {
		cb.AddString(kvPair.second);
	}
#endif
}
void ListThemes(CComboBox& cb)
{
	while (cb.DeleteString(0) != CB_ERR);
#ifdef RA2_MODE

	for (auto const& kvPair : theme.GetSection("Themes")) {
		CString record = kvPair.second;// now is ini ID

		TruncSpace(record);

		if (record.IsEmpty()) {
			continue;
		}

		record += " ";
		record += AllStrings[sound.GetSection(record).GetString("Name")].cString;

		cb.AddString(record);
	}
#endif
}
void ListSpeeches(CComboBox& cb)
{
	while (cb.DeleteString(0) != CB_ERR);
#ifdef RA2_MODE

	for (auto const& kvPair : eva.GetSection("DialogList")) {
		cb.AddString(kvPair.second);
	}
#endif
}
void ListSpecialWeapons(CComboBox& cb)
{
	listSpecifcTypesWithSequence(cb, "SuperWeaponTypes");
}

void ListAnimations(CComboBox& cb)
{
	listSpecifcTypesWithSequence(cb, "Animations");
}

void ListParticles(CComboBox& cb)
{
	listSpecifcTypesWithSequence(cb, "Particles");
}

void ListCrateTypes(CComboBox& cb)
{
	while (cb.DeleteString(0) != CB_ERR);
}
void ListSpeechBubbleTypes(CComboBox& cb)
{
	while (cb.DeleteString(0) != CB_ERR);
}

void ListMovies(CComboBox& cb, BOOL bListNone, BOOL bListParam)
{
	if (!bListParam) {
		int sel = cb.GetCurSel();

		while (cb.DeleteString(0) != CB_ERR);

		if (bListNone) {
			cb.AddString("<none>");
		}
		auto const& movieList = art.GetSection("Movies");
		for (auto idx = 0; idx < movieList.Size(); ++idx) {
			if (idx < atoi(g_data.GetString("MovieList", "Start"))) {
				continue;
			}
			CString movieID = movieList.Nth(idx).second;
			cb.AddString(movieID);
		}
		if (sel >= 0) {
			cb.SetCurSel(sel);
		}
		return;
	}

	while (cb.DeleteString(0) != CB_ERR);
	auto const& movieList = art.GetSection("Movies");
	char idxStr[50];
	for (auto i = 0; i < movieList.Size(); i++) {
		if (i < atoi(g_data.GetString("MovieList", "Start"))) {
			continue;
		}
		_itoa_s(i, idxStr, 10);
		CString desc(idxStr);
		desc += " ";
		desc += movieList.Nth(i).second;
		cb.AddString(desc);
	}
}

void ListTags(CComboBox& cb, BOOL bListNone)
{
	CIniFile& ini = Map->GetIniFile();

	int sel = cb.GetCurSel();

	while (cb.DeleteString(0) != CB_ERR);

	int i;
	if (bListNone) {
		cb.AddString("None");
	}
	for (auto const& kvPair : ini.GetSection("Tags")) {
		CString s = kvPair.first;
		s += " ";
		s += GetParam(kvPair.second, 1);
		cb.AddString(s);
	}

	if (sel >= 0) {
		cb.SetCurSel(sel);
	}
}

int GetRulesHousesSize()
{
	return rules.GetSection(HOUSES).Size();
}

// a bug adds an empty house to the rules section, delete it here
int RepairRulesHouses()
{
	auto const& sec = rules.GetSection(HOUSES);
	auto toDelete = std::vector<size_t>();
	toDelete.reserve(sec.Size());

	for (auto idx = 0; idx < sec.Size(); idx++) {
		if (sec.Nth(idx).second.IsEmpty()) {
			toDelete.push_back(idx);
		}
	}

	// ascending sequence
	std::stable_sort(toDelete.begin(), toDelete.end());

	if (!toDelete.empty()) {
		auto const mutSec = rules.TryGetSection(HOUSES);
		ASSERT(mutSec != nullptr);
		for (auto const idx : toDelete) {
			mutSec->RemoveAt(idx);
		}
	}

	return GetRulesHousesSize();
}

// MW 07/27/01: Modified for <Player @ A> etc in YR
void ListHouses(CComboBox& cb, BOOL bNumbers, BOOL bCountries, BOOL bPlayers)
{
	CIniFile& ini = Map->GetIniFile();

	int sel = cb.GetCurSel();
	int crulesh = GetRulesHousesSize();

	// TODO: align with RN edition
	if (Map->IsMultiplayer() == FALSE) {
		bPlayers = FALSE; // really only for multi maps!
	}

	CString sSection = bCountries ? HOUSES : MAPHOUSES;

	while (cb.DeleteString(0) != CB_ERR);
	// houses:  rules.ini + map definitions!
	auto const mapHouseList = ini.TryGetSection(sSection);
	if (mapHouseList) {
		if (mapHouseList->Size() == 0) {
			goto wasnohouse;
		}
		// we use the map definitions!

		if (yuri_mode && bPlayers) {
			if (bNumbers) {
				cb.AddString("4475 <Player @ A>");
				cb.AddString("4476 <Player @ B>");
				cb.AddString("4477 <Player @ C>");
				cb.AddString("4478 <Player @ D>");
				cb.AddString("4479 <Player @ E>");
				cb.AddString("4480 <Player @ F>");
				cb.AddString("4481 <Player @ G>");
				cb.AddString("4482 <Player @ H>");
			} else {
				cb.AddString("<Player @ A>");
				cb.AddString("<Player @ B>");
				cb.AddString("<Player @ C>");
				cb.AddString("<Player @ D>");
				cb.AddString("<Player @ E>");
				cb.AddString("<Player @ F>");
				cb.AddString("<Player @ G>");
				cb.AddString("<Player @ H>");
			}


		}

		for (auto i = 0; i < mapHouseList->Size(); i++) {
			CString j;

#ifdef RA2_MODE
			j = mapHouseList->Nth(i).second;
			j.MakeLower();
			if (j == "nod" || j == "gdi") {
				continue;
			}
#endif

			if (bNumbers) {
				char idxStr[50];
				_itoa_s(i, idxStr, 10);
#ifdef RA2_MODE
				if (bCountries) {
					int preexisting = 0;
					int e;
					auto const& rulesMapList = rules.GetSection(sSection);
					for (e = 0; e < i; e++) {
						if (rulesMapList.HasValue(mapHouseList->Nth(e).second)) {
							preexisting++;
						}
					}
					if (rulesMapList.HasValue(mapHouseList->Nth(i).second)) {
						auto const& mapHouseID = mapHouseList->Nth(i).second;
						auto const& idxInRules = rulesMapList.FindValue(mapHouseID);
						_itoa_s(idxInRules, idxStr, 10);
					} else {
						_itoa_s(i + crulesh - preexisting, idxStr, 10);
					}
				}
#endif
				j = idxStr;
				j += " ";
				j += TranslateHouse(mapHouseList->Nth(i).second, TRUE);
			} else {
				j = TranslateHouse(mapHouseList->Nth(i).second, TRUE);
			}
			cb.AddString(j);
		}
	} else {
	wasnohouse:

		if (bNumbers) {
			auto const& rulesHouseList = rules.GetSection(HOUSES);
			for (auto const& [key, val] : rulesHouseList) {
				CString houseRecord;
#ifdef RA2_MODE
				houseRecord = val;
				houseRecord.MakeLower();
				if (houseRecord == "nod" || houseRecord == "gdi") {
					continue;
				}
#endif				
				houseRecord = key;
				houseRecord += " ";
				houseRecord += TranslateHouse(val, TRUE);

				cb.AddString(houseRecord);
			}


			if (!yuri_mode || !bPlayers) {
				for (auto i = 0; i < 8; i++) {
					int k = i;
#ifdef RA2_MODE
					k += crulesh;


					//rules.sections[HOUSES].values.size();
#endif
					CString j;
					char c[50];
					_itoa_s(k, c, 10);
					j += c;
					j += " Multi-Player ";
					_itoa_s(i, c, 10);
					j += c;
					cb.AddString(j);
				}
			} else {
				cb.AddString("4475 <Player @ A>");
				cb.AddString("4476 <Player @ B>");
				cb.AddString("4477 <Player @ C>");
				cb.AddString("4478 <Player @ D>");
				cb.AddString("4479 <Player @ E>");
				cb.AddString("4480 <Player @ F>");
				cb.AddString("4481 <Player @ G>");
				cb.AddString("4482 <Player @ H>");
			}
		} else {
			if (yuri_mode && bPlayers) {
				cb.AddString("<Player @ A>");
				cb.AddString("<Player @ B>");
				cb.AddString("<Player @ C>");
				cb.AddString("<Player @ D>");
				cb.AddString("<Player @ E>");
				cb.AddString("<Player @ F>");
				cb.AddString("<Player @ G>");
				cb.AddString("<Player @ H>");
			}

			auto const& rulesHouseList = rules.GetSection(HOUSES);
			for (auto const& [key, val] : rulesHouseList) {
				CString houseRecord;

#ifdef RA2_MODE
				houseRecord = val;
				houseRecord.MakeLower();
				if (houseRecord == "nod" || houseRecord == "gdi") {
					continue;
				}
#endif				

				if (bNumbers) {
					houseRecord = key;
					houseRecord += " ";
					houseRecord += TranslateHouse(val, TRUE);
				} else {
					houseRecord = TranslateHouse(val, TRUE);
				}

				cb.AddString(houseRecord);
			}
		}
	}

	if (sel >= 0) {
		cb.SetCurSel(sel);
	}
}


void ListTeamTypes(CComboBox& cb, BOOL bListNone)
{
	CIniFile& ini = Map->GetIniFile();

	int sel = cb.GetCurSel();

	while (cb.DeleteString(0) != CB_ERR);

	int i;
	if (bListNone) {
		cb.AddString("<none>");
	}
	auto const& teamTypeList = ini.GetSection("TeamTypes");
	for (auto const& [seq, id] : teamTypeList) {
		auto const& teamTypeDetail = ini.GetSection(id);
		CString record = id + " " + teamTypeDetail.GetString("Name");
		cb.AddString(record);
	}

	if (sel >= 0) {
		cb.SetCurSel(sel);
	}
}

void ListWaypoints(CComboBox& cb)
{
	CIniFile& ini = Map->GetIniFile();

	int sel = cb.GetCurSel();

	while (cb.DeleteString(0) != CB_ERR);

	for (auto const& [idx, val] : ini.GetSection("Waypoints")) {
		cb.AddString(idx);
	}

	if (sel >= 0) {
		cb.SetCurSel(sel);
	}
}

void ListTargets(CComboBox& cb)
{
	int sel = cb.GetCurSel();

	while (cb.DeleteString(0) != CB_ERR);

	cb.AddString(TranslateStringACP("1 - Not specified"));
	cb.AddString(TranslateStringACP("2 - Buildings"));
	cb.AddString(TranslateStringACP("3 - Harvesters"));
	cb.AddString(TranslateStringACP("4 - Infantry"));
	cb.AddString(TranslateStringACP("5 - Vehicles"));
	cb.AddString(TranslateStringACP("6 - Factories"));
	cb.AddString(TranslateStringACP("7 - Base defenses"));
	cb.AddString(TranslateStringACP("9 - Power plants"));

	if (sel >= 0) {
		cb.SetCurSel(sel);
	}
}

void ComboBoxHelper::Clear(CComboBox& combobox)
{
	while (combobox.DeleteString(0) != -1);
}

void ComboBoxHelper::ListCountries(CComboBox& combobox, bool bShowIndex)
{
	ComboBoxHelper::Clear(combobox);
	auto& doc = Map->GetIniFile();
	bool bMultiOnly = doc.GetBool("Basic", "MultiplayerOnly");
	if (bMultiOnly) {
		ListHouses(combobox, bShowIndex);
		return;
	}
	auto const& rules = IniMegaFile::GetRules();
	auto const& items = rules.GetSection("Countries");
	CString buffer;
	for (auto it = items.begin(); it != items.end(); ++it) {
		auto const& [idxStr, id] = *it;
		auto const idx = atoi(idxStr);
		if (bShowIndex) {
			buffer.Format("%u - %s", idx, id.operator LPCSTR());
		} else {
			buffer = id;
		}
		combobox.InsertString(idx, buffer);
	}
}

void ComboBoxHelper::ListBoolean(CComboBox& combobox)
{
	ComboBoxHelper::Clear(combobox);
	combobox.InsertString(0, "0 - FALSE");
	combobox.InsertString(1, "1 - TRUE");
}

CString GetHouseSectionName(CString lpHouse)
{
#ifndef RA2_MODE
	return lpHouse;
#else
	return lpHouse + " House";
#endif

}


CString GetFreeID()
{
	auto const& ini = Map->GetIniFile();

	int n = 1000000;

	auto isIDInUse = [&ini](const CString& input) {
		// [TypeList] 
		// 0=TYPE1
		// 1=TYPE2
		static const CString typeLists[] = {
			"ScriptTypes",
			"TaskForces",
			"TeamTypes",
		};
		// [ItemList]
		// ID1=SOME_DEFINITION1
		// ID2=SOME_DEFINITION2
		static const CString itemLists[] = {
			"Triggers",
			"Events",
			"Tags",
			"Actions",
			"AITriggerTypes",
		};

		for (auto const& id : typeLists) {
			for (auto const& [_, id] : ini[id]) {
				if (id == input) {
					return true;
				}
			}
		}
		for (auto const& id : itemLists) {
			for (auto const& [id, _] : ini[id]) {
				if (id == input) {
					return true;
				}
			}
		}

		return false;
	};

	for (;;) {
		char p[50];
		p[0] = '0';
		_itoa_s(n, p + 1, sizeof(p) - 1, 10);

		if (!isIDInUse(p)) {
			return p;
		}
		n++;
	}
	return "";
}

void GetNodeName(CString& name, int n)
{
	char c[5];
	char p[6];
	memset(p, 0, 6);
	_itoa_s(n, c, 10);
	strcpy_s(p, c);

	if (strlen(c) == 1) {
		memcpy(c, "00", 2);
		strcpy_s(c + 2, sizeof(c) - 2, p);
	} else if (strlen(c) == 2) {
		memcpy(c, "0", 1);
		strcpy_s(c + 1, sizeof(c) - 1, p);
	} else if (strlen(c) == 3) {
		strcpy_s(c, p);
	}

	name = c;
}

int GetNodeAt(CString& owner, CString& buildingTypeID, int x, int y)
{
	CIniFile& ini = Map->GetIniFile();

	buildingTypeID = "";
	owner = "";

	int owners;
	auto const& houseList = ini.GetSection(HOUSES);
	if (!houseList.Size()) {
		return -1;
	}
	for (auto const& [idx, ownerID] : houseList) {
		owner = ownerID;

		// okay now owner is correct!
		auto const& ownerSection = ini.GetSection(owner);
		auto const nodeCount = ownerSection.GetInteger("NodeCount");

		for (auto i = 0; i < nodeCount; i++) {
			CString nodeName;
			GetNodeName(nodeName, i);

			CString sx, sy;
			buildingTypeID = GetParam(ownerSection.GetString(nodeName), 0);
			sy = GetParam(ownerSection.GetString(nodeName), 1);
			sx = GetParam(ownerSection.GetString(nodeName), 2);

			CString arttype = buildingTypeID;
			auto const& imageID = rules.GetString(buildingTypeID, "Image");
			// pointing to another art
			if (!imageID.IsEmpty()) {
				arttype = imageID;
			}
			// pointing to another art because of map definition
			auto const overrideImageID = ini.GetString(buildingTypeID, "Image");
			if (!overrideImageID.IsEmpty()) {
				arttype = overrideImageID;
			}

			int w, h;
			char d[6];
			memcpy(d, (LPCTSTR)art.GetString(arttype, "Foundation"), 1);
			d[1] = 0;
			w = atoi(d);
			if (w == 0) {
				w = 1;
			}
			memcpy(d, (LPCTSTR)art.GetString(arttype, "Foundation") + 2, 1);
			d[1] = 0;
			h = atoi(d);
			if (h == 0) {
				h = 1;
			}

			int j, k;
			for (j = 0; j < h; j++) {
				for (k = 0; k < w; k++) {
					if (atoi(sx) + j == x && atoi(sy) + k == y) {
						return i;
					}
				}
			}
		}
	}

	return -1;
}


std::unique_ptr<CBitmap> BitmapFromResource(int resource_id)
{
	std::unique_ptr<CBitmap> bm(new CBitmap);
	if (!bm->LoadBitmap(resource_id))
		throw BitmapNotFound();
	return bm;
}

std::unique_ptr<CBitmap> BitmapFromFile(const CString& filepath)
{
	std::unique_ptr<CBitmap> bm(new CBitmap);
	if (!bm->LoadBitmap(filepath))
		throw BitmapNotFound();
	return bm;
}

/*
Returns the area in the current line that should be painted
Truncates areas that are transparent, and therefore increases display speed!
flags must be set to 0
*/
void GetDrawBorder(const BYTE* data, int width, int line, int& left, int& right, unsigned int flags, BOOL* TranspInside)
{
	int i;
	const BYTE* lpStart = data + line * width;

	if (flags == 0) {
		// left border:
		for (i = 0; i < width; i++) {
			if (lpStart[i] || i == width - 1) {
				left = i;
				break;
			}
		}

		// right border:
		for (i = width - 1; i >= 0; i--) {
			if (lpStart[i] || i == 0) {
				right = i;
				break;
			}
		}

		if (TranspInside) {
			for (i = left; i <= right; i++) {
				if (!lpStart[i]) {
					*TranspInside = TRUE;
					break;
				}
			}
		}
	}
}

CComPtr<IDirectDrawSurface4> BitmapToSurface(IDirectDraw4* pDD, const CBitmap& bitmap)
{
	BITMAP bm;
	GetObject(bitmap, sizeof(bm), &bm);

	DDSURFACEDESC2 desc = { 0 };
	ZeroMemory(&desc, sizeof(desc));
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	desc.dwWidth = bm.bmWidth;
	desc.dwHeight = bm.bmHeight;

	auto pSurface = CComPtr<IDirectDrawSurface4>();
	if (pDD->CreateSurface(&desc, &pSurface, nullptr) != DD_OK)
		return nullptr;

	pSurface->Restore();

	CDC bitmapDC;
	if (!bitmapDC.CreateCompatibleDC(nullptr))
		return nullptr;
	bitmapDC.SelectObject(bitmap);

	HDC hSurfaceDC = nullptr;
	if (pSurface->GetDC(&hSurfaceDC) != DD_OK)
		return nullptr;

	CDC surfaceDC;
	surfaceDC.Attach(hSurfaceDC);

	auto success = surfaceDC.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &bitmapDC, 0, 0, SRCCOPY);
	surfaceDC.Detach();
	pSurface->ReleaseDC(hSurfaceDC);

	return pSurface;
}



