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

#include "stdafx.h"
#include "shp_decode.h"

#include <lzo/lzo1x.h>
#include <virtual_binary.h>
#include "cc_structures.h"

static const char* encode64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const int decode64_table[256] =
{
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
	52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
	-1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
	15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
	-1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
	41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};

static int read_w(const byte*& r)
{
	int v = *reinterpret_cast<const unsigned __int16*>(r);
	r += 2;
	return v;
}

static void write_w(int v, byte*& w)
{
	*w++ = v & 0xff;
	*w++ = v >> 8;
}

static void write_v40(byte v, int count, byte*& d)
{
	while (count) {
		if (v) {
			if (count < 0x100) {
				*d++ = 0x00;
				*d++ = count;
				*d++ = v;
				break;
			}
			int c_write = min(count, 0x3fff);
			*d++ = 0x80;
			write_w(0xc000 | c_write, d);
			count -= c_write;
			*d++ = v;
		} else if (count < 0x80) {
			*d++ = 0x80 | count;
			count = 0;
		} else {
			int c_write = count < 0x8000 ? count : 0x7fff;
			*d++ = 0x80;
			write_w(c_write, d);
			count -= c_write;
		}
	}
}

int get_run_length(const byte* r, const byte* s_end)
{
	int count = 1;
	int v = *r++;
	while (r < s_end && *r++ == v)
		count++;
	return count;
}

static void write40_c0(byte*& w, int count, int v)
{
	*w++ = 0;
	*w++ = count;
	*w++ = v;
}

static void write40_c1(byte*& w, int count, const byte* r)
{
	*w++ = count;
	memcpy(w, r, count);
	w += count;
}

static void write40_c2(byte*& w, int count)
{
	*w++ = 0x80;
	write_w(count, w);
}

static void write40_c3(byte*& w, int count, const byte* r)
{
	*w++ = 0x80;
	write_w(0x8000 | count, w);
	memcpy(w, r, count);
	w += count;
}

static void write40_c4(byte*& w, int count, int v)
{
	*w++ = 0x80;
	write_w(0xc000 | count, w);
	*w++ = v;
}

static void write40_c5(byte*& w, int count)
{
	*w++ = 0x80 | count;
}

static void write40_copy(byte*& w, int count, const byte* r)
{
	while (count) {
		if (count < 0x80) {
			write40_c1(w, count, r);
			count = 0;
		} else {
			int c_write = count < 0x4000 ? count : 0x3fff;
			write40_c3(w, c_write, r);
			r += c_write;
			count -= c_write;
		}
	}
}

static void write40_fill(byte*& w, int count, int v)
{
	while (count) {
		if (count < 0x100) {
			write40_c0(w, count, v);
			count = 0;
		} else {
			int c_write = count < 0x4000 ? count : 0x3fff;
			write40_c4(w, c_write, v);
			count -= c_write;
		}
	}
}

static void write40_skip(byte*& w, int count)
{
	while (count) {
		if (count < 0x80) {
			write40_c5(w, count);
			count = 0;
		} else {
			int c_write = count < 0x8000 ? count : 0x7fff;
			write40_c2(w, c_write);
			count -= c_write;
		}
	}
}

static void flush_copy(byte*& w, const byte* r, const byte*& copy_from)
{
	if (copy_from) {
		write40_copy(w, r - copy_from, copy_from);
		copy_from = NULL;
	}
}

static void write_v80(byte v, int count, byte*& d)
{
	if (count > 3) {
		*d++ = 0xfe;
		write_w(count, d);
		*d++ = v;
	} else if (count) {
		*d++ = 0x80 | count;
		while (count--)
			*d++ = v;
	}
}

void get_same(const byte* src_start, const byte* src_pos, const byte* src_end, const byte*& p_same_content_beg, int& same_content_len)
{
	// init
	p_same_content_beg = nullptr;
	same_content_len = 0;

	while (src_start++ < src_end) {
		// reset round
		int counted = 0;
		// early safe check
		if (src_start >= src_pos) {
			break;
		}
		while (src_start[counted] == src_pos[counted]) {
			// match begins, early safe check
			if (src_start + counted >= src_end) {
				break;
			}
			if (src_pos + counted >= src_end) {
				break;
			}
			counted++;
		}
		if (counted >= same_content_len) {
			same_content_len = counted;
			p_same_content_beg = src_start;
		}
	}
}

static void write80_c0(byte*& w, int count, int p)
{
	*w++ = (count - 3) << 4 | p >> 8;
	*w++ = p & 0xff;
}

