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

// Loading.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "Loading.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include "resource.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"
#include "inlines.h"
#include "MissionEditorPackLib.h"
#include <chrono>
#include <thread>
#include "VoxelNormals.h"
#include <format>
#include "IniMega.h"
#include "VoxelDrawer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CLoading 


CLoading::CLoading(CWnd* pParent /*=NULL*/) :
	CDialog(CLoading::IDD, pParent),
	m_palettes(*this)
{
	//{{AFX_DATA_INIT(CLoading)
	//}}AFX_DATA_INIT

	Create(CLoading::IDD, pParent);

	loaded = FALSE;
	cur_theat = 'T';



	m_hCache = NULL;
	m_hConquer = NULL;
	m_hIsoSnow = NULL;
	m_hIsoTemp = NULL;
	m_hLocal = NULL;
	m_hTemperat = NULL;
	m_hTibSun = NULL;
	m_hLangMD = NULL;
	m_pic_count = 0;

	s_tiledata = NULL;
	t_tiledata = NULL;
	u_tiledata = NULL;
	s_tiledata_count = 0;
	t_tiledata_count = 0;
	u_tiledata_count = 0;
	tiledata = NULL;
	tiledata_count = 0;

	int i = 0;
	for (i = 0; i < 101; i++) {

		m_hExpand[i].hExpand = NULL;
	}
	for (i = 0; i < 100; i++) {

		m_hECache[i] = NULL;
	}

	VXL_Reset();
	VoxelDrawer::Initalize();

	errstream << "CLoading::CLoading() called" << endl;
	errstream.flush();
}


void CLoading::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoading)
	DDX_Control(pDX, IDC_VERSION, m_Version);
	DDX_Control(pDX, IDC_BUILTBY, m_BuiltBy);
	DDX_Control(pDX, IDC_CAP, m_cap);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoading, CDialog)
	//{{AFX_MSG_MAP(CLoading)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// message handlers CLoading 

BOOL bUseFirestorm = TRUE;

void CLoading::Load()
{
	m_progress.SetRange(0, 100);


	CString artFile;




	// show a wait cursor
	SetCursor(LoadCursor(NULL, IDC_WAIT));

	// write a log
	errstream << "CLoading::Load() called" << std::endl << std::endl;
	errstream.flush();


	// initialize the FSunPackLib::XCC
	errstream << "Initializing FSunPackLib::XCC" << std::endl;
	errstream.flush();
	FSunPackLib::XCC_Initialize(TRUE);
	m_cap.SetWindowText(GetLanguageStringACP("LoadExtractStdMixFiles"));

	errstream << "Initializing mix files" << std::endl;
	errstream.flush();
	MEMORYSTATUSEX ms;
	ms.dwLength = sizeof(ms);
	GlobalMemoryStatusEx(&ms);
	int cs = ms.ullAvailPhys + ms.ullAvailPageFile;

	m_progress.SetPos(10);
	UpdateWindow();

	InitMixFiles();
	errstream << "Loading palettes" << std::endl << std::endl;
	errstream.flush();
	InitPalettes();

	// Load voxel normal tables
	InitVoxelNormalTables();


	// create a ini file containing some info XCC Mixer needs
	CreateINI();

	// rules.ini
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadRules"));
	m_progress.SetPos(30);
	UpdateWindow();

	LoadTSIni("Rules.ini", &rules, FALSE);

#ifdef TS_MODE
	if (bUseFirestorm) {
		// very special case, the types list don´t have any order according to the original rules!
		CIniFile fstorm;
		LoadTSIni("firestrm.ini", &fstorm, TRUE);

		int i;
		CString cuSection;
		cuSection = "InfantryTypes";
		for (i = 0; i < fstorm.sections[cuSection].values.size(); i++) {
			// we simply add a FS to the index
			char c[50];
			itoa(i, c, 10);
			strcat(c, "FS");
			rules.sections[cuSection].values[c] = *fstorm.sections[cuSection].GetValue(i);
			rules.sections[cuSection].value_orig_pos[c] = rules.sections[cuSection].values.size() - 1;
		}
		cuSection = "VehicleTypes";
		for (i = 0; i < fstorm.sections[cuSection].values.size(); i++) {
			// we simply add a FS to the index
			char c[50];
			itoa(i, c, 10);
			strcat(c, "FS");
			rules.sections[cuSection].values[c] = *fstorm.sections[cuSection].GetValue(i);
			rules.sections[cuSection].value_orig_pos[c] = rules.sections[cuSection].values.size() - 1;
		}
		cuSection = "AircraftTypes";
		for (i = 0; i < fstorm.sections[cuSection].values.size(); i++) {
			// we simply add a FS to the index
			char c[50];
			itoa(i, c, 10);
			strcat(c, "FS");
			rules.sections[cuSection].values[c] = *fstorm.sections[cuSection].GetValue(i);
			rules.sections[cuSection].value_orig_pos[c] = rules.sections[cuSection].values.size() - 1;
		}
		cuSection = "BuildingTypes";
		for (i = 0; i < fstorm.sections[cuSection].values.size(); i++) {
			// we simply add a FS to the index
			char c[50];
			itoa(i, c, 10);
			strcat(c, "FS");
			rules.sections[cuSection].values[c] = *fstorm.sections[cuSection].GetValue(i);
			rules.sections[cuSection].value_orig_pos[c] = rules.sections[cuSection].values.size() - 1;
		}
		cuSection = "SuperWeaponTypes";
		for (i = 0; i < fstorm.sections[cuSection].values.size(); i++) {
			// we simply add a FS to the index
			char c[50];
			itoa(i, c, 10);
			strcat(c, "FS");
			rules.sections[cuSection].values[c] = *fstorm.sections[cuSection].GetValue(i);
			rules.sections[cuSection].value_orig_pos[c] = rules.sections[cuSection].values.size() - 1;
		}

		for (i = 0; i < fstorm.sections.size(); i++) {
			cuSection = *fstorm.GetSectionName(i);
			if (cuSection != "SuperWeaponTypes" && cuSection != "InfantryTypes" && cuSection != "VehicleTypes" && cuSection != "AircraftTypes" && cuSection != "BuildingTypes") {
				int e;
				for (e = 0; e < fstorm.sections[cuSection].values.size(); e++) {
					CString cuValue = *fstorm.sections[cuSection].GetValueName(e);
					rules.sections[cuSection].values[cuValue] = *fstorm.sections[cuSection].GetValue(e);
				}
			}
		}
	}
#else
	// MW actually this is Yuri's Revenge
	if (bUseFirestorm && yuri_mode) {
		CIniFile rulesmd;
		LoadTSIni("rulesmd.ini", &rulesmd, TRUE);
		if (rulesmd.Size() > 0) {
			rules = std::move(rulesmd);
		}
	}
#endif
	//rules.DeleteLeadingSpaces(TRUE, TRUE);
	//rules.DeleteEndingSpaces(TRUE, TRUE);

	PrepareHouses();
	HackRules();

	// art.ini
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadArt"));
	m_progress.SetPos(35);
	UpdateWindow();

	LoadTSIni("Art.ini", &art, FALSE);
#ifdef TS_MODE
	if (bUseFirestorm) // LoadTSIni("ArtFs.ini", &art, TRUE);
	{
		CIniFile artfs;
		LoadTSIni("artfs.ini", &artfs, TRUE);

		// MW April 8th: bugfix... we erased Movies even if there was no new data!!!
		if (artfs.sections.size()) {

			int i;
			CString cuSection;
			cuSection = "Movies";
			art.sections.erase(cuSection);
			/*for(i=0;i<artfs.sections[cuSection].values.size();i++)
			{
				// we simply add a FS to the index
				char c[50];
				itoa(i, c, 10);
				strcat(c, "FS");
				art.sections[cuSection].values[c]=*artfs.sections[cuSection].GetValue(i);
				art.sections[cuSection].value_orig_pos[c]=art.sections[cuSection].values.size()-1;
			}*/

			for (i = 0; i < artfs.sections.size(); i++) {
				cuSection = *artfs.GetSectionName(i);
				//if(cuSection!="Movies")
				{
					int e;
					for (e = 0; e < artfs.sections[cuSection].values.size(); e++) {
						CString cuValue = *artfs.sections[cuSection].GetValueName(e);
						art.sections[cuSection].values[cuValue] = *artfs.sections[cuSection].GetValue(e);
						if (cuSection == "Movies") art.sections[cuSection].value_orig_pos[cuValue] = artfs.sections[cuSection].GetValueOrigPos(e);
					}
				}
			}
		}

	}
#else
	if (bUseFirestorm && yuri_mode) // Yuri's Revenge
	{
		CIniFile artmd;
		LoadTSIni("artmd.ini", &artmd, TRUE);
		if (artmd.Size() > 0) {
			art = std::move(artmd);
		}
	}
#endif

	//art.DeleteLeadingSpaces(TRUE, TRUE);
	//art.DeleteEndingSpaces(TRUE, TRUE);

	// tutorial.ini
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadTutorial"));
	LoadTSIni("Tutorial.ini", &tutorial, FALSE);

	// sound.ini
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadSound"));
	m_progress.SetPos(40);
	UpdateWindow();


	if (bUseFirestorm) {
		int b;
		for (b = 99; b > 0; b--) {
			char p[50];
			itoa(b, p, 10);
			CString name = "Sound";
			if (strlen(p) < 2) name += "0";
			name += p;
			name += ".ini";

			LoadTSIni(name, &sound, FALSE);
			if (sound.Size() > 0) {
				break;
			}
		}

	} else {
		LoadTSIni("Sound01.ini", &sound, FALSE);
	}
	if (sound.Size() == 0) {
		LoadTSIni("Sound.ini", &sound, FALSE);
	}

	// eva.INI
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadEva"));
	m_progress.SetPos(45);
	UpdateWindow();

	LoadTSIni("eva.ini", &eva, FALSE);

	// theme.INI
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadTheme"));
	m_progress.SetPos(50);
	UpdateWindow();

	LoadTSIni("theme.ini", &theme, FALSE);

	// AI.INI
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadAI"));
	m_progress.SetPos(55);
	UpdateWindow();

	LoadTSIni("Ai.ini", &ai, FALSE);
#ifdef TS_MODE
	if (bUseFirestorm) LoadTSIni("aifs.ini", &ai, TRUE);;
#else
	if (bUseFirestorm && yuri_mode) {
		// YR
		LoadTSIni("aimd.ini", &ai, TRUE);
	}

#endif

	const BOOL preferLocalTheater = theApp.m_Options.bPreferLocalTheaterFiles ? TRUE : FALSE;

	// Temperat.INI
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadTemperat"));
	m_progress.SetPos(60);
	UpdateWindow();

	LoadTSIni("Temperat.ini", &tiles_t, FALSE, preferLocalTheater);
	if (yuri_mode) {
		LoadTSIni("TemperatMD.ini", &tiles_t, TRUE, preferLocalTheater);
	}

	// Snow.INI
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadSnow"));
	m_progress.SetPos(65);
	UpdateWindow();

	LoadTSIni("Snow.ini", &tiles_s, FALSE, preferLocalTheater);
	if (yuri_mode) {
		LoadTSIni("SnowMD.ini", &tiles_s, TRUE, preferLocalTheater);
	}

	// Urban.INI
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadUrban"));
	m_progress.SetPos(70);
	UpdateWindow();

	LoadTSIni("Urban.ini", &tiles_u, FALSE, preferLocalTheater);
	if (yuri_mode) {
		LoadTSIni("UrbanMD.ini", &tiles_u, TRUE, preferLocalTheater);
	}

	if (yuri_mode) {
		m_cap.SetWindowText(GetLanguageStringACP("LoadLoadUrbanN"));
		m_progress.SetPos(75);
		UpdateWindow();
		LoadTSIni("UrbanNMD.ini", &tiles_un, FALSE, preferLocalTheater);

		// MW FIX: MAKE URBAN RAMPS MORPHABLE:
		if (!tiles_un["TileSet0117"].Exists("Morphable")) {
			tiles_un.SetBool("TileSet0117", "Morphable", true);
		}

		m_cap.SetWindowText(GetLanguageStringACP("LoadLoadLunar"));
		m_progress.SetPos(80);
		UpdateWindow();

		LoadTSIni("LunarMD.ini", &tiles_l, FALSE, preferLocalTheater);

		m_cap.SetWindowText(GetLanguageStringACP("LoadLoadDesert"));
		m_progress.SetPos(85);
		UpdateWindow();

		LoadTSIni("DesertMD.ini", &tiles_d, FALSE, preferLocalTheater);
	}



	// load Command & Conquer Rules.ini section names
	LoadStrings();

	// ok now directdraw
	m_cap.SetWindowText(GetLanguageStringACP("LoadInitDDraw"));
	m_progress.SetPos(95);
	UpdateWindow();

	InitDirectDraw();

	m_progress.SetPos(100);
	UpdateWindow();

	/*errstream << "Now calling InitPics()\n";
	errstream.flush();
	m_cap.SetWindowText(GetLanguageStringACP("LoadInitPics"));
	InitPics();
	errstream << "InitPics() finished\n\n\n";
	errstream.flush();*/

	DestroyWindow();

}

void SetTheaterLetter(CString& basename, char TheaterIdentifier)
{
	if (TheaterIdentifier != 0 && basename.GetLength() >= 2) {
		char c0 = basename[0];
		char c1 = basename[1] & ~0x20; // evil hack to uppercase, copied from Ares
		if (isalpha(static_cast<unsigned char>(c0))) {
			if (c1 == 'A' || c1 == 'T') {
				basename.SetAt(1, TheaterIdentifier);
			}
		}
	}
}

void SetGenericTheaterLetter(CString& name)
{
	name.SetAt(1, 'G');
}

bool CLoading::LoadSingleFrameShape(const CString& name, int nFrame, int deltaX, int deltaY)
{
	CString file = name + ".SHP";
	SetTheaterLetter(file, this->cur_theat);


	auto nMix = FindFileInMix(file);
	//check whether there can be a valid file
	do {
		if (FSunPackLib::XCC_DoesFileExist(file, nMix)) {
			break;
		}
		SetGenericTheaterLetter(file);
		nMix = FindFileInMix(file);
		if (FSunPackLib::XCC_DoesFileExist(file, nMix)) {
			break;
		}
		file = name + ".SHP";
		nMix = FindFileInMix(file);
		if (FSunPackLib::XCC_DoesFileExist(file, nMix)) {
			break;
		}
		return false;
	} while (0);

	SHPHEADER header;
	unsigned char* pBuffer = nullptr;

	if (!FSunPackLib::SetCurrentSHP(file, nMix)) {
		return false;
	}
	if (!FSunPackLib::XCC_GetSHPHeader(&header)) {
		return false;
	}
	if (nFrame >= header.c_images) {
		nFrame = 0;
	}
	if (!FSunPackLib::LoadSHPImage(nFrame, 1, &pBuffer)) {
		return false;
	}

	UnionSHP_Add(pBuffer, header.cx, header.cy, deltaX, deltaY);

	return true;
};

void CLoading::UnionSHP_Add(unsigned char* pBuffer, int Width, int Height, int DeltaX, int DeltaY, bool UseTemp)
{
	UnionSHP_Data[UseTemp].push_back(SHPUnionData{ pBuffer,Width,Height,DeltaX,DeltaY });
}


void CLoading::UnionSHP_GetAndClear(unsigned char*& pOutBuffer, int* OutWidth, int* OutHeight, bool clearBuffer, bool UseTemp)
{
	// never calls it when UnionSHP_Data is empty

	if (UnionSHP_Data[UseTemp].size() == 1) {
		pOutBuffer = UnionSHP_Data[UseTemp][0].Buffer;
		if (OutWidth) {
			*OutWidth = UnionSHP_Data[UseTemp][0].Width;
		}
		if (OutHeight) {
			*OutHeight = UnionSHP_Data[UseTemp][0].Height;
		}
		UnionSHP_Data[UseTemp].clear();
		return;
	}

	// For each shp, we make their center at the same point, this will give us proper result.
	int W = 0, H = 0;

	for (auto& data : UnionSHP_Data[UseTemp]) {
		if (W < data.Width + 2 * abs(data.DeltaX)) {
			W = data.Width + 2 * abs(data.DeltaX);
		}
		if (H < data.Height + 2 * abs(data.DeltaY)) {
			H = data.Height + 2 * abs(data.DeltaY);
		}
	}

	// just make it work like unsigned char[W][H];
	pOutBuffer = new(unsigned char[W * H]);
	ZeroMemory(pOutBuffer, W * H);
	if (OutWidth) {
		*OutWidth = W;
	}
	if (OutHeight) {
		*OutHeight = H;
	}

	int ImageCenterX = W / 2;
	int ImageCenterY = H / 2;

	// Image[X][Y] <=> pOutBuffer[Y * W + X];
	for (auto& data : UnionSHP_Data[UseTemp]) {
		int nStartX = ImageCenterX - data.Width / 2 + data.DeltaX;
		int nStartY = ImageCenterY - data.Height / 2 + data.DeltaY;

		for (int j = 0; j < data.Height; ++j)
			for (int i = 0; i < data.Width; ++i)
				if (auto nPalIdx = data.Buffer[j * data.Width + i])
					pOutBuffer[(nStartY + j) * W + nStartX + i] = nPalIdx;

		if (clearBuffer) {
			delete[](data.Buffer);
		}
	}

	UnionSHP_Data[UseTemp].clear();
}


void CLoading::VXL_Add(const unsigned char* pCache, int X, int Y, int Width, int Height)
{
	for (int j = 0; j < Height; ++j)
		for (int i = 0; i < Width; ++i)
			if (auto ch = pCache[j * Width + i])
				VXL_Data[(j + Y) * 0x100 + X + i] = ch;
}

void CLoading::VXL_GetAndClear(unsigned char*& pBuffer, int* OutWidth, int* OutHeight)
{
	/* TODO : Save memory
	int validFirstX = 0x100 - 1;
	int validFirstY = 0x100 - 1;
	int validLastX = 0;
	int validLastY = 0;

	for (int j = 0; j < 0x100; ++j)
	{
		for (int i = 0; i < 0x100; ++i)
		{
			unsigned char ch = VXL_Data[j * 0x100 + i];
			if (ch != 0)
			{
				if (i < validFirstX)
					validFirstX = i;
				if (j < validFirstY)
					validFirstY = j;
				if (i > validLastX)
					validLastX = i;
				if (j > validLastY)
					validLastY = j;
			}
		}
	}
	*/
	pBuffer = new unsigned char[VoxelBlendCacheLength];
	memcpy_s(pBuffer, VoxelBlendCacheLength, VXL_Data, VoxelBlendCacheLength);
	VXL_Reset();
}

