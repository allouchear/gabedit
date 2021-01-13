/* Help.c */
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
#include "Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"

/***************************************************************************/
void help_commands()
{
	gchar temp[BSIZE];
	GtkWidget* win;
	sprintf(temp,
		_("     * For add a command to the list:\n"
	        "       Type the new command and enter. \n"
		"       This command becomes the default command.\n\n"
		"     * For change the default command:\n"
	        "       Select a command from the list and type enter.\n\n" 
		"     * For remove a command from the list:\n"
	        "       Select the command to be removed and clicks on the\n"
	        "       \"Remove from list\" button.\n\n\n"
		"     * NB:\n"
	        "      You can choose as commands the name of any\n"
	       	"      script which accepts any number of parameters.\n"
	        "      However last parameter accepted by script must be the name\n"
	        "      of data file(included the extension \n"
	        "             .com for Gaussian, Molpro, Molcas and\n"
	        "             .inp for Gamess, FireFly and Q-Chem).\n"
		)
		 );
	win = Message(temp,_(" Info "),FALSE);
	gtk_window_set_modal (GTK_WINDOW (win), TRUE);
}
/***************************************************************************/
void help_ftp_rsh()
{
	gchar temp[BSIZE];
	GtkWidget* win;
	sprintf(temp,
		_("     To be able to use this protocol:\n\n"
		"     *  the server must support rsh \n"
		"     *  on the remot host\n"
		"        - edit .rhosts file and add the line: \n"
		"            yourlocalmachine yourlocallogin\n"
		"        - save file and \n"
		"        - execute the orders \n"
		"             chmod a-wx .rhosts\n"
		"             chmod u+rw .rhosts\n"
		)
		 );
	win = Message(temp,_(" Info "),FALSE);
	gtk_window_set_modal (GTK_WINDOW (win), TRUE);
}
#ifdef G_OS_WIN32
/***************************************************************************/
void help_ssh()
{
	gchar temp[2048];
	GtkWidget* win;
	sprintf(temp,
		_("     To be able to use ssh protocol:\n\n"
		"     *  the server must support ssh protocol.\n\n"
		"     *  on the local host,the pscp and plink programs  must installed\n"
		"        the 2 programes are livred with Gabedit\n"
		"        but you can download the programs from \n"
		"        http://www.chiark.greenend.org.uk/~sgtatham/putty/download.html\n"
		)
		);
	win = Message(temp," Info ",FALSE);
	gtk_window_set_modal (GTK_WINDOW (win), TRUE);
}
#else
/***************************************************************************/
void help_ssh()
{
	gchar temp[2048];
	GtkWidget* win;
	sprintf(temp,
		_("     To be able to use ssh protocol:\n\n"
		"     *  the server must support ssh protocol.\n\n"
		"     *  the local host must support ssh protocol.\n\n"
		"     *  on the local host, execute the orders:\n"
		"           ssh-keygen -t rsa (for create id_rsa ans id_rsa.pub files)\n"
		"           ssh-keygen -t dsa (for create id_dsa ans id_dsa.pub files)\n"
		"              for the 2 ordres : you must create the codes without passphrase.\n\n"
		"           copy id_rsa.pub, id_dsa.pub files at HOME directory at REMOT host.\n"
		"           (example scp id_*.pub remotlogin@remotehost:.)\n\n"
		"     *  on the remot host, execute the orders:\n"
		"          cat id_rsa.pub >> .ssh/authorized_keys\n"
		"          cat id_dsa.pub >> .ssh/authorized_keys2\n"
		"          chmod go-w .ssh/authorized_keys\n"
		"          chmod go-w .ssh/authorized_keys2\n"
		)
		);
	win = Message(temp,_(" Info "),FALSE);
	gtk_window_set_modal (GTK_WINDOW (win), TRUE);
}
#endif /* G_OS_WIN32 */
/***************************************************************************/
void help_variables_buttons()
{
 gchar *temp;
 temp=       g_strdup_printf(
	_("               In this frame you have 6 buttons :\n\n"
 	"New : For create a new variable\n"
 	"Edit : For modify the selected variable\n"
 	"Delete : For delete the selected variable\n"
 	"<= All : For tansform all variables in constants\n"
 	"<= One : For tansform the selected variable in constant\n"
 	"Help : For view this window\n"
	)
		 );
 
 Message(temp," Info ",FALSE);
 g_free(temp);
}
/***************************************************************************/
void help_geometry_buttons()
{
 gchar *temp;
 temp=       g_strdup_printf(
		 _("               In this frame you have 8 buttons :\n\n"
		 "New : For create a new center\n"
		 "Edit : For modify a selected center\n"
		 "Delete Last  : For delete the last center\n"
		 "Draw : For draw your geometry\n"
		 "Save in file : For save your geometry in file\n"
		 "All => : For tansform  all contants in variables\n"
		 "One => : For tansform the contants of selected center in variables\n"
		 "Help : For view this window\n"
		 )
		 );
 
 Message(temp," Info ",FALSE);
 g_free(temp);
}
/***************************************************************************/
gchar* messagePovray()
{
#ifndef G_OS_WIN32
		gchar* t = g_strdup_printf(
		_("You can create an animated file using gab*.pov files. For this  :\n"
        "    You must initially create files gab*.bmp using gab*.pov files. For example :\n"
		"           povray +Igab1.pov +Ogab1.bmp +W900 +H900\n"
		"           povray +Igab2.pov +Ogab2.bmp +W900 +H900\n"
		"             	.........................................\n"
		"           povray +Igabn.pov +Ogabn.bmp +W900 +H900\n"
        "   Then you can create the animated file from gab*.bmp files using convert program :\n"
		"            convert -delay 10 -loop 1000 gab*.bmp imageAnim.gif (for a gif animated file)\n"
		"            convert -delay 10 -loop 1000 gab*.bmp imageAnim.mng (for a png animated file)\n\n"
        "            You can also use the xPovAnim shell (from utils/povray of Gabedit directory) for create the gif animated file from gab*.pov files.\n\n"

	    "            povray is a free software. You can download this(for any system) from http://www.povray.org\n"
		"            convert is a free software. You can download this(for any system) from http://www.imagemagick.org\n"
		"           (in Linux convert is probably installed)\n\n"
		)
		);

		return t;
#else
		gchar* t = g_strdup_printf(
		_("You can create an animated file using gab*.pov files. For this  :\n"
        "    You must initially create files gab*.bmp using gab*.pov files.\n"
		"           run povray\n"
		"           in toolbar select '[320x240, AA 0.3]'\n"
		"           int toolbar add '+W500 +H500' option\n"
		"           click to Queue icon, you abtain a new window.\n"
		"           click to add File and select all gab*.pov files created by Gabedit\n"
		"           click to OK\n"
        "   Then you can create the animated file from gab*.bmp files using convert program, for this :\n"
		"            copy 'xAnim.bat' from Gabedit directory in directory of gab*pov files\n"
		"            click to xAnim.bat\n\n"
	    
		"            convert is a free software. You can download this(for any system) from http://www.imagemagick.org\n"
	    "            povray is a free software. You can download this(for any system) from http://www.povray.org\n\n"
	    )
	);

		return t;

#endif
}