static void write80_c1(byte*& w, int count, const byte* r)
{
	do {
		int c_write = count < 0x40 ? count : 0x3f;
		*w++ = 0x80 | c_write;
		memcpy(w, r, c_write);
		r += c_write;
		w += c_write;
		count -= c_write;
	} while (count);
}

static void write80_c2(byte*& w, int count, int p)
{
	*w++ = 0xc0 | (count - 3);
	write_w(p, w);
}

static void write80_c3(byte*& w, int count, int v)
{
	*w++ = 0xfe;
	write_w(count, w);
	*w++ = v;
}

static void write80_c4(byte*& w, int count, int p)
{
	*w++ = 0xff;
	write_w(count, w);
	write_w(p, w);
}

static void flush_c1(byte*& w, const byte* r, const byte*& copy_from)
{
	if (copy_from) {
		write80_c1(w, r - copy_from, copy_from);
		copy_from = NULL;
	}
}

int encode80(const byte* src, byte* dst, int src_len)
{
	//using std::cout;
	//using std::endl;
	// full compression
	const byte* src_end = src + src_len;
	const byte* src_pos = src;
	byte* w = dst;
	const byte* copy_from = NULL;
	while (src_pos < src_end) {
		const byte* p_block_begin;
		int block_len;
		int same_data_len = get_run_length(src_pos, src_end);
		get_same(src, src_pos, src_end, p_block_begin, block_len);

		//cout << "p_block_begin:" << std::showbase << std::hex << (std::uintptr_t)p_block_begin << ", block_len:" << block_len << endl;
		if (same_data_len < block_len && block_len > 2) {
			flush_c1(w, src_pos, copy_from);
			if (block_len - 3 < 8 && src_pos - p_block_begin < 0x1000)
				write80_c0(w, block_len, src_pos - p_block_begin);
			else if (block_len - 3 < 0x3e)
				write80_c2(w, block_len, p_block_begin - src);
			else
				write80_c4(w, block_len, p_block_begin - src);
			src_pos += block_len;
		}
		else {
			if (same_data_len < 3) {
				if (!copy_from)
					copy_from = src_pos;
			}
			else {
				flush_c1(w, src_pos, copy_from);
				write80_c3(w, same_data_len, *src_pos);
			}
			src_pos += same_data_len;
		}
	}
	flush_c1(w, src_pos, copy_from);
	write80_c1(w, 0, NULL);
	return w - dst;
}

int decode80(const byte source[], byte dest[])
{
	uint8_t* source_ptr, * dest_ptr, * copy_ptr, op_code, data;
	uint64_t count, * word_dest_ptr, word_data;

	source_ptr = (uint8_t*)source;
	dest_ptr = (uint8_t*)dest;

	while (true) {
		op_code = *source_ptr++;
		if (!(op_code & 0x80)) {
			count = (op_code >> 4) + 3;
			copy_ptr = dest_ptr - ((uint64_t)*source_ptr++ + ((uint64_t)(op_code & 0x0f) << 8));
			while (count--) *dest_ptr++ = *copy_ptr++;
		} else {
			if (!(op_code & 0x40)) {
				if (op_code == 0x80)
					return ((uint64_t)(dest_ptr - (uint8_t*)dest));
				else {
					count = op_code & 0x3f;
					while (count--) *dest_ptr++ = *source_ptr++;
				}
			} else {
				if (op_code == 0xfe) {
					count = *source_ptr + ((uint64_t) * (source_ptr + 1) << 8);
					word_data = data = *(source_ptr + 2);
					word_data = (word_data << 56) + (word_data << 48) + (word_data << 40) + (word_data << 32) + (word_data << 24) + (word_data << 16) + (word_data << 8) + word_data;
					source_ptr += 3;
					copy_ptr = dest_ptr + 4 - ((uint64_t)dest_ptr & 0x3);
					count -= (copy_ptr - dest_ptr);
					while (dest_ptr < copy_ptr) *dest_ptr++ = data;
					word_dest_ptr = (uint64_t*)dest_ptr;
					dest_ptr += (count & 0xfffffffffffffffc);
					while (word_dest_ptr < (uint64_t*)dest_ptr) {
						*word_dest_ptr = word_data;
						*(word_dest_ptr + 1) = word_data;
						word_dest_ptr += 2;
					}
					copy_ptr = dest_ptr + (count & 0x3);
					while (dest_ptr < copy_ptr) *dest_ptr++ = data;
				} else {
					if (op_code == 0xff) {
						count = *source_ptr + ((uint64_t) * (source_ptr + 1) << 8);
						copy_ptr = (uint8_t*)dest + *(source_ptr + 2) + ((uint64_t) * (source_ptr + 3) << 8);
						source_ptr += 4;
						while (count--) *dest_ptr++ = *copy_ptr++;
					} else {
						count = (op_code & 0x3f) + 3;
						copy_ptr = (uint8_t*)dest + *source_ptr + ((uint64_t) * (source_ptr + 1) << 8);
						source_ptr += 2;
						while (count--) *dest_ptr++ = *copy_ptr++;
					}
				}
			}
		}
	}
}