void CLoading::VXL_Reset()
{
	memset(VXL_Data, 0, VoxelBlendCacheLength);
}

bool IsImageLoaded(const CString& ID) {
	auto const it = pics.find(ID);
	if (it == pics.end()) {
		return false;
	}
	return it->second.pic != nullptr;
}

void GetFullPaletteName(CString& PaletteName, char theater)
{
	switch (theater) {
	case 'A':
		PaletteName += "sno.pal";
		return;
	case 'U':
		PaletteName += "urb.pal";
		return;
	case 'N':
		PaletteName += "ubn.pal";
		return;
	case 'D':
		PaletteName += "des.pal";
		return;
	case 'L':
		PaletteName += "lun.pal";
		return;
	case 'T':
	default:
		PaletteName += "tem.pal";
		return;
	}
}

//
// InitPics loads all graphics except terrain graphics!
void CLoading::InitPics(CProgressCtrl* prog)
{
	MEMORYSTATUSEX ms;
	ms.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&ms);
	auto cs = ms.ullAvailPhys + ms.ullAvailPageFile;

	errstream << "InitPics() called. Available memory: " << cs << endl;
	errstream.flush();




	CalcPicCount();

	if (!theApp.m_Options.bDoNotLoadBMPs) {
		int k;
		CFileFind ff;
		CString bmps = (CString)AppPath + "\\pics2\\*.bmp";
		if (ff.FindFile(bmps)) {

			BOOL lastFile = FALSE;

			for (k = 0; k < m_bmp_count + 1; k++) {

				if (ff.FindNextFile() == 0) {
					if (lastFile) // we already were at last file, so that´s an error here!
						break;
					lastFile = TRUE;
				}

				try {
					pics[(LPCTSTR)ff.GetFileName()].pic = BitmapToSurface(((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->dd, *BitmapFromFile(ff.GetFilePath())).Detach();

					DDSURFACEDESC2 desc;
					::memset(&desc, 0, sizeof(DDSURFACEDESC2));
					desc.dwSize = sizeof(DDSURFACEDESC2);
					desc.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
					((LPDIRECTDRAWSURFACE4)pics[(LPCTSTR)ff.GetFileName()].pic)->GetSurfaceDesc(&desc);
					pics[(LPCTSTR)ff.GetFileName()].wHeight = desc.dwHeight;
					pics[(LPCTSTR)ff.GetFileName()].wWidth = desc.dwWidth;
					pics[(LPCTSTR)ff.GetFileName()].wMaxWidth = desc.dwWidth;
					pics[(LPCTSTR)ff.GetFileName()].wMaxHeight = desc.dwHeight;
					pics[(LPCTSTR)ff.GetFileName()].bType = PICDATA_TYPE_BMP;

					FSunPackLib::SetColorKey(((LPDIRECTDRAWSURFACE4)(pics[(LPCTSTR)ff.GetFileName()].pic)), -1);
				} catch (const BitmapNotFound&) {
				}
			}
		}
	}

	DDSURFACEDESC2 desc;

	try {
		auto pPic = BitmapToSurface(((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->dd, *BitmapFromResource(IDB_SCROLLCURSOR)).Detach();
		// This is really dangerous to store a dangling ComPtr
		pics["SCROLLCURSOR"].pic = pPic;
		if (!pPic) {
			throw new BitmapNotFound();
		}
		FSunPackLib::SetColorKey((LPDIRECTDRAWSURFACE4)pPic, -1);
		::memset(&desc, 0, sizeof(DDSURFACEDESC2));
		desc.dwSize = sizeof(DDSURFACEDESC2);
		desc.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
		((LPDIRECTDRAWSURFACE4)pPic)->GetSurfaceDesc(&desc);
		pics["SCROLLCURSOR"].wHeight = desc.dwHeight;
		pics["SCROLLCURSOR"].wWidth = desc.dwWidth;
		pics["SCROLLCURSOR"].bType = PICDATA_TYPE_BMP;
	} catch (const BitmapNotFound&) {
	}

	try {
		pics["CELLTAG"].pic = BitmapToSurface(((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->dd, *BitmapFromResource(IDB_CELLTAG)).Detach();
		FSunPackLib::SetColorKey((LPDIRECTDRAWSURFACE4)pics["CELLTAG"].pic, CLR_INVALID);
		::memset(&desc, 0, sizeof(DDSURFACEDESC2));
		desc.dwSize = sizeof(DDSURFACEDESC2);
		desc.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
		((LPDIRECTDRAWSURFACE4)pics["CELLTAG"].pic)->GetSurfaceDesc(&desc);
		pics["CELLTAG"].wHeight = desc.dwHeight;
		pics["CELLTAG"].wWidth = desc.dwWidth;
#ifdef RA2_MODE
		pics["CELLTAG"].x = -1;
		pics["CELLTAG"].y = 0;
#else
		pics["CELLTAG"].x = -1;
		pics["CELLTAG"].y = -1;
#endif
		pics["CELLTAG"].bType = PICDATA_TYPE_BMP;
	} catch (const BitmapNotFound&) {
	}

	try {
		pics["FLAG"].pic = BitmapToSurface(((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->dd, *BitmapFromResource(IDB_FLAG)).Detach();
		FSunPackLib::SetColorKey((LPDIRECTDRAWSURFACE4)pics["FLAG"].pic, -1);
		::memset(&desc, 0, sizeof(DDSURFACEDESC2));
		desc.dwSize = sizeof(DDSURFACEDESC2);
		desc.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
		((LPDIRECTDRAWSURFACE4)pics["FLAG"].pic)->GetSurfaceDesc(&desc);
		pics["FLAG"].wHeight = desc.dwHeight;
		pics["FLAG"].wWidth = desc.dwWidth;
		pics["FLAG"].bType = PICDATA_TYPE_BMP;
	} catch (const BitmapNotFound&) {
	}



	// MW April 2nd, 2001
	// prepare 1x1 hidden tile replacement
	DDSURFACEDESC2 ddsd;
	::memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.dwWidth = f_x;
	ddsd.dwHeight = f_y;

	LPDIRECTDRAWSURFACE4 srf = NULL;
	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->dd->CreateSurface(&ddsd, &srf, 0);

	pics["HTILE"].pic = srf;
	pics["HTILE"].wHeight = ddsd.dwHeight;
	pics["HTILE"].wWidth = ddsd.dwWidth;
	pics["HTILE"].bType = PICDATA_TYPE_BMP;

	HDC hDC;
	srf->GetDC(&hDC);

	HPEN p;
	int width = 1;
	p = CreatePen(PS_DOT, 0, RGB(0, 120, 0));

	SelectObject(hDC, p);

	POINT p1, p2, p3, p4;
	p1.x = f_x / 2;
	p1.y = 0;
	p2.x = f_x / 2 + f_x / 2;
	p2.y = f_y / 2;
	p3.x = f_x / 2 - f_x / 2 + f_x / 2 - 1;
	p3.y = f_y / 2 + f_y / 2 - 1;
	p4.x = f_x / 2 + f_x / 2 - 1;
	p4.y = f_y / 2 - 1;


	SetBkMode(hDC, TRANSPARENT);
	MoveToEx(hDC, p1.x, p1.y - 1, NULL);
	LineTo(hDC, p2.x + 1, p2.y);
	LineTo(hDC, p3.x, p3.y + 1);
	LineTo(hDC, p4.x - 1, p4.y);
	LineTo(hDC, p1.x, p1.y - 1);

	srf->ReleaseDC(hDC);

	DeleteObject(p);


	// new: Prepare building terrain information:
	for (auto const& [seq, id] : rules.GetSection("BuildingTypes")) {
		PrepareUnitGraphic(id);

	}
	ms.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatusEx(&ms);
	cs = ms.ullAvailPhys + ms.ullAvailPageFile;

	int piccount = pics.size();

	errstream << "InitPics() finished and loaded " << piccount << " pictures. Available memory: " << cs << endl;
	errstream.flush();

	return;
}

BOOL CLoading::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString version;
	version.LoadString(IDS_VERSIONTEXT);
#ifdef TS_MODE
	version.LoadString(IDS_VERSIONTEXTTS);
#endif

	m_Version.SetWindowText(version);

	CString builder;
	builder.LoadString(IDS_BUILTBY);
	m_BuiltBy.SetWindowText(builder);

	SetDlgItemText(IDC_LBUILTBY, GetLanguageStringACP("LoadBuiltBy"));
	SetDlgItemText(IDC_LVERSION, GetLanguageStringACP("LoadVersion"));
	SetDlgItemText(IDC_CAP, GetLanguageStringACP("LoadLoading"));



	UpdateWindow();

	return TRUE;
}

// write a small ini file containing the FinalSun path and version (XCC needs this)
// TODO: this was made for Win9x. It does not work anymore on modern operating systems if you don't run the editor as administrator (which you should not do)
void CLoading::CreateINI()
{


	wchar_t iniFile_[MAX_PATH];
	CIniFile path;
	CString version;

	GetWindowsDirectoryW(iniFile_, MAX_PATH);
	std::string iniFile = utf16ToUtf8(iniFile_);
#ifdef RA2_MODE
	iniFile += "\\FinalAlert2.ini";
#else
	iniFile += "\\FinalSun.ini";
#endif

#ifdef RA2_MODE
	CString app = "FinalAlert";
#else
	CString app = "FinalSun";
#endif

	version.LoadString(IDS_VERSION);
	path.SetString(app, "Path", AppPath);
	path.SetString(app, "Version", version);

	path.SaveFile(iniFile);
}










void CLoading::LoadTSIni(LPCTSTR lpFilename, CIniFile* lpIniFile, BOOL bIsExpansion, BOOL bCheckEditorDir)
{
	errstream << "LoadTSIni(" << lpFilename << "," << lpIniFile << "," << bIsExpansion << ") called" << endl;
	errstream.flush();

	if (bCheckEditorDir) {
		// check if file is available in the editors application data folder
		if (DoesFileExist((u8AppDataPath + lpFilename).c_str())) {
			errstream << "File found in mission editor AppData directory (" << u8AppDataPath << ")" << endl;
			errstream.flush();
			if (!bIsExpansion)
				lpIniFile->LoadFile(u8AppDataPath + lpFilename, TRUE);
			else
				lpIniFile->InsertFile(u8AppDataPath + lpFilename, NULL, TRUE);
			return;
		}

		// check if file is available in the editors root folder
		if (DoesFileExist(CString(AppPath) + lpFilename)) {
			errstream << "File found in Mission Editor directory (" << AppPath << ")" << endl;
			errstream.flush();
			if (!bIsExpansion)
				lpIniFile->LoadFile((CString)AppPath + lpFilename, TRUE);
			else
				lpIniFile->InsertFile((CString)AppPath + lpFilename, NULL, TRUE);
			return;
		}
	}

	if (theApp.m_Options.bSearchLikeTS) {


		// check if Rules.ini is available
		if (DoesFileExist((CString)TSPath + lpFilename)) {
			errstream << "File found in TS directory (" << TSPath << ")" << endl;
			errstream.flush();
			if (!bIsExpansion)
				lpIniFile->LoadFile((CString)TSPath + lpFilename, TRUE);
			else
				lpIniFile->InsertFile((CString)TSPath + lpFilename, NULL, TRUE);
			return;
		}


		BOOL bExpandFound = FALSE;



		/*int i;
		for(i=1;i<101;i++)
		{
			if(m_hExpand[i].hExpand!=NULL)
			{
				if(FSunPackLib::XCC_ExtractFile(lpFilename, (CString)TSPath+(CString)"FinalSun"+lpFilename, m_hExpand[i].hExpand))
				{

					errstream << lpFilename << " found in expansion #" << i << endl;
					errstream.flush();

					//if(!bIsExpansion)
					//	lpIniFile->InsertFile((CString)TSPath+(CString)"FinalSun"+lpFilename, NULL);
					//else
					lpIniFile->InsertFile((CString)TSPath+(CString)"FinalSun"+lpFilename, NULL, TRUE);

					DeleteFile((CString)TSPath+(CString)"FinalSun"+lpFilename);

					bExpandFound=TRUE;
				}

			}
		}*/

		HMIXFILE hMix = FindFileInMix(lpFilename);
		if (hMix) {
			if (FSunPackLib::XCC_ExtractFile(lpFilename, (CString)TSPath + (CString)"FinalSun" + lpFilename, hMix)) {

				errstream << lpFilename << " found " << endl;
				errstream.flush();

				//if(!bIsExpansion)
				//	lpIniFile->InsertFile((CString)TSPath+(CString)"FinalSun"+lpFilename, NULL);
				//else
				lpIniFile->InsertFile((CString)TSPath + (CString)"FinalSun" + lpFilename, NULL, TRUE);

				DeleteFile((CString)TSPath + (CString)"FinalSun" + lpFilename);

				bExpandFound = TRUE;
			}
		}

		if (!bIsExpansion && !bExpandFound) {
			// not found, go ahead if this is not a expansion only file... 
			FSunPackLib::XCC_ExtractFile(lpFilename, (CString)TSPath + (CString)"FinalSun" + lpFilename, m_hLocal);

			lpIniFile->LoadFile((CString)TSPath + (CString)"FinalSun" + lpFilename, TRUE);

			DeleteFile((CString)TSPath + (CString)"FinalSun" + lpFilename);
		}


	} else if (bIsExpansion == FALSE) {
		FSunPackLib::XCC_ExtractFile(lpFilename, (CString)TSPath + (CString)"FinalSun" + lpFilename, m_hLocal);

		lpIniFile->LoadFile((CString)TSPath + (CString)"FinalSun" + lpFilename, TRUE);
		DeleteFile((CString)TSPath + (CString)"FinalSun" + lpFilename);
		return;
	}
}


void CLoading::InitSHPs(CProgressCtrl* prog)
{
	MEMORYSTATUS ms;
	ms.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	int cs = ms.dwAvailPhys + ms.dwAvailPageFile;

	errstream << "InitSHPs() called. Available memory: " << cs << endl;
	errstream.flush();

	int i;

	// overlay:
	if (!theApp.m_Options.bDoNotLoadOverlayGraphics) {
		auto const& sec = rules["OverlayTypes"];
		for (i = 0; i < sec.Size(); i++) {
			LoadOverlayGraphic(sec.Nth(i).second, i);
		}
	}

	if (!theApp.m_Options.bDoNotLoadVehicleGraphics) {
		auto const& sec = rules["VehicleTypes"];
		for (i = 0; i < sec.Size(); i++) {
			LoadUnitGraphic(sec.Nth(i).second);
		}
	}

	if (!theApp.m_Options.bDoNotLoadInfantryGraphics) {
		auto const& sec = rules.GetSection("InfantryTypes");
		for (i = 0; i < sec.Size(); i++) {
			LoadUnitGraphic(sec.Nth(i).second);
		}
	}


	if (!theApp.m_Options.bDoNotLoadBuildingGraphics) {
		auto const& sec = rules.GetSection("BuildingTypes");
		for (i = 0; i < sec.Size(); i++) {
			LoadUnitGraphic(sec.Nth(i).second);
		}
	}

	if (!theApp.m_Options.bDoNotLoadAircraftGraphics) {
		auto const& sec = rules.GetSection("AircraftTypes");
		for (i = 0; i < sec.Size(); i++) {
			LoadUnitGraphic(sec.Nth(i).second);
		}
	}

	if (!theApp.m_Options.bDoNotLoadTreeGraphics) {
		auto const& sec = rules.GetSection("TerrainTypes");
		for (i = 0; i < sec.Size(); i++) {
			LoadUnitGraphic(sec.Nth(i).second);
		}
	}

#ifdef SMUDGE_SUPP
	for (auto const& [seq, id] : rules.GetSection("SmudgeTypes")) {
		LoadUnitGraphic(id);
		/*if(m_progress.m_hWnd!=NULL && i%15==0)
		{
			m_progress.SetPos(m_progress.GetPos()+1);
			UpdateWindow();
		}*/
	}
#endif

}

/*
CLoading::LoadUnitGraphic();

Matze:
June 15th:
- Added bib support, works fine.
*/

#ifdef NOSURFACES_OBJECTS // palettized

// blit util
__forceinline void Blit_Pal(BYTE* dst, int x, int y, int dwidth, int dheight, BYTE* src, int swidth, int sheight)
{

	if (src == NULL || dst == NULL) return;



	if (x + swidth < 0 || y + sheight < 0) return;
	if (x >= dwidth || y >= dheight) return;


	RECT blrect;
	RECT srcRect;
	srcRect.left = 0;
	srcRect.top = 0;
	srcRect.right = swidth;
	srcRect.bottom = sheight;
	blrect.left = x;
	if (blrect.left < 0) {
		srcRect.left = 1 - blrect.left;
		blrect.left = 1;
	}
	blrect.top = y;
	if (blrect.top < 0) {
		srcRect.top = 1 - blrect.top;
		blrect.top = 1;
	}
	blrect.right = (x + swidth);
	if (x + swidth > dwidth) {
		srcRect.right = swidth - ((x + swidth) - dwidth);
		blrect.right = dwidth;
	}
	blrect.bottom = (y + sheight);
	if (y + sheight > dheight) {
		srcRect.bottom = sheight - ((y + sheight) - dheight);
		blrect.bottom = dheight;
	}


	int i, e;
	for (i = srcRect.left; i < srcRect.right; i++) {
		for (e = srcRect.top; e < srcRect.bottom; e++) {
			BYTE& val = src[i + e * swidth];
			if (val) {
				BYTE* dest = dst + (blrect.left + i) + (blrect.top + e) * dwidth;
				*dest = val;
			}
		}
	}

}

__forceinline void Blit_PalD(BYTE* dst, RECT blrect, const BYTE* src, RECT srcRect, int swidth, int dwidth, int sheight, int dheight, const BYTE* const srcMask = nullptr)
{

	if (src == NULL || dst == NULL) return;

	if (blrect.left + swidth < 0 || blrect.top + sheight < 0) return;
	if (blrect.left >= dwidth || blrect.top >= dheight) return;

	int x = blrect.left;
	int y = blrect.top;

	int i, e;
	for (i = srcRect.left; i < srcRect.right; i++) {
		for (e = srcRect.top; e < srcRect.bottom; e++) {
			auto pos = i + e * swidth;
			const BYTE& val = src[pos];
			if (blrect.left + i > 0 && blrect.top + e > 0 && blrect.left + i < dwidth && blrect.top + e < dheight) {
				if (srcMask) {
					if (srcMask[pos] == 0)
						continue;
				} else {
					if (val == 0)
						continue;
				}

				BYTE* dest = dst + (blrect.left + i) + (blrect.top + e) * dwidth;
				*dest = val;
			}
		}
	}

}

#ifdef TS_MODE
const Vec3f lightDirection = Vec3f(1.0f, 0.0f, -1.0f).normalize();
#else
const Vec3f lightDirection = Vec3f(1.0f, 0.0f, -1.0f).normalize();
#endif


CString theatToSuffix(char theat)
{
	if (theat == 'T') return ".tem";
	else if (theat == 'A') return ".sno";
	else if (theat == 'U') return ".urb";
	else if (theat == 'L') return ".lun";
	else if (theat == 'D') return ".des";
	else if (theat == 'N') return ".ubn";
	return ".tem";
}

char suffixToTheat(const CString& suffix)
{
	if (suffix == ".tem")
		return 'T';
	else if (suffix == ".sno")
		return 'A';
	else if (suffix == ".urb")
		return 'U';
	else if (suffix == ".lun")
		return 'L';
	else if (suffix == ".des")
		return 'D';
	else if (suffix == ".ubn")
		return 'N';
	return 'T';
}

std::optional<FindShpResult> CLoading::FindUnitShp(const CString& image, char preferred_theat, const CIniFileSection& artSection)
{
	if (image.IsEmpty())
		return std::nullopt;

	const char kTheatersToTry[] = { preferred_theat, 'G', 'T', 'A', 'U', 'N', 'D', 'L', 0 };
	const CString kSuffixesToTry[] = { theatToSuffix(preferred_theat), ".tem", ".sno", ".urb", ".lun", ".des", ".ubn", "" };

	const char first = image.GetAt(0);
	const bool firstCharSupportsTheater = first == 'G' || first == 'N' || first == 'C' || first == 'Y';

	HTSPALETTE forcedPalette = 0;
	const auto& unitPalettePrefixes = g_data["ForceUnitPalettePrefix"];
	if (unitPalettePrefixes.end() != std::find_if(unitPalettePrefixes.begin(), unitPalettePrefixes.end(),
		[&image](const auto& pair) {return image.Find(pair.second) == 0; })) {
		forcedPalette = m_palettes.GetUnitPalette(preferred_theat);
	}

	const auto& isoPalettePrefixes = g_data["ForceIsoPalettePrefix"];
	if (isoPalettePrefixes.end() != std::find_if(isoPalettePrefixes.begin(), isoPalettePrefixes.end(),
		[&image](const auto& pair) {return image.Find(pair.second) == 0; })) {
		forcedPalette = m_palettes.GetIsoPalette(preferred_theat);
	}

	const bool isTheater = artSection.GetBool("Theater");
	const bool isNewTheater = artSection.GetBool("NewTheater");
	const bool terrainPalette = artSection.GetBool("TerrainPalette");

	auto unitOrIsoPalette = terrainPalette ? m_palettes.GetIsoPalette(preferred_theat) : m_palettes.GetUnitPalette(preferred_theat);

	HMIXFILE curMixFile = 0;
	CString curFilename = image + ".shp";
	TheaterChar curMixTheater = TheaterChar::None;
	char curTheater = 'G';
	CString curSuffix;

	// Phase 0: theater with .tem, .sno etc
	if (isTheater) {
		for (int t = 0; !(curSuffix = kSuffixesToTry[t]).IsEmpty(); ++t) {
			curFilename = image + curSuffix;
			curFilename.MakeLower();
			curMixFile = FindFileInMix(curFilename, &curMixTheater);
			if (curMixFile)
				return FindShpResult(curMixFile, curMixTheater, curFilename, toTheaterChar(suffixToTheat(curSuffix)), forcedPalette ? forcedPalette : m_palettes.GetIsoPalette(suffixToTheat(curSuffix)));
		}
	}
	// Phase 1: if NewTheater is enabled and first character indicates support, try with real theater, then in order of kTheatersToTry
	// Otherwise,
	if (isNewTheater) {
		if (firstCharSupportsTheater) {
			/*curFilename.SetAt(1, preferred_theat);
			curMixFile = FindFileInMix(curFilename, &curMixTheater);
			if (curMixFile)
				return FindShpResult(curMixFile, curMixTheater, curFilename, toTheaterChar(preferred_theat), GetUnitPalette(preferred_theat));*/
			for (int t = 0; curTheater = kTheatersToTry[t]; ++t) {
				curFilename.SetAt(1, curTheater);
				curMixFile = FindFileInMix(curFilename, &curMixTheater);
				if (curMixFile)
					return FindShpResult(curMixFile, curMixTheater, curFilename, toTheaterChar(curTheater), forcedPalette ? forcedPalette : unitOrIsoPalette);
			}
		}
	}
	// Phase 2: try again even if isNewTheater is not true but the first char signals it should support theaters
	if (firstCharSupportsTheater) {
		/*curFilename.SetAt(1, preferred_theat);
		curMixFile = FindFileInMix(curFilename, &curMixTheater);
		if (curMixFile)
			return FindShpResult(curMixFile, curMixTheater, curFilename, toTheaterChar(preferred_theat), GetUnitPalette(preferred_theat));*/
		for (int t = 0; curTheater = kTheatersToTry[t]; ++t) {
			curFilename.SetAt(1, curTheater);
			curMixFile = FindFileInMix(curFilename, &curMixTheater);
			if (curMixFile)
				return FindShpResult(curMixFile, curMixTheater, curFilename, toTheaterChar(curTheater), forcedPalette ? forcedPalette : unitOrIsoPalette);
		}
	}
	// Phase 3: try unchanged filename
	curFilename = image + ".shp";
	curMixFile = FindFileInMix(curFilename, &curMixTheater);
	if (curMixFile)
		return FindShpResult(curMixFile, curMixTheater, curFilename, toTheaterChar(preferred_theat), forcedPalette ? forcedPalette : unitOrIsoPalette);
	// Phase 4: try lowercase and otherwise unchanged filename
	curFilename = image + ".shp";
	curFilename.MakeLower();
	curMixFile = FindFileInMix(curFilename, &curMixTheater);
	if (curMixFile)
		return FindShpResult(curMixFile, curMixTheater, curFilename, toTheaterChar(preferred_theat), forcedPalette ? forcedPalette : unitOrIsoPalette);
	// Phase 5: try with .tem, .sno etc endings with preferred theater
	for (int t = 0; !(curSuffix = kSuffixesToTry[t]).IsEmpty(); ++t) {
		curFilename = image + curSuffix;
		curFilename.MakeLower();
		curMixFile = FindFileInMix(curFilename, &curMixTheater);
		if (curMixFile)
			return FindShpResult(curMixFile, curMixTheater, curFilename, toTheaterChar(suffixToTheat(curSuffix)), forcedPalette ? forcedPalette : m_palettes.GetIsoPalette(suffixToTheat(curSuffix)));
	}
	// Phase 6: try with theater in 2nd char even if first char does not indicate support
	curFilename = image + ".shp";
	for (int t = 0; curTheater = kTheatersToTry[t]; ++t) {
		curFilename.SetAt(1, curTheater);
		curMixFile = FindFileInMix(curFilename, &curMixTheater);
		if (curMixFile)
			return FindShpResult(curMixFile, curMixTheater, curFilename, toTheaterChar(curTheater), forcedPalette ? forcedPalette : m_palettes.GetIsoPalette(curTheater));
	}

	return std::nullopt;
}

int lepton_to_screen_y(int leptons)
{
	return leptons * f_y / 256;
}

CLoading::ObjectType CLoading::GetItemType(const CString& ID)
{
	if (ObjectTypes.size() == 0) {
		auto load = [this](const CString& typeListName, ObjectType e) {
			auto const& rules = IniMegaFile::GetRules();
			auto const& items = rules.GetSection(typeListName);
			for (auto it = items.begin(); it != items.end(); ++it) {
				auto const& [_, id] = *it;
				ObjectTypes.insert({ id, e });
			}
		};

		load("InfantryTypes", ObjectType::Infantry);
		load("VehicleTypes", ObjectType::Vehicle);
		load("AircraftTypes", ObjectType::Aircraft);
		load("BuildingTypes", ObjectType::Building);
		load("SmudgeTypes", ObjectType::Smudge);
		load("TerrainTypes", ObjectType::Terrain);
	}

	auto itr = ObjectTypes.find(ID);
	if (itr != ObjectTypes.end())
		return itr->second;
	return ObjectType::Unknown;
}

BOOL CLoading::LoadUnitGraphic(const CString& ID)
{
	errstream << "Loading: " << ID << endl;
	errstream.flush();

	last_succeeded_operation = 10;

	char theat = cur_theat; // standard theater char is t (Temperat). a is snow.


	auto eItemType = GetItemType(ID);
	switch (eItemType) {
	case ObjectType::Infantry:
		this->LoadInfantry(ID);
		break;
	case ObjectType::Terrain:
	case ObjectType::Smudge:
		this->LoadTerrainOrSmudge(ID);
		break;
	case ObjectType::Vehicle:
	case ObjectType::Aircraft:
		this->LoadVehicleOrAircraft(ID);
		break;
	case ObjectType::Building:
		this->LoadBuilding(ID);
		break;
	case CLoading::ObjectType::Unknown:
	default:
		break;
	}

	return FALSE;
}



CString CLoading::GetTerrainOrSmudgeFileID(const CString& ID)
{
	CString ArtID = GetArtID(ID);
	CString ImageID = art.GetStringOr(ArtID, "Image", ArtID);

	return ImageID;
}

CString CLoading::GetBuildingFileID(const CString& ID)
{
	CString ArtID = GetArtID(ID);
	CString ImageID = art.GetStringOr(ArtID, "Image", ArtID);

	CString backupID = ImageID;
	SetTheaterLetter(ImageID, cur_theat);

	CString validator = ImageID + ".SHP";
	int nMix = this->FindFileInMix(validator);
	if (!FSunPackLib::XCC_DoesFileExist(validator, nMix)) {
		SetGenericTheaterLetter(ImageID);
		validator = ImageID + ".SHP";
		nMix = this->FindFileInMix(validator);
		if (!FSunPackLib::XCC_DoesFileExist(validator, nMix)) {
			ImageID = backupID;
		}
	}
	return ImageID;
}

CString CLoading::GetInfantryFileID(const CString& ID)
{
	CString ArtID = GetArtID(ID);

	CString ImageID = art.GetStringOr(ArtID, "Image", ArtID);
	auto const& rules = IniMegaFile::GetRules();

	if (rules.GetBool(ID, "AlternateTheaterArt")) {
		ImageID += this->cur_theat;
	} else if (rules.GetBool(ID, "AlternateArcticArt")) {
		if (this->cur_theat == 'A') {
			ImageID += 'A';
		}
	}
	if (!art.TryGetSection(ImageID)) {
		ImageID = ArtID;
	}
	return ImageID;
}

CString CLoading::GetArtID(const CString& ID)
{
	auto const& rules = IniMegaFile::GetRules();
	return rules.GetStringOr(ID, "Image", ID);
}

CString CLoading::GetVehicleOrAircraftFileID(const CString& ID)
{
	CString ArtID = GetArtID(ID);

	CString ImageID = art.GetStringOr(ArtID, "Image", ArtID);

	return ImageID;
}

void CLoading::LoadBuilding(const CString& ID)
{
	CString ArtID = GetArtID(ID);
	CString ImageID = GetBuildingFileID(ID);

	auto const& rules = IniMegaFile::GetRules();
	auto const& powerUpBldId = rules.GetString(ID, "PowersUpBuilding");
	// Early load
	if (!powerUpBldId.IsEmpty()) {
		CString SrcBldName = powerUpBldId + "0";
		if (!IsImageLoaded(SrcBldName)) {
			LoadBuilding(powerUpBldId);
		}
	}

	auto loadAnimFrame = [this, &ArtID, &ID](const CString& key, const CString& controlKey) {
		auto const imageID = art.GetString(ArtID, key);
		if (!imageID.IsEmpty()) {
			if (!g_data.GetBool(controlKey, ID)) {
				int nStartFrame = art.GetInteger(imageID, "LoopStart");
				LoadSingleFrameShape(art.GetStringOr(imageID, "Image", imageID), nStartFrame);
			}
		}
	};

	int nBldStartFrame = art.GetInteger(ArtID, "LoopStart", 0);

	if (!this->LoadSingleFrameShape(ImageID, nBldStartFrame)) {
		return;
	}

	loadAnimFrame("IdleAnim", "IgnoreIdleAnim");
	loadAnimFrame("ActiveAnim", "IgnoreActiveAnim1");
	loadAnimFrame("ActiveAnimTwo", "IgnoreActiveAnim2");
	loadAnimFrame("ActiveAnimThree", "IgnoreActiveAnim3");
	loadAnimFrame("ActiveAnimFour", "IgnoreActiveAnim4");
	loadAnimFrame("SuperAnim", "IgnoreSuperAnim1");
	loadAnimFrame("SuperAnimTwo", "IgnoreSuperAnim2");
	loadAnimFrame("SuperAnimThree", "IgnoreSuperAnim3");
	loadAnimFrame("SuperAnimFour", "IgnoreSuperAnim4");
	auto bibImageName = art.GetString(ArtID, "BibShape");
	if (bibImageName.GetLength()) {
		LoadSingleFrameShape(art.GetStringOr(bibImageName, "Image", bibImageName));
	}
	bool paletteRemappable = true;
	CString PaletteName = art.GetStringOr(ArtID, "Palette", "unit");
	if (art.GetBool(ArtID, "TerrainPalette")) {
		PaletteName = "iso";
		paletteRemappable = false;
	}
	GetFullPaletteName(PaletteName, cur_theat);


	unsigned char* pBuffer;
	int width, height;
	UnionSHP_GetAndClear(pBuffer, &width, &height);

	// No turret
	if (!rules.GetBool(ID, "Turret")) {
		CString DictName;
		DictName.Format("%s%d", ID.operator LPCSTR(), 0);
		SetImageData(pBuffer, DictName, width, height, m_palettes.LoadPalette(PaletteName, paletteRemappable));
		return;
	}
	// Has turret
	if (rules.GetBool(ID, "TurretAnimIsVoxel")) {
		int turzadjust = rules.GetInteger(ID, "TurretAnimZAdjust"); // no idea why apply it but it worked

		CString TurName = rules.GetStringOr(ID, "TurretAnim", ID + "tur");
		CString BarlName = ID + "barl";

		auto finder = [this](LPCTSTR lpFilename, char* pTheaterChar) {
			return this->FindFileInMix(lpFilename, reinterpret_cast<TheaterChar*>(pTheaterChar));
		};

		if (!VoxelDrawer::IsVPLLoaded()) {
			VoxelDrawer::LoadVPLFile("voxels.vpl", finder);
		}

		unsigned char* pTurImages[8]{ nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };
		unsigned char* pBarlImages[8]{ nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };
		VoxelRectangle turrect[8] = { 0 };
		VoxelRectangle barlrect[8] = { 0 };


		CString VXLName = BarlName + ".vxl";
		CString HVAName = BarlName + ".hva";

		if (VoxelDrawer::LoadVXLFile(VXLName, finder)) {
			if (VoxelDrawer::LoadHVAFile(HVAName, finder)) {
				for (int i = 0; i < 8; ++i) {
					// (13 - i) % 8 for facing fix
					bool result = VoxelDrawer::GetImageData((13 - i) % 8, pBarlImages[i], barlrect[i], turzadjust);
					if (!result)
						break;
				}
			}
		}

		VXLName = TurName + ".vxl";
		HVAName = TurName + ".hva";

		if (VoxelDrawer::LoadVXLFile(VXLName, finder) && VoxelDrawer::LoadHVAFile(HVAName, finder)) {
			for (int i = 0; i < 8; ++i) {
				// (13 - i) % 8 for facing fix
				bool result = VoxelDrawer::GetImageData((13 - i) % 8, pTurImages[i], turrect[i], pBarlImages[i] ? 0 : turzadjust);
				if (!result) {
					break;
				}
			}
		}

		for (int i = 0; i < 8; ++i) {
			auto pTempBuf = new(unsigned char[width * height]);
			memcpy_s(pTempBuf, width * height, pBuffer, width * height);
			UnionSHP_Add(pTempBuf, width, height);

			int deltaX = rules.GetInteger(ID, "TurretAnimX", 0);
			int deltaY = rules.GetInteger(ID, "TurretAnimY", 0);

			auto const& turretRect = turrect[i];
			auto const& barlRect = barlrect[i];

			if (pTurImages[i]) {
				CString pKey;

				pKey.Format("%sX%d", ID, (15 - i) % 8);
				int turdeltaX = g_data.GetInteger("BuildingVoxelTurretsRA2", pKey);
				pKey.Format("%sY%d", ID, (15 - i) % 8);
				int turdeltaY = g_data.GetInteger("BuildingVoxelTurretsRA2", pKey);

				VXL_Add(pTurImages[i], turretRect.X + turdeltaX, turretRect.Y + turdeltaY, turretRect.W, turretRect.H);
				delete[] pTurImages[i];

				if (pBarlImages[i]) {
					pKey.Format("%sX%d", ID, (15 - i) % 8);
					int barldeltaX = g_data.GetInteger("BuildingVoxelBarrelsRA2", pKey);
					pKey.Format("%sY%d", ID, (15 - i) % 8);
					int barldeltaY = g_data.GetInteger("BuildingVoxelBarrelsRA2", pKey);

					VXL_Add(pBarlImages[i], barlRect.X + barldeltaX, barlRect.Y + barldeltaY, barlRect.W, barlRect.H);
					delete[] pBarlImages[i];
				}
			}

			int nW = 0x100, nH = 0x100;
			VXL_GetAndClear(pTurImages[i], &nW, &nH);

			UnionSHP_Add(pTurImages[i], 0x100, 0x100, deltaX, deltaY);

			unsigned char* pImage;
			int width1, height1;

			UnionSHP_GetAndClear(pImage, &width1, &height1);
			CString DictName;
			DictName.Format("%s%d", ID.operator LPCSTR(), i);
			SetImageData(pImage, DictName, width1, height1, m_palettes.LoadPalette(PaletteName));
		}
		delete[](pBuffer);
		return;
	}

	//SHP anim
	CString TurName = rules.GetStringOr(ID, "TurretAnim", ID + "tur");
	int nStartFrame = art.GetInteger(TurName, "LoopStart");
	int deltaX = rules.GetInteger(ID, "TurretAnimX", 0);
	int deltaY = rules.GetInteger(ID, "TurretAnimY", 0);
	auto const turImageName = art.GetStringOr(TurName, "Image", TurName);
	auto const frameCount = art.GetInteger(TurName, "LoopEnd", 32);
	auto const frameInterval = frameCount / 8;

	for (int seqIdx = 0; seqIdx < 8; ++seqIdx) {
		auto pTempBuf = new(unsigned char[width * height]);
		memcpy_s(pTempBuf, width * height, pBuffer, width * height);
		UnionSHP_Add(pTempBuf, width, height);
		LoadSingleFrameShape(turImageName, nStartFrame + seqIdx * frameInterval, deltaX, deltaY);

		unsigned char* pImage;
		int width1, height1;
		UnionSHP_GetAndClear(pImage, &width1, &height1);
		CString DictName;
		DictName.Format("%s%d", ID.operator LPCSTR(), seqIdx);
		SetImageData(pImage, DictName, width1, height1, m_palettes.LoadPalette(PaletteName));
	}
	delete(pBuffer);
}

void CLoading::LoadInfantry(const CString& ID)
{
	CString ArtID = GetArtID(ID);
	CString ImageID = GetInfantryFileID(ID);

	CString sequenceName = art.GetString(ImageID, "Sequence");
	CString frames = art.GetStringOr(sequenceName, "Guard", "0,1,1");
	int framesToRead[8];
	int frameStart, frameStep;
	sscanf_s(frames, "%d,%d,%d", &frameStart, &framesToRead[0], &frameStep);
	for (int i = 0; i < 8; ++i) {
		framesToRead[i] = frameStart + i * frameStep;
	}

	CString FileName = ImageID + ".shp";
	int nMix = this->FindFileInMix(FileName);
	if (FSunPackLib::XCC_DoesFileExist(FileName, nMix)) {
		SHPHEADER header;
		unsigned char* FramesBuffers;
		FSunPackLib::SetCurrentSHP(FileName, nMix);
		FSunPackLib::XCC_GetSHPHeader(&header);
		for (int i = 0; i < 8; ++i) {
			FSunPackLib::LoadSHPImage(framesToRead[i], 1, &FramesBuffers);
			CString DictName;
			DictName.Format("%s%d", ImageID, i);
			CString PaletteName = art.GetStringOr(ArtID, "Palette", "unit");
			GetFullPaletteName(PaletteName, cur_theat);
			SetImageData(FramesBuffers, DictName, header.cx, header.cy, m_palettes.LoadPalette(PaletteName));
		}
	}
}

void CLoading::LoadTerrainOrSmudge(const CString& ID)
{
	CString ArtID = GetArtID(ID);
	CString ImageID = GetTerrainOrSmudgeFileID(ID);
	CString FileName = ImageID + theatToSuffix(this->cur_theat);
	int nMix = this->FindFileInMix(FileName);
	if (FSunPackLib::XCC_DoesFileExist(FileName, nMix)) {
		SHPHEADER header;
		unsigned char* FramesBuffers[1];
		FSunPackLib::SetCurrentSHP(FileName, nMix);
		FSunPackLib::XCC_GetSHPHeader(&header);
		FSunPackLib::LoadSHPImage(0, 1, &FramesBuffers[0]);
		CString DictName;
		DictName.Format("%s%d", ImageID.operator LPCSTR(), 0);
		CString PaletteName;

		if (ID.GetLength() >= 6 && *(DWORD*)ID.operator LPCTSTR() == *(DWORD*)("TIBT")) {
			PaletteName = "unitsno.pal";
		} else {
			PaletteName = art.GetStringOr(ArtID, "Palette", "iso");
			GetFullPaletteName(PaletteName, cur_theat);
		}
		SetImageData(FramesBuffers[0], DictName, header.cx, header.cy, m_palettes.LoadPalette(PaletteName));
	}
}

void CLoading::LoadVehicleOrAircraft(const CString& ID)
{
	CString ArtID = GetArtID(ID);
	CString ImageID = GetVehicleOrAircraftFileID(ID);
	auto const& rules = IniMegaFile::GetRules();
	bool bHasTurret = rules.GetBool(ID, "Turret");


	// As SHP
	if (!art.GetBool(ArtID, "Voxel")) {
		int framesToRead[8];
		int nStandingFrames = art.GetInteger(ArtID, "StandingFrames", 0);
		if (nStandingFrames) {
			int nStartStandFrame = art.GetInteger(ArtID, "StartStandFrame", 0);
			for (int i = 0; i < 8; ++i) {
				framesToRead[i] = nStartStandFrame + i * nStandingFrames;
			}
		} else {
			int nStartWalkFrame = art.GetInteger(ArtID, "StartWalkFrame", 0);
			int nWalkFrames = art.GetInteger(ArtID, "WalkFrames", 1);
			for (int i = 0; i < 8; ++i) {
				framesToRead[i] = nStartWalkFrame + i * nWalkFrames;
			}
		}

		CString FileName = ImageID + ".shp";
		int nMix = this->FindFileInMix(FileName);
		if (FSunPackLib::XCC_DoesFileExist(FileName, nMix)) {
			SHPHEADER header;
			unsigned char* FramesBuffers[2];
			FSunPackLib::SetCurrentSHP(FileName, nMix);
			FSunPackLib::XCC_GetSHPHeader(&header);
			for (int i = 0; i < 8; ++i) {
				FSunPackLib::LoadSHPImage(framesToRead[i], 1, &FramesBuffers[0]);
				CString DictName;
				DictName.Format("%s%d", ImageID, i);
				CString PaletteName = art.GetStringOr(ArtID, "Palette", "unit");
				GetFullPaletteName(PaletteName, cur_theat);

				if (bHasTurret) {
					int nStartWalkFrame = art.GetInteger(ArtID, "StartWalkFrame", 0);
					int nWalkFrames = art.GetInteger(ArtID, "WalkFrames", 1);
					int turretFramesToRead[8];

					// fix from cmcc
					turretFramesToRead[i] = nStartWalkFrame + 8 * nWalkFrames + 4 * ((i + 1) % 8);

					FSunPackLib::LoadSHPImage(turretFramesToRead[i], 1, &FramesBuffers[1]);
					UnionSHP_Add(FramesBuffers[0], header.cx, header.cy);
					UnionSHP_Add(FramesBuffers[1], header.cx, header.cy);
					unsigned char* outBuffer;
					int outW, outH;
					UnionSHP_GetAndClear(outBuffer, &outW, &outH);

					SetImageData(outBuffer, DictName, outW, outH, m_palettes.LoadPalette(PaletteName));
				} else {
					SetImageData(FramesBuffers[0], DictName, header.cx, header.cy, m_palettes.LoadPalette(PaletteName));
				}
			}
		}
		return;
	}

	auto finder = [this](LPCTSTR lpFilename, char* pTheaterChar) {
		return this->FindFileInMix(lpFilename, reinterpret_cast<TheaterChar*>(pTheaterChar));
	};

	// As VXL
	CString FileName = ImageID + ".vxl";
	CString HVAName = ImageID + ".hva";

	if (!VoxelDrawer::IsVPLLoaded()) {
		VoxelDrawer::LoadVPLFile("voxels.vpl", finder);
	}

	CString PaletteName = art.GetStringOr(ArtID, "Palette", "unit");
	GetFullPaletteName(PaletteName, cur_theat);

	unsigned char* pImage[8]{ nullptr,nullptr ,nullptr ,nullptr ,nullptr ,nullptr ,nullptr ,nullptr };
	unsigned char* pTurretImage[8]{ nullptr ,nullptr ,nullptr ,nullptr ,nullptr ,nullptr ,nullptr ,nullptr };
	unsigned char* pBarrelImage[8]{ nullptr ,nullptr ,nullptr ,nullptr ,nullptr ,nullptr ,nullptr ,nullptr };
	VoxelRectangle rect[8];
	VoxelRectangle turretrect[8];
	VoxelRectangle barrelrect[8];

	int nMix = this->FindFileInMix(FileName);
	if (!nMix || !FSunPackLib::SetCurrentVXL(FileName, nMix)) {
		return;
	}

	std::vector<BYTE> vxlColors[8];
	std::vector<BYTE> vxlLighting[8];

	if (!VoxelDrawer::LoadVXLFile(FileName, finder)) {
		return;
	}
	if (!VoxelDrawer::LoadHVAFile(HVAName, finder)) {
		return;
	}
	for (int i = 0; i < 8; ++i) {
		// (i+6) % 8 to fix the facing
		bool result = VoxelDrawer::GetImageData((i + 6) % 8, pImage[i], rect[i]);
		if (!result) {
			return;
		}
	}

	if (!bHasTurret) {
		for (int i = 0; i < 8; ++i) {
			CString DictName;
			DictName.Format("%s%d", ImageID.operator LPCSTR(), i);

			unsigned char* outBuffer;
			int outW = 0x100, outH = 0x100;

			VXL_Add(pImage[i], rect[i].X, rect[i].Y, rect[i].W, rect[i].H);
			delete[] pImage[i];
			VXL_GetAndClear(outBuffer, &outW, &outH);

			SetImageData(outBuffer, DictName, outW, outH, m_palettes.LoadPalette(PaletteName));
		}
		return;
	}

	int F, L, H;
	int s_count = sscanf_s(art.GetStringOr(ArtID, "TurretOffset", "0,0,0"), "%d,%d,%d", &F, &L, &H);
	if (s_count == 0) {
		F = L = H = 0;
	} else if (s_count == 1) {
		L = H = 0;
	} else if (s_count == 2) {
		H = 0;
	}

	CString turFileName = ImageID + "tur.vxl";
	CString turHVAName = ImageID + "tur.hva";

	if (VoxelDrawer::LoadVXLFile(turFileName, finder)) {
		return;
	}
	if (VoxelDrawer::LoadHVAFile(turHVAName, finder)) {
		return;
	}

	for (int i = 0; i < 8; ++i) {
		// (i+6) % 8 to fix the facing
		bool result = VoxelDrawer::GetImageData((i + 6) % 8, pTurretImage[i],
			turretrect[i], F, L, H);

		if (!result) {
			break;
		}
	}

	CString barlFileName = ImageID + "barl.vxl";
	CString barlHVAName = ImageID + "barl.hva";

	if (!VoxelDrawer::LoadVXLFile(barlFileName, finder)) {
		return;
	}
	if (!VoxelDrawer::LoadHVAFile(barlHVAName, finder)) {
		return;
	}

	for (int i = 0; i < 8; ++i) {
		// (i+6) % 8 to fix the facing
		bool result = VoxelDrawer::GetImageData((i + 6) % 8, pBarrelImage[i],
			barrelrect[i], F, L, H);

		if (!result) {
			break;
		}
	}

	for (int i = 0; i < 8; ++i) {
		CString DictName;
		DictName.Format("%s%d", ImageID.operator LPCSTR(), i);

		unsigned char* outBuffer;
		int outW = 0x100, outH = 0x100;

		if (pImage[i]) {
			VXL_Add(pImage[i], rect[i].X, rect[i].Y, rect[i].W, rect[i].H);
			delete[] pImage[i];
		}
		CString pKey;
		if (pTurretImage[i]) {
			pKey.Format("%sX%d", ID.operator LPCSTR(), i);
			int turdeltaX = g_data.GetInteger("VehicleVoxelTurretsRA2", pKey);
			pKey.Format("%sY%d", ID.operator LPCSTR(), i);
			int turdeltaY = g_data.GetInteger("VehicleVoxelTurretsRA2", pKey);
			VXL_Add(pTurretImage[i], turretrect[i].X + turdeltaX, turretrect[i].Y + turdeltaY, turretrect[i].W, turretrect[i].H);
			delete[] pTurretImage[i];

			if (pBarrelImage[i]) {
				pKey.Format("%sX%d", ID.operator LPCSTR(), i);
				int barldeltaX = g_data.GetInteger("VehicleVoxelBarrelsRA2", pKey);
				pKey.Format("%sY%d", ID.operator LPCSTR(), i);
				int barldeltaY = g_data.GetInteger("VehicleVoxelBarrelsRA2", pKey);

				VXL_Add(pBarrelImage[i], barrelrect[i].X + barldeltaX, barrelrect[i].Y + barldeltaY, barrelrect[i].W, barrelrect[i].H);
				delete[] pBarrelImage[i];
			}
		}

		VXL_GetAndClear(outBuffer, &outW, &outH);

		SetImageData(outBuffer, DictName, outW, outH, m_palettes.LoadPalette(PaletteName));
	}


}

void CLoading::SetImageData(unsigned char* pBuffer, const CString& NameInDict, int FullWidth, int FullHeight, Palette* pPal, bool forceNoRemap)
{
	ASSERT(!NameInDict.IsEmpty());
	auto& data = pics[NameInDict];

	if (NameInDict.Find("CNST") >= 0) {
		printf("");
	}

	SetImageData(pBuffer, data, FullWidth, FullHeight, pPal, forceNoRemap);
}

void CLoading::SetImageData(unsigned char* pBuffer, PICDATA& pData, const int FullWidth, const int FullHeight, Palette* pPal, bool forceNoRemap)
{
	if (auto pPic = std::exchange(pData.pic, nullptr)) {
		delete[](pPic);
	}
	if (auto pBorder = std::exchange(pData.vborder, nullptr)) {
		delete[](pBorder);
	}

	// Get available area
	int counter = 0;
	int validFirstX = FullWidth - 1;
	int validFirstY = FullHeight - 1;
	int validLastX = 0;
	int validLastY = 0;
	for (int j = 0; j < FullHeight; ++j) {
		for (int i = 0; i < FullWidth; ++i) {
			unsigned char ch = pBuffer[counter++];
			if (ch != 0) {
				if (i < validFirstX)
					validFirstX = i;
				if (j < validFirstY)
					validFirstY = j;
				if (i > validLastX)
					validLastX = i;
				if (j > validLastY)
					validLastY = j;
			}
		}
	}

	pData.x = validFirstX;
	pData.y = validFirstY;
	pData.wWidth = validLastX - validFirstX + 1;
	pData.wHeight = validLastY - validFirstY + 1;

	pData.pic = pBuffer;
	pData.wMaxHeight = FullHeight;
	pData.wMaxWidth = FullWidth;

	pData.vborder = new(VBORDER[FullHeight]);
	for (auto k = 0; k < FullHeight; k++) {
		int l, r;
		GetDrawBorder(pBuffer, FullWidth, k, l, r, 0);
		pData.vborder[k].left = l;
		pData.vborder[k].right = r;
	}

	pData.bType = PICDATA_TYPE_SHP;
	pData.bTried = false;
	//auto limited_to_theater = artSection.GetBool("TerrainPalette") ? shp->mixfile_theater : TheaterChar::None;
	auto limited_to_theater = TheaterChar::None;
	pData.bTerrain = limited_to_theater;
	if (pPal) {
		pData.pal  = reinterpret_cast<const int*>(pPal->GetData());
		pData.bHouseColor = pPal->IsRemappable() && !forceNoRemap;
		return;
	}
	pData.pal = iPalUnit;
	pData.bHouseColor = true;
}

void CLoading::LoadBuildingSubGraphic(const CString& subkey, const CIniFileSection& artSection, BOOL bAlwaysSetChar, char theat, HMIXFILE hShpMix, SHPHEADER& shp_h, BYTE*& shp)
{
	CString subname = artSection.GetString(subkey);
	if (subname.GetLength() > 0) {
		auto res = FindUnitShp(subname, theat, artSection);
		/*CString subfilename = subname + ".shp";

		if (isTrue(artSection.GetValueByName("NewTheater")) || bAlwaysSetChar || subfilename.GetAt(0) == 'G' || subfilename.GetAt(0) == 'N' || subfilename.GetAt(0) == 'Y' || subfilename.GetAt(0) == 'C')
		{
			auto subfilename_theat = subfilename;
			subfilename_theat.SetAt(1, theat);
			if (FSunPackLib::XCC_DoesFileExist(subfilename_theat, hShpMix))
				subfilename = subfilename_theat;
		}*/

		if (res && FSunPackLib::XCC_DoesFileExist(res->filename, res->mixfile)) {
			FSunPackLib::SetCurrentSHP(res->filename, res->mixfile);
			FSunPackLib::XCC_GetSHPHeader(&shp_h);
			FSunPackLib::LoadSHPImage(0, 1, &shp);

		}
	}
}
#endif


BOOL CLoading::InitMixFiles()
{
	last_succeeded_operation = 8;

	MEMORYSTATUS ms;
	ms.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	int cs = ms.dwAvailPhys + ms.dwAvailPageFile;

	errstream << "InitMixFiles() called. Available memory: " << cs << endl;
	errstream.flush();


	// load tibsun.mix and local.mix
	if (DoesFileExist((CString)TSPath + (CString)"\\" + MAINMIX)) {
		errstream << "Loading " MAINMIX ".mix";
		errstream.flush();
		m_hTibSun = FSunPackLib::XCC_OpenMix((CString)TSPath + (CString)"\\" + MAINMIX, NULL);
		if (m_hTibSun != NULL) {
			errstream << " success" << endl;
			errstream.flush();
		} else {
			ShowWindow(SW_HIDE);
			MessageBox(GetLanguageStringACP("Err_TSNotInstalled"));
			exit(200);
		}

		m_hLanguage = FSunPackLib::XCC_OpenMix((CString)TSPath + (CString)"\\Language.mix", NULL);
		m_hLangMD = FSunPackLib::XCC_OpenMix((CString)TSPath + (CString)"\\Langmd.mix", NULL);
		m_hMarble = FSunPackLib::XCC_OpenMix((CString)TSPath + (CString)"\\marble.mix", NULL);

		//if(!m_hLanguage) MessageBox("No language file found");

		if (!m_hMarble) {
			m_hMarble = FSunPackLib::XCC_OpenMix((CString)AppPath + (CString)"\\marble.mix", NULL);
		}
	} else {
		ShowWindow(SW_HIDE);
		MessageBox(GetLanguageStringACP("Err_TSNotInstalled"));
		exit(199);
	}

	errstream << "Loading local.mix";
	errstream.flush();
	if (DoesFileExist((CString)TSPath + "\\Local.mix") == FALSE || theApp.m_Options.bSearchLikeTS == FALSE) {
		m_hLocal = FSunPackLib::XCC_OpenMix("Local.mix", m_hTibSun);
	} else
		m_hLocal = FSunPackLib::XCC_OpenMix((CString)TSPath + "\\Local.mix", NULL);
	errstream << " successful" << endl;
	errstream.flush();



	errstream << "Loading conquer.mix, temperat.mix, isotemp.mix, isosnow.mix, isourb.mix and cache.mix";
	errstream.flush();
	m_hConquer = FSunPackLib::XCC_OpenMix("conquer.mix", m_hTibSun);
	m_hTemperat = FSunPackLib::XCC_OpenMix("temperat.mix", m_hTibSun);
	m_hUrban = FSunPackLib::XCC_OpenMix("urban.mix", m_hTibSun);
	m_hSnow = FSunPackLib::XCC_OpenMix("snow.mix", m_hTibSun);
	m_hUrbanN = FSunPackLib::XCC_OpenMix("urbann.mix", m_hTibSun);
	m_hLunar = FSunPackLib::XCC_OpenMix("lunar.mix", m_hTibSun);
	m_hDesert = FSunPackLib::XCC_OpenMix("desert.mix", m_hTibSun);
	m_hIsoTemp = FSunPackLib::XCC_OpenMix("isotemp.mix", m_hTibSun);
	m_hIsoSnow = FSunPackLib::XCC_OpenMix("isosnow.mix", m_hTibSun);
	m_hIsoUrb = FSunPackLib::XCC_OpenMix("isourb.mix", m_hTibSun);
	m_hIsoUbn = FSunPackLib::XCC_OpenMix("isoubn.mix", m_hTibSun);
	m_hIsoLun = FSunPackLib::XCC_OpenMix("isolun.mix", m_hTibSun);
	m_hIsoDes = FSunPackLib::XCC_OpenMix("isodes.mix", m_hTibSun);
	m_hIsoGen = FSunPackLib::XCC_OpenMix("isogen.mix", m_hTibSun);
	//m_hBuildings=FSunPackLib::XCC_OpenMix("_ID1085587737", m_hTibSun);	
	m_hBuildings = FSunPackLib::XCC_OpenMix("Generic.mix", m_hTibSun);
	m_hCache = FSunPackLib::XCC_OpenMix("cache.mix", m_hTibSun);
	m_hTem = FSunPackLib::XCC_OpenMix("tem.mix", m_hTibSun);
	m_hSno = FSunPackLib::XCC_OpenMix("sno.mix", m_hTibSun);
	m_hUrb = FSunPackLib::XCC_OpenMix("urb.mix", m_hTibSun);
	m_hUbn = FSunPackLib::XCC_OpenMix("ubn.mix", m_hTibSun);
	m_hLun = FSunPackLib::XCC_OpenMix("lun.mix", m_hTibSun);
	m_hDes = FSunPackLib::XCC_OpenMix("des.mix", m_hTibSun);
	if (!m_hMarble) FSunPackLib::XCC_OpenMix("marble.mix", m_hTibSun);

	if (m_hMarble) theApp.m_Options.bSupportMarbleMadness = TRUE;

	errstream << " successful" << endl;
	errstream.flush();

	int i;

	yuri_mode = FALSE;


	if (!theApp.m_Options.bSearchLikeTS) return TRUE;

	if (DoesFileExist((CString)TSPath + "\\ra2md.mix"))
		yuri_mode = TRUE; // MW Apr 17th, make it available right here!

	// load expansion mix files
	for (i = 0; i < 101; i++) {
		CString expand;
		char n[50];

		// MW April 17th, 2002:
		// read expandxxmd.mix files in yurimode		

		itoa(i, n, 10);
		expand = TSPath;
		expand += "\\Expand";
		if (yuri_mode) expand += "md";
		if (i < 10) expand += "0";
		expand += n;
		expand += ".mix";

		CString nappend = ".mix";

		CString append = ".mix";
		if (i == 100) append = "md.mix";


		if (yuri_mode) append = "md.mix";


		if (i == 100) expand = (CString)TSPath + "\\ra2md.mix"; // support the mission disk!!!

		errstream << "Searching " << (LPCTSTR)expand << endl;
		errstream.flush();

		if (DoesFileExist(expand)) {

			OutputDebugString(expand);
			OutputDebugString(": ");


			m_hExpand[i].hExpand = FSunPackLib::XCC_OpenMix(expand, NULL);

#ifdef YR_MODE
			if (i == 100 && m_hExpand[i].hExpand) yuri_mode = TRUE;
#endif

			errstream << (LPCTSTR)expand << " found: loading ";
			errstream.flush();

			CString conquer = "conquer";
			if (i == 100) conquer = "conq";
			if (FSunPackLib::XCC_DoesFileExist(conquer + append, m_hExpand[i].hExpand)) {
				OutputDebugString(conquer + append);
				OutputDebugString(": ");
				m_hExpand[i].hConquer = FSunPackLib::XCC_OpenMix((CString)conquer + append, m_hExpand[i].hExpand);
				errstream << "conquer.mix, ";
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"local" + append, m_hExpand[i].hExpand)) {
				OutputDebugString((CString)"local" + append);
				OutputDebugString(": ");
				m_hExpand[i].hLocal = FSunPackLib::XCC_OpenMix((CString)"local" + append, m_hExpand[i].hExpand);
				errstream << "local.mix, ";
			}
			//if(FSunPackLib::XCC_DoesFileExist("_ID1085587737", m_hExpand[i].hExpand))
			{
				//m_hExpand[i].hConquer=FSunPackLib::XCC_OpenMix("_ID1085587737", m_hExpand[i].hExpand);
				//errstream << "1085587737, ";
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"temperat" + append, m_hExpand[i].hExpand)) {
				OutputDebugString((CString)"temperat" + append);
				OutputDebugString(": ");
				m_hExpand[i].hTemperat = FSunPackLib::XCC_OpenMix((CString)"temperat" + append, m_hExpand[i].hExpand);
				errstream << "temperat.mix, ";
				errstream.flush();
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"urban" + append, m_hExpand[i].hExpand)) {
				OutputDebugString((CString)"urban" + append);
				OutputDebugString(": ");
				m_hExpand[i].hUrban = FSunPackLib::XCC_OpenMix((CString)"urban" + append, m_hExpand[i].hExpand);
				errstream << "urban.mix, ";
				errstream.flush();
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"snow" + append, m_hExpand[i].hExpand)) {
				OutputDebugString((CString)"snow" + append);
				OutputDebugString(": ");

				FSunPackLib::_DEBUG_EnableLogs = true;

				HMIXFILE hM = FSunPackLib::XCC_OpenMix((CString)"snow" + append, m_hExpand[i].hExpand);
				m_hExpand[i].hSnow = hM;
				errstream << "snow.mix, ";
				errstream.flush();

				FSunPackLib::_DEBUG_EnableLogs = false;
			}

			CString generic = "generic";
			if (i == 100) generic = "gener";
			if (FSunPackLib::XCC_DoesFileExist((CString)generic + append, m_hExpand[i].hExpand)) {
				m_hExpand[i].hGeneric = FSunPackLib::XCC_OpenMix((CString)generic + append, m_hExpand[i].hExpand);
				errstream << "generic.mix, ";
				errstream.flush();
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"urbann" + nappend, m_hExpand[i].hExpand)) {
				m_hExpand[i].hUrbanN = FSunPackLib::XCC_OpenMix((CString)"urbann" + nappend, m_hExpand[i].hExpand);
				errstream << "urbann.mix, ";
				errstream.flush();
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"lunar" + nappend, m_hExpand[i].hExpand)) {
				m_hExpand[i].hLunar = FSunPackLib::XCC_OpenMix((CString)"lunar" + nappend, m_hExpand[i].hExpand);
				errstream << "lunar.mix, ";
				errstream.flush();
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"desert" + nappend, m_hExpand[i].hExpand)) {
				m_hExpand[i].hDesert = FSunPackLib::XCC_OpenMix((CString)"desert" + nappend, m_hExpand[i].hExpand);
				errstream << "desert.mix, ";
				errstream.flush();
			}
			CString isotemp = "isotemp";
			if (i == 100) isotemp = "isotem";
			if (FSunPackLib::XCC_DoesFileExist(isotemp + append, m_hExpand[i].hExpand)) {
				m_hExpand[i].hIsoTemp = FSunPackLib::XCC_OpenMix((CString)isotemp + append, m_hExpand[i].hExpand);
				errstream << "isotemp.mix, ";
				errstream.flush();
			}
			CString isosnow = "isosnow";
			if (i == 100) isosnow = "isosno";
			if (FSunPackLib::XCC_DoesFileExist((CString)isosnow + append, m_hExpand[i].hExpand)) {
				m_hExpand[i].hIsoSnow = FSunPackLib::XCC_OpenMix((CString)isosnow + append, m_hExpand[i].hExpand);
				errstream << "isosnow.mix, ";
				errstream.flush();
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"isourb" + append, m_hExpand[i].hExpand)) {
				m_hExpand[i].hIsoUrb = FSunPackLib::XCC_OpenMix((CString)"isourb" + append, m_hExpand[i].hExpand);
				errstream << "isourb.mix, ";
				errstream.flush();
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"isoubn" + append, m_hExpand[i].hExpand)) {
				m_hExpand[i].hIsoUbnMd = FSunPackLib::XCC_OpenMix((CString)"isoubn" + append, m_hExpand[i].hExpand);
				errstream << "isoubn.mix, ";
				errstream.flush();
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"isolun" + append, m_hExpand[i].hExpand)) {
				m_hExpand[i].hIsoLunMd = FSunPackLib::XCC_OpenMix((CString)"isolun" + append, m_hExpand[i].hExpand);
				errstream << "isolun.mix, ";
				errstream.flush();
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"isodes" + append, m_hExpand[i].hExpand)) {
				m_hExpand[i].hIsoDesMd = FSunPackLib::XCC_OpenMix((CString)"isodes" + append, m_hExpand[i].hExpand);
				errstream << "isodes.mix, ";
				errstream.flush();
			}

			if (FSunPackLib::XCC_DoesFileExist((CString)"isoubn" + nappend, m_hExpand[i].hExpand)) {
				m_hExpand[i].hIsoUbn = FSunPackLib::XCC_OpenMix((CString)"isoubn" + nappend, m_hExpand[i].hExpand);
				errstream << "isoubn.mix, ";
				errstream.flush();
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"isolun" + nappend, m_hExpand[i].hExpand)) {
				m_hExpand[i].hIsoLun = FSunPackLib::XCC_OpenMix((CString)"isolun" + nappend, m_hExpand[i].hExpand);
				errstream << "isolun.mix, ";
				errstream.flush();
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"isodes" + nappend, m_hExpand[i].hExpand)) {
				m_hExpand[i].hIsoDes = FSunPackLib::XCC_OpenMix((CString)"isodes" + nappend, m_hExpand[i].hExpand);
				errstream << "isodes.mix, ";
				errstream.flush();
			}

			if (FSunPackLib::XCC_DoesFileExist((CString)"isogen" + append, m_hExpand[i].hExpand)) {
				m_hExpand[i].hIsoGenMd = FSunPackLib::XCC_OpenMix((CString)"isogen" + append, m_hExpand[i].hExpand);
				errstream << "isogen.mix, ";
				errstream.flush();
			}

			if (FSunPackLib::XCC_DoesFileExist((CString)"isogen" + nappend, m_hExpand[i].hExpand)) {
				m_hExpand[i].hIsoGen = FSunPackLib::XCC_OpenMix((CString)"isogen" + nappend, m_hExpand[i].hExpand);
				errstream << "isogen.mix, ";
				errstream.flush();
			}

			CString cache = "ecache01";
			if (i == 100) cache = "cache";
			if (FSunPackLib::XCC_DoesFileExist((CString)cache + append, m_hExpand[i].hExpand)) {
				m_hExpand[i].hECache = FSunPackLib::XCC_OpenMix((CString)cache + append, m_hExpand[i].hExpand);
				errstream << LPCSTR("ecache01" + append + ", ");
				errstream.flush();
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"tem" + append, m_hExpand[i].hExpand)) {
				m_hExpand[i].hTem = FSunPackLib::XCC_OpenMix((CString)"tem" + append, m_hExpand[i].hExpand);
				errstream << LPCSTR("tem" + append + ", ");
				errstream.flush();
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"sno" + append, m_hExpand[i].hExpand)) {
				m_hExpand[i].hSno = FSunPackLib::XCC_OpenMix((CString)"sno" + append, m_hExpand[i].hExpand);
				errstream << LPCSTR("sno" + append + ", ");
				errstream.flush();
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"urb" + append, m_hExpand[i].hExpand)) {
				m_hExpand[i].hUrb = FSunPackLib::XCC_OpenMix((CString)"urb" + append, m_hExpand[i].hExpand);
				errstream << LPCSTR("urb" + append + ", ");
				errstream.flush();
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"ubn" + nappend, m_hExpand[i].hExpand)) {
				m_hExpand[i].hUbn = FSunPackLib::XCC_OpenMix((CString)"ubn" + nappend, m_hExpand[i].hExpand);
				errstream << LPCSTR("ubn" + nappend + ", ");
				errstream.flush();
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"lun" + nappend, m_hExpand[i].hExpand)) {
				m_hExpand[i].hLun = FSunPackLib::XCC_OpenMix((CString)"lun" + nappend, m_hExpand[i].hExpand);
				errstream << LPCSTR("lun" + nappend + ", ");
				errstream.flush();
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"des" + nappend, m_hExpand[i].hExpand)) {
				m_hExpand[i].hDes = FSunPackLib::XCC_OpenMix((CString)"des" + nappend, m_hExpand[i].hExpand);
				errstream << LPCSTR("des" + nappend + ", ");
				errstream.flush();
			}
			if (FSunPackLib::XCC_DoesFileExist((CString)"marble" + append, m_hExpand[i].hExpand)) {
				theApp.m_Options.bSupportMarbleMadness = TRUE;

				m_hExpand[i].hMarble = FSunPackLib::XCC_OpenMix((CString)"marble" + append, m_hExpand[i].hExpand);
				errstream << LPCSTR("marble" + append + ", ");
				errstream.flush();
			}

			errstream << endl;
			errstream.flush();
		} else {
			m_hExpand[i].hExpand = NULL;
		}
	}

	// load expansion ecache mix files
	for (i = 0; i < 100; i++) {
		CString expand;
		char n[50];

		itoa(i, n, 10);
		expand = TSPath;
		if (!yuri_mode) expand += "\\ECache";
		else expand += "\\ecachemd";

		if (i < 10) expand += "0";
		expand += n;
		expand += ".mix";


		if (DoesFileExist(expand)) {
			m_hECache[i] = FSunPackLib::XCC_OpenMix(expand, NULL);
			errstream << (LPCTSTR)expand << " found and loaded" << endl;
			errstream.flush();
		} else {
			m_hECache[i] = NULL;
		}
	}

	ms.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	cs = ms.dwAvailPhys + ms.dwAvailPageFile;

	errstream << "InitMixFiles() finished. Available memory: " << cs << endl;
	errstream.flush();

	return TRUE;
}

