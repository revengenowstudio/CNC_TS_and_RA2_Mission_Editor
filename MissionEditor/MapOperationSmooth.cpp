#include "stdafx.h"
#include "MapData.h"

void CMapData::SmoothAt(DWORD dwPos, int iSmoothSet, int iLatSet, int iTargetSet, BOOL bIgnoreShore)
{
	int i;
	int its, iss, ils;
	int iLatGround = tilesets_start[iLatSet];
	int iSmoothGround = tilesets_start[iSmoothSet];
	its = (*tiledata)[tilesets_start[iTargetSet]].tiles[0].bTerrainType;
	iss = (*tiledata)[iSmoothGround].tiles[0].bTerrainType;
	ils = (*tiledata)[iLatGround].tiles[0].bTerrainType;

	FIELDDATA m = *GetFielddataAt(dwPos);
	if (m.wGround == 0xFFFF) {
		m.wGround = 0;
	}

	// do we have that certain LAT tile here?
	if ((*tiledata)[m.wGround].wTileSet != iSmoothSet && (*tiledata)[m.wGround].wTileSet != iLatSet) {
		return;
	}

	//if(m.wGround==iLatGround) m.wGround=iSmoothGround;

	if (its == iss && (*tiledata)[m.wGround].wTileSet == iSmoothSet) {
		m.wGround = iLatGround;
	}

	if (its == iss) {
		ils += 1;
	}

	BOOL bOnlyLat = TRUE;
	//BOOL bNoLat=TRUE;
	int ns = -1;
	int e, p = 0;
	int ts[3][3];  // terrain info

	int set = (*tiledata)[m.wGround].wTileSet;

	for (i = 0; i < 3; i++) {
		for (e = 0; e < 3; e++) {
			int pos = dwPos + (i - 1) + (e - 1) * m_IsoSize;
			if (pos < 0 || pos >= fielddata.size()) {
				ts[i][e] = 0;
			} else {
				FIELDDATA m2 = *GetFielddataAt(pos);
				if (m2.wGround == 0xFFFF) m2.wGround = 0;

				int cur_set = (*tiledata)[m2.wGround].wTileSet;

				//if(cur_set==iSmoothSet) bNoLat=FALSE;
				//

				if (its == iss && cur_set == iSmoothSet) {
					m2.wGround = iLatGround; cur_set = iLatSet;
				}

				if (cur_set == iSmoothSet || cur_set == iTargetSet) bOnlyLat = FALSE;

				/*if(cur_set != iSmoothSet && cur_set!= iLatSet && cur_set!=iTargetSet)
				{
					ts[i][e]=(*tiledata)[m2.wGround].tiles[m2.bSubTile].bTerrainType;
					if((*tiledata)[m2.wGround].wTileSet!=shoreset)
					{
						ts[i][e]=0;//ts[i][e]+1; // make sure you don´t smooth at it except it´s shore
					}
					//if(bIgnoreShore && (*tiledata)[m2.wGround].wTileSet==shoreset)
					//	ts[i][e]=0;//ts[i][e]+1;
				}
				else*/ if (its == iss && cur_set != set) {
					if (cur_set == shoreset && !bIgnoreShore)
						ts[i][e] = its;
					else if (cur_set != iSmoothSet && cur_set != iTargetSet && cur_set != iLatSet)
						ts[i][e] = 0;
					else
						ts[i][e] = its;
				} else if (its == iss && cur_set == set)
					ts[i][e] = ils;
				else {
					ts[i][e] = (*tiledata)[m2.wGround].tiles[m2.bSubTile].bTerrainType;

					if (cur_set != shoreset && cur_set != iLatSet && cur_set != iSmoothSet) {
						ts[i][e] = 0;//ts[i][e]+1; // make sure you don´t smooth at it except it´s shore
					}

				}
			}
		}
	}

	//if(bOnlyLat) return;


	int needed = -1;

	// 1/1 is smoothed tile

	if (ts[1][1] == ils) {
		// single lat
		if (ts[0][1] != ils && ts[1][0] != ils
			&& ts[1][2] != ils && ts[2][1] != ils)
			needed = 16;
		else if (ts[0][1] == ils && ts[1][0] == ils
			&& ts[1][2] == ils && ts[2][1] == ils)
			needed = 0;
		else if (ts[0][1] == ils && ts[2][1] == ils &&
			ts[1][0] != ils && ts[1][2] != ils)
			needed = 11;
		else if (ts[1][0] == ils && ts[1][2] == ils &&
			ts[0][1] != ils && ts[2][1] != ils)
			needed = 6;
		else if (ts[1][0] != ils && ts[0][1] == ils &&
			ts[2][1] == ils)
			needed = 9;
		else if (ts[2][1] != ils && ts[1][0] == ils &&
			ts[1][2] == ils)
			needed = 5;
		else if (ts[1][2] != ils && ts[0][1] == ils &&
			ts[2][1] == ils)
			needed = 3;
		else if (ts[0][1] != ils && ts[1][0] == ils &&
			ts[1][2] == ils)
			needed = 2;
		else if (ts[0][1] == ils && ts[1][0] != ils &&
			ts[1][2] != ils && ts[2][1] != ils)
			needed = 15;
		else if (ts[1][2] == ils && ts[1][0] != ils &&
			ts[0][1] != ils && ts[2][1] != ils)
			needed = 14;
		else if (ts[2][1] == ils && ts[1][0] != ils &&
			ts[0][1] != ils && ts[1][2] != ils)
			needed = 12;
		else if (ts[1][0] == ils && ts[0][1] != ils &&
			ts[1][2] != ils && ts[2][1] != ils)
			needed = 8;
		else if (ts[1][0] != ils && ts[2][1] != ils)
			needed = 13;
		else if (ts[1][0] != ils && ts[0][1] != ils)
			needed = 10;
		else if (ts[2][1] != ils && ts[1][2] != ils)
			needed = 7;
		else if (ts[0][1] != ils && ts[1][2] != ils)
			needed = 4;


	} else if (ts[1][1] == its) {
		// replace target set instead of smooth set
		//if(st(ts[0][0], && ts[0][1]
	}


	needed -= 1;
	if (needed >= 0) {
		/*for(i=0;i<*tiledata_count;i++)
		{
			if((*tiledata)[i].wTileSet==iLatSet)
			{
				break;
			}
		}*/
		i = tilesets_start[iLatSet];

		// i is first lat tile
		int e;
		for (e = 0; e < needed; e++) {
			i += (*tiledata)[i].wTileCount;
		}

		SetTileAt(dwPos, i, 0);
	} else if (needed == -1) {
		/*for(i=0;i<*tiledata_count;i++)
		{
			if((*tiledata)[i].wTileSet==iSmoothSet)
			{
				break;
			}
		}*/
		i = tilesets_start[iSmoothSet];

		// i is first lat tile		
		SetTileAt(dwPos, i, 0);
	}
}