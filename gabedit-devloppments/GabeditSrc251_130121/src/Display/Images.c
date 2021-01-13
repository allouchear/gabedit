/* Images.c */
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
#include "GlobalOrb.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "GLArea.h"
#include <unistd.h>

/**************************************************************************/
static void snapshot_pixbuf_free (guchar   *pixels, gpointer  data)
{
	g_free (pixels);
}
/**************************************************************************/
/**************************************************************************/
static GdkPixbuf  *get_pixbuf_gl(guchar* colorTrans)
{       
      	gint stride;
	GdkPixbuf  *pixbuf = NULL;
	GdkPixbuf  *tmp = NULL;
	GdkPixbuf  *tmp2 = NULL;
	guchar *data;
  	gint height;
  	gint width;
	GLint viewport[4];

	glGetIntegerv(GL_VIEWPORT, viewport);
  	width  = viewport[2];
  	height = viewport[3];

	stride = width*4;

	data = g_malloc0 (sizeof (guchar) * stride * height);
#ifdef G_OS_WIN32 
  	glReadBuffer(GL_BACK);
  	glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,data);
#else
  	glReadBuffer(GL_FRONT);
  	glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,data);
#endif

	tmp = gdk_pixbuf_new_from_data (data, GDK_COLORSPACE_RGB, TRUE, 
                                      8, width, height, stride, snapshot_pixbuf_free,
                                      NULL);
	if(tmp)
	{
		tmp2 = gdk_pixbuf_flip (tmp, TRUE); 
		g_object_unref (tmp);
	}

	if(tmp2)
	{
		pixbuf = gdk_pixbuf_rotate_simple (tmp2, GDK_PIXBUF_ROTATE_UPSIDEDOWN);
		g_object_unref (tmp2);
	}

	if(colorTrans)
	{
		tmp = gdk_pixbuf_add_alpha(pixbuf, TRUE, colorTrans[0], colorTrans[1], colorTrans[2]);
		if(tmp!=pixbuf)
		{
 			g_object_unref (pixbuf);
			pixbuf = tmp;
		}
	}
	
	return pixbuf;
}
/*************************************************************************/
void gabedit_save_image_gl(GtkWidget* widget, gchar *fileName, gchar* type, guchar* colorTrans)
{       
	GError *error = NULL;
	GdkPixbuf  *pixbuf = NULL;
	pixbuf = get_pixbuf_gl(colorTrans);
	if(pixbuf)
	{
		if(!fileName)
		{
			GtkClipboard * clipboard;
			clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
			if(clipboard)
			{
				gtk_clipboard_clear(clipboard);
				gtk_clipboard_set_image(clipboard, pixbuf);
			}
		}
		else 
		{
			if(type && strstr(type,"j") && strstr(type,"g") )
			gdk_pixbuf_save(pixbuf, fileName, type, &error, "quality", "100", NULL);
			else if(type && strstr(type,"png"))
			gdk_pixbuf_save(pixbuf, fileName, type, &error, "compression", "9", NULL);
			else if(type && (strstr(type,"tif") || strstr(type,"tiff")))
			gdk_pixbuf_save(pixbuf, fileName, "tiff", &error, "compression", "1", NULL);
			else
			gdk_pixbuf_save(pixbuf, fileName, type, &error, NULL);
		}
	 	g_object_unref (pixbuf);
	}
}
/*******************************************************************************/
/*       Save in a png (without background) format file */
/*******************************************************************************/
void save_png_without_background_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;
	guchar color[3];

	gint numCol = get_background_color(color);

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}

	gtk_widget_hide(GTK_WIDGET(SelecFile));
	gtk_window_move(GTK_WINDOW(PrincipalWindow),0,0);
	glarea_rafresh(GLArea);
	while( gtk_events_pending() ) gtk_main_iteration();
	if(numCol>=0) gabedit_save_image_gl(GLArea, fileName, "png",color);
	else gabedit_save_image_gl(GLArea, fileName, "png",NULL);
} 
/**************************************************************************
*       Save the Frame Buffer in a png format file
**************************************************************************/
void save_png_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}

	gtk_widget_hide(GTK_WIDGET(SelecFile));
	gtk_window_move(GTK_WINDOW(PrincipalWindow),0,0);
	while( gtk_events_pending() ) gtk_main_iteration();
	glarea_rafresh(GLArea);
	while( gtk_events_pending() ) gtk_main_iteration();
	gabedit_save_image_gl(GLArea, fileName, "png",NULL);
} 
/**************************************************************************
*       Save the Frame Buffer in a jpeg format file
**************************************************************************/
void save_jpeg_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}

	gtk_widget_hide(GTK_WIDGET(SelecFile));
	gtk_window_move(GTK_WINDOW(PrincipalWindow),0,0);
	glarea_rafresh(GLArea);
	while( gtk_events_pending() )
		gtk_main_iteration();
	gabedit_save_image_gl(GLArea, fileName, "jpeg",NULL);
} 
/**************************************************************************/
void copy_to_clipboard()
{       
	glarea_rafresh(GLArea);
	while( gtk_events_pending() ) gtk_main_iteration();
	 gabedit_save_image_gl(GLArea, NULL, NULL,NULL);
} 
/**************************************************************************/
void copy_to_clipboard_without_background()
{       
	glarea_rafresh(GLArea);
	guchar color[3];
	gint numCol = get_background_color(color);
	while( gtk_events_pending() ) gtk_main_iteration();
	if(numCol>-1) gabedit_save_image_gl(GLArea, NULL, NULL,color);
	else gabedit_save_image_gl(GLArea, NULL, NULL,NULL);
} 
/**************************************************************************
*       Save the Frame Buffer in a ppm format file
**************************************************************************/
static gchar* save_ppm(gchar* fileName)
{       
	FILE *file;
	int i;
	int j;
	int k;
	int width;
	int height;
	GLubyte *rgbbuf;
	static gchar message[1024];

 	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		sprintf(message,_("Sorry\n No selected file"));
	       	return message;
	}

	file = FOpen(fileName,"wb");

	if (!file)
	{
		sprintf(message,_("Sorry: can't open %s file\n"), fileName);
		return message;
	}
  
	width =  GLArea->allocation.width;
	height = GLArea->allocation.height;

	glPixelStorei(GL_PACK_ROW_LENGTH,width);
	glPixelStorei(GL_PACK_ALIGNMENT,1);

	rgbbuf = (GLubyte *) malloc(3*width*height*sizeof(GLubyte));
  	if (!rgbbuf)
	{
		sprintf(message,_("Sorry: couldn't allocate memory\n"));
	    	fclose(file);
		return message;
  	}