CLoading::~CLoading()
{
	Unload();
	VoxelDrawer::Finalize();
}

void CLoading::Unload()
{
	FSunPackLib::XCC_CloseMix(m_hCache);
	FSunPackLib::XCC_CloseMix(m_hConquer);
	FSunPackLib::XCC_CloseMix(m_hIsoSnow);
	FSunPackLib::XCC_CloseMix(m_hIsoTemp);
	FSunPackLib::XCC_CloseMix(m_hIsoUrb);
	FSunPackLib::XCC_CloseMix(m_hIsoGen);
	FSunPackLib::XCC_CloseMix(m_hLocal);
	FSunPackLib::XCC_CloseMix(m_hTemperat);
	FSunPackLib::XCC_CloseMix(m_hSnow);
	FSunPackLib::XCC_CloseMix(m_hUrban);
	FSunPackLib::XCC_CloseMix(m_hTibSun);
	FSunPackLib::XCC_CloseMix(m_hSno);
	FSunPackLib::XCC_CloseMix(m_hTem);
	FSunPackLib::XCC_CloseMix(m_hUrb);
	FSunPackLib::XCC_CloseMix(m_hBuildings);


	m_hCache = NULL;
	m_hConquer = NULL;
	m_hIsoSnow = NULL;
	m_hIsoTemp = NULL;
	m_hIsoUrb = NULL;
	m_hLocal = NULL;
	m_hTemperat = NULL;
	m_hSnow = NULL;
	m_hUrban = NULL;
	m_hTibSun = NULL;
	m_hBuildings = NULL;
	m_hIsoGen = NULL;


	int i = 0;
	for (i = 0; i < 100; i++) {
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hExpand);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hConquer);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hECache);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hIsoSnow);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hIsoTemp);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hIsoUrb);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hIsoGen);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hTemperat);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hSnow);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hUrban);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hSno);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hTem);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hUrb);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hBuildings);
		m_hExpand[i].hExpand = NULL;
	}

	for (i = 0; i < 100; i++) {
		FSunPackLib::XCC_CloseMix(m_hECache[i]);
	}

	MEMORYSTATUS ms;
	ms.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	int cs = ms.dwAvailPhys + ms.dwAvailPageFile;

	errstream << "CLoading::Unload finished. Available memory: " << cs << endl;
	errstream.flush();
}


