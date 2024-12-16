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

// Map->h: Interface for the class CMap.
//
// CMap holds all information for the current map
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAP_H__9278CAC0_D4E7_11D3_B63B_444553540001__INCLUDED_)
#define AFX_MAP_H__9278CAC0_D4E7_11D3_B63B_444553540001__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <array>
#include <vector>
#include "structs.h"
#include "variables.h"
#include "macros.h"
#include "ovrlinline.h"
#include "Tube.h"
#include "IniFile.h"	// Hinzugefügt von der Klassenansicht

extern TILEDATA** tiledata;
extern DWORD* tiledata_count;
extern std::ofstream errstream;
extern map<int, int> tilesets_start;
extern CIniFile* tiles;
extern CFinalSunApp theApp;
extern int shoreset;

extern CIniFile rules;

extern TILEDATA* un_tiledata;
extern CIniFile g_data;

extern int ramp2set;
extern int pave2set;
extern int ramp2set_start;
extern int pave2set_start;
extern int rampset_start;
extern int rampset;



#define MAPDATA_UPDATE_FROM_INI 0
#define MAPDATA_UPDATE_TO_INI 1
#define MAPDATA_UPDATE_TO_INI_ALL 2
#define MAPDATA_UPDATE_TO_INI_ALL_PREVIEW 4
#define MAPDATA_UPDATE_TO_INI_ALL_COMPRESSED 8

struct NODEDATA
{
	NODEDATA();
	int type;
	int index;
	CString house;
};

// mapfielddata is the data of every field in an extracted isomappack!
struct MAPFIELDDATA
{
	unsigned short wX;
	unsigned short wY;
	WORD wGround;
	BYTE bData[3];
	BYTE bHeight;
	BYTE bData2[1];
};
#define MAPFIELDDATA_SIZE 11

/*
struct TILEDATA{};

contains the information needed for one field of the map.
*/
struct FIELDDATA
{
	FIELDDATA();
	short unit; // unit number
	short infantry[SUBPOS_COUNT]; // infantry number
	short aircraft; // aircraft number
	short structure; // structure number 
	short structuretype; // structure type id
	short terrain; // terrain number
	int terraintype; // terrain type id
#ifdef SMUDGE_SUPP
	short smudge;
	int smudgetype;
#endif
	short waypoint; // waypoint number

	NODEDATA node; // node info
	BYTE overlay; // overlay number
	BYTE overlaydata; // overlay data info
	WORD wGround; // ground type (tile)
	WORD bMapData; // add. data
	BYTE bSubTile;
	BYTE bHeight; // height of tile
	BYTE bMapData2; // add. data2
	short celltag; // celltag uses	

	//std::uint16_t wTubeId; // tube ID
	//char cTubePart; // 0 is start, 1 is exit, and 2-101 are tube parts
	unsigned bReserved : 1; // for program usage
	unsigned bHide : 1;
	unsigned bRedrawTerrain : 1; // force redraw
	unsigned bCliffHack : 1;
	unsigned bRNDImage : 4; // for using a,b,c of tmp tiles
};

struct SNAPSHOTDATA
{
	SNAPSHOTDATA();
	int left;
	int top;
	int bottom;
	int right;

	BOOL* bRedrawTerrain;
	BYTE* overlay;
	BYTE* overlaydata;
	WORD* wGround;
	WORD* bMapData;
	BYTE* bSubTile;
	BYTE* bHeight;
	BYTE* bMapData2;
	BYTE* bRNDData;
	//CIniFile mapfile;
};



class CMapData
{
public:

	BOOL m_noAutoObjectUpdate;

	void SetFielddataAt(DWORD dwPos, FIELDDATA* lpFd)
	{
		if (dwPos >= fielddata_size) return;

		RemoveOvrlMoney(fielddata[dwPos].overlay, fielddata[dwPos].overlaydata);
		fielddata[dwPos] = (*lpFd);
		AddOvrlMoney(fielddata[dwPos].overlay, fielddata[dwPos].overlaydata);
	};