#ifdef G_OS_WIN32 
  	glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,rgbbuf);
#else
  	glReadBuffer(GL_FRONT);
  	glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,rgbbuf);
#endif

        fprintf(file,"P6\n");
        fprintf(file,"#Image rendered with gabedit\n");
        fprintf(file,"%d\n%d\n255\n", width,height);

	for(i=height-1; i>= 0; i--){
	   for(j=0; j< width; j++){
		/* int ierr;*/
		k = 3*(j + i*width);
		/*
		ierr = fwrite( &rgbbuf[k] ,sizeof(*rgbbuf), 1, file);
		ierr = fwrite( &rgbbuf[k+1] ,sizeof(*rgbbuf), 1, file);
		ierr = fwrite( &rgbbuf[k+2] ,sizeof(*rgbbuf), 1, file);
		*/
		fwrite( &rgbbuf[k] ,sizeof(*rgbbuf), 1, file);
		fwrite( &rgbbuf[k+1] ,sizeof(*rgbbuf), 1, file);
		fwrite( &rgbbuf[k+2] ,sizeof(*rgbbuf), 1, file);
	   }
	}

	fclose(file);
	free(rgbbuf);
	return NULL;
} 
/**************************************************************************/
void save_ppm_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;
	gchar* message;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}

	gtk_widget_hide(GTK_WIDGET(SelecFile));
	gtk_window_move(GTK_WINDOW(PrincipalWindow),0,0);
	glarea_rafresh(GLArea);
	while( gtk_events_pending() )
		gtk_main_iteration();
	message = save_ppm(fileName);
	if(message != NULL)
	{
		Message(message,_("Error"),TRUE);
	}
} 
/**************************************************************************
*       Save the Frame Buffer in a bmp format file
**************************************************************************/
static void WLSBL(int val,char* arr)
{
    arr[0] = (char) (val&0xff);
    arr[1] = (char) ((val>>8) &0xff);
    arr[2] = (char) ((val>>16)&0xff);
    arr[3] = (char) ((val>>24)&0xff);
}
/**************************************************************************/
static gchar* save_bmp(gchar* fileName)
{       
	/* int ierr;*/
	FILE *file;
	int i;
	int j;
	int width;
	int height;
  	GLubyte *rgbbuf;
  	GLubyte rgbtmp[3];
  	int pad;
	static gchar message[1024];
	char bmp_header[]=
	{ 'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0,
  	40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0, 0,0,0,0, 0,0,0,0,
  	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };

 	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		sprintf(message,_("Sorry\n No selected file"));
	       	return message;
	}

	file = FOpen(fileName,"wb");

	if (!file)
	{
		sprintf(message,_("Sorry: can't open %s file\n"), fileName);
		return message;
	}
  
	width =  GLArea->allocation.width;
	height = GLArea->allocation.height;

	glPixelStorei(GL_PACK_ROW_LENGTH,width);
	glPixelStorei(GL_PACK_ALIGNMENT,1);

  	rgbbuf = (GLubyte *)malloc(3*width*height*sizeof(GLubyte));
  	if (!rgbbuf)
	{
		sprintf(message,_("Sorry: couldn't allocate memory\n"));
	    	fclose(file);
		return message;
  	}
