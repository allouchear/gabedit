/* MolcasSeward.c */
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
#include "../Molcas/MolcasTypes.h"
#include "../Molcas/MolcasGlobal.h"
#include "../Molcas/MolcasBasis.h"
#include "../Molcas/MolcasScf.h"
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
#include "../Utils/AtomsProp.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Symmetry/MoleculeSymmetry.h"
#include "../Symmetry/MoleculeSymmetryInterface.h"

/************************************************************************************************************/
static void putBeginSewardInTextEditor()
{
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.program, &molcasColorBack.program, " &SEWARD\n",-1);
}
/************************************************************************************************************/
static void putTitleSewardInTextEditor()
{
        gchar buffer[BSIZE];

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.subProgram, NULL, "Title=",-1);

	sprintf(buffer,"Seward data By Gabedit\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);

}
/************************************************************************************************************/
static void putEndSewardInTextEditor()
{
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.program, &molcasColorBack.program, "End of input\n\n",-1);
}
/************************************************************************************************************/
void putSewardInfoInTextEditor()
{
	putBeginSewardInTextEditor();
	putTitleSewardInTextEditor();
	putEndSewardInTextEditor();
}
/************************************************************************************************************/