	DWORD GetMapPos(int mapX, int mapY) const
	{
		return mapX + mapY * m_IsoSize;
	}
	DWORD GetMapPos(const MapCoords& coords) const
	{
		return coords.x + coords.y * m_IsoSize;
	}

	bool hasLat(WORD wTileSet) const;

	void HideField(DWORD dwPos, BOOL bHide);
	void SetReserved(DWORD dwPos, BYTE val);
	DWORD GetTileID(DWORD dwTileSet, int iTile);
	int GetNecessarySlope(DWORD dwPos);
	void CreateSlopesAt(DWORD dwPos);
	void CreateMap(DWORD dwWidth, DWORD dwHeight, LPCTSTR lpTerrainType, DWORD dwGroundHeight);
	BOOL SetTileAt(DWORD dwPos, DWORD dwID, DWORD dwTile)
	{
		if (dwPos > fielddata_size) {
			return FALSE;
		}

		int replacement = 0; // MW fix: ignore for bridges
		if ((*tiledata)[dwID].bReplacementCount && tiles->GetInteger("General", "BridgeSet") != (*tiledata)[dwID].wTileSet) {
			replacement = rand() * (1 + (*tiledata)[dwID].bReplacementCount) / RAND_MAX;
		}

		fielddata[dwPos].wGround = dwID;
		fielddata[dwPos].bSubTile = dwTile;
		fielddata[dwPos].bRNDImage = replacement;

		int e;
		fielddata[dwPos].bRedrawTerrain = FALSE;
		int xx, yy;
		for (xx = -2; xx < 0; xx++) {
			for (yy = -2; yy < 0; yy++) {
				int npos = dwPos + xx + yy * m_IsoSize;
				if (npos > 0 && fielddata[dwPos].bHeight - fielddata[npos].bHeight >= 4) {
					fielddata[dwPos].bRedrawTerrain = TRUE;
					break;
				}
			}
			if (fielddata[dwPos].bRedrawTerrain) {
				break;
			}
		}

		Mini_UpdatePos(dwPos % m_IsoSize, dwPos / m_IsoSize, IsMultiplayer());

		return TRUE;
	}

	void SetHeightAt(DWORD dwPos, BYTE bHeight)
	{
		int height = (char)bHeight;
		if (height > MAXHEIGHT) height = MAXHEIGHT; // too high else
		if (height < 0) height = 0;
		if (dwPos < fielddata_size) fielddata[dwPos].bHeight = height;
	}


