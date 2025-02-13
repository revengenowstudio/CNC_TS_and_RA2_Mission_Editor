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

#if !defined(AFX_IMPORTINI_H__9DC7B324_6DF2_11D3_99E1_D14A1D4DCF07__INCLUDED_)
#define AFX_IMPORTINI_H__9DC7B324_6DF2_11D3_99E1_D14A1D4DCF07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImportINI.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CImportINI 

class CImportINI : public CDialog
{
	// Konstruktion
public:
	CString m_FileName;
	CImportINI(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CImportINI)
	enum { IDD = IDD_IMPORTINI };
	CListBox	m_Available;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CImportINI)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CImportINI)
	afx_msg void OnAllsections();
	afx_msg void OnSpecifysections();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	WORD m_inicount;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_IMPORTINI_H__9DC7B324_6DF2_11D3_99E1_D14A1D4DCF07__INCLUDED_