int decode3(const byte* s, byte* d, int cx, int cy)
{
	const byte* r = s;
	byte* w = d;
	for (int y = 0; y < cy; y++) {
		int count = *reinterpret_cast<const unsigned __int16*>(r) - 2;
		r += 2;
		int x = 0;
		while (count--) {
			int v = *r++;
			if (v) {
				x++;
				*w++ = v;
			} else {
				count--;
				v = *r++;
				if (x + v > cx)
					v = cx - x;
				x += v;
				while (v--)
					*w++ = 0;
			}
		}
	}
	return w - d;
}

Cvirtual_binary encode64(data_ref s)
{
	Cvirtual_binary d;
	const byte* r = s.data();
	int cb_s = s.size();
	byte* w = d.write_start(s.size() << 1);
	while (cb_s) {
		int c1 = *r++;
		*w++ = encode64_table[c1 >> 2];

		int c2 = --cb_s == 0 ? 0 : *r++;
		*w++ = encode64_table[((c1 & 0x3) << 4) | ((c2 & 0xf0) >> 4)];
		if (cb_s == 0) {
			*w++ = '=';
			*w++ = '=';
			break;
		}

		int c3 = --cb_s == 0 ? 0 : *r++;

		*w++ = encode64_table[((c2 & 0xf) << 2) | ((c3 & 0xc0) >> 6)];
		if (cb_s == 0) {
			*w++ = '=';
			break;
		}
		--cb_s;
		*w++ = encode64_table[c3 & 0x3f];
	}
	d.set_size(w - d.data());
	return d;
}

Cvirtual_binary decode64(data_ref s)
{
	Cvirtual_binary d;
	const byte* r = s.data();
	byte* w = d.write_start(s.size() << 1);
	while (*r) {
		int c1 = *r++;
		if (decode64_table[c1] == -1)
			return Cvirtual_binary();
		int c2 = *r++;
		if (decode64_table[c2] == -1)
			return Cvirtual_binary();
		int c3 = *r++;
		if (c3 != '=' && decode64_table[c3] == -1)
			return Cvirtual_binary();
		int c4 = *r++;
		if (c4 != '=' && decode64_table[c4] == -1)
			return Cvirtual_binary();
		*w++ = (decode64_table[c1] << 2) | (decode64_table[c2] >> 4);
		if (c3 == '=')
			break;
		*w++ = ((decode64_table[c2] << 4) & 0xf0) | (decode64_table[c3] >> 2);
		if (c4 == '=')
			break;
		*w++ = ((decode64_table[c3] << 6) & 0xc0) | decode64_table[c4];
	}
	d.set_size(w - d.data());
	return d;
}

static void write5_count(byte*& w, int count)
{
	while (count > 255) {
		*w++ = 0;
		count -= 255;
	}
	*w++ = count;
}

static void write5_c0(byte*& w, int count, const byte* r, byte* small_copy)
{
	if (count < 4 && !small_copy)
		count = count;
	if ((count < 4 || count > 7) && small_copy) {
		int small_count = min(count, 3);
		*small_copy |= small_count;
		memcpy(w, r, small_count);
		r += small_count;
		w += small_count;
		count -= small_count;
	}
	if (count) {
		assert(count > 3);
		if (count > 18) {
			*w++ = 0;
			write5_count(w, count - 18);
		} else
			*w++ = count - 3;
		memcpy(w, r, count);
		w += count;
	}
}

static void write5_c1(byte*& w, int count, int p)
{
	assert(count > 2);
	assert(p >= 0);
	assert(p < 32768);
	count -= 2;
	if (count > 7) {
		*w++ = 0x10 | (p >> 11) & 8;
		write5_count(w, count - 7);
	} else
		*w++ = 0x10 | (p >> 11) & 8 | count;
	write_w((p << 2) & 0xfffc, w);
}

static void write5_c2(byte*& w, int count, int p)
{
	assert(count > 2);
	assert(p > 0);
	assert(p <= 16384);
	count -= 2;
	p--;
	if (count > 31) {
		*w++ = 0x20;
		write5_count(w, count - 31);
	} else
		*w++ = 0x20 | count;
	write_w(p << 2, w);
}