	int GetBuildingID(const CString& lpBuildingName);
	void ImportRUL(LPCTSTR lpFilename);
	void ExportRulesChanges(const char* filename);
	void DeleteRulesSections();
	DWORD GetWaypointCount() const;
	DWORD GetCelltagCount() const;
	WCHAR* GetUnitName(LPCTSTR lpID) const;
	DWORD GetTerrainCount() const;
	DWORD GetAircraftCount() const;
	DWORD GetStructureCount() const;
	DWORD GetUnitCount() const;
	DWORD GetInfantryCount() const;
	void GetStdUnitData(DWORD dwIndex, STDOBJECTDATA* lpStdUnit) const;
	void GetStdAircraftData(DWORD dwIndex, STDOBJECTDATA* lpStdAircraft) const;
	void GetNthWaypointData(DWORD dwIdx, CString* lpID, DWORD* lpdwPos) const;
	void GetWaypointData(DWORD dwId, CString* lpID, DWORD* lpdwPos) const;
	BOOL IsGroundObjectAt(DWORD dwPos) const;
	BOOL AddTerrain(LPCTSTR lpType, DWORD dwPos, int suggestedIndex = -1);
	void GetTerrainData(DWORD dwIndex, CString* lpType) const;
	void GetTerrainData(DWORD dwIndex, TERRAIN* lpTerrain) const;
	BOOL AddUnit(UNIT* lpUnit, LPCTSTR lpType = NULL, LPCTSTR lpHouse = NULL, DWORD dwPos = 0, CString suggestedID = "");
	BOOL AddAircraft(AIRCRAFT* lpAircraft, LPCTSTR lpType = NULL, LPCTSTR lpHouse = NULL, DWORD dwPos = 0, CString suggestedID = "");
	void GetCelltagData(DWORD dwIndex, CString* lpTag, DWORD* lpdwPos) const;
	BOOL AddCelltag(LPCTSTR lpTag, DWORD dwPos);
	CString GetAircraftData(DWORD dwIndex, AIRCRAFT* lpAircraft) const;
	CString GetUnitData(DWORD dwIndex, UNIT* lpUnit) const;
	void GetInfantryData(DWORD dwIndex, INFANTRY* lpInfantry) const;
	void GetStdInfantryData(DWORD dwIndex, STDOBJECTDATA* lpStdInfantry) const;
	INT GetUnitTypeID(LPCTSTR lpType);
	void InitializeUnitTypes();
	BOOL AddStructure(STRUCTURE* lpStructure, LPCTSTR lpType = NULL, LPCTSTR lpHouse = NULL, DWORD dwPos = 0, CString suggestedID = "");
	BOOL AddInfantry(INFANTRY* lpInfantry, int suggestedIndex = -1, LPCTSTR lpType = NULL, LPCTSTR lpHouse = NULL, DWORD dwPos = 0);
	BOOL AddNode(NODE* lpNode, WORD dwPos);
	void GetStdStructureData(DWORD dwIndex, STDOBJECTDATA* lpStdStructure) const;
	CString GetStructureData(DWORD dwIndex, STRUCTURE* lpStructure) const;
	BOOL AddWaypoint(CString lpID, DWORD dwPos);

	void DeleteNode(const CString& house, const int index);
	void DeleteTerrain(DWORD dwIndex);
	void DeleteAircraft(DWORD dwIndex);
	void DeleteStructure(DWORD dwIndex);
	void DeleteUnit(DWORD dwIndex);
	void DeleteCelltag(DWORD dwIndex);
	void DeleteWaypoint(DWORD id);
	void DeleteInfantry(DWORD dwIndex);

	INT GetCelltagAt(DWORD dwPos) const
	{
		return fielddata[dwPos].celltag;
	}
	INT GetCelltagAt(MapCoords pos) const
	{
		return GetCelltagAt(GetMapPos(pos));
	}
	INT GetWaypointAt(DWORD dwPos) const
	{
		return fielddata[dwPos].waypoint;
	}
	INT GetWaypointAt(MapCoords pos) const
	{
		return GetWaypointAt(GetMapPos(pos));
	}
	INT GetTerrainAt(DWORD dwPos) const
	{
		return fielddata[dwPos].terrain;
	}
	INT GetTerrainAt(MapCoords pos) const
	{
		return GetTerrainAt(GetMapPos(pos));
	}
	INT GetAirAt(DWORD dwPos) const
	{
		return fielddata[dwPos].aircraft;
	}
	INT GetAirAt(MapCoords pos) const
	{
		return GetAirAt(GetMapPos(pos));
	}
	INT GetStructureAt(DWORD dwPos) const
	{
		if (fielddata[dwPos].structure > -1) {
			return fielddata[dwPos].structure;
		}
		return -1;
	}
	INT GetStructureAt(MapCoords pos) const
	{
		return GetStructureAt(GetMapPos(pos));
	}
	INT GetUnitAt(DWORD dwPos) const
	{
		return fielddata[dwPos].unit;
	}
	INT GetUnitAt(MapCoords pos) const
	{
		return GetUnitAt(GetMapPos(pos));
	}
	INT GetInfantryAt(DWORD dwPos, DWORD dwSubPos = 0xFFFFFFFF) const
	{
		if (dwSubPos == 0xFFFFFFFF) {
			int i;
			for (i = 0; i < SUBPOS_COUNT; i++)
				if (fielddata[dwPos].infantry[i] != -1)
					return fielddata[dwPos].infantry[i];
			return -1;
		}
		return fielddata[dwPos].infantry[dwSubPos];
	}
	INT GetInfantryAt(MapCoords pos, DWORD dwSubPos = 0xFFFFFFFF)
	{
		return GetInfantryAt(GetMapPos(pos), dwSubPos);
	}
	INT GetNodeAt(DWORD dwPos, CString& lpHouse) const;
	INT GetNodeAt(MapCoords pos, CString& lpHouse) const
	{
		return GetNodeAt(GetMapPos(pos), lpHouse);
	}
	INT GetHeightAt(DWORD dwPos) const
	{
		return fielddata[dwPos].bHeight;
	}
	INT GetHeightAt(const MapCoords& coords) const
	{
		return GetHeightAt(GetMapPos(coords));
	}