HMIXFILE CLoading::FindFileInMix(LPCTSTR lpFilename, TheaterChar* pTheaterChar)
{
	if (pTheaterChar)
		*pTheaterChar = TheaterChar::None;

	int i;
	// MW: added ecache support
	for (i = 100; i >= 0; i--) {
		HMIXFILE cuExp = m_hECache[i];

		if (cuExp != NULL) {
			if (FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp))
				return cuExp;
		}
	}

	for (i = 100; i >= 0; i--) {
		EXPANDMIX cuExp = m_hExpand[i];

		if (cuExp.hExpand != NULL) {
			if (FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hExpand))
				return cuExp.hExpand;
			if (cuExp.hECache != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hECache))
				return cuExp.hECache;
			if (cuExp.hConquer != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hConquer))
				return cuExp.hConquer;
			if (cuExp.hLocal != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hLocal))
				return cuExp.hLocal;
			if (cuExp.hTemperat != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hTemperat))
				return cuExp.hTemperat;
			if (strcmp(lpFilename, "yayard.shp") == NULL)
				FSunPackLib::_DEBUG_EnableLogs = true;
			if (cuExp.hSnow != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hSnow))
				return cuExp.hSnow;
			FSunPackLib::_DEBUG_EnableLogs = false;
			if (cuExp.hGeneric != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hGeneric))
				return cuExp.hGeneric;
			if (cuExp.hUrban != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hUrban))
				return cuExp.hUrban;
			if (cuExp.hUrbanN != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hUrbanN))
				return cuExp.hUrbanN;
			if (cuExp.hLunar != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hLunar))
				return cuExp.hLunar;
			if (cuExp.hDesert != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hDesert))
				return cuExp.hDesert;
			if (cuExp.hBuildings != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hBuildings))
				return cuExp.hBuildings;
			if (cuExp.hIsoGen != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoGen))
				return cuExp.hIsoGen;
			if (cuExp.hIsoGenMd != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoGenMd))
				return cuExp.hIsoGenMd;
			if (cuExp.hIsoTemp != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoTemp)) {
				if (pTheaterChar) *pTheaterChar = TheaterChar::T;
				return cuExp.hIsoTemp;
			}
			if (cuExp.hIsoSnow != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoSnow)) {
				if (pTheaterChar) *pTheaterChar = TheaterChar::A;
				return cuExp.hIsoSnow;
			}
			if (cuExp.hIsoUrb != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoUrb)) {
				if (pTheaterChar) *pTheaterChar = TheaterChar::U;
				return cuExp.hIsoUrb;
			}
			if (cuExp.hIsoUbn != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoUbn)) {
				if (pTheaterChar) *pTheaterChar = TheaterChar::N;
				return cuExp.hIsoUbn;
			}
			if (cuExp.hIsoDes != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoDes)) {
				if (pTheaterChar) *pTheaterChar = TheaterChar::D;
				return cuExp.hIsoDes;
			}
			if (cuExp.hIsoLun != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoLun)) {
				if (pTheaterChar) *pTheaterChar = TheaterChar::L;
				return cuExp.hIsoLun;
			}
			if (cuExp.hIsoUbnMd != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoUbnMd)) {
				if (pTheaterChar) *pTheaterChar = TheaterChar::N;
				return cuExp.hIsoUbnMd;
			}
			if (cuExp.hIsoDesMd != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoDesMd)) {
				if (pTheaterChar) *pTheaterChar = TheaterChar::D;
				return cuExp.hIsoDesMd;
			}
			if (cuExp.hIsoLunMd != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoLunMd)) {
				if (pTheaterChar) *pTheaterChar = TheaterChar::L;
				return cuExp.hIsoLunMd;
			}
			if (cuExp.hSno != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hSno))
				return cuExp.hSno;
			if (cuExp.hTem != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hTem))
				return cuExp.hTem;
			if (cuExp.hUrb != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hUrb))
				return cuExp.hUrb;
			if (cuExp.hUbn != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hUbn))
				return cuExp.hUbn;
			if (cuExp.hLun != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hLun))
				return cuExp.hLun;
			if (cuExp.hDes != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hDes))
				return cuExp.hDes;
			if (cuExp.hMarble != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hMarble))
				return cuExp.hMarble;
		}
	}

	if (m_hTibSun != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hTibSun))
		return m_hTibSun;
	if (m_hLanguage != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hLanguage))
		return m_hLanguage;
	if (m_hLangMD != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hLangMD))
		return m_hLangMD;
	if (m_hLocal != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hLocal))
		return m_hLocal;
	if (m_hBuildings != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hBuildings))
		return m_hBuildings;
	if (m_hCache != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hCache))
		return m_hCache;
	if (m_hConquer != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hConquer))
		return m_hConquer;
	if (m_hTemperat != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hTemperat))
		return m_hTemperat;
	if (m_hSnow != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hSnow))
		return m_hSnow;
	if (m_hUrban != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hUrban))
		return m_hUrban;
	if (m_hUrbanN != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hUrbanN))
		return m_hUrbanN;
	if (m_hLunar != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hLunar))
		return m_hLunar;
	if (m_hDesert != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hDesert))
		return m_hDesert;
	if (m_hIsoGen != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hIsoGen)) {
		return m_hIsoGen;
	}
	if (m_hIsoTemp != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hIsoTemp)) {
		if (pTheaterChar) *pTheaterChar = TheaterChar::T;
		return m_hIsoTemp;
	}
	if (m_hIsoSnow != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hIsoSnow)) {
		if (pTheaterChar) *pTheaterChar = TheaterChar::A;
		return m_hIsoSnow;
	}
	if (m_hIsoUrb != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hIsoUrb)) {
		if (pTheaterChar) *pTheaterChar = TheaterChar::U;
		return m_hIsoUrb;
	}
	if (m_hIsoUbn != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hIsoUbn)) {
		if (pTheaterChar) *pTheaterChar = TheaterChar::N;
		return m_hIsoUbn;
	}
	if (m_hIsoLun != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hIsoLun)) {
		if (pTheaterChar) *pTheaterChar = TheaterChar::L;
		return m_hIsoLun;
	}
	if (m_hIsoDes != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hIsoDes)) {
		if (pTheaterChar) *pTheaterChar = TheaterChar::D;
		return m_hIsoDes;
	}
	if (m_hTem != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hTem))
		return m_hTem;
	if (m_hSno != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hSno))
		return m_hSno;
	if (m_hUrb != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hUrb))
		return m_hUrb;
	if (m_hUbn != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hUbn))
		return m_hUbn;
	if (m_hLun != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hLun))
		return m_hLun;
	if (m_hDes != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hDes))
		return m_hDes;

	if (m_hMarble != NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hMarble))
		return m_hMarble;



	return NULL;
}



