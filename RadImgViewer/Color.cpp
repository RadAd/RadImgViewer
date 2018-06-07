#include "Color.h"

static inline double maxrgb(double r, double g, double b)
{
	double max;

	if (r > g)
		max = r;
	else
		max = g;
	if (b > max)
		max = b;
	return max;
}

static inline double minrgb(double r, double g, double b)
{
	double min;

	if (r < g)
		min = r;
	else
		min = g;
	if (b < min)
		min = b;
	return min;
}

/* Taken from "Fund'l of 3D Computer Graphics", Alan Watt (1989)
   Assumes (r,g,b) range from 0.0 to 1.0
   Sets h in degrees: 0.0 to 360.;
   s,v in [0.,1.]
*/
ColorHSV ToHSV(const ColorRGB& c)
{
	ColorHSV	hsv(0, 1.0, 1.0);
	double undefined = 0.0;

	double max_v = maxrgb(c.r, c.g, c.b);
	double min_v = minrgb(c.r, c.g, c.b);
	double diff = max_v - min_v;
	hsv.v = max_v;

	if (max_v != 0)
		hsv.s = diff/max_v;
	else
		hsv.s = 0.0;
	if (hsv.s == 0)
		hsv.h = undefined;
	else
	{
		double r_dist = (max_v - c.r) / diff;
		double g_dist = (max_v - c.g) / diff;
		double b_dist = (max_v - c.b) / diff;
		if (c.r == max_v) 
			hsv.h = b_dist - g_dist;
		else
			if (c.g == max_v)
				hsv.h = 2 + r_dist - b_dist;
			else
				if (c.b == max_v)
					hsv.h = 4 + g_dist - r_dist;
				//else
					//printf("rgb2hsv::How did I get here?\n");
		hsv.h *= 60;
		if (hsv.h < 0)
			hsv.h += 360.0;
	}
	return hsv;
}

/* Taken from "Fund'l of 3D Computer Graphics", Alan Watt (1989)
   Assumes H in degrees, s,v in [0.,1.0];
   (r,g,b) range from 0.0 to 1.0
*/
ColorRGB ToRGB(const ColorHSV& c)
{
	ColorRGB rgb(0, 0, 0);

	if (c.s == 0)
	{
		rgb.r = c.v;
		rgb.g = c.v;
		rgb.b = c.v;
	}
	else
	{
		double h = c.h;
		if (h == 360.)
			h = 0.0;
		h /= 60.;
		int i = (int) h;
		double f = h - i;
		double p = c.v * (1 - c.s);
		double q = c.v * (1 - (c.s * f));
		double t = c.v * (1 - c.s * (1 - f));
		switch(i)
		{
		case 0:
			rgb.r = c.v;
			rgb.g = t;
			rgb.b = p;
			break;
		case 1:
			rgb.r = q;
			rgb.g = c.v;
			rgb.b = p;
			break;
		case 2:
			rgb.r = p;
			rgb.g = c.v;
			rgb.b = t;
			break;
		case 3:
			rgb.r = p;
			rgb.g = q;
			rgb.b = c.v;
			break;
		case 4:
			rgb.r = t;
			rgb.g = p;
			rgb.b = c.v;
			break;
		case 5:
			rgb.r = c.v;
			rgb.g = p;
			rgb.b = q;
			break;
		default:
			rgb.r = 1.0;
			rgb.g = 1.0;
			rgb.b = 1.0;
			//printf("hsv2rgb::How did I get here?\n");
			// printf("h: %f, s: %f, v: %f; i:  %d\n",hin,s,v,i);
			break;
		}
	}
	return rgb;
}