#ifdef G_OS_WIN32 
  	glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,rgbbuf);
#else
  	glReadBuffer(GL_FRONT);
  	glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,rgbbuf);
#endif

/* The number of bytes on a screenline should be wholly devisible by 4 */

  	pad = (width*3)%4;
  	if (pad) pad = 4 - pad;

  	WLSBL((int) (3*width+pad)*height+54,bmp_header+2);
  	WLSBL((int) width,bmp_header+18);
  	WLSBL((int) height,bmp_header+22);
  	WLSBL((int) 3*width*height,bmp_header+34);

  	/* ierr = fwrite(bmp_header,1,54,file);*/
  	fwrite(bmp_header,1,54,file);

  	for (i=0;i<height;i++)
	{
    		for (j=0;j<width;j++)
		{
			rgbtmp[0] = rgbbuf[(j+width*i)*3+2];
			rgbtmp[1] = rgbbuf[(j+width*i)*3+1];
			rgbtmp[2] = rgbbuf[(j+width*i)*3+0];
			/* ierr = fwrite(rgbtmp,3,1,file);*/
			fwrite(rgbtmp,3,1,file);
    		}
    	rgbtmp[0] = (char) 0;
    	for (j=0;j<pad;j++) 
		/* ierr = fwrite(rgbtmp,1,1,file);*/
		fwrite(rgbtmp,1,1,file);
  	}

  	fclose(file);
  	free(rgbbuf);
	return NULL;
}
/**************************************************************************/
void save_bmp_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;
	gchar* message;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}

	gtk_widget_hide(GTK_WIDGET(SelecFile));
	gtk_window_move(GTK_WINDOW(PrincipalWindow),0,0);
	glarea_rafresh(GLArea);
	while( gtk_events_pending() )
		gtk_main_iteration();
	message = save_bmp(fileName);
	if(message != NULL)
	{
		Message(message,_("Error"),TRUE);
	}
}
/**************************************************************************
*       Save the Frame Buffer in a ps format file
**************************************************************************/
void ps_header(FILE* file)
{
   fprintf(file,"%%true {\n");
   fprintf(file,"systemdict /colorimage known not {\n");
   fprintf(file,"%%\n");
   fprintf(file,"/colorImageDict 50 dict def\n");
   fprintf(file,"/colorimage {\n");
   fprintf(file,"    colorImageDict begin\n");
   fprintf(file,"    /Ncomp exch def\n");
   fprintf(file,"    {\n");
   fprintf(file,"        (Multi-source not implemented\\n) print flush\n");
   fprintf(file,"        limitcheck\n");
   fprintf(file,"    } {\n");
   fprintf(file,"        /Dsrc exch def\n");
   fprintf(file,"        /Matrix exch def\n");
   fprintf(file,"        /Bcomp exch def\n");
   fprintf(file,"        /Height exch def\n");
   fprintf(file,"        /Width exch def\n");
   fprintf(file,"        /Bycomp Bcomp 7 add 8 idiv def\n");
   fprintf(file,"        Bcomp 8 gt { (Only 8 bit per sample images \\n)\n");
   fprintf(file,"                     print flush limitcheck\n");
   fprintf(file,"                   } if\n");
   fprintf(file,"        Width Height Bcomp Matrix\n");
   fprintf(file,"        Ncomp 1 eq {\n");
   fprintf(file,"            { Dsrc exec }\n");
   fprintf(file,"        } if\n");
   fprintf(file,"        Ncomp 3 eq {\n");
   fprintf(file,"          /Gstr Bycomp Width mul string def\n");
   fprintf(file,"          { Dsrc exec\n");
   fprintf(file,"             /Cstr exch def\n");
   fprintf(file,"             0 1 Width 1 sub {\n");
   fprintf(file,"               /I exch def\n");
   fprintf(file,"               /X I 3 mul def\n");
   fprintf(file,"               Gstr I\n");
   fprintf(file,"                 Cstr X       get 0.3  mul\n");
   fprintf(file,"                 Cstr X 1 add get 0.59 mul\n");
   fprintf(file,"                 Cstr X 2 add get 0.11 mul\n");
   fprintf(file,"                 add add cvi\n");
   fprintf(file,"               put\n");
   fprintf(file,"              } for\n");
   fprintf(file,"             Gstr\n");
   fprintf(file,"          }\n");
   fprintf(file,"        } if\n");
   fprintf(file,"        Ncomp 4 eq {\n");
   fprintf(file,"          /Gstr Bycomp Width mul string def\n");
   fprintf(file,"          { Dsrc exec\n");
   fprintf(file,"             /Cstr exch def\n");
   fprintf(file,"             0 1 Width 1 sub {\n");
   fprintf(file,"               /I exch def\n");
   fprintf(file,"               /X I 4 mul def\n");
   fprintf(file,"               Gstr I\n");
   fprintf(file,"                 2 Bcomp exp 1 sub\n");
   fprintf(file,"                 Cstr X       get 0.3  mul\n");
   fprintf(file,"                 Cstr X 1 add get 0.59 mul\n");
   fprintf(file,"                 Cstr X 2 add get 0.11 mul\n");
   fprintf(file,"                 Cstr X 3 add get\n");
   fprintf(file,"                 add add add dup 2 index gt {pop dup} if\n");
   fprintf(file,"                 sub cvi\n");
   fprintf(file,"               put\n");
   fprintf(file,"              } for\n");
   fprintf(file,"             Gstr\n");
   fprintf(file,"          }\n");
   fprintf(file,"        } if\n");
   fprintf(file,"        image\n");
   fprintf(file,"    } ifelse\n");
   fprintf(file,"    end\n");
   fprintf(file,"} bind def\n");
   fprintf(file,"} if\n");
}
/****************************************************************************************************************/
void save_ps_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;
	FILE *file;
	int i;
	int j;
	int k;
	int width;
	int height;
	GLubyte *rgbbuf;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    	return ;
 	}
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	gtk_window_move(GTK_WINDOW(PrincipalWindow),0,0);
	glarea_rafresh(GLArea);
	while( gtk_events_pending() )
		gtk_main_iteration();
  
	width =  GLArea->allocation.width;
	height = GLArea->allocation.height;


	glPixelStorei(GL_PACK_ROW_LENGTH,width);
	glPixelStorei(GL_PACK_ALIGNMENT,1);

	rgbbuf = (GLubyte *) malloc(3*width*height*sizeof(GLubyte));
	if (!rgbbuf) {
            Message(_("Sorry: couldn't allocate memory\n"),_("Error"),TRUE);
            return;
	}