	const FIELDDATA* GetFielddataAt(DWORD dwPos) const
	{
		if (dwPos >= fielddata_size) {
			outside_f.bReserved = 1;
			return &outside_f;
		}

		return &fielddata[dwPos];
	};

	FIELDDATA* GetFielddataAt(DWORD dwPos)
	{
		if (dwPos >= fielddata_size) {
			outside_f.bReserved = 1;
			return &outside_f;
		}

		return &fielddata[dwPos];
	};

	const FIELDDATA* GetFielddataAt(const MapCoords& pos) const
	{
		auto dwPos = GetMapPos(pos);
		if (dwPos >= fielddata_size) {
			outside_f.bReserved = 1;
			return &outside_f;
		}

		return &fielddata[dwPos];
	};

	FIELDDATA* GetFielddataAt(const MapCoords& pos)
	{
		auto dwPos = GetMapPos(pos);
		if (dwPos >= fielddata_size) {
			outside_f.bReserved = 1;
			return &outside_f;
		}

		return &fielddata[dwPos];
	};

	BYTE GetOverlayDataAt(DWORD dwPos);
	void SetOverlayDataAt(DWORD dwPos, BYTE bValue);
	BYTE GetOverlayAt(DWORD dwPos);
	void SetOverlayAt(DWORD dwPos, BYTE bValue);
	void ClearOverlay();
	void ClearOverlayData();

	const std::vector<std::unique_ptr<CTube>>& GetTubes() const
	{
		return m_tubes;
	}

	DWORD GetIsoSize() const
	{
		return m_IsoSize;
	}
	void LoadMap(const std::string& file);
	void UpdateIniFile(DWORD dwFlags = MAPDATA_UPDATE_TO_INI);
	CIniFile& GetIniFile();
	CString GetAITriggerTypeID(DWORD dwAITriggerType);
	DWORD GetAITriggerTypeIndex(LPCTSTR lpID);
	WORD GetHouseIndex(LPCTSTR lpHouse);
	void GetAITriggerType(DWORD dwAITriggerType, AITRIGGERTYPE* pAITrg);
	DWORD GetAITriggerTypeCount();
	CString GetHouseID(WORD wHouse, BOOL bCountry = FALSE);
	WORD GetHousesCount(BOOL bCountries = FALSE);
	WORD GetHeight() const
	{
		return m_maprect.bottom;
	};
	WORD GetWidth() const
	{
		return m_maprect.right;
	};
	BOOL IsRulesSection(LPCTSTR lpSection);

	CMapData();
	virtual ~CMapData();
	void Pack(BOOL bCreatePreview = FALSE, BOOL bCompression = FALSE);
	void Unpack();
	void UpdateTreeInfo(const CString* lpTreeType = NULL);
	void UpdateBuildingInfo(const CString* lpUnitType = NULL);
	void CalcMapRect();

	// MW change: UpdateStructures() public, so that houses dialog can access it
	void UpdateStructures(BOOL bSave = FALSE);