void CLoading::InitPalettes()
{
	errstream << "InitPalettes() called\n";

	m_palettes.Init();

	errstream << "\n";
	errstream.flush();
}

void CLoading::InitTMPs(CProgressCtrl* prog)
{
	m_palettes.FetchPalettes();


	MEMORYSTATUS ms;
	ms.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	int cs = ms.dwAvailPhys + ms.dwAvailPageFile;

	errstream << "InitTMPs() called. Available memory: " << cs << endl;
	errstream.flush();

	// we need to have that here, CMapData::UpdateIniFile() is too late for the shore hack
	shoreset = tiles->GetInteger("General", "ShorePieces");
	waterset = tiles->GetInteger("General", "WaterSet");






	int i, tcount = 0;

	for (i = 0; i < 10000; i++) {
		CString tset;
		char c[50];
		itoa(i, c, 10);
		int e;
		for (e = 0; e < 4 - strlen(c); e++) {
			tset += "0";
		}
		tset += c;
		CString sec = "TileSet";
		sec += tset;

		auto const pSec = tiles->TryGetSection(sec);
		if (!pSec) {
			break;
		}

		for (e = 0; e < pSec->GetInteger("TilesInSet"); e++) {
			tcount++;
		}
	}

	if (prog) prog->SetRange(0, tcount);
	if (prog) prog->SetPos(0);

	if (*tiledata != NULL) delete[] * tiledata;
	*tiledata = new(TILEDATA[tcount]);
	*tiledata_count = tcount;

	DWORD tilecount = 0;
	for (i = 0; i < 10000; i++) {
		CString tset;
		char c[50];
		itoa(i, c, 10);
		int e;
		for (e = 0; e < 4 - strlen(c); e++)
			tset += "0";
		tset += c;
		CString sec = "TileSet";
		sec += tset;

		auto const pSec = tiles->TryGetSection(sec);
		if (!pSec) {
			break;
		}

		BOOL bTib, bMorph, bPlace, bMadness;
		bPlace = TRUE;
		bTib = FALSE;
		bMorph = FALSE;
		bMadness = FALSE;

		if (pSec->GetBool("AllowTiberium")) {
			bTib = TRUE;
		}

		if (pSec->GetBool("Morphable")) {
			bMorph = TRUE;
		}

		if (!pSec->GetBool("AllowToPlace", true)) {
			bPlace = FALSE;
		}

		if (!pSec->GetString("NonMarbleMadness").IsEmpty()) {
			bMadness = TRUE;
		}
		auto const tilesetAnimSection = tiles->TryGetSection(tiles->GetString(sec, "SetName"));

		tilesets_start[i] = tilecount;


		for (e = 0; e < tiles->GetInteger(sec, "TilesInSet"); e++) {
			std::string sId = std::format("{:02}", e + 1);
			CString filename = tiles->GetString(sec, "FileName");
			filename += sId.c_str();

			CString bas_f = filename;

			CString suffix;

			if (tiles == &tiles_t) suffix = ".tem";
			if (tiles == &tiles_s) suffix = ".sno";
			if (tiles == &tiles_u) suffix = ".urb";
			if (tiles == &tiles_un) suffix = ".ubn";
			if (tiles == &tiles_l) suffix = ".lun";
			if (tiles == &tiles_d) suffix = ".des";

			filename += suffix;
			HTSPALETTE hPalette = m_palettes.m_hPalIsoTemp;
			if (tiles == &tiles_s) {
				hPalette = m_palettes.m_hPalIsoSnow;
			}
			if (tiles == &tiles_u) {
				hPalette = m_palettes.m_hPalIsoUrb;
			}
			if (tiles == &tiles_t) {
				hPalette = m_palettes.m_hPalIsoTemp;
			}
			if (tiles == &tiles_un) {
				hPalette = m_palettes.m_hPalIsoUbn;
			}
			if (tiles == &tiles_d) {
				hPalette = m_palettes.m_hPalIsoDes;
			}
			if (tiles == &tiles_l) {
				hPalette = m_palettes.m_hPalIsoLun;
			}

			// MW add: use other...
			if (FindFileInMix(filename) == NULL && tiles == &tiles_un) {
				filename = bas_f + ".urb";
				hPalette = m_palettes.m_hPalIsoUrb;
			}
			if (FindFileInMix(filename) == NULL) {
				filename = bas_f + ".tem";
				hPalette = m_palettes.m_hPalIsoTemp;
			}

			(*tiledata)[tilecount].bAllowTiberium = bTib;
			(*tiledata)[tilecount].bAllowToPlace = bPlace;
			(*tiledata)[tilecount].bMorphable = bMorph;
			(*tiledata)[tilecount].bMarbleMadness = bMadness;
			(*tiledata)[tilecount].wTileSet = i;

			int reps;
			for (reps = 0; reps < 5; reps++) {
				CString r_filename = filename;

				if (reps > 0) {
					char c[3];
					c[0] = 'a' + reps - 1;
					c[1] = '.';
					c[2] = 0;

					r_filename.Replace(".", c);

					if (!LoadTile(r_filename, FindFileInMix(filename), hPalette, tilecount, TRUE))
						break;
				} else {
					LoadTile(filename, FindFileInMix(filename), hPalette, tilecount, FALSE);
				}
			}

			if (tilesetAnimSection) {
				auto anim = tilesetAnimSection->GetString(std::format("Tile{}Anim", sId).c_str());
				auto offsetX = tilesetAnimSection->GetInteger(std::format("Tile{}XOffset", sId).c_str());
				auto offsetY = tilesetAnimSection->GetInteger(std::format("Tile{}YOffset", sId).c_str());
				auto attachesTo = tilesetAnimSection->GetInteger(std::format("Tile{}AttachesTo", sId).c_str());
				auto animFileName = anim + suffix;
				HMIXFILE hAnimMix = FindFileInMix(animFileName);
				if (hAnimMix) {
					auto& tile = (*tiledata)[tilecount];
					if (tile.wTileCount <= attachesTo) {
						ASSERT(tile.wTileCount > attachesTo);
					} else {
						auto& subtile = tile.tiles[attachesTo];
						SHPHEADER shp_h;
						SHPIMAGEHEADER shpi_h;
						auto animPic = std::make_shared<PICDATA>();
						auto rawPic = std::make_shared<std::vector<BYTE>>();
						animPic->rawPic = rawPic;
						FSunPackLib::SetCurrentSHP(animFileName, hAnimMix);
						FSunPackLib::XCC_GetSHPHeader(&shp_h);
						int imageNum = 1;
						FSunPackLib::XCC_GetSHPImageHeader(1, &shpi_h);
						if (shpi_h.unknown == 0) {
							// shadow
							imageNum = 0;
						}
						FSunPackLib::XCC_GetSHPImageHeader(imageNum, &shpi_h);
						if (FSunPackLib::LoadSHPImage(imageNum, *rawPic)) {
							subtile.anim = animPic;
							animPic->pic = animPic->rawPic->data();
							animPic->bType = PICDATA_TYPE_SHP;
							animPic->pal = iPalIso;
							animPic->wWidth = animPic->wMaxWidth = shp_h.cx;
							animPic->wHeight = animPic->wMaxHeight = shp_h.cy;
							animPic->x = offsetX;
							animPic->y = offsetY;
							animPic->createVBorder();
						}
					}
				}
			}

			tilecount++;
			if (prog != NULL /*&& tilecount%15==0*/) {
				prog->SetPos(tilecount);
				prog->UpdateWindow();
			}
		}


	}

	tilecount = 0;
	for (i = 0; i < 10000; i++) {
		CString tset;
		char c[50];
		itoa(i, c, 10);
		int e;
		for (e = 0; e < 4 - strlen(c); e++)
			tset += "0";
		tset += c;
		CString sec = "TileSet";
		sec += tset;

		auto const pSec = tiles->TryGetSection(sec);
		if (!pSec) {
			break;
		}

		int madnessid = pSec->GetInteger("MarbleMadness");

		for (e = 0; e < pSec->GetInteger("TilesInSet"); e++) {
			if (madnessid) {
				(*tiledata)[tilecount].wMarbleGround = tilesets_start[madnessid] + (tilecount - tilesets_start[i]);
			} else {
				(*tiledata)[tilecount].wMarbleGround = 0xFFFF;
			}
			tilecount++;
		}
	}

}