static void write5_c3(byte*& w, int count, int p)
{
	assert(count > 1);
	assert(count < 7);
	assert(p > 0);
	assert(p <= 2048);
	count -= 2;
	p--;
	*w++ = (count + 1) << 5 | (p & 7) << 2;
	*w++ = p >> 3;
}

int get_count(const byte*& r)
{
	int count = -255;
	int v;
	do {
		count += 255;
		v = *r++;
	} while (!v);
	return count + v;
}

static void flush_c0(byte*& w, const byte* r, const byte*& copy_from, byte* small_copy, bool start)
{
	if (copy_from) {
		int count = r - copy_from;
		/*
		if (start)
		{
			int small_count = count;
			if (count > 241)
				small_count = 238;
			else if (count > 238)
				small_count = count - 4;
			*w++ = small_count + 17;
			memcpy(w, copy_from, small_count);
			copy_from += small_count;
			w += small_count;
			count -= small_count;
		}
		*/
		if (count)
			write5_c0(w, count, copy_from, small_copy);
		copy_from = NULL;
	}
}

int encode5s(const byte* s, byte* d, int cb_s)
{
	lzo_init();
	static Cvirtual_binary t;
	lzo_uint cb_d;
	if (LZO_E_OK != lzo1x_1_compress(s, cb_s, d, &cb_d, t.write_start(LZO1X_1_MEM_COMPRESS)))
		cb_d = 0;
	return cb_d;
}

int encode5s_z(const byte* s, byte* d, int cb_s)
{
	// no compression
	const byte* r = s;
	const byte* r_end = s + cb_s;
	byte* w = d;
	write5_c0(w, cb_s, r, NULL);
	r += cb_s;
	write5_c1(w, 3, 0);
	assert(cb_s == r - s);
	return w - d;
}

int decode5s(const byte* s, byte* d, int cb_s)
{
	lzo_init();
	lzo_uint cb_d;
	if (LZO_E_OK != lzo1x_decompress(s, cb_s, d, &cb_d, NULL))
		return 0;
	return cb_d;
	/*
	0 copy 0000cccc
	1 copy 0001pccc ppppppzz p
	2 copy 001ccccc ppppppzz p
	3 copy cccpppzz p
	*/

	const byte* c;
	const byte* r = s;
	byte* w = d;
	int code;
	int count;
	code = *r;
	if (code > 17) {
		r++;
		count = code - 17;
		while (count--)
			*w++ = *r++;
	}
	while (1) {
		code = *r++;
		if (code & 0xf0) {
			if (code & 0xc0) {
				count = (code >> 5) - 1;
				c = w - (code >> 2 & 7);
				c -= *r++ << 3;
				c--;
			} else if (code & 0x20) {
				count = code & 0x1f;
				if (!count)
					count = get_count(r) + 31;
				c = w - (read_w(r) >> 2);
				c--;
			} else {
				c = w - ((code & 8) << 11);
				count = code & 7;
				if (!count)
					count = get_count(r) + 7;
				c -= read_w(r) >> 2;
				if (c == w)
					break;
			}
			count += 2;
			while (count--) {
				if (*w != *c)
					*w = *c;
				w++;
				c++;
			}
			count = *(r - 2) & 3;
			while (count--) {
				if (*w != *r)
					*w = *r;
				w++;
				r++;
			}
		} else {
			count = code ? code + 3 : get_count(r) + 18;
			while (count--) {
				if (*w != *r)
					*w = *r;
				w++;
				r++;
			}
		}
	}
	assert(cb_s == r - s);
	return w - d;
}

int encode5(const byte* s, byte* d, int cb_s, int format)
{
	const byte* r = s;
	const byte* r_end = s + cb_s;
	byte* w = d;
	while (r < r_end) {
		int cb_section = min<size_t>(r_end - r, 8192);
		t_pack_section_header& header = *reinterpret_cast<t_pack_section_header*>(w);
		w += sizeof(t_pack_section_header);
		w += header.size_in = format == 80 ? encode80(r, w, cb_section) : encode5s(r, w, cb_section);
		r += header.size_out = cb_section;
	}
	return w - d;
}

int decode5(const byte* s, byte* d, int cb_s, int format)
{
	const byte* r = s;
	const byte* r_end = s + cb_s;
	byte* w = d;
	while (r < r_end) {
		const t_pack_section_header& header = *reinterpret_cast<const t_pack_section_header*>(r);
		r += sizeof(t_pack_section_header);
		if (format == 80)
			decode80(r, w);
		else
			decode5s(r, w, header.size_in);
		r += header.size_in;
		w += header.size_out;
	}
	return w - d;
}
