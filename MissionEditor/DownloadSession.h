﻿/*
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

#include <afxinet.h>

#if !defined(AFX_DOWNLOADSESSION_H__944482A1_E98F_11D4_9C88_82B0A7E03C49__INCLUDED_)
#define AFX_DOWNLOADSESSION_H__944482A1_E98F_11D4_9C88_82B0A7E03C49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDownloadSession : public CInternetSession
{
public:
	CDownloadSession();
	virtual ~CDownloadSession();
protected:
	virtual void OnStatusCallback(DWORD dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);

};

#endif // !defined(AFX_DOWNLOADSESSION_H__944482A1_E98F_11D4_9C88_82B0A7E03C49__INCLUDED_)