#ifdef NOSURFACES // first version, using palettized (or extracted) data
BOOL CLoading::LoadTile(LPCSTR lpFilename, HMIXFILE hOwner, HTSPALETTE hPalette, DWORD dwID, BOOL bReplacement)
{
	last_succeeded_operation = 12;


	int tileCount;

	if (FSunPackLib::XCC_DoesFileExist(lpFilename, hOwner)) {
		FSunPackLib::SetCurrentTMP(lpFilename, hOwner);
		{

			//FSunPackLib::SetCurrentTMP((CString)AppPath+"\\TmpTmp.tmp"/* lpFilename*/, NULL/*hOwner*/);
			int tileWidth, tileHeight;
			RECT rect;
			FSunPackLib::XCC_GetTMPInfo(&rect, &tileCount, &tileWidth, &tileHeight);


			BYTE** pics = new(BYTE * [tileCount]);

			if (FSunPackLib::LoadTMPImage(0, tileCount, pics)) // be aware this allocates memory!
			//if(FSunPackLib::LoadTMPImageInSurface(v.dd,lpFilename, 0, tileCount, pics, hPalette, hOwner))
			{
				TILEDATA* td;
				if (!bReplacement) td = &(*tiledata)[dwID];
				else {

					TILEDATA* lpTmp = NULL;
					if ((*tiledata)[dwID].bReplacementCount) {
						lpTmp = new(TILEDATA[(*tiledata)[dwID].bReplacementCount]);
						memcpy(lpTmp, (*tiledata)[dwID].lpReplacements, sizeof(TILEDATA) * (*tiledata)[dwID].bReplacementCount);
					}

					(*tiledata)[dwID].lpReplacements = new(TILEDATA[(*tiledata)[dwID].bReplacementCount + 1]);

					if ((*tiledata)[dwID].bReplacementCount) {
						memcpy((*tiledata)[dwID].lpReplacements, lpTmp, sizeof(TILEDATA) * (*tiledata)[dwID].bReplacementCount);
						delete[] lpTmp;
					}

					td = &(*tiledata)[dwID].lpReplacements[(*tiledata)[dwID].bReplacementCount];
					(*tiledata)[dwID].bReplacementCount++;
				}


				td->tiles = new(SUBTILE[tileCount]);
				td->wTileCount = tileCount;
				td->cx = tileWidth;
				td->cy = tileHeight;
				td->rect = rect;

				int i;
				for (i = 0; i < tileCount; i++) {
					if (pics[i] != NULL) {

						int cx, cy;
						BYTE height, terraintype, direction;
						POINT p;
						FSunPackLib::XCC_GetTMPTileInfo(i, &p, &cx, &cy, &direction, &height, &terraintype, &td->tiles[i].rgbLeft, &td->tiles[i].rgbRight);

						td->tiles[i].pic = pics[i];
						td->tiles[i].sX = p.x;
						td->tiles[i].sY = p.y;
						td->tiles[i].wWidth = cx;
						td->tiles[i].wHeight = cy;
						td->tiles[i].bZHeight = height;
						td->tiles[i].bTerrainType = terraintype;
						td->tiles[i].bHackedTerrainType = terraintype;
						td->tiles[i].bDirection = direction;

						td->tiles[i].vborder = new(VBORDER[cy]);

						int k;
						int size = 0;
						BOOL TranspInside = FALSE;
						for (k = 0; k < cy; k++) {
							int l, r;
							BOOL* ti = NULL;
							if (!TranspInside) ti = &TranspInside;
							GetDrawBorder(pics[i], cx, k, l, r, 0, ti);
							td->tiles[i].vborder[k].left = l;
							td->tiles[i].vborder[k].right = r;

							if (r >= l)
								size += r - l + 1;
						}

#ifdef NOSURFACES_EXTRACT

						if (!TranspInside) {
							// extract the palette data!
							td->tiles[i].bNotExtracted = FALSE;
							td->tiles[i].pic = new(BYTE[size * bpp]);
							int l;
							int pos = 0;
							for (k = 0; k < cy; k++) {
								int left, right;
								left = td->tiles[i].vborder[k].left;
								right = td->tiles[i].vborder[k].right;
								for (l = left; l <= right; l++) {
									memcpy(&td->tiles[i].pic[pos], &iPalIso[pics[i][l + k * cx]], bpp);
									pos += bpp;
								}
							}
							delete[] pics[i];
						} else
							td->tiles[i].bNotExtracted = TRUE;
#endif


						if (terraintype == 0xa) {
#ifdef RA2_MODE
							td->tiles[i].bHackedTerrainType = TERRAINTYPE_WATER;
#else
							td->tiles[i].bHackedTerrainType = TERRAINTYPE_WATER;
#endif
						}
						if (terraintype == TERRAINTYPE_ROUGH) td->tiles[i].bHackedTerrainType = TERRAINTYPE_GROUND;

						//if((*tiledata)[dwID].wTileSet==waterset) (*tiledata)[dwID].tiles[i].bHackedTerrainType=TERRAINTYPE_WATER;

						// shore hack: check fsdata.ini for new shore terrain
						if (td->wTileSet == shoreset) {
							int h;
							for (h = 0; h < (*tiledata_count); h++) {
								if ((*tiledata)[h].wTileSet == shoreset) break;
							}

							int pos = dwID - h;
							char c[50];
							itoa(pos, c, 10);
							CString hack = c;
							hack += "_";
							itoa(i, c, 10);
							hack += c;/*
							hack+="_";
							itoa(i/tileWidth, c, 10);
							hack+=c;*/

							CString section = "ShoreTerrainTS";
#ifdef RA2_MODE
							section = "ShoreTerrainRA2";
#endif
							auto const& sec = g_data.GetSection(section);
							auto const hackValIdx = sec.FindIndex(hack);
							if (hackValIdx >= 0) {
								int t = atoi(sec.Nth(hackValIdx).second);
								if (t) {
									td->tiles[i].bHackedTerrainType = TERRAINTYPE_WATER;
								} else {
									td->tiles[i].bHackedTerrainType = 0xe;
								}
							}
						}
						if ((*tiledata)[dwID].wTileSet == waterset) (*tiledata)[dwID].tiles[i].bHackedTerrainType = TERRAINTYPE_WATER;
					} else {
						td->tiles[i].pic = NULL;
						td->tiles[i].sX = 0;
						td->tiles[i].sY = 0;
						td->tiles[i].wWidth = 0;
						td->tiles[i].wHeight = 0;
						td->tiles[i].bZHeight = 0;
						td->tiles[i].bTerrainType = 0;
						td->tiles[i].bDirection = 0;
						td->tiles[i].vborder = NULL;

					}
				}
			}

			if (tileCount > 0) delete[] pics;
		}
	} else {
		errstream << lpFilename << " not found" << endl;
		return FALSE;
	}





	return TRUE;


}
#endif

