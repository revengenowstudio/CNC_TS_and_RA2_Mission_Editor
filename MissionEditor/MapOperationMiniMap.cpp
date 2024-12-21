#include "stdafx.h"
#include "MapData.h"

void CMapData::Mini_UpdatePos(const int i, const int e, bool isMultiplayer)
{
	const int pwidth = m_mini_biinfo.bmiHeader.biWidth;
	const int pheight = m_mini_biinfo.bmiHeader.biHeight;

	if (m_mini_colors.empty() || !tiledata) {
		return;
	}


	const DWORD dwIsoSize = m_IsoSize;
	const int pitch = m_mini_pitch;

	int x = 0;
	int y = 0;
	GetMiniMapPos(i, e, x, y);
	y = pheight - y - 1;

	int xiso = i;
	int yiso = e;

	if (xiso >= m_IsoSize)
		xiso = m_IsoSize - 1;
	if (yiso >= m_IsoSize)
		yiso = m_IsoSize - 1;
	if (xiso < 0)
		xiso = 0;
	if (yiso < 0)
		yiso = 0;

	DWORD dwPos = xiso + yiso * dwIsoSize;

	int dwDrawPos = (x * 3 + y * pitch);

	int size = pitch * pheight;

	if (dwDrawPos >= size || x >= pwidth || y >= pheight || x < 0 || y < 0) {
		return;
	}
	if (dwPos >= m_IsoSize * m_IsoSize) {
		return;
	}
	if (dwDrawPos + 3 >= m_mini_colors.size()) {
		return;
	}

	RGBTRIPLE& col = (RGBTRIPLE&)m_mini_colors[dwDrawPos];
	RGBTRIPLE& col_r = (RGBTRIPLE&)m_mini_colors[(dwDrawPos + sizeof(RGBTRIPLE)) < size ? dwDrawPos + sizeof(RGBTRIPLE) : dwDrawPos];


	FIELDDATA td;
	td = *GetFielddataAt(dwPos);

	STDOBJECTDATA sod;
	sod.house = "";
	int ic;
	for (ic = 0; ic < SUBPOS_COUNT; ic++) {
		if (td.infantry[ic] >= 0) {
			GetStdInfantryData(td.infantry[ic], &sod);
		}
	}
	if (td.structure >= 0) {
		GetStdStructureData(td.structure, &sod);
	}
	if (td.aircraft >= 0) {
		GetStdAircraftData(td.aircraft, &sod);
	}
	if (td.unit >= 0) {
		GetStdUnitData(td.unit, &sod);
	}

	int ground = (td.wGround >= (*tiledata_count)) ? 0 : td.wGround;
	int subt = td.bSubTile;
	int ttype = 0;

	if (subt < (*tiledata)[ground].wTileCount) {
		ttype = (*tiledata)[ground].tiles[subt].bTerrainType;
	}

	// mw added check:
	if (subt >= (*tiledata)[ground].wTileCount) {
		return;
	}

	RGBTRIPLE& l = (*tiledata)[ground].tiles[subt].rgbLeft;
	RGBTRIPLE& r = (*tiledata)[ground].tiles[subt].rgbRight;

	int pos = GetHeightAt(dwPos);

	col.rgbtBlue = l.rgbtBlue;
	col.rgbtGreen = l.rgbtGreen;
	col.rgbtRed = l.rgbtRed;
	col_r.rgbtBlue = r.rgbtBlue;
	col_r.rgbtGreen = r.rgbtGreen;
	col_r.rgbtRed = r.rgbtRed;

	if (isGreenTiberium(td.overlay)) {
#ifndef RA2_MODE
		col.rgbtBlue = 0;
		col.rgbtGreen = 200;
		col.rgbtRed = 0;
		col_r = col;
#else
		col.rgbtBlue = 0;
		col.rgbtGreen = 250;
		col.rgbtRed = 250;
		col_r = col;
#endif
	} else if (td.overlay == OVRL_VEINS) {
		col.rgbtBlue = 120;
		col.rgbtGreen = 180;
		col.rgbtRed = 190;
		col_r = col;
	} else if (td.overlay == OVRL_VEINHOLE || td.overlay == OVRL_VEINHOLEBORDER) {
		col.rgbtBlue = 120;
		col.rgbtGreen = 160;
		col.rgbtRed = 165;
		col_r = col;
	} else if (td.overlay != 0xFF) {
		col.rgbtBlue = 20;
		col.rgbtGreen = 20;
		col.rgbtRed = 20;
		col_r = col;
	}


	if (sod.house.GetLength() > 0) {
		/*
		if(strstr(sod.house, houses[1].name))
		{
			col.rgbtRed=255;
			col.rgbtGreen=0;
			col.rgbtBlue=0;
		}
		else if(strstr(sod.house,houses[0].name))
		{
			col.rgbtRed=180;
			col.rgbtGreen=180;
			col.rgbtBlue=0;
		}
		else
		{
			col.rgbtRed=200;
			col.rgbtGreen=200;
			col.rgbtBlue=200;
		}*/
		COLORREF c = ((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->GetColor(sod.house);

		col.rgbtRed = GetRValue(c);
		col.rgbtBlue = GetBValue(c);
		col.rgbtGreen = GetGValue(c);
		col_r = col;
	}

	// MW: ADD: make red start pos dots
	if (isMultiplayer) {
		CString id;
		DWORD p;
		int wp = td.waypoint;
		BOOL startpos = FALSE;
		int i, e;
		for (i = -1; i < 2; i++) {
			for (e = -1; e < 2; e++) {
				if (dwPos + i + e * m_IsoSize < fielddata.size()) {
					int w = GetWaypointAt(dwPos + i + e * m_IsoSize);
					if (w >= 0) {
						GetWaypointData(w, &id, &p);
						if (atoi(id) < 8) {
							startpos = TRUE;
							break;
						}
					}
				}
			}
			if (startpos) {
				break;
			}
		}
		if (startpos) {
			col.rgbtBlue = 0;
			col.rgbtGreen = 0;
			col.rgbtRed = 255;
			col_r = col;
		}
	}
}