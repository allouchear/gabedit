/* MPQCMpqc.c */
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

#include <stdlib.h>
#include <ctype.h>

#include "../../Config.h"
#include "../Common/Global.h"
#include "../MPQC/MPQCTypes.h"
#include "../MPQC/MPQCGlobal.h"
#include "../MPQC/MPQCProperties.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/GeomConversion.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Common/Windows.h"
#include "../Utils/Constants.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Symmetry/MoleculeSymmetry.h"
#include "../Symmetry/MoleculeSymmetryInterface.h"
/************************************************************************************************************/
void initMPQCMpqc()
{
	mpqcMpqc.mole = g_strdup("$:mole");
	mpqcMpqc.opt = g_strdup("$:opt");
	mpqcMpqc.do_gradient = FALSE;
	mpqcMpqc.optimize = FALSE;
	/*mpqcMpqc.optimize = TRUE;*/
	mpqcMpqc.restart = FALSE;
	mpqcMpqc.checkpoint = FALSE;
	mpqcMpqc.savestate = FALSE;
	mpqcMpqc.frequencies = FALSE;
}
/************************************************************************************************************/
void freeMPQCMpqc()
{
	if(mpqcMpqc.mole ) g_free(mpqcMpqc.mole);
	if(mpqcMpqc.opt ) g_free(mpqcMpqc.opt);
	mpqcMpqc.mole = NULL;
	mpqcMpqc.opt = NULL;
}
/**************************************************************************************************************************************/
static void putMPQCMpqcInTextEditor()
{
        gchar buffer[BSIZE];

	sprintf(buffer,"%c----------------------------------------------------------------\n",'%');

	sprintf(buffer,"mpqc: (\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);

	if(mpqcMpqc.do_gradient) sprintf(buffer,"\tdo_gradient =  yes\n");
	else sprintf(buffer,"\tdo_gradient =  no\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	if(mpqcMpqc.optimize) sprintf(buffer,"\toptimize =  yes\n");
	else sprintf(buffer,"\toptimize =  no\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	if(mpqcMpqc.restart) sprintf(buffer,"\trestart =  yes\n");
	else sprintf(buffer,"\trestart =  no\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	if(mpqcMpqc.checkpoint) sprintf(buffer,"\tcheckpoint =  yes\n");
	else sprintf(buffer,"\tcheckpoint =  no\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	if(mpqcMpqc.savestate) sprintf(buffer,"\tsavestate =  yes\n");
	else sprintf(buffer,"\tsavestate =  no\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	sprintf(buffer,"\tmole = %s\n", mpqcMpqc.mole);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	if(
		mpqcMpqc.optimize 
		&& !strstr(mpqcMole.method,"ZA") 
		&& !strstr(mpqcMole.method,"OPT") 
	)
	{
		sprintf(buffer,"\topt = %s\n", mpqcMpqc.opt);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	}
	if(mpqcMpqc.frequencies)
	{
		sprintf(buffer,"\tfreq = $:freq\n");
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	}

	sprintf(buffer,")\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);

	sprintf(buffer,"%c----------------------------------------------------------------\n",'%');
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
}
/************************************************************************************************************/
void putMPQCMpqcInfoInTextEditor()
{
	if(mpqcMpqc.frequencies) putMPQCPropertiesInfoInTextEditor();
	putMPQCMpqcInTextEditor();
}