#ifdef NOSURFACES_OBJECTS // palettized
void CLoading::LoadOverlayGraphic(const CString& lpOvrlName_, int iOvrlNum)
{
	last_succeeded_operation = 11;

	CString image; // the image used
	SHPHEADER head;
	char theat = cur_theat;
	BYTE** lpT = NULL;

	char OvrlID[50];
	itoa(iOvrlNum, OvrlID, 10);

	HTSPALETTE hPalette = m_palettes.m_hPalIsoTemp;
	if (cur_theat == 'T') {
		hPalette = m_palettes.m_hPalIsoTemp;
#ifdef RA2_MODE
		hPalette = m_palettes.m_hPalIsoTemp;
#endif
	}
	if (cur_theat == 'A') {
		hPalette = m_palettes.m_hPalIsoSnow;
#ifdef RA2_MODE
		hPalette = m_palettes.m_hPalIsoSnow;
#endif
	}
	if (cur_theat == 'U' && m_palettes.m_hPalIsoUrb) {
		hPalette = m_palettes.m_hPalIsoUrb;
#ifdef RA2_MODE
		hPalette = m_palettes.m_hPalIsoUrb;
#endif		
	}
	if (cur_theat == 'N' && m_palettes.m_hPalIsoUbn) {
		hPalette = m_palettes.m_hPalIsoUbn;
#ifdef RA2_MODE
		hPalette = m_palettes.m_hPalIsoUbn;
#endif		
	}
	if (cur_theat == 'L' && m_palettes.m_hPalIsoLun) {
		hPalette = m_palettes.m_hPalIsoLun;
#ifdef RA2_MODE
		hPalette = m_palettes.m_hPalIsoLun;
#endif		
	}
	if (cur_theat == 'D' && m_palettes.m_hPalIsoDes) {
		hPalette = m_palettes.m_hPalIsoDes;
#ifdef RA2_MODE
		hPalette = m_palettes.m_hPalIsoDes;
#endif		
	}

	HTSPALETTE forcedPalette = 0;
	const auto& isoPalettePrefixes = g_data["ForceOvrlIsoPalettePrefix"];
	const auto& unitPalettePrefixes = g_data["ForceOvrlUnitPalettePrefix"];
	const CString sOvrlName(lpOvrlName_);
	if (unitPalettePrefixes.end() != std::find_if(unitPalettePrefixes.begin(), unitPalettePrefixes.end(), [&sOvrlName](const auto& pair) {return sOvrlName.Find(pair.second) == 0; })) {
		forcedPalette = m_palettes.GetUnitPalette(cur_theat);
	}
	if (isoPalettePrefixes.end() != std::find_if(isoPalettePrefixes.begin(), isoPalettePrefixes.end(), [&sOvrlName](const auto& pair) {return sOvrlName.Find(pair.second) == 0; })) {
		forcedPalette = m_palettes.GetIsoPalette(cur_theat);
	}

	HMIXFILE hMix;

	CString lpOvrlName = lpOvrlName_;
	if (lpOvrlName.Find(' ') >= 0) {
		lpOvrlName = lpOvrlName.Left(lpOvrlName.Find(' '));
	}
	//if(strchr(lpOvrlName, ' ')!=NULL) strchr(lpOvrlName, ' ')[0]=0;
	//if(lpOvrlName

	auto const isveinhole = rules.GetBool(lpOvrlName, "IsVeinholeMonster");
	auto const istiberium = rules.GetBool(lpOvrlName, "Tiberium");
	auto const isveins = rules.GetBool(lpOvrlName, "IsVeins");

	image = rules.GetStringOr(lpOvrlName, "Image", lpOvrlName);

	TruncSpace(image);

	CString imagerules = image;
	auto const& imageID = art.GetString(image, "Image");
	if (!imageID.IsEmpty()) {
		image = imageID;
	}

	TruncSpace(image);

	CString filename = image + theatToSuffix(cur_theat);

	hMix = FindFileInMix(filename);

	const auto& artSection = art[image];

	if (hMix == NULL) {
		filename = image + ".shp";
		if (artSection.GetBool("NewTheater")) {
			filename.SetAt(1, theat);
		}

		if (cur_theat == 'U' && m_palettes.m_hPalUnitUrb) {
			hPalette = m_palettes.m_hPalUnitUrb;
		}
		if (cur_theat == 'T') {
			hPalette = m_palettes.m_hPalUnitTemp;
		}
		if (cur_theat == 'A') {
			hPalette = m_palettes.m_hPalUnitSnow;
		}
		if (cur_theat == 'N') {
			hPalette = m_palettes.m_hPalUnitUbn;
		}
		if (cur_theat == 'L') {
			hPalette = m_palettes.m_hPalUnitLun;
		}
		if (cur_theat == 'D') {
			hPalette = m_palettes.m_hPalUnitDes;
		}

		hMix = FindFileInMix(filename);

		//errstream << (LPCSTR)filename << " " << endl;
		//errstream.flush();

		if (hMix == NULL) {
			filename.SetAt(1, 'T');
			hMix = FindFileInMix(filename);
		}
		if (hMix == NULL) {
			filename.SetAt(1, 'A');
			hMix = FindFileInMix(filename);
		}
		if (hMix == NULL) {
			filename.SetAt(1, 'U');
			hMix = FindFileInMix(filename);
		}
		if (hMix == NULL) {
			filename.SetAt(1, 'N');
			hMix = FindFileInMix(filename);
		}
		if (hMix == NULL) {
			filename.SetAt(1, 'L');
			hMix = FindFileInMix(filename);
		}
		if (hMix == NULL) {
			filename.SetAt(1, 'D');
			hMix = FindFileInMix(filename);
		}

		if (cur_theat == 'T' || cur_theat == 'U') {
			hPalette = m_palettes.m_hPalUnitTemp;
		} else
			hPalette = m_palettes.m_hPalUnitSnow;

	}

	if (hMix == NULL) {
		filename = image + ".tem";
		hMix = FindFileInMix(filename);
		if (hMix) {
			hPalette = m_palettes.m_hPalIsoTemp;
		}
	}
	if (hMix == NULL) {
		filename = image + ".sno";
		hMix = FindFileInMix(filename);
		if (hMix) {
			hPalette = m_palettes.m_hPalIsoSnow;
		}
	}
	if (hMix == NULL) {
		filename = image + ".urb";
		hMix = FindFileInMix(filename);
		if (hMix && m_palettes.m_hPalIsoUrb) {
			hPalette = m_palettes.m_hPalIsoUrb;
		}
	}
	if (hMix == NULL) {
		filename = image + ".ubn";
		hMix = FindFileInMix(filename);
		if (hMix && m_palettes.m_hPalIsoUbn) {
			hPalette = m_palettes.m_hPalIsoUbn;
		}
	}
	if (hMix == NULL) {
		filename = image + ".lun";
		hMix = FindFileInMix(filename);
		if (hMix && m_palettes.m_hPalIsoLun) {
			hPalette = m_palettes.m_hPalIsoLun;
		}
	}
	if (hMix == NULL) {
		filename = image + ".des";
		hMix = FindFileInMix(filename);
		if (hMix && m_palettes.m_hPalIsoDes) {
			hPalette = m_palettes.m_hPalIsoDes;
		}
	}

	if (isveinhole == TRUE || isveins == TRUE || istiberium == TRUE) {
		hPalette = m_palettes.m_hPalTemp;
#ifndef RA2_MODE
		hPalette = m_palettes.m_hPalUnitTemp;
#endif
	}

	hPalette = forcedPalette ? forcedPalette : hPalette;


	if (hMix != NULL) {

		errstream << "Overlay: " << (LPCSTR)filename << endl;
		errstream.flush();

		FSunPackLib::SetCurrentSHP(filename, hMix);
		{

			FSunPackLib::XCC_GetSHPHeader(&head);

			int i;
			int maxPics = head.c_images;
			if (maxPics > max_ovrl_img) {
				maxPics = max_ovrl_img;
			}


			// create an array of pointers to directdraw surfaces
			lpT = new(BYTE * [maxPics]);
			memset(lpT, 0, sizeof(BYTE) * maxPics);

			// if tiberium, change color
			BOOL bIsBlueTib = FALSE;
			BOOL bIsGreenTib = FALSE;
			RGBTRIPLE rgbOld[16], rgbNew;
#ifndef RA2_MODE
			if (istiberium) {
				if (lpOvrlName[4] == '_') // other than green!
					bIsBlueTib = TRUE;
				else
					bIsGreenTib = TRUE;



				int i;
				for (i = 0; i < 16; i++) {
					if (bIsGreenTib) {
						rgbNew.rgbtBlue = 0;
						if (i != 0)
							rgbNew.rgbtGreen = 255 - i * 16 + 1;
						else
							rgbNew.rgbtGreen = 255;
						rgbNew.rgbtRed = 0;
					} else if (bIsBlueTib) {
						if (i != 0)
							rgbNew.rgbtBlue = 255 - i * 16 + 1;
						else
							rgbNew.rgbtBlue = 255;
						rgbNew.rgbtGreen = 0;
						rgbNew.rgbtRed = 0; // change green/blue (not RGB but BGR)
					}

					int rb = rgbNew.rgbtBlue;
					int rr = rgbNew.rgbtRed;

					FSunPackLib::SetTSPaletteEntry(hPalette, 0x10 + i, &rgbNew, &rgbOld[i]);
				}
			}
#endif

			FSunPackLib::LoadSHPImage(0, maxPics, lpT);

#ifndef RA2_MODE
			if (istiberium)
				for (i = 0; i < 16; i++)
					FSunPackLib::SetTSPaletteEntry(hPalette, 0x10 + i, &rgbOld[i], NULL);
#endif

			for (i = 0; i < maxPics; i++) {
				SHPIMAGEHEADER imghead;
				FSunPackLib::XCC_GetSHPImageHeader(i, &imghead);

				// MW: fixed april 20th, 2002
				 // is it a shadow or not used image?
				if (imghead.unknown == 0 && !g_data.GetBool("Debug", "IgnoreSHPImageHeadUnused")) {
					if (lpT[i]) {
						delete[] lpT[i];
						lpT[i] = NULL;
					}
				}

				if (/*imghead.unknown &&*/ lpT[i]) {
					char ic[50];
					itoa(i, ic, 10);

					PICDATA p;
					p.pic = lpT[i];

					if (hPalette == m_palettes.m_hPalIsoTemp
						|| hPalette == m_palettes.m_hPalIsoUrb
						|| hPalette == m_palettes.m_hPalIsoSnow
						|| hPalette == m_palettes.m_hPalIsoDes
						|| hPalette == m_palettes.m_hPalIsoLun
						|| hPalette == m_palettes.m_hPalIsoUbn) {
						p.pal = iPalIso;
					}
					if (hPalette == m_palettes.m_hPalTemp
						|| hPalette == m_palettes.m_hPalUrb
						|| hPalette == m_palettes.m_hPalSnow
						|| hPalette == m_palettes.m_hPalDes
						|| hPalette == m_palettes.m_hPalLun
						|| hPalette == m_palettes.m_hPalUbn) {
						p.pal = iPalTheater;
					}
					if (hPalette == m_palettes.m_hPalUnitTemp
						|| hPalette == m_palettes.m_hPalUnitUrb
						|| hPalette == m_palettes.m_hPalUnitSnow
						|| hPalette == m_palettes.m_hPalUnitDes
						|| hPalette == m_palettes.m_hPalUnitLun
						|| hPalette == m_palettes.m_hPalUnitUbn) {
						p.pal = iPalUnit;
					}

					p.vborder = new(VBORDER[head.cy]);
					int k;
					for (k = 0; k < head.cy; k++) {
						int l, r;
						GetDrawBorder(lpT[i], head.cx, k, l, r, 0);
						p.vborder[k].left = l;
						p.vborder[k].right = r;
					}

					p.x = imghead.x;
					p.y = imghead.y;

					p.wHeight = imghead.cy;
					p.wWidth = imghead.cx;
					p.wMaxWidth = head.cx;
					p.wMaxHeight = head.cy;
					p.bType = PICDATA_TYPE_SHP;


					pics[(CString)"OVRL" + OvrlID + "_" + ic] = p;
				}
			}


			delete[] lpT;
		}

	}

	int i;
	for (i = 0; i < 0xFF; i++) {
		char ic[50];
		itoa(i, ic, 10);

		pics[(CString)"OVRL" + OvrlID + "_" + ic].bTried = true;
	}


}
#endif

void CLoading::OnDestroy()
{
	CDialog::OnDestroy();
}

void CLoading::InitVoxelNormalTables()
{
	try {
		std::ifstream f(std::string(AppPath) + "\\voxel_normal_tables.bin", std::ios::binary);
		m_voxelNormalTables.reset(new VoxelNormalTables(f));
	} catch (const std::runtime_error& e) {
		errstream << e.what() << std::endl;
		m_voxelNormalTables.reset(new VoxelNormalTables());
	}
}

void CLoading::CalcPicCount()
{
	m_pic_count = 0;

	CString bmps = (CString)AppPath + "\\pics\\*.bmp";
	if (!theApp.m_Options.bDoNotLoadBMPs) {
		CFileFind ff;
		if (ff.FindFile(bmps)) {
			// found bmp
			m_pic_count++;
			while (ff.FindNextFile()) m_pic_count++;
		}
	}

	m_bmp_count = m_pic_count;

	if (!theApp.m_Options.bDoNotLoadVehicleGraphics)	m_pic_count += rules["VehicleTypes"].Size();
	if (!theApp.m_Options.bDoNotLoadOverlayGraphics) m_pic_count += rules["OverlayTypes"].Size();
	if (!theApp.m_Options.bDoNotLoadInfantryGraphics) m_pic_count += rules["InfantryTypes"].Size();
	if (!theApp.m_Options.bDoNotLoadBuildingGraphics) m_pic_count += rules["BuildingTypes"].Size();
	if (!theApp.m_Options.bDoNotLoadAircraftGraphics) m_pic_count += rules["AircraftTypes"].Size();
	if (!theApp.m_Options.bDoNotLoadTreeGraphics) m_pic_count += rules["TerrainTypes"].Size();

	int i;
	/*
		if(!theApp.m_Options.bDoNotLoadSnowGraphics)
		{
			tiledata=&s_tiledata;
			tiles=&tiles_s;
			tiledata_count=&s_tiledata_count;
			for(i=0;i<10000;i++)
			{
				CString tset;
				char c[50];
				itoa(i, c, 10);
				int e;
				for(e=0;e<4-strlen(c);e++)
					tset+="0";
				tset+=c;
				CString sec="TileSet";
				sec+=tset;

				if(tiles->sections.find(sec)==tiles->sections.end()) break;

				for(e=0;e<atoi(tiles->sections[sec].values["TilesInSet"]);e++)
				{
					m_pic_count++;
				}

			}
		}

		if(!theApp.m_Options.bDoNotLoadTemperateGraphics)
		{
			tiledata=&t_tiledata;
			tiles=&tiles_t;
			tiledata_count=&t_tiledata_count;
			for(i=0;i<10000;i++)
			{
				CString tset;
				char c[50];
				itoa(i, c, 10);
				int e;
				for(e=0;e<4-strlen(c);e++)
					tset+="0";
				tset+=c;
				CString sec="TileSet";
				sec+=tset;

				if(tiles->sections.find(sec)==tiles->sections.end()) break;

				for(e=0;e<atoi(tiles->sections[sec].values["TilesInSet"]);e++)
				{
					m_pic_count++;
				}

			}
		}*/
}