	MapCoords ToMapCoords(ProjectedCoords xy) const;
	MapCoords ToMapCoords3d(ProjectedCoords xy, int mapZ) const;
	MapCoords ToMapCoords3d(ProjectedCoords xy, bool bAllowAccessBehindCliffs = false, bool ignoreHideFlagsAndOutside = false) const;
	ProjectedCoords ProjectCoords(MapCoords xy) const;
	ProjectedCoords ProjectCoords3d(MapCoords xy) const;
	ProjectedCoords ProjectCoords3d(MapCoords xy, int z) const;
	bool isInside(MapCoords xy) const;

	__forceinline CPoint GetMiniMapPos(MapCoords mapCoords)
	{
		int x, y;
		GetMiniMapPos(mapCoords.x, mapCoords.y, x, y);
		return CPoint(static_cast<LONG>(x), static_cast<LONG>(y));
	}

private:
	void UpdateTubes(BOOL bSave);
	MAPFIELDDATA* GetMappackPointer(DWORD dwPos);

	void UpdateMapFieldData(BOOL bSave = FALSE);

	DWORD m_IsoSize;
	mutable FIELDDATA outside_f;
	BOOL isInitialized;
	void UpdateCelltags(BOOL bSave = FALSE);
	void UpdateOverlay(BOOL bSave = FALSE);
	void UpdateNodes(BOOL bSave = FALSE);
	void UpdateWaypoints(BOOL bSave = FALSE);
	void UpdateUnits(BOOL bSave = FALSE);
	void UpdateTerrain(BOOL bSave = FALSE, int num = -1);
	void UpdateInfantry(BOOL bSave = FALSE);
	void UpdateAircraft(BOOL bSave = FALSE);





	map<CString, int> buildingid;
	map<CString, int> terrainid;
#ifdef SMUDGE_SUPP
	map<CString, int> smudgeid;
#endif

	static constexpr size_t overlayDataCapacity = 262144;

	BYTE m_Overlay[overlayDataCapacity]; // overlay byte values (extracted)
	BYTE m_OverlayData[overlayDataCapacity]; // overlay data byte values (extracted)
	BYTE* m_mfd;	// map field data buffer
	DWORD dwIsoMapSize;
	CIniFile m_mapfile;
	RECT m_maprect;
	RECT m_vismaprect;
	FIELDDATA* fielddata;
	int fielddata_size;
	SNAPSHOTDATA* m_snapshots;
	DWORD dwSnapShotCount;
	int m_cursnapshot;
	int m_money;

	vector<STRUCTUREPAINT> m_structurepaint;


protected:

	void InitMinimap();
	vector<std::unique_ptr<CTube>> m_tubes;

	// vectors for terrain, infantry, structures and units, as those need to be displayed very fast.
	// we don´t need them for aircraft right now, as there won´t be many aircrafts on the map anyway.
#ifdef SMUDGE_SUPP
	vector<SMUDGE> m_smudges;
#endif
	vector<TERRAIN> m_terrain;
	vector<INFANTRY> m_infantry;
	vector<UNIT> m_units;
	vector<STRUCTURE> m_structures;

	// we use a dib to draw the minimap
	std::vector<BYTE> m_mini_colors;
	BITMAPINFO m_mini_biinfo;
	int m_mini_pitch;

	enum OverlayCreditsType
	{
		OverlayCredits_Riparius = 0,
		OverlayCredits_Cruentus = 1,
		OverlayCredits_Vinifera = 2,
		OverlayCredits_Aboreus = 3,
		OverlayCredits_NumOf
	};

	std::array<int, OverlayCredits_NumOf> m_overlayCredits;


	__forceinline void RemoveOvrlMoney(unsigned char ovrl, unsigned char ovrld)
	{
		if (ovrl >= RIPARIUS_BEGIN && ovrl <= RIPARIUS_END) {
			m_money -= (ovrld + 1) * m_overlayCredits[OverlayCredits_Riparius];
		}

		if (ovrl >= CRUENTUS_BEGIN && ovrl <= CRUENTUS_END) {
			m_money -= (ovrld + 1) * m_overlayCredits[OverlayCredits_Cruentus];
		}

		if (ovrl >= VINIFERA_BEGIN && ovrl <= VINIFERA_END) {
			m_money -= (ovrld + 1) * m_overlayCredits[OverlayCredits_Vinifera];
		}

		if (ovrl >= ABOREUS_BEGIN && ovrl <= ABOREUS_END) {
			m_money -= (ovrld + 1) * m_overlayCredits[OverlayCredits_Aboreus];
		}
	}

