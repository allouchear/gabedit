/**********************************************************************************************************
Copyright (c) 2002-2013 Abdul-Rahman Allouche. All rights reserved

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the Gabedit), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions
  of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
************************************************************************************************************/

#ifndef __GABEDIT_COLORMAP_H__
#define __GABEDIT_COLORMAP_H__

typedef enum
{
  COLORMAP_GRID = 0,
  OLORMAP_CONTOUR,
  COLORMAP_ISOSURFACE,
  COLORMAP_MAX
} ColorMapTypes;

typedef struct _OneColorValue
{
	gdouble color[3];
	gdouble value;
}OneColorValue;

typedef struct _ColorMap
{
	gint numberOfColors;
	OneColorValue* colorValue;
}ColorMap;

void colormap_free(ColorMap* colorMap);
ColorMap*  new_colorMap_fromGrid_multicolor(Grid* grid);
ColorMap*  new_colorMap_min_max_multicolor(gdouble minValue, gdouble maxValue);
ColorMap*  new_colorMap_min_max_unicolor(gdouble minValue, gdouble maxValue, gdouble color[]);
ColorMap*  new_colorMap_fromGrid_unicolor(Grid* grid, gdouble color[]);
ColorMap*  new_colorMap_fromGrid(Grid* grid);
ColorMap*  new_colorMap_min_max(gdouble minValue, gdouble maxValue);
void  set_Color_From_colorMap(ColorMap* colorMap, gdouble color[], gdouble value);
GtkWidget* create_color_map_show(GtkWidget* box, ColorMap* colorMap, gchar* label);
void color_map_show(GtkWidget* handlebox);
void color_map_refresh(GtkWidget* handlebox);
void color_map_hide(GtkWidget* handlebox);

#endif /* __GABEDIT_COLORMAP_H__ */