#ifdef G_OS_WIN32 
  	glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,rgbbuf);
#else
  	glReadBuffer(GL_FRONT);
  	glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,rgbbuf);
#endif

        file = FOpen(fileName,"w");

        if (!file) {
            Message(_("Sorry: can't open output file\n"),_("Error"),TRUE);
            return;
        }

        fprintf(file,"%%!PS-Adobe-2.0 EPSF-2.0\n");
        fprintf(file,"%%%%BoundingBox: 16 16 %d %d\n",width+16,height+16);
        fprintf(file,"%%%%Creator: gabedit\n");
        fprintf(file,"%%%%Title: gabedit output file\n");
        fprintf(file,"%%%%EndComments\n");

	ps_header(file);

	fprintf(file,"/picstr %d string def\n",height*3);

	fprintf(file,"16 16 translate\n");
        fprintf(file,"%d %d scale\n",width,height);
        fprintf(file,"%d %d 8 [ %d 0 0 %d 0 0] \n",width,height,width,height);
        fprintf(file,"{ currentfile picstr readhexstring  pop }\n");
        fprintf(file,"false 3 colorimage\n");

        j = k = 0;
	for (i=0;i<width*height+1;i++){

	   fprintf(file,"%02x%02x%02x",rgbbuf[j],rgbbuf[j+1],rgbbuf[j+2]);
	   j += 3;
           k += 6;

           if (k>70){ 
                fprintf(file,"\n");
                k=0;
           }
        }
        fprintf(file,"\nshowpage\n");   
        fprintf(file,"%%%%Trailer\n");
        fclose(file);
        free(rgbbuf);
}
/**************************************************************************/
gchar* new_bmp(gchar* dirname, int i)
{
	gchar* fileName = g_strdup_printf("%s%sgab%d.bmp",dirname,G_DIR_SEPARATOR_S,i);
	gchar* message;
	if(i==1)
	{
		gint j;
		for(j=0;j<100;j++)
		{
			gchar* filestoDelete = g_strdup_printf("%s%sgab%d.bmp",dirname,G_DIR_SEPARATOR_S,j);
			unlink(filestoDelete);
			g_free(filestoDelete);

		}
	}

	message = save_bmp(fileName);
	g_free(fileName);
	return message;
}
/**************************************************************************/
gchar* new_ppm(gchar* dirname, int i)
{
	gchar* fileName = g_strdup_printf("%s%sgab%d.ppm",dirname,G_DIR_SEPARATOR_S,i);
	gchar* message;
	if(i==1)
	{
		gint j;
		for(j=0;j<100;j++)
		{
			gchar* filestoDelete = g_strdup_printf("%s%sgab%d.ppm",dirname,G_DIR_SEPARATOR_S,j);
			unlink(filestoDelete);
			g_free(filestoDelete);

		}
	}

	message = save_ppm(fileName);
	g_free(fileName);
	return message;
}
/**************************************************************************/
gchar* new_jpeg(gchar* dirname, int i)
{
	gchar* fileName = g_strdup_printf("%s%sgab%d.jpg",dirname,G_DIR_SEPARATOR_S,i);
	if(i==1)
	{
		gint j;
		for(j=0;j<100;j++)
		{
			gchar* filestoDelete = g_strdup_printf("%s%sgab%d.jpg",dirname,G_DIR_SEPARATOR_S,j);
			unlink(filestoDelete);
			g_free(filestoDelete);

		}
	}

	gabedit_save_image_gl(GLArea, fileName, "jpeg",NULL);
	g_free(fileName);
	return NULL;
}
/**************************************************************************/
gchar* new_png(gchar* dirname, int i)
{
	gchar* fileName = g_strdup_printf("%s%sgab%d.png",dirname,G_DIR_SEPARATOR_S,i);
	if(i==1)
	{
		gint j;
		for(j=0;j<100;j++)
		{
			gchar* filestoDelete = g_strdup_printf("%s%sgab%d.png",dirname,G_DIR_SEPARATOR_S,j);
			unlink(filestoDelete);
			g_free(filestoDelete);

		}
	}

	gabedit_save_image_gl(GLArea, fileName, "png",NULL);
	g_free(fileName);
	return NULL;
}
/**************************************************************************/
gchar* new_png_without_background(gchar* dirname, int i)
{
	gchar* fileName = g_strdup_printf("%s%sgab%d.png",dirname,G_DIR_SEPARATOR_S,i);
	guchar color[3];
	gint numCol = get_background_color(color);

	if(i==1)
	{
		gint j;
		for(j=0;j<100;j++)
		{
			gchar* filestoDelete = g_strdup_printf("%s%sgab%d.png",dirname,G_DIR_SEPARATOR_S,j);
			unlink(filestoDelete);
			g_free(filestoDelete);

		}
	}

	if(numCol>=0) gabedit_save_image_gl(GLArea, fileName, "png",color);
	else gabedit_save_image_gl(GLArea, fileName, "png",NULL);
	g_free(fileName);
	return NULL;
}
