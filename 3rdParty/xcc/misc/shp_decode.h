/*
	XCC Utilities and Library
	Copyright (C) 2000  Olaf van der Spek  <olafvdspek@gmail.com>

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

#pragma once

#include <virtual_binary.h>

int decode3(const byte* s, byte* d, int cx, int cy);
int decode5(const byte* s, byte* d, const size_t cb_s, int format);
int encode5(const byte* s, byte* d, int cb_s, int format);
int decode5s(const byte* s, byte* d, int cb_s);
int encode5s(const byte* s, byte* d, int cb_s);
Cvirtual_binary decode64(data_ref);
Cvirtual_binary encode64(data_ref);
int decode80(const byte image_in[], byte image_out[]);
int encode80(const byte* s, byte* d, const size_t cb_s);
int get_run_length(const byte* r, const byte* s_end);
