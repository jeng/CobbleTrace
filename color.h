#ifndef __COLOR_H__
#define __COLOR_H__

#include <math.h>
#include "mymath.h"

struct hsv_t{
  float h, s, v;
};

struct rgb_t{
  float r, g, b;
};

/* input: r,g,b in range [0..1] */
/* outputs: h,s,v in range h = [0..360], [0..1]*/

inline rgb_t
HsvToRgb(hsv_t h){
  int i;
  float aa, bb, cc, f;
  rgb_t r;

  if (h.s == 0){ /* Grayscale */
    r.r = r.g = r.b = h.v;
    return r;
  }
  else {
    h.h /= 60.0;
    i = floor(h.h);
    f = h.h - i;
    aa = h.v * (1 -  h.s);
    bb = h.v * (1 - (h.s * f));
    cc = h.v * (1 - (h.s * (1 - f)));
    switch(i){
    case 0: r.r = h.v; r.g = cc;  r.b = aa;  break;
    case 1: r.r = bb;  r.g = h.v; r.b = aa;  break;
    case 2: r.r = aa;  r.g = h.v; r.b = cc;  break;
    case 3: r.r = aa;  r.g = bb;  r.b = h.v; break;
    case 4: r.r = cc;  r.g = aa;  r.b = h.v; break;
    case 5: r.r = h.v; r.g = aa;  r.b = bb;  break;
    }
  }
  return r;
}

/* input: r,g,b in range [0..1] */
/* outputs: h,s,v in range h = [0..360] [0..1]*/
inline hsv_t
RgbToHsv (rgb_t r){
  float maxColor = max(r.r, max(r.g, r.b));
  float minColor = min(r.r, min(r.g, r.b));
  float delta = maxColor - minColor;
  hsv_t h;

  h.v = maxColor;
  if (maxColor != 0.0)
    h.s = delta/maxColor;
  else{
    h.s = 0.0;
    h.h = -1; /*NO_HUE;*/ 
    return h;
  }
  if (r.r == maxColor)
    h.h = (r.g - r.b) / delta;
  else if (r.g == maxColor)
    h.h = 2 + (r.b - r.r) / delta;
  else 
    h.h = 4 + (r.r - r.g) / delta;
  h.h *= 60.0;
  if (h.h < 0) 
    h.h += 360.0;
  
  return h;
}

inline uint32_t 
RgbToColor(uint8_t r, uint8_t g, uint8_t b){
  return (b << 16) | (g << 8) | r;
}

inline uint32_t
RgbToColor(rgb_t rgb){
  return RgbToColor(rgb.r, rgb.g, rgb.b);
}

inline void
ColorToRgb(uint32_t color, uint8_t *r, uint8_t *g, uint8_t *b){
    *r = color & 0xff;    
    color = color >> 8;
    *g = color & 0xff;
    color = color >> 8;
    *b = color & 0xff;
}

inline rgb_t
ColorToRgb(uint32_t color){
  uint8_t r, g, b;
  rgb_t result;
  ColorToRgb(color, &r, &g, &b);
  result.r = r;
  result.g = g;
  result.b = b;
  return result;
}

inline v3_t
ColorToRgbV3(uint32_t color){
  uint8_t r, g, b;
  ColorToRgb(color, &r, &g, &b);
  return {(double)r, (double)g, (double)b};
}

inline hsv_t
ColorToHsv(uint32_t color){
  uint8_t r, g, b;
  ColorToRgb(color, &r, &g, &b);
  rgb_t rgb = {(float)r/0xff, (float)g/0xff, (float)b/0xff};
  return RgbToHsv(rgb);
}

inline uint32_t
HsvToColor(hsv_t hsv){
  rgb_t rgb = HsvToRgb(hsv);
  return RgbToColor(min(rgb.r * 0xff, 0xff), min(rgb.g * 0xff, 0xff), min(rgb.b * 0xff,0xff));
}

#endif
