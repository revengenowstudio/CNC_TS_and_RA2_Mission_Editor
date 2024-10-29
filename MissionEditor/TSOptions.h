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

#if !defined(AFX_TSOPTIONS_H__AFDC93E0_651A_11D3_99E0_B9A145C86B12__INCLUDED_)
#define AFX_TSOPTIONS_H__AFDC93E0_651A_11D3_99E0_B9A145C86B12__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TSOptions.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CTSOptions 

class CTSOptions : public CDialog
{
	// Konstruktion
public:
	BOOL m_PreferLocalTheaterFiles;
	CString m_LanguageName;
	CString m_TSEXE;
	CTSOptions(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CTSOptions)
	enum { IDD = IDD_TSOPTIONS };
	CComboBox	m_Language;
	CEdit	m_TSExe;
	BOOL		m_LikeTS;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CTSOptions)
protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV-Unterstützung
	virtual void OnOK() override;
	virtual BOOL OnInitDialog() override;
	//}}AFX_VIRTUAL

// Implementierung
	DECLARE_MESSAGE_MAP()

	// Generierte Nachrichtenzuordnungsfunktionen

	afx_msg void OnChoose();
	afx_msg void OnCbnSelchangeLanguage();

	void updateUI();
	CString getLanguageSelected();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_TSOPTIONS_H__AFDC93E0_651A_11D3_99E0_B9A145C86B12__INCLUDED_