	__forceinline void AddOvrlMoney(unsigned char ovrl, unsigned char ovrld)
	{
		if (ovrl >= RIPARIUS_BEGIN && ovrl <= RIPARIUS_END) {
			m_money += (ovrld + 1) * m_overlayCredits[OverlayCredits_Riparius];
		}

		if (ovrl >= CRUENTUS_BEGIN && ovrl <= CRUENTUS_END) {
			m_money += (ovrld + 1) * m_overlayCredits[OverlayCredits_Cruentus];
		}

		if (ovrl >= VINIFERA_BEGIN && ovrl <= VINIFERA_END) {
			m_money += (ovrld + 1) * m_overlayCredits[OverlayCredits_Vinifera];
		}

		if (ovrl >= ABOREUS_BEGIN && ovrl <= ABOREUS_END) {
			m_money += (ovrld + 1) * m_overlayCredits[OverlayCredits_Aboreus];
		}
	}

	__forceinline void GetMiniMapPos(int i, int e, int& x, int& y)
	{
		const int pheight = m_mini_biinfo.bmiHeader.biHeight;

		const DWORD dwIsoSize = m_IsoSize;
		y = e / 2 + i / 2;
		x = dwIsoSize - i + e;

		int tx, ty;
		tx = GetWidth();
		ty = GetHeight();

		ty = ty / 2 + tx / 2;
		tx = dwIsoSize - GetWidth() + GetHeight();

		x -= tx;
		y -= ty;

		x += pheight;
		y += pheight / 2;
	}


	void Mini_UpdatePos(const int i, const int e, bool isMultiplayer);
	
	// helper function. Is val==iSet1 or val=iSet2?	
	__forceinline BOOL st(int val, int iSet)
	{

		if (val == iSet) return TRUE;
		return FALSE;
	}


public:
	void RedrawMinimap();
	BOOL GetInfantryINIData(int index, CString* lpINI);


#ifdef SMUDGE_SUPP
	void UpdateSmudges(BOOL bSave = FALSE, int num = -1);
	void DeleteSmudge(DWORD dwIndex);
	BOOL AddSmudge(SMUDGE* lpSmudge);
	void GetSmudgeData(DWORD dwIndex, SMUDGE* lpData) const;
	void UpdateSmudgeInfo(LPCSTR lpSmudgeType = NULL);
#endif

	BOOL IsYRMap();
	bool IsMapSection(const CString& sectionName);
	void ResizeMap(int iLeft, int iTop, DWORD dwNewWidth, DWORD dwNewHeight);
	void SmoothTiberium(DWORD dwPos);
	int GetPowerOfHouse(LPCTSTR lpHouse);
	int GetMoneyOnMap() const;
	int CalcMoneyOnMap();
	void GetMinimap(BYTE** lpData, BITMAPINFO* lpBI, int* pitch);
	void GetStructurePaint(int index, STRUCTUREPAINT* lpStructurePaint) const;
	void Paste(int x, int y, int z_mod);
	void Copy(int left = 0, int top = 0, int right = 0, int bottom = 0);
	CString GetTheater();
	BOOL IsMultiplayer();
	void CreateShore(int left, int top, int right, int bottom, BOOL bRemoveUseless = TRUE);
	void Redo();
	void SmoothAllAt(DWORD dwPos);

	void SmoothAt(DWORD dwPos, int iSmoothSet, int iLatSet, int iTargetSet, BOOL bIgnoreShore = TRUE);