BOOL CLoading::InitDirectDraw()
{
	return ((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->RecreateSurfaces();
}

void CLoading::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CWnd* poswindow = GetDlgItem(IDC_SAVEOFTEN);

	if (!poswindow) return;

	RECT r1, r2;
	poswindow->GetWindowRect(&r1);
	GetWindowRect(&r2);

	dc.SetTextColor(RGB(255, 0, 0));
	dc.SetBkMode(TRANSPARENT);

	RECT dest;
	dest.top = r1.bottom - r2.top - 45;
	dest.bottom = r1.bottom - r2.top + 10;
	dest.left = 10;
	dest.right = r1.right - r1.left + 10;

	CFont f;
	f.CreatePointFont(90, "Tahoma");
	dc.SelectObject(&f);

}

void CLoading::FreeTileSet()
{
	// free the current tileset

	int i;
	for (i = 0; i < (*tiledata_count); i++) {
		int e;
		int z;

		// delete replacements. Replacements themself MUST NOT have replacements
		for (z = 0; z < (*tiledata)[i].bReplacementCount; z++) {
			TILEDATA& rept = (*tiledata)[i].lpReplacements[z];

			for (e = 0; e < rept.wTileCount; e++) {
#ifdef NOSURFACES
				BYTE* curSur = rept.tiles[e].pic;
				if (curSur) delete[] curSur;
				if (rept.tiles[e].vborder) delete[] rept.tiles[e].vborder;
#else
				LPDIRECTDRAWSURFACE4 curSur = rept.tiles[e].pic;
				if (curSur) curSur->Release();
#endif
			}

			delete[] rept.tiles;
			rept.tiles = NULL;
			rept.wTileCount = 0;
			rept.bReplacementCount = 0;

			ASSERT(!rept.lpReplacements); // make sure we don´t have replacements for this replacement
		}

		for (e = 0; e < (*tiledata)[i].wTileCount; e++) {
#ifdef NOSURFACES
			BYTE* curSur = (*tiledata)[i].tiles[e].pic;
			if (curSur) delete[] curSur;
			if ((*tiledata)[i].tiles[e].vborder) delete[](*tiledata)[i].tiles[e].vborder;
#else
			LPDIRECTDRAWSURFACE4 curSur = (*tiledata)[i].tiles[e].pic;
			if (curSur) curSur->Release();
#endif
		}

		delete[]((*tiledata)[i].tiles);
		(*tiledata)[i].tiles = NULL;
		(*tiledata)[i].wTileCount = 0;
		(*tiledata)[i].bReplacementCount = 0;
		if ((*tiledata)[i].lpReplacements) delete[](*tiledata)[i].lpReplacements;
		(*tiledata)[i].lpReplacements = NULL;
	}
	if (*tiledata) delete[](*tiledata);
	(*tiledata) = NULL;
	(*tiledata_count) = 0;
}

void CLoading::FreeAll()
{
	last_succeeded_operation = 14;

	// MW fix: We need to set tiledata and tiledata_count to the old pointers again
	int t = 0;
	if (tiledata == &t_tiledata) t = 0;
	if (tiledata == &s_tiledata) t = 1;
	if (tiledata == &u_tiledata) t = 2;
	if (tiledata == &un_tiledata) t = 3;
	if (tiledata == &l_tiledata) t = 4;
	if (tiledata == &d_tiledata) t = 4;

	//try{
	tiledata = &t_tiledata;
	tiledata_count = &t_tiledata_count;
	FreeTileSet();
	tiledata = &s_tiledata;
	tiledata_count = &s_tiledata_count;
	FreeTileSet();
	tiledata = &u_tiledata;
	tiledata_count = &u_tiledata_count;
	FreeTileSet();

	// MW New tilesets:
	tiledata = &un_tiledata;
	tiledata_count = &un_tiledata_count;
	FreeTileSet();
	tiledata = &l_tiledata;
	tiledata_count = &l_tiledata_count;
	FreeTileSet();
	tiledata = &d_tiledata;
	tiledata_count = &d_tiledata_count;
	FreeTileSet();
	/* }
	catch(...)
	{
		errstream << "Exception in FreeTileSet()" << endl;
	}*/

	// MW Reset tiledata & tiledata_count
	if (t == 0) { tiledata = &t_tiledata; tiledata_count = &t_tiledata_count; }
	if (t == 1) { tiledata = &s_tiledata; tiledata_count = &s_tiledata_count; }
	if (t == 2) { tiledata = &u_tiledata; tiledata_count = &u_tiledata_count; }
	if (t == 3) { tiledata = &un_tiledata; tiledata_count = &un_tiledata_count; }
	if (t == 4) { tiledata = &l_tiledata; tiledata_count = &l_tiledata_count; }
	if (t == 5) { tiledata = &d_tiledata; tiledata_count = &d_tiledata_count; }

	map<CString, PICDATA>::iterator i = pics.begin();
	for (int e = 0; e < pics.size(); e++) {
		try {
#ifdef NOSURFACES_OBJECTS			
			if (i->second.bType == PICDATA_TYPE_BMP) {
				if (i->second.pic != NULL) {
					((LPDIRECTDRAWSURFACE4)i->second.pic)->Release();
				}
			} else {
				if (auto pPic = std::exchange(i->second.pic, nullptr)) {
					delete[](pPic);
				}
				if (auto pBorder = std::exchange(i->second.vborder, nullptr)) {
					delete[](pBorder);
				}
			}
#else
			if (i->second.pic != NULL) i->second.pic->Release();
#endif

			i->second.pic = NULL;
		} catch (...) {
			CString err;
			err = "Access violation while trying to release surface ";
			char c[6];
			itoa(e, c, 10);
			err += c;

			err += "\n";
			OutputDebugString(err);
			continue;
		}

		i++;
	}



	try {
		CFinalSunDlg* dlg = ((CFinalSunDlg*)theApp.m_pMainWnd);
		if (dlg->m_view.m_isoview->lpds != NULL) {
			dlg->m_view.m_isoview->lpds->Release();
			dlg->m_view.m_isoview->lpds = NULL;
		}
		if (dlg->m_view.m_isoview->lpdsBack != NULL) {
			dlg->m_view.m_isoview->lpdsBack->Release();
			dlg->m_view.m_isoview->lpdsBack = NULL;
		}
		if (dlg->m_view.m_isoview->lpdsBackHighRes != NULL) {
			dlg->m_view.m_isoview->lpdsBackHighRes->Release();
			dlg->m_view.m_isoview->lpdsBackHighRes = NULL;
		}
		if (dlg->m_view.m_isoview->lpdsTemp != NULL) {
			dlg->m_view.m_isoview->lpdsTemp->Release();
			dlg->m_view.m_isoview->lpdsTemp = NULL;
		}
		if (dlg->m_view.m_isoview->dd != NULL) {
			dlg->m_view.m_isoview->dd->Release();
			dlg->m_view.m_isoview->dd = NULL;
		}
		if (dlg->m_view.m_isoview->dd_1 != NULL) {
			dlg->m_view.m_isoview->dd_1->Release();
			dlg->m_view.m_isoview->dd_1 = NULL;
		}
	} catch (...) {
		errstream << "Exception while freeing DirectDraw" << endl;
	}
}

void CLoading::PostNcDestroy()
{

	//	delete this; // on stack!
	//	CDialog::PostNcDestroy();
}

void CLoading::PrepareHouses()
{
	int p = 0;
	auto const& sideSec = rules["Sides"];
	for (auto i = 0; i < sideSec.Size(); i++) {
		int t = 0;
		auto const& sideParams = sideSec.Nth(i).second;
		for (;;) {
			auto const paramN = GetParam(sideParams, t);
			if (paramN.IsEmpty()) {
				break;
			}
			sides[p].name = paramN;
			sides[p].orig_n = i; // mw fix instead of =i
			t++;
			p++;
		}
	}

	auto const houseSec = rules[HOUSES];
	for (auto i = 0; i < houseSec.Size(); i++) {
		HOUSEINFO& house = houses[i];
		house.name = houseSec.Nth(i).second;
		house.bPlayable = rules.GetBool(house.name, "Multiplay");
		memset(&house.color, 0, sizeof(RGBTRIPLE));
		int e;
		for (e = 0; e < sides.size(); e++) {
			house.side = nullptr;
			if (sides[e].name == rules.GetString(house.name, "Side")) {
				house.side = &sides[e];
			}
		}

	}
}


BYTE* Search(BYTE** lpData, BYTE* lpSearched)
{
	BYTE* lpDat = *lpData;

	lpDat = (BYTE*)strstr((LPCSTR)lpDat, (LPCSTR)lpSearched) + strlen((LPCSTR)lpSearched);

	return lpDat;
}

class SortDummy2 {
public:
	bool operator() (const CString& x, const CString& y) const
	{
		// the length is more important than spelling (numbers!!!)...
		if (x.GetLength() < y.GetLength()) return true;
		if (x.GetLength() == y.GetLength()) {
			CString x2 = x;
			CString y2 = y;
			x2.MakeLower();
			y2.MakeLower();
			if (x2 < y2) return true;
		}

		return false;

	}
};

extern TranslationMap AllStrings;
void CLoading::LoadStrings()
{
	last_succeeded_operation = 9;

#ifdef RA2_MODE

	// MW April 17th, 2002:
	// ra2md.csf supported!
	std::string file = "RA2.CSF";
	if (yuri_mode)
		file = "RA2MD.CSF";

	errstream << "LoadStrings() executing" << endl;
	errstream.flush();

	BYTE* lpData = NULL;
	DWORD dwSize;
	if (DoesFileExist((std::string(TSPath) + "\\" + file).c_str())) {
		std::ifstream f(std::string(TSPath) + "\\" + file, std::ios::binary);
		if (f.good()) {
			f.seekg(0, std::ios::end);
			auto size = f.tellg();
			if (size > 0) {
				lpData = new(BYTE[size]);
				dwSize = size;
				f.seekg(0, std::ios::beg);
				f.read(reinterpret_cast<char*>(lpData), dwSize);
			}
		}
	}
	errstream << "LoadStrings() loading from mix" << endl;
	errstream.flush();

	if (!lpData) {
		HMIXFILE hMix = FindFileInMix(file.c_str());
		//HMIXFILE hMix=m_hLanguage;
		if (hMix) {
			if (FSunPackLib::XCC_ExtractFile(file, u8AppDataPath + "\\RA2Tmp.csf", hMix)) {
				std::ifstream f(u8AppDataPath + "\\RA2Tmp.csf", std::ios::binary);
				if (f.good()) {
					f.seekg(0, std::ios::end);
					auto size = f.tellg();
					if (size > 0) {
						lpData = new(BYTE[size]);
						dwSize = size;
						f.seekg(0, std::ios::beg);
						f.read(reinterpret_cast<char*>(lpData), dwSize);
					}
				}
			}

			if (!lpData) {
				MessageBox("String file not found, using rules.ini names", "Error");
				return;
			}
		} else {
			MessageBox("String file not found, using rules.ini names", "Error");
			return;
		}

	}

	BYTE* orig = static_cast<BYTE*>(lpData);

	if (!(lpData = Search(&lpData, (BYTE*)" FSC"))) return;

	RA2STRFILEHEAD head;
	memcpy(&head, lpData, RA2STRFILEHEADSIZE);


	lpData += RA2STRFILEHEADSIZE;

	map<CString, XCString, SortDummy2> strings;

	int i;
	//try{
	for (i = 0; i < head.dwCount1; i++) {
		ASSERT(orig + dwSize > lpData);

		if (!(lpData = lpData + 4))//Search(&lpData, (BYTE*)" LBL"))) 
		{

			return;
		}

		RA2STRINGENTRY entry;
		memcpy(&entry.dwFlag, lpData, 4);
		lpData += 4;

		DWORD dwCharCount;
		memcpy(&dwCharCount, lpData, 4);
		lpData += 4;

		BYTE* lpID = new(BYTE[dwCharCount + 1]);
		memcpy(lpID, lpData, dwCharCount);
		lpData += dwCharCount;
		lpID[dwCharCount] = 0;
		entry.id = (CHAR*)new(BYTE[dwCharCount + 1]);
		strcpy(entry.id, (LPCSTR)lpID);
		entry.id[dwCharCount] = 0;
		entry.id_size = dwCharCount;
		// enable this to show the string ID
		delete[](lpID);


		BOOL b2Strings = FALSE;

		if (lpData[0] == 'W')
			b2Strings = TRUE;

		if (!(lpData = lpData + 4))//Search(&lpData, (BYTE*)" RTS")))
		{
			return;
		}

		memcpy(&dwCharCount, lpData, 4);
		lpData += 4;

		WCHAR* lpwID = new(WCHAR[dwCharCount + 1]);
		int e;
		for (e = 0; e < dwCharCount; e++) {
			WCHAR w;
			memcpy(&w, lpData, 2);
			lpData += 2;
			lpwID[e] = ~w;
		}
		lpwID[dwCharCount] = 0;
		entry.value = lpwID;
		entry.value_size = dwCharCount;

		if (b2Strings) {

			memcpy(&dwCharCount, lpData, 4);
			lpData += 4;



			CHAR* lpwID2 = new(CHAR[dwCharCount + 1]);
			int e;
			for (e = 0; e < dwCharCount; e++) {
				WCHAR w;
				memcpy(&w, lpData, 1);
				lpData += 1;
				lpwID2[e] = w;
			}
			lpwID2[dwCharCount] = 0;
			entry.value_asc = lpwID2;
			entry.value_asc_size = dwCharCount;
		}

		/*BYTE* bByte=new(BYTE[entry.value_size+1]);
		wcstombs((LPSTR)bByte, entry.value, entry.value_size+1);
		bByte[entry.value_size]=0;*/
		strings[entry.id].SetString(entry.value, entry.value_size);
		AllStrings[entry.id].SetString(entry.value, entry.value_size);


		//delete[] bByte;
	}



	for (auto const& [secName, sec] : rules) {
		auto const& uiNameLabel = sec.GetString("UIName");
		if (uiNameLabel.IsEmpty()) {
			CCStrings[secName].SetString(sec.GetString("Name"));
			continue;
		}
		auto const found = strings.find(uiNameLabel);
		// no record
		if (found == strings.end()) {
			//MessageBox((LPSTR)(LPCSTR)rules.GetSection(i)->values["Name"], secName);
			CCStrings[secName].SetString(sec.GetString("Name"));
			continue;
		}
		//MessageBox(strings[uiNameLabel].cString);
		if (!found->second.bUsedDefault) {
			//CCStrings[secName].cString=strings[uiNameLabel].cString; //.SetString(strings[uiNameLabel].wString, strings[uiNameLabel].len);
			CCStrings[secName].SetString(strings[uiNameLabel].wString, strings[uiNameLabel].len);
			continue;
		}
		CCStrings[secName].SetString(strings[uiNameLabel].wString, strings[uiNameLabel].len);
		CCStrings[secName].cString = sec.GetString("Name");
	}


#else
	int i;
	for (i = 0; i < rules.sections.size(); i++) {
		if (rules.GetSection(i)->FindIndex("Name") >= 0) {
			//CCStrings[*rules.GetSectionName(i)].cString=rules.GetSection(i)->values["Name"];
			//CCStrings[*rules.GetSectionName(i)].SetString=rul
			CCStrings[*rules.GetSectionName(i)].SetString((LPSTR)(LPCSTR)rules.GetSection(i)->values["Name"]);
		}
	}
#endif


}

void CLoading::HackRules()
{
	if (editor_mode == ra2_mode) {

		auto const& gateOne = rules.GetString("General", "GDIGateOne");
		if (!gateOne.IsEmpty()) {
			auto const pSec = rules.TryGetSection("BuildingTypes");
			ASSERT(pSec != nullptr);
			if (!pSec->HasValue(gateOne)) {
				pSec->Append(gateOne, gateOne);
			}
		}

#ifdef RA2_MODE
		// RULES(MD).INI has the incorrect colors set for the following houses, let's remap them to the expected values.
		// Fixup YuriCountry colour
		if (rules.GetString("YuriCountry", "Color") == "DarkRed") {
			rules.SetString("YuriCountry", "Color", "Purple");
		}
		// Fixup Allied colors
		static const char* allied_houses[] = {
			"British",
			"French",
			"Germans",
			"Americans",
			"Alliance",
		};

		for (auto const& name : allied_houses) {
			if (rules.GetString(name, "Color") == "Gold") {
				rules.SetString(name, "Color", "DarkBlue");
			}
		}
		// Fixup Nod color
		if (rules.GetString("Nod", "Color") == "Gold") {
			rules.SetString("Nod", "Color", "DarkRed");
		}
#endif

	}

}

void CLoading::PrepareBuildingTheaters()
{
	// stub

}

/*
This actually just checks what palette the unit (only buildings make sense)
uses. Used for the ObjectBrowser (so it only shows those buildings that really exist
in the specific terrain)

Added: MW March 20th 2001
*/
void CLoading::PrepareUnitGraphic(const CString& lpUnittype)
{
	CString _rules_image; // the image used
	CString filename; // filename of the image
	char theat = cur_theat; // standard theater char is t (Temperat). a is snow.

	BOOL bAlwaysSetChar; // second char is always theater, even if NewTheater not specified!
	WORD wStep = 1; // step is 1 for infantry, buildings, etc, and for shp vehicles it specifies the step rate between every direction
	WORD wStartWalkFrame = 0; // for examply cyborg reaper has another walk starting frame
	int iTurretOffset = 0; // used for centering y pos of turret (if existing)
	BOOL bStructure = rules["BuildingTypes"].HasValue(lpUnittype); // is this a structure?

	 // make sure we only use it for buildings now
	if (!bStructure) {
		return;
	}
	auto const bPowerUp = !rules.GetString(lpUnittype, "PowersUpBuilding").IsEmpty();

	HTSPALETTE hPalette;
	if (theat == 'T') hPalette = m_palettes.m_hPalIsoTemp;
	if (theat == 'A') hPalette = m_palettes.m_hPalIsoSnow;
	if (theat == 'U') hPalette = m_palettes.m_hPalIsoUrb;
	if (theat == 'L') hPalette = m_palettes.m_hPalIsoLun;
	if (theat == 'D') hPalette = m_palettes.m_hPalIsoDes;
	if (theat == 'N') hPalette = m_palettes.m_hPalIsoUbn;

	CIsoView& v = *((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview;

	_rules_image = rules.GetStringOr(lpUnittype, "Image", lpUnittype);

	CString _art_image = _rules_image;
	auto const& imageID = art.GetString(_rules_image, "Image");
	if (!imageID.IsEmpty() && !g_data.GetBool("IgnoreArtImage", _rules_image)) {
		_art_image = imageID;
	}

	const CString& image = _art_image;
	const auto& rulesSection = rules[lpUnittype];
	const auto& artSection = art[image];

	// is it a shp graphic?
	if (!artSection.GetBool("Voxel")) {
		try {
			auto shp = FindUnitShp(image, cur_theat, artSection);
			if (!shp) {
				return;
			}

			filename = shp->filename;
			hPalette = shp->palette;

			auto limited_to_theater = artSection.GetBool("TerrainPalette") ? shp->mixfile_theater : TheaterChar::None;

			if (filename == "tibtre01.tem" || filename == "tibtre02.tem" || filename == "tibtre03.tem" || filename == "veinhole.tem") {
				hPalette = m_palettes.m_hPalUnitTemp;
			}

			if (shp->mixfile > 0) {

				BOOL bSuccess = FSunPackLib::SetCurrentSHP(filename, shp->mixfile);
				if (!bSuccess) {
					filename = image + ".sno";
					if (cur_theat == 'T' || cur_theat == 'U') hPalette = m_palettes.m_hPalIsoTemp;
					HMIXFILE hShpMix = FindFileInMix(filename);
					bSuccess = FSunPackLib::SetCurrentSHP(filename, hShpMix);

					if (!bSuccess) {
						return;
					}
				}

				if (bSuccess) {


					char ic[50];
					int i = 0;
					itoa(i, ic, 10);

					// just fill in a stub entry - Final* will automatically retry loading once the graphic really must be loaded
					PICDATA p;
					p.pic = NULL;
					p.x = 0;
					p.y = 0;
					p.wHeight = 0;
					p.wWidth = 0;
					p.wMaxWidth = 0;
					p.wMaxHeight = 0;
					p.bType = PICDATA_TYPE_SHP;
					p.bTerrain = limited_to_theater;


					pics[image + ic] = p;


				}

			}

		} catch (...) {
			errstream << " exception " << endl;
			errstream.flush();
		}


	}

}