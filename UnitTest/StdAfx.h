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

// stdafx.h : include everything that should be available everywhere in the code
//

#pragma once

#include <gtest/gtest.h>
#include <iostream>
#include <afxwin.h> 
#include <string>
#include <fstream>

using std::cout;
using std::endl;

#if !defined(ASSERT)
#define ASSERT(x) if (!(x)) throw("assertion failed");
#endif