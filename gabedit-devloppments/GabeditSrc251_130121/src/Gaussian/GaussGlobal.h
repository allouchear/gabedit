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

#ifndef __GABEDIT_GAUSSGLOBAL_H__
#define __GABEDIT_GAUSSGLOBAL_H__

#define NHBOXT_MAX 6
#define NHBOXM_MAX 6
#define NHBOXB_MAX 5
#define NMETHODS 3
#define NCHECKBUTTON 9
#include <gtk/gtk.h>


extern GtkWidget *TextTitle;
extern gchar *Tfreq;
extern gchar *Tscf;
extern gchar *Tpolar;
extern gchar* TtimeDependent;
extern gchar *Types;

extern GtkWidget* FrameT;
extern GtkWidget* VboxT;
extern guint nHboxT;
extern GtkWidget* HboxT[NHBOXT_MAX];
extern GtkWidget *EntryTypes[NHBOXT_MAX];

extern gchar *Methodes[NMETHODS];
extern gchar *Basis[NMETHODS];
extern GtkWidget* FrameM[NMETHODS];
extern GtkWidget* VboxM[NMETHODS];
extern guint nHboxM;
extern GtkWidget* HboxM[NHBOXM_MAX][NMETHODS];
extern GtkWidget *EntryMethods[NHBOXM_MAX][NMETHODS];
extern GtkWidget* FrameB[NMETHODS];
extern GtkWidget* VboxB[NMETHODS];
extern guint nHboxB;
extern GtkWidget* HboxB[NHBOXB_MAX][NMETHODS];
extern GtkWidget *EntryBasis[NHBOXB_MAX][NMETHODS];
extern GtkWidget *NoteBookMB;
extern gint NM;

extern GtkWidget* CheckButtons[NCHECKBUTTON];
extern GtkWidget *EntryCS[6];
extern gboolean GeomFromCheck;
extern gchar* StrLink;

#endif /* __GABEDIT_GAUSSGLOBAL_H__ */

