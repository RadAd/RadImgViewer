#pragma once

class ColorRGB
{
public:
	ColorRGB(double R, double G, double B)
		: r(R), g(G), b(B)
	{ }

	double r;
	double g;
	double b;
};

class ColorHSV
{
public:
	ColorHSV(double H, double S, double V)
		: h(H), s(S), v(V)
	{ }

	double h;
	double s;
	double v;
};

ColorHSV ToHSV(const ColorRGB& c);
ColorRGB ToRGB(const ColorHSV& c);