/***************************************************************************/
gchar* messageBMP()
{
#ifndef G_OS_WIN32
		gchar* t = g_strdup(
		_(" You can create an animated file using convert software :\n"
		"     convert -delay 10 -loop 1000 gab*.bmp imageAnim.gif (for a gif animated file)\n"
		"     or\n"
		"     convert -delay 10 -loop 1000 gab*.bmp imageAnim.mng (for a png animated file)\n\n"
	    "convert is a free software. You can download this(for any system) from http://www.imagemagick.org\n"
		"(in Linux convert is probably installed)\n\n"
		)
		);
		return t;
#else
		gchar* t = g_strdup(
		_("You can create an animated file using convert software, for this :\n"
		"     copy 'xAmin.bat' from Gabedit directory in directory of gab*.bmp files\n"
		"     or\n"
		"     click to xAnim.bat\n\n"
	    "convert is a free software. You can download this(for any system) from http://www.imagemagick.org\n\n"
	    )
		);
		return t;

#endif
}
/***************************************************************************/
gchar* messagePPM()
{
#ifndef G_OS_WIN32
		gchar* t = g_strdup(
		_(" You can create an animated file using convert software :\n"
		"     convert -delay 10 -loop 1000 gab*.ppm imageAnim.gif (for a gif animated file)\n"
		"     or\n"
		"     convert -delay 10 -loop 1000 gab*.ppm imageAnim.mng (for a png animated file)\n\n"
	    "convert is a free software. You can download this(for any system) from http://www.imagemagick.org\n"
		"(in Linux convert is probably installed)\n\n"
		)
		);
		return t;
#else
		gchar* t = g_strdup(
		_("You can create an animated file using convert software, for this :\n"
		"     copy 'xAminPPM.bat' from Gabedit directory in directory of gab*.ppm files\n"
		"     or\n"
		"     click to xAnimPPM.bat\n\n"
	    "convert is a free software. You can download this(for any system) from http://www.imagemagick.org\n\n"
	    )
		);
		return t;
#endif
}
/***************************************************************************/
gchar* messageAnimatedImage(gchar* format)
{
	if(strcmp(format,"pov")==0) return messagePovray();
	gchar* t = g_strdup_printf(
	_(" You can create an animated file using convert software :\n"
	"     convert -delay 10 -loop 1000 gab*.%s imageAnim.gif (for a gif animated file)\n"
	"     or\n"
	"     convert -delay 10 -loop 1000 gab*.%s imageAnim.mng (for a png animated file)\n\n"
        "convert is a free software. You can download this(for any system) from http://www.imagemagick.org\n"
	"(in Linux convert is probably installed)\n\n"),
	format, format
	);
	return t;
}

/***************************************************************************/
