#pragma once
#include <afx.h>
#include <algorithm>

// coordinate functions
inline void PosToXY(const char* pos, int* X, int* Y)
{
	size_t Posleng;
	//int XX, YY;
	char Pos[100];
	strcpy_s(Pos, pos);
	char XS[10], YS[10];
	Posleng = strlen(Pos);
	strcpy_s(YS, Pos + Posleng - 3);
	Pos[Posleng - 3] = 0;
	strcpy_s(XS, Pos);

	*X = atoi(XS);
	*Y = atoi(YS);

}

// check whether is string is all made of digit numbers
inline bool IsNumeric(const CString& str) {
	return std::all_of(str.operator LPCSTR(), str.operator LPCSTR() + str.GetLength(), [](char c) {
		return std::isdigit(c);
	});
}

// HSV -> RGB
inline bool HSVToRGB(const double h, const double s, const double v, double& r, double& g, double& b)
{
	if (h < 0.0 || h >= 360.0 || s < 0.0 || s > 1.0 || v < 0.0 || v > 1.0)
		return false;
	auto const  h_ = static_cast<int>(floor(h / 60.0));
	auto const c = s * v;
	auto const x = static_cast<float>(c * (1 - fabs(fmod(h / 60.0, 2.0) - 1)));
	auto const m = v - c;
	switch (h_) {
	case 0:
		r = c, g = x, b = 0.0;
		break;
	case 1:
		r = x, g = c, b = 0.0;
		break;
	case 2:
		r = 0.0, g = c, b = x;
		break;
	case 3:
		r = 0.0, g = x, b = c;
		break;
	case 4:
		r = x, g = 0.0, b = c;
		break;
	case 5:
		r = c, g = 0.0, b = x;
		break;
	}
	r += m;
	g += m;
	b += m;
	return true;
}

inline void HSVToRGB(const unsigned char hsv[3], unsigned char rgb[3])
{
	double frgb[3];
	HSVToRGB(hsv[0] * 360.0 / 255.0, hsv[1] / 255.0, hsv[2] / 255.0, frgb[0], frgb[1], frgb[2]);
	for (int i = 0; i < 3; ++i) {
		auto const rgbVal = (frgb[i] < 0.0 ? 0.0 : (frgb[i] > 1.0 ? 1.0 : frgb[i])) * 255.0;
		rgb[i] = static_cast<unsigned char>(rgbVal);
	}
}

inline std::array<unsigned char, 3> HSVToRGB(const float h, const float s, const float v)
{
	std::array<double, 3> frgb;
	HSVToRGB(h, s, v, frgb[0], frgb[1], frgb[2]);
	std::array<unsigned char, 3> ret;
	for (int i = 0; i < 3; ++i) {
		auto const rgbVal = (frgb[i] < 0.0 ? 0.0 : (frgb[i] > 1.0 ? 1.0 : frgb[i])) * 255.0;
		ret[i] = static_cast<unsigned char>(rgbVal);
	}
	return ret;
}

inline std::array<unsigned char, 3> HSVToRGB(const unsigned char hsv[3])
{
	std::array<unsigned char, 3> ret;
	HSVToRGB(hsv, ret.data());
	return ret;
}