/* ColorMap.c */
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

#include "../../Config.h"
#include "../Common/Global.h"
#include "GlobalOrb.h"
#include <gdk/gdkkeysyms.h>
#include "../Utils/Utils.h"
#include "../Utils/UtilsCairo.h"
#include "../Utils/Constants.h"
#include "ColorMap.h"
#include "GLArea.h"

/*
static ColorMap myColorMap =  {0,NULL};
static GtkWidget *handleboxShow;
static GdkGC *gc = NULL;
*/
/******************************************************************************************************************************/
/*
static void print_colorMap(ColorMap* colorMap)
{
	gint i;
	printf("Number of Color = %d\n",colorMap->numberOfColors);
	for(i=0; i<colorMap->numberOfColors; i++)
	{
		printf("%lf %0.2f %0.2f %0.2f\n",colorMap->colorValue[i].value,colorMap->colorValue[i].color[0],colorMap->colorValue[i].color[1],colorMap->colorValue[i].color[2]);
	}
	printf("\n");
}
*/
/******************************************************************************************************************************/
void colormap_free(ColorMap* colorMap)
{
	if(!colorMap) return;
	if(!colorMap->colorValue)
	{

		colorMap->numberOfColors = 0;
		return;
	}
	g_free(colorMap->colorValue);
	colorMap->numberOfColors = 0;
}
/******************************************************************************************************************************/
static void colormap_alloc(ColorMap* colorMap, gint numberOfColors)
{
	if(colorMap->colorValue)
		colormap_free(colorMap);

	if(numberOfColors<1) numberOfColors = 1;
	colorMap->numberOfColors = numberOfColors;
	colorMap->colorValue = g_malloc(numberOfColors*sizeof(OneColorValue));
}
/******************************************************************************************************************************/
static ColorMap new_colorMap(gdouble color[], gdouble value)
{
	gint c;
	ColorMap colorMap = {0,NULL};
	colormap_alloc(&colorMap, 1);
	for(c=0; c<3; c++)
		colorMap.colorValue[0].color[c] = color[c];
	colorMap.colorValue[0].value = value;
	return colorMap;
}
/******************************************************************************************************************************/
static void add_to_colorMap(ColorMap* colorMap, gdouble color[], gdouble value)
{
	gint i;
	gint c;
	OneColorValue* colorValue = NULL;
	colorValue = g_malloc((colorMap->numberOfColors+1)*sizeof(OneColorValue));
	for(i=0; i<colorMap->numberOfColors; i++)
		colorValue[i] = colorMap->colorValue[i];

	for(c=0;c<3;c++)
		colorValue[colorMap->numberOfColors].color[c] = color[c];
	colorValue[colorMap->numberOfColors].value = value;
	g_free( colorMap->colorValue);
	colorMap->colorValue = colorValue;
	colorMap->numberOfColors++;
}
/******************************************************************************************************************************/
static void sort_colorMap(ColorMap* colorMap)
{
	gint i;
	gint j;
	gint k;
	OneColorValue tmp;
	for(i=0;i<colorMap->numberOfColors-1;i++)
	{
		k = i;
		for(j=i+1;j<colorMap->numberOfColors;j++)
			if(colorMap->colorValue[k].value>colorMap->colorValue[j].value)
				k = j;
		if(k != i)
		{
			tmp = colorMap->colorValue[i];
			colorMap->colorValue[i] = colorMap->colorValue[k];
			colorMap->colorValue[k] = tmp;
		}
	}
}
/******************************************************************************************************************************/
ColorMap*  new_colorMap_min_max_multicolor(gdouble minValue, gdouble maxValue)
{
	gint i;
	gint j;
	gint k;
	gdouble h = 0;
	gint ns = 4;
	gint nc = 20;
	gint n = nc *ns+1;
	gdouble color[3];
	ColorMap* colorMap;

	h = (maxValue-minValue)/(n-1) ;
	color[0] = 1.0; color[1] = 0.0; color[2] = 0.0; 

	colorMap = g_malloc(sizeof(ColorMap));
	*colorMap = new_colorMap(color, minValue);
	k = 1;
	for(j=0;j<ns;j++)
	for(i=0;i<nc;i++)
	{
		
		if(j==0) color[1] += 1.0/nc;
		if(j==1) color[0] -= 1.0/nc;
		if(j==2) color[2] += 1.0/nc;
		if(j==3) color[1] -= 1.0/nc;
		add_to_colorMap(colorMap, color, minValue +k*h);
		k++;
	}
	sort_colorMap(colorMap);
	return colorMap;
}
/******************************************************************************************************************************/
ColorMap*  new_colorMap_fromGrid_multicolor(Grid* grid)
{
	gint i;
	gint j;
	gint k;
	gdouble maxValue = 0;
	gdouble minValue = 0;
	gboolean beg = TRUE;
	gdouble h = 0;
	gint ns = 4;
	gint nc = 20;
	gint n = nc *ns+1;
	gdouble color[3];
	gdouble v;
	ColorMap* colorMap;

	if(!grid)return NULL;

	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				v = grid->point[i][j][k].C[4] ;
				if(beg)
				{
					beg = FALSE;
        				minValue =  v;
        				maxValue =  v;
				}
                		else
				{
        				if(minValue > v) minValue =  v;
        				if(maxValue < v) maxValue =  v;
				}
			}
		}
	}
	h = (maxValue-minValue)/(n-1) ;
	color[0] = 1.0; color[1] = 0.0; color[2] = 0.0; 

	colorMap = g_malloc(sizeof(ColorMap));
	*colorMap = new_colorMap(color, minValue);
	k = 1;
	for(j=0;j<ns;j++)
	for(i=0;i<nc;i++)
	{
		
		if(j==0) color[1] += 1.0/nc;
		if(j==1) color[0] -= 1.0/nc;
		if(j==2) color[2] += 1.0/nc;
		if(j==3) color[1] -= 1.0/nc;
		add_to_colorMap(colorMap, color, minValue +k*h);
		k++;
	}
	sort_colorMap(colorMap);
	/*myColorMap = colorMap;*/
	return colorMap;
	/* print_colorMap(myColorMap);*/
}
/******************************************************************************************************************************/
ColorMap*  new_colorMap_min_max_unicolor(gdouble minValue, gdouble maxValue, gdouble Color[])
{
	gdouble color[3];
	ColorMap* colorMap;

	gdouble max = color[0];
	gint c;

	for(c=0;c<3;c++) color[c] = Color[c];
	max = color[0];
	if(max<color[1]) max = color[1];
	if(max<color[2]) max = color[2];
	if(max<1e-3)
	{
		color[0] = 1.0;
		color[1] = 1.0;
		color[2] = 1.0;
		max = 1.0;
	}

	for(c=0;c<3;c++) color[c] /= max;
	colorMap = g_malloc(sizeof(ColorMap));
	*colorMap = new_colorMap(color, minValue);
	color[0] = 0; color[1] = 0; color[2] = 0; 
	add_to_colorMap(colorMap, color, maxValue );
	sort_colorMap(colorMap);
	return colorMap;
}
/******************************************************************************************************************************/
ColorMap*  new_colorMap_fromGrid_unicolor(Grid* grid, gdouble Color[])
{
	gint i;
	gint j;
	gint k;
	gdouble maxValue = 0;
	gdouble minValue = 0;
	gboolean beg = TRUE;
	gdouble color[3];
	gdouble v;
	ColorMap* colorMap;
	gdouble max;
	gint c;

	if(!grid)return NULL;

	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				v = grid->point[i][j][k].C[4] ;
				if(beg)
				{
					beg = FALSE;
        				minValue =  v;
        				maxValue =  v;
				}
                		else
				{
        				if(minValue > v) minValue =  v;
        				if(maxValue < v) maxValue =  v;
				}
			}
		}
	}

	for(c=0;c<3;c++) color[c] = Color[c];
	max = color[0];
	if(max<color[1]) max = color[1];
	if(max<color[2]) max = color[2];
	if(max<1e-3)
	{
		color[0] = 1.0;
		color[1] = 1.0;
		color[2] = 1.0;
		max = 1;
	}
	for(c=0;c<3;c++) color[c] /= max;
	colorMap = g_malloc(sizeof(ColorMap));
	*colorMap = new_colorMap(color, minValue);
	color[0] = 0; color[1] = 0; color[2] = 0; 
	add_to_colorMap(colorMap, color, maxValue );
	sort_colorMap(colorMap);
	/*myColorMap = colorMap;*/
	return colorMap;
	/* print_colorMap(myColorMap);*/
}
/******************************************************************************************************************************/
ColorMap*  new_colorMap_min_max_2colors(gdouble minValue, gdouble maxValue, gdouble Color1[], gdouble Color2[])
{
	gdouble color[3];
	gdouble color1[3];
	gdouble color2[3];
	ColorMap* colorMap;
	gint c;

	gdouble max1;
	gdouble max2;

	for(c=0;c<3;c++) color1[c] = Color1[c];
	for(c=0;c<3;c++) color2[c] = Color2[c];

	max1 = color1[0];
	if(max1<color1[1]) max1 = color1[1];
	if(max1<color1[2]) max1 = color1[2];
	if(max1<1e-3)
	{
		color1[0] = 1.0;
		color1[1] = 1.0;
		color1[2] = 1.0;
		max1 = 1.0;
	}
	max2 = color2[0];
	if(max2<color2[1]) max2 = color2[1];
	if(max2<color2[2]) max2 = color2[2];
	if(max2<1e-3)
	{
		color2[0] = 1.0;
		color2[1] = 1.0;
		color2[2] = 1.0;
		max2 = 1.0;
	}

	color[0] = 0; color[1] = 0; color[2] = 0; 
	for(c=0;c<3;c++) color1[c] /= max1;
	for(c=0;c<3;c++) color2[c] /= max2;
	colorMap = g_malloc(sizeof(ColorMap));
	*colorMap = new_colorMap(color1, minValue);
	add_to_colorMap(colorMap, color, minValue+(maxValue-minValue)/2 );
	add_to_colorMap(colorMap, color2, maxValue );

	sort_colorMap(colorMap);
	return colorMap;
}
/******************************************************************************************************************************/
ColorMap*  new_colorMap_fromGrid_2colors(Grid* grid, gdouble Color1[], gdouble Color2[])
{
	gint i;
	gint j;
	gint k;
	gint c;
	gdouble maxValue = 0;
	gdouble minValue = 0;
	gboolean beg = TRUE;
	gdouble color[3];
	gdouble color1[3];
	gdouble color2[3];
	gdouble v;
	ColorMap* colorMap;
	gdouble max1;
	gdouble max2;

	for(c=0;c<3;c++) color1[c] = Color1[c];
	for(c=0;c<3;c++) color2[c] = Color2[c];

	if(!grid)return NULL;

	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				v = grid->point[i][j][k].C[4] ;
				if(beg)
				{
					beg = FALSE;
        				minValue =  v;
        				maxValue =  v;
				}
                		else
				{
        				if(minValue > v) minValue =  v;
        				if(maxValue < v) maxValue =  v;
				}
			}
		}
	}
	max1 = color1[0];
	if(max1<color1[1]) max1 = color1[1];
	if(max1<color1[2]) max1 = color1[2];
	if(max1<1e-3)
	{
		color1[0] = 1.0;
		color1[1] = 1.0;
		color1[2] = 1.0;
		max1 = 1.0;
	}
	max2 = color2[0];
	if(max2<color2[1]) max2 = color2[1];
	if(max2<color2[2]) max2 = color2[2];
	if(max2<1e-3)
	{
		color2[0] = 1.0;
		color2[1] = 1.0;
		color2[2] = 1.0;
		max2 = 1.0;
	}

	for(c=0;c<3;c++) color1[c] /= max1;
	for(c=0;c<3;c++) color2[c] /= max2;
	colorMap = g_malloc(sizeof(ColorMap));
	*colorMap = new_colorMap(color1, minValue);
	add_to_colorMap(colorMap, color, minValue+(maxValue-minValue)/2 );
	add_to_colorMap(colorMap, color2, maxValue );
	sort_colorMap(colorMap);
	return colorMap;
}
/******************************************************************************************************************************/
ColorMap*  new_colorMap_min_max(gdouble minValue, gdouble maxValue)
{
	if(colorMapType == 1) 
		return new_colorMap_min_max_multicolor(minValue,maxValue);
	else if(colorMapType == 2)
		return new_colorMap_min_max_2colors(minValue,maxValue, colorMapColors[0], colorMapColors[1]);
	else 
		return new_colorMap_min_max_unicolor(minValue,maxValue, colorMapColors[2]);
}
/******************************************************************************************************************************/
ColorMap*  new_colorMap_fromGrid(Grid* grid)
{
	if(colorMapType == 1) 
		return new_colorMap_fromGrid_multicolor(grid);
	else if(colorMapType == 2)
		return new_colorMap_fromGrid_2colors(grid,colorMapColors[0], colorMapColors[1]);
	else 
		return new_colorMap_fromGrid_unicolor(grid,colorMapColors[2]);

}
/******************************************************************************************************************************/
void  set_Color_From_colorMap(ColorMap* colorMap, gdouble color[], gdouble value)
{
	gint i = 0;
	gint k = 0;
	gint c;
	gdouble l,l1,l2;
	if(colorMap->numberOfColors<1)
	{
		for(c=0;c<3;c++)
			color[c] = 0.5;
		return;
	}
	if(value<=colorMap->colorValue[i].value)
	{
		for(c=0;c<3;c++)
			color[c] = colorMap->colorValue[i].color[c];
		return;
	}
	i = colorMap->numberOfColors-1;
	if(value>=colorMap->colorValue[i].value)
	{
		for(c=0;c<3;c++)
			color[c] = colorMap->colorValue[i].color[c];
		return;
	}

	k = 0;
	for(i=1;i<colorMap->numberOfColors-1;i++)
	{
		if(value>=colorMap->colorValue[i].value && value<=colorMap->colorValue[i+1].value)
		{
			k = i;
			break;
		}
	}
	l = colorMap->colorValue[k+1].value-colorMap->colorValue[k].value;
	l1 = value-colorMap->colorValue[k].value;
	l2 = l-l1;
	if(l>0 && l1>=0 && l2>=0)
	{
		l1 = l1/l;
		l2 = l2/l;
		for(c=0;c<3;c++) 
			color[c] = colorMap->colorValue[k].color[c]*l2+colorMap->colorValue[k+1].color[c]*l1;
	}
	else
	for(c=0;c<3;c++)
		color[c] = colorMap->colorValue[k].color[c];
}
/******************************************************************************************************************************/
void  reset_colorMap(GtkWidget* entry, gpointer data)
{
	gdouble maxValue = 0;
	gdouble minValue = 0;
	G_CONST_RETURN gchar* tmp;
	ColorMap* colorMap = g_object_get_data(G_OBJECT(entry),"ColorMap");
	ColorMap* newColorMap = NULL;
	GtkWidget* handlebox = g_object_get_data(G_OBJECT(entry), "Handlebox");
	GtkWidget* darea = g_object_get_data(G_OBJECT(entry), "DrawingArea");
	GtkWidget* otherEntry = g_object_get_data(G_OBJECT(entry), "OtherEntry");

	if(!colorMap) return;

	if(colorMap->numberOfColors<1) return;


	if(data) /* this is right entry */
	{
		minValue = colorMap->colorValue[0].value;
		tmp  = gtk_entry_get_text(GTK_ENTRY(entry));
		maxValue = atof(tmp);
	}
	else
	{
		maxValue = colorMap->colorValue[colorMap->numberOfColors-1].value;
		tmp  = gtk_entry_get_text(GTK_ENTRY(entry));
		minValue = atof(tmp);
	}
	newColorMap = new_colorMap_min_max(minValue, maxValue);
	colormap_free(colorMap);
	g_free(colorMap);

	colorMap = newColorMap;
	g_object_set_data(G_OBJECT(entry),"ColorMap", colorMap);
	g_object_set_data(G_OBJECT(handlebox),"ColorMap", colorMap);
	g_object_set_data(G_OBJECT(darea),"ColorMap", colorMap);
	g_object_set_data(G_OBJECT(otherEntry),"ColorMap", colorMap);

	/* print_colorMap(&myColorMap);*/
	RebuildSurf = TRUE;
	glarea_rafresh(GLArea);
	color_map_hide(handlebox);
	color_map_refresh(handlebox);
	color_map_show(handlebox);
}
/********************************************************************************/
static gint configure_event( GtkWidget *widget, GdkEventConfigure *event )
{
 
  	gint height = 0;
	GdkColor color;
	GdkColor tmpcolor;
	GdkColormap *colormap   = NULL;
  	GdkPixmap *pixmap = (GdkPixmap *)g_object_get_data(G_OBJECT(widget), "Pixmap");
 	PangoFontDescription *font_desc = pango_font_description_from_string ("helvetica bold 12");
	cairo_t* cr = (cairo_t *)g_object_get_data(G_OBJECT(widget), "Cairo");
	gboolean Ok = TRUE;
        GdkVisual* vis;
	gchar t[BSIZE];
	ColorMap* myColorMap = g_object_get_data(G_OBJECT(widget),"ColorMap");
	GdkGC *gc = g_object_get_data(G_OBJECT(widget),"Gdkgc");

	gint i;


	if (pixmap) g_object_unref(pixmap);
	if (cr) cairo_destroy (cr);

	pixmap = gdk_pixmap_new(widget->window, widget->allocation.width, widget->allocation.height, -1);
	cr = gdk_cairo_create (pixmap);
	
	colormap  = gdk_drawable_get_colormap(widget->window);

  	height = widget->allocation.height;
        vis = gdk_colormap_get_visual(colormap);
        if(vis->depth >15) Ok = TRUE;
	else Ok = FALSE;

	 color.red = 40000; 
	 color.green = 40000; 
	 color.blue = 40000; 

	gdk_colormap_alloc_color(colormap, &color, FALSE, TRUE);

	if(Ok)
	{
		
		gdouble max  = myColorMap->colorValue[myColorMap->numberOfColors-1].value;
		gdouble min  = myColorMap->colorValue[0].value;
  		for(i=0;i<widget->allocation.width;i++)
  		{
			gdouble v = i/(gdouble)(widget->allocation.width)*(max-min)+min;
			gdouble color[3];

			set_Color_From_colorMap(myColorMap, color, v);
			tmpcolor.red = (gushort)(color[0]*65535);
			tmpcolor.green = (gushort)(color[1]*65535);
			tmpcolor.blue = (gushort)(color[2]*65535);

			gdk_colormap_alloc_color(colormap, &tmpcolor, FALSE, TRUE);
			gdk_gc_set_foreground(gc,&tmpcolor);
    			gdk_draw_line(pixmap,gc,i,0,i,height);
			
  		}
	
  		for(i=widget->allocation.width/4;i<widget->allocation.width-widget->allocation.width/8;i+=widget->allocation.width/4)
  		{
			gdouble v = i/(gdouble)(widget->allocation.width)*(max-min)+min;
			{
				gint x = i;
				gint y = height-height/4;

				if(fabs(v)>1e-3) sprintf(t,"%0.3f",v);
				else sprintf(t,"%0.3e",v);
				
	 			color.red = 0; 
	 			color.green = 0; 
	 			color.blue = 0; 
				gdk_colormap_alloc_color(colormap, &color,TRUE,TRUE);
				gdk_gc_set_foreground(gc,&color);

				gdk_gc_set_line_attributes(gc,0,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);
				if(font_desc) 
					gabedit_cairo_string(cr, widget, font_desc, gc, x, y, t, TRUE,TRUE);
			}
		}
	}
	else
	{
		gdk_gc_set_foreground(gc,&color);
  		for(i=0;i<widget->allocation.width;i++)
    			gdk_draw_line(pixmap,gc,i,0,i,height);
  	}

  	g_object_set_data(G_OBJECT(widget), "Pixmap", pixmap);
  	g_object_set_data(G_OBJECT(widget), "Cairo", cr);
	if(font_desc) pango_font_description_free (font_desc);
  	return TRUE;
}
/********************************************************************************/   
static gint expose_event(GtkWidget  *widget,GdkEventExpose *event )
{
  	GdkPixmap *pixmap = NULL;

	if(event->count >0)
		return FALSE;

  	pixmap = (GdkPixmap *)g_object_get_data(G_OBJECT(widget), "Pixmap");
	if(pixmap)
		gdk_draw_drawable(widget->window,
                  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                  pixmap,
                  event->area.x, event->area.y,
                  event->area.x, event->area.y,
                  event->area.width, event->area.height);
  	return FALSE;
}
/********************************************************************************/
static GtkWidget *add_drawing_area(GtkWidget *table, gint i)
{
	GtkWidget *darea;
	GdkPixmap *pixmap = NULL;
	cairo_t* cr = NULL;

	darea = gtk_drawing_area_new();
	 gtk_widget_set_size_request(GTK_WIDGET(darea), 300, -1);


	gtk_table_attach(GTK_TABLE(table),darea,i,i+1,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			0,0);
	gtk_widget_realize(darea);
	pixmap = gdk_pixmap_new(darea->window,darea->allocation.width,darea->allocation.height,-1);
	cr = gdk_cairo_create (pixmap);
	g_object_set_data(G_OBJECT(darea), "Pixmap", pixmap);
	g_object_set_data(G_OBJECT(darea), "Cairo", cr);

	g_signal_connect(G_OBJECT(darea),"configure_event",(GCallback)configure_event,NULL);
	g_signal_connect(G_OBJECT(darea),"expose_event",(GCallback)expose_event,NULL);
	return darea;
}
/******************************************************************************************************************************/
GtkWidget* create_color_map_show(GtkWidget* box, ColorMap* colorMap, gchar* label)
{
  	GtkWidget *handlebox;
	GtkWidget* table; 
	GtkWidget* entry; 
	GtkWidget* entryLeft; 
	GtkWidget* entryRight; 
	GtkWidget *darea;
	GdkGC *gc = NULL;
	gint i;

  	handlebox = gtk_handle_box_new ();
  	gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(handlebox),GTK_SHADOW_NONE);    
	gtk_handle_box_set_handle_position  (GTK_HANDLE_BOX(handlebox),GTK_POS_LEFT);
  	gtk_widget_show (handlebox);
  	gtk_box_pack_start (GTK_BOX (box), handlebox, FALSE, FALSE, 0);
	table = gtk_table_new(1,4,FALSE);

	gtk_container_add (GTK_CONTAINER (handlebox), table);
	
	i = 0;
	if(label)
	{
		GtkWidget* labelWid = gtk_label_new(label);
		gtk_widget_show(labelWid);
		gtk_table_attach(GTK_TABLE(table),labelWid,i,i+1,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			0,0);
	}
	i = 1;
	entry = gtk_entry_new();
	gtk_widget_set_size_request(entry,80,-1);
	gtk_widget_show(entry);
	gtk_table_attach(GTK_TABLE(table),entry,i,i+1,0,i,
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			0,0);
	entryLeft = entry;

	i = 2;
	gc = gdk_gc_new(PrincipalWindow->window);
	darea = add_drawing_area(table, i);

	i = 3;
	entry = gtk_entry_new();
	gtk_widget_set_size_request(entry,80,-1);
	gtk_widget_show(entry);
	gtk_table_attach(GTK_TABLE(table),entry,i,i+1,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			0,0);
	entryRight = entry;

	gtk_widget_show(table);
	g_object_set_data(G_OBJECT(handlebox), "EntryLeft", entryLeft);
	g_object_set_data(G_OBJECT(handlebox), "EntryRight", entryRight);
	g_object_set_data(G_OBJECT(handlebox), "DrawingArea", darea);
	g_object_set_data(G_OBJECT(handlebox),"ColorMap", colorMap);
	g_object_set_data(G_OBJECT(handlebox),"Gdkgc", gc);

	g_object_set_data(G_OBJECT(darea),"ColorMap", colorMap);
	g_object_set_data(G_OBJECT(darea),"Gdkgc", gc);

	g_object_set_data(G_OBJECT(entryLeft), "DrawingArea", darea);
	g_object_set_data(G_OBJECT(entryRight), "DrawingArea", darea);

	g_object_set_data(G_OBJECT(entryLeft),"ColorMap", colorMap);
	g_object_set_data(G_OBJECT(entryRight),"ColorMap", colorMap);

	g_object_set_data(G_OBJECT(entryLeft),"Handlebox", handlebox);
	g_object_set_data(G_OBJECT(entryRight),"Handlebox", handlebox);

	g_object_set_data(G_OBJECT(entryLeft),"DrawingArea", darea);
	g_object_set_data(G_OBJECT(entryRight),"DrawingArea", darea);

	g_object_set_data(G_OBJECT(entryLeft),"OtherEntry", entryRight);
	g_object_set_data(G_OBJECT(entryRight),"OtherEntry", entryLeft);


	g_signal_connect(G_OBJECT (entryLeft), "activate",(GCallback)reset_colorMap, NULL);
	g_signal_connect(G_OBJECT (entryRight), "activate",(GCallback)reset_colorMap, GTK_OBJECT(entryLeft));

	return handlebox;
}
/******************************************************************************************************************************/
void color_map_show(GtkWidget* handlebox)
{
	GtkWidget* entryLeft = g_object_get_data(G_OBJECT(handlebox), "EntryLeft");
	GtkWidget* entryRight = g_object_get_data(G_OBJECT(handlebox), "EntryRight");
	ColorMap* myColorMap = g_object_get_data(G_OBJECT(handlebox), "ColorMap");
	gchar tmp[BSIZE];

	if(myColorMap->numberOfColors>0)
	{
		if(fabs(myColorMap->colorValue[0].value)>1e-3)
			sprintf(tmp,"%0.3f",myColorMap->colorValue[0].value);
		else
			if(fabs(myColorMap->colorValue[0].value)<1e-8)
				sprintf(tmp,"%0.1e",myColorMap->colorValue[0].value);
			else
				sprintf(tmp,"%lf",myColorMap->colorValue[0].value);

		gtk_entry_set_text(GTK_ENTRY(entryLeft),tmp);
		if(fabs(myColorMap->colorValue[myColorMap->numberOfColors-1].value)>1e-3)
			sprintf(tmp,"%0.3f",myColorMap->colorValue[myColorMap->numberOfColors-1].value);
		else
			if(fabs(myColorMap->colorValue[myColorMap->numberOfColors-1].value)<1e-8)
				sprintf(tmp,"%0.1e",myColorMap->colorValue[myColorMap->numberOfColors-1].value);
			else
				sprintf(tmp,"%lf",myColorMap->colorValue[myColorMap->numberOfColors-1].value);
		gtk_entry_set_text(GTK_ENTRY(entryRight),tmp);
	}

	gtk_widget_show_all(handlebox);
}
/******************************************************************************************************************************/
void color_map_refresh(GtkWidget* handlebox)
{
	GtkWidget* darea = g_object_get_data(G_OBJECT(handlebox), "DrawingArea");
	configure_event(darea, NULL);
}
/******************************************************************************************************************************/
void color_map_hide(GtkWidget* handlebox)
{
	gtk_widget_hide(handlebox);
}
