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

#if !defined(AFX_GLOBALSDLG_H__A8AAF461_9DE3_11D4_9C87_A7C4798A5242__INCLUDED_)
#define AFX_GLOBALSDLG_H__A8AAF461_9DE3_11D4_9C87_A7C4798A5242__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GlobalsDlg.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CGlobalsDlg 

class CGlobalsDlg : public CDialog
{
	// Konstruktion
public:
	void UpdateDialog();
	CGlobalsDlg(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CGlobalsDlg)
	enum { IDD = IDD_GLOBALS };
	CMyComboBox	m_Value;
	CComboBox	m_Global;
	CString	m_Description;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CGlobalsDlg)
protected:
	virtual BOOL OnInitDialog() override;	
	virtual void OnOK() override;
	virtual void OnCancel() override;
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	void translateUI();
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CGlobalsDlg)

	afx_msg void OnChangeDescription();
	afx_msg void OnSelchangeGlobal();
	afx_msg void OnSelchangeValue();
	afx_msg void OnEditchangeValue();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_GLOBALSDLG_H__A8AAF461_9DE3_11D4_9C87_A7C4798A5242__INCLUDED_