	BOOL GetLocalSize(RECT* rect) const;
	void Undo();
	void TakeSnapshot(BOOL bEraseFollowing = TRUE, int left = 0, int top = 0, int right = 0, int bottom = 0);
	BOOL CheckMapPackData();
	int GetInfantryCountAt(DWORD dwPos);
	void DeleteTube(std::uint16_t wID);
	// void SetTube(WORD wID, CTube *lpTI);
	void SetTube(CTube* lpTI);
	CTube* GetTube(std::uint16_t wID);
};

inline bool CMapData::isInside(MapCoords xy) const
{
	return xy.x >= 0 && xy.y >= 0 && xy.x < m_IsoSize&& xy.y < m_IsoSize;
}

inline MapCoords CMapData::ToMapCoords(ProjectedCoords xy) const
{
	return ToMapCoords3d(xy, 0);
}

inline MapCoords CMapData::ToMapCoords3d(ProjectedCoords xy, int mapZ) const
{
	auto const cx = static_cast<float>(xy.x);
	auto const cy = static_cast<float>(xy.y + mapZ * f_y / 2);
	return MapCoords(
		cy / (float)f_y - cx / (float)f_x + (float)(m_IsoSize - 2) / 2 + (float)0.5,
		cy / (float)f_y + cx / (float)f_x - (float)(m_IsoSize - 2) / 2.0f - (float)0.5
	);
}

inline MapCoords CMapData::ToMapCoords3d(const ProjectedCoords xy, bool bAllowAccessBehindCliffs, bool ignoreHideFlagsAndOutside) const
{
	auto xy2d = ToMapCoords(xy);
	static const auto fxy = ProjectedVec(f_x, f_y).convertT<float>();
	auto ret = ignoreHideFlagsAndOutside ? xy2d : MapCoords(-1, -1);

	for (int i = 15; i >= 0; i--) {
		for (int e = 0; e < 3; e++) {
			const MapVec off(i, i);  // this is a vertical line starting from the bottom
			const MapVec off2(e == 1 ? -1 : 0, e == 2 ? -1 : 0);  // increase x or y or neither in map coordinates by one so that we have a broader area to check
			const MapCoords cur = xy2d + off + off2;

			if (isInside(cur)) {
				const auto& mfd = *GetFielddataAt(GetMapPos(cur));
				const int ground = mfd.wGround == 0xFFFF ? 0 : mfd.wGround;
				if (ignoreHideFlagsAndOutside || (!mfd.bHide && !(tiledata && (*tiledata)[ground].bHide))) {
					const auto curProj = ProjectCoords3d(cur);

					// now projCoords hold the logical pixel coordinates for the current field... 
					// we now need to check if cx and cy are in this field
					//if(*x >= m && *x<= m+f_x && *y>=n && *y<=n+f_y)
					{
						auto df1 = (xy - curProj).convertT<float>();
						auto dfScaled = df1 / fxy;
						auto d = Vec2<CSProjected, float>(dfScaled.y - dfScaled.x + 0.5f, dfScaled.y + dfScaled.x - 0.5f);
						if (d.x >= 0.0f && d.y >= 0.0f && d.x <= 1.0f && d.y <= 1.0f)// || (!bAllowAccessBehindCliffs && xy.y >= curProj.y)))
							//if (d == MapCoords(0, 0))
						{
							if (bAllowAccessBehindCliffs)
								ret = cur;
							else
								return cur;
						}
					}
				}
			}
		}
	}
	return ret;
}

inline ProjectedCoords CMapData::ProjectCoords(MapCoords xy) const
{
	return ProjectCoords3d(xy, 0);
}

inline ProjectedCoords CMapData::ProjectCoords3d(MapCoords xy, int z) const
{
	return ProjectedCoords(
		(m_IsoSize - 2 - xy.x + xy.y) * f_x / 2,
		(xy.y + xy.x - z) * f_y / 2
	);
}

inline ProjectedCoords CMapData::ProjectCoords3d(MapCoords xy) const
{
	return ProjectCoords3d(xy, isInside(xy) ? GetHeightAt(xy) : 0);
}

#endif // !defined(AFX_MAP_H__9278CAC0_D4E7_11D3_B63B_444553540001__INCLUDED_)
