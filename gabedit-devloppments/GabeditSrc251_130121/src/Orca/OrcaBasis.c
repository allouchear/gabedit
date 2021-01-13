/* OrcaBasis.c */
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
#include "../Orca/OrcaTypes.h"
#include "../Orca/OrcaGlobal.h"
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

static gchar selectedTypeBasis[BSIZE]="";
static gchar** listBasisReal = NULL;
static gchar** listBasisView = NULL;
static gint numberOfBasis = 0;
/*************************************************************************************************************/
static gchar* listTypeBasisView[] = 
{ 
	"Pople Style basis sets", 
	"Pople with one diffuse function on non-hydrogen atoms", 
	"Pople with one diffuse function on all atoms", 
	"Dunning basis sets", 
	"Ahlrichs basis sets", 
	"Def2 Ahlrichs basis sets", 
	"Jensen Basis Sets", 
	"Atomic Natural Orbital Basis Sets", 
	"Miscellenous and Specialized Basis Sets", 
};
static gchar* listTypeBasisReal[] = 
{ 
	"Pople", 
	"PopleDiffuseNonHydrogen", 
	"PopleDiffuseAllAtoms", 
	"Dunning", 
	"Ahlrichs", 
	"Def2Ahlrichs", 
	"Jensen", 
	"ANO", 
	"Miscellenous", 
};
static guint numberOfTypeBasis = G_N_ELEMENTS (listTypeBasisView);
/*************************************************************************************************************/
static gchar* listPopleBasisView[] = 
{ 
	"3-21G Pople 3-21G",
	"3-21GSP Buenker 3-21GSP",
	"4-22GSP Buenker 4-22GSP",
	"6-31G Pople 6-31G and its modifications",
	"6-311G Pople 6-311G and its modifications",

	"3-21G*  3-21G plus one polarisation function all non-hydrogens atoms",
	"3-21GSP* 3-21GSP plus one polarisation function all non-hydrogens atoms",
	"4-22GSP* 4-22GSP plus one polarisation function all non-hydrogens atoms",
	"6-31G* 6-31G plus one polarisation function all non-hydrogens atoms",
	"6-311G* 6-311G plus one polarisation function all non-hydrogens atoms",

	"3-21G**  3-21G plus one polarisation function all atoms",
	"3-21GSP** 3-21GSP plus one polarisation function all atoms",
	"4-22GSP** 4-22GSP plus one polarisation function all atoms",
	"6-31G** 6-31G plus one polarisation function all atoms",
	"6-311G** 6-311G plus one polarisation function all atoms",

	"3-21G(2d)  3-21G plus two polarisation functions all non-hydrogens atoms",
	"3-21GSP(2d) 3-21GSP plus two polarisation functions all non-hydrogens atoms",
	"4-22GSP(2d) 4-22GSP plus two polarisation functions all non-hydrogens atoms",
	"6-31G(2d) 6-31G plus two polarisation functions all non-hydrogens atoms",
	"6-311G(2d) 6-311G plus two polarisation functions all non-hydrogens atoms",

	"3-21G(2d,2p)  3-21G plus two polarisation functions all atoms",
	"3-21GSP(2d,2p) 3-21GSP plus two polarisation functions all atoms",
	"4-22GSP(2d,2p) 4-22GSP plus two polarisation functions all atoms",
	"6-31G(2d,2p) 6-31G plus two polarisation functions all atoms",
	"6-311G(2d,2p) 6-311G plus two polarisation functions all atoms",

	"3-21G(2df)  3-21G plus three polarisation functions all non-hydrogens atoms",
	"3-21GSP(2df) 3-21GSP plus three polarisation functions all non-hydrogens atoms",
	"4-22GSP(2df) 4-22GSP plus three polarisation functions all non-hydrogens atoms",
	"6-31G(2df) 6-31G plus three polarisation functions all non-hydrogens atoms",
	"6-311G(2df) 6-311G plus three polarisation functions all non-hydrogens atoms",

	"3-21G(2df,2pd)  3-21G plus three polarisation functions all atoms",
	"3-21GSP(2df,2pd) 3-21GSP plus three polarisation functions all atoms",
	"4-22GSP(2df,2pd) 4-22GSP plus three polarisation functions all atoms",
	"6-31G(2df,2pd) 6-31G plus three polarisation functions all atoms",
	"6-311G(2df,2pd) 6-311G plus three polarisation functions all atoms",

	"3-21G(3df)  3-21G plus four polarisation functions all non-hydrogens atoms",
	"3-21GSP(3df) 3-21GSP plus four polarisation functions all non-hydrogens atoms",
	"4-22GSP(3df) 4-22GSP plus four polarisation functions all non-hydrogens atoms",
	"6-31G(3df) 6-31G plus four polarisation functions all non-hydrogens atoms",
	"6-311G(3df) 6-311G plus four polarisation functions all non-hydrogens atoms",

	"3-21G(3df,3pd)  3-21G plus four polarisation functions all atoms",
	"3-21GSP(3df,3pd) 3-21GSP plus four polarisation functions all atoms",
	"4-22GSP(3df,3pd) 4-22GSP plus four polarisation functions all atoms",
	"6-31G(3df,3pd) 6-31G plus four polarisation functions all atoms",
	"6-311G(3df,3pd) 6-311G plus four polarisation functions all atoms",
};
static gchar* listPopleBasisReal[] = 
{ 
	"3-21G",
	"3-21GSP",
	"4-22GSP",
	"6-31G",
	"6-311G",

	"3-21G*",
	"3-21GSP*",
	"4-22GSP*",
	"6-31G*",
	"6-311G*",

	"3-21G**",
	"3-21GSP**",
	"4-22GSP**",
	"6-31G**",
	"6-311G**",

	"3-21G(2d)",
	"3-21GSP(2d)",
	"4-22GSP(2d)",
	"6-31G(2d)",
	"6-311G(2d)",

	"3-21G(2d,2p)",
	"3-21GSP(2d,2p)",
	"4-22GSP(2d,2p)",
	"6-31G(2d,2p)",
	"6-311G(2d,2p)",

	"3-21G(2df)",
	"3-21GSP(2df)",
	"4-22GSP(2df)",
	"6-31G(2df)",
	"6-311G(2df)",

	"3-21G(2df,2pd)",
	"3-21GSP(2df,2pd)",
	"4-22GSP(2df,2pd)",
	"6-31G(2df,2pd)",
	"6-311G(2df,2pd)",

	"3-21G(3df)",
	"3-21GSP(3df)",
	"4-22GSP(3df)",
	"6-31G(3df)",
	"6-311G(3df)",

	"3-21G(3df,3pd)",
	"3-21GSP(3df,3pd)",
	"4-22GSP(3df,3pd)",
	"6-31G(3df,3pd)",
	"6-311G(3df,3pd)",
};
static guint numberOfPopleBasis = G_N_ELEMENTS (listPopleBasisView);
/*************************************************************************************************************/
static gchar* listPopleDiffuseNonHydrogenBasisView[] = 
{ 
	"3-21+G  3-21G plus diffuse functions on all non-hydrogens atoms",
	"3-21+GSP 3-21GSP plus diffuse functions on all non-hydrogens atoms",
	"4-22+GSP 4-22GSP plus diffuse functions on all non-hydrogens atoms",
	"6-31+G 6-31G plus diffuse functions on all non-hydrogens atoms",
	"6-311+G 6-311G plus diffuse functions on all non-hydrogens atoms",

	"3-21+G*  3-21G + diff. non-hydrogens + 1 pol. non-hydrogens",
	"3-21+GSP* 3-21GSP + diff. non-hydrogens + 1 pol. non-hydrogens",
	"4-22+GSP* 4-22GSP + diff. non-hydrogens + 1 pol. non-hydrogens",
	"6-31+G* 6-31G + diff. non-hydrogens + 1 pol. non-hydrogens",
	"6-311+G* 6-311G + diff. non-hydrogens + 1 pol. non-hydrogens",

	"3-21+G**  3-21G + diff. non-hydrogens + 1 pol. ",
	"3-21+GSP** 3-21GSP + diff. non-hydrogens + 1 pol. ",
	"4-22+GSP** 4-22GSP + diff. non-hydrogens + 1 pol. ",
	"6-31+G** 6-31G + diff. non-hydrogens + 1 pol. ",
	"6-311+G** 6-311G + diff. non-hydrogens + 1 pol. ",

	"3-21+G(2d)  3-21G + diff. non-hydrogens + 2 pol. non-hydrogens",
	"3-21+GSP(2d) 3-21GSP + diff. non-hydrogens + 2 pol. non-hydrogens",
	"4-22+GSP(2d) 4-22GSP + diff. non-hydrogens + 2 pol. non-hydrogens",
	"6-31+G(2d) 6-31G + diff. non-hydrogens + 2 pol. non-hydrogens",
	"6-311+G(2d) 6-311G + diff. non-hydrogens + 2 pol. non-hydrogens",

	"3-21+G(2d,2p)  3-21G + diff. non-hydrogens + 2 pol.",
	"3-21+GSP(2d,2p) 3-21GSP + diff. non-hydrogens + 2 pol.",
	"4-22+GSP(2d,2p) 4-22GSP + diff. non-hydrogens + 2 pol.",
	"6-31+G(2d,2p) 6-31G + diff. non-hydrogens + 2 pol.",
	"6-311+G(2d,2p) 6-311G + diff. non-hydrogens + 2 pol.",

	"3-21+G(2df)  3-21G + diff. non-hydrogens + 3 pol. non-hydrogens",
	"3-21+GSP(2df) 3-21GSP + diff. non-hydrogens + 3 pol. non-hydrogens",
	"4-22+GSP(2df) 4-22GSP + diff. non-hydrogens + 3 pol. non-hydrogens",
	"6-31+G(2df) 6-31G + diff. non-hydrogens + 3 pol. non-hydrogens",
	"6-311+G(2df) 6-311G + diff. non-hydrogens + 3 pol. non-hydrogens",

	"3-21+G(2df,2pd)  3-21G + diff. non-hydrogens + 3 pol.",
	"3-21+GSP(2df,2pd) 3-21GSP + diff. non-hydrogens + 3 pol.",
	"4-22+GSP(2df,2pd) 4-22GSP + diff. non-hydrogens + 3 pol.",
	"6-31+G(2df,2pd) 6-31G + diff. non-hydrogens + 3 pol.",
	"6-311+G(2df,2pd) 6-311G + diff. non-hydrogens + 3 pol.",

	"3-21+G(3df)  3-21G + diff. non-hydrogens + 4 pol. non-hydrogens",
	"3-21+GSP(3df) 3-21GSP + diff. non-hydrogens + 4 pol. non-hydrogens",
	"4-22+GSP(3df) 4-22GSP + diff. non-hydrogens + 4 pol. non-hydrogens",
	"6-31+G(3df) 6-31G + diff. non-hydrogens + 4 pol. non-hydrogens",
	"6-311+G(3df) 6-311G + diff. non-hydrogens + 4 pol. non-hydrogens",

	"3-21+G(3df,3pd)  3-21G + diff. non-hydrogens + 4 pol.",
	"3-21+GSP(3df,3pd) 3-21GSP + diff. non-hydrogens + 4 pol.",
	"4-22+GSP(3df,3pd) 4-22GSP + diff. non-hydrogens + 4 pol.",
	"6-31+G(3df,3pd) 6-31G + diff. non-hydrogens + 4 pol.",
	"6-311+G(3df,3pd) 6-311G + diff. non-hydrogens + 4 pol.",
};
static gchar* listPopleDiffuseNonHydrogenBasisReal[] = 
{ 
	"3-21+G",
	"3-21+GSP",
	"4-22+GSP",
	"6-31+G",
	"6-311+G",

	"3-21+G*",
	"3-21+GSP*",
	"4-22+GSP*",
	"6-31+G*",
	"6-311+G*",

	"3-21+G**",
	"3-21+GSP**",
	"4-22+GSP**",
	"6-31+G**",
	"6-311+G**",

	"3-21+G(2d)",
	"3-21+GSP(2d)",
	"4-22+GSP(2d)",
	"6-31+G(2d)",
	"6-311+G(2d)",

	"3-21+G(2d,2p)",
	"3-21+GSP(2d,2p)",
	"4-22+GSP(2d,2p)",
	"6-31+G(2d,2p)",
	"6-311+G(2d,2p)",

	"3-21+G(2df)",
	"3-21+GSP(2df)",
	"4-22+GSP(2df)",
	"6-31+G(2df)",
	"6-311+G(2df)",

	"3-21+G(2df,2pd)",
	"3-21+GSP(2df,2pd)",
	"4-22+GSP(2df,2pd)",
	"6-31+G(2df,2pd)",
	"6-311+G(2df,2pd)",

	"3-21+G(3df)",
	"3-21+GSP(3df)",
	"4-22+GSP(3df)",
	"6-31+G(3df)",
	"6-311+G(3df)",

	"3-21+G(3df,3pd)",
	"3-21+GSP(3df,3pd)",
	"4-22+GSP(3df,3pd)",
	"6-31+G(3df,3pd)",
	"6-311+G(3df,3pd)",
};
static guint numberOfPopleDiffuseNonHydrogenBasis = G_N_ELEMENTS (listPopleDiffuseNonHydrogenBasisView);
/*************************************************************************************************************/
static gchar* listPopleDiffuseAllAtomsBasisView[] = 
{ 
	"3-21++G  3-21G plus diffuse functions on all atoms",
	"3-21++GSP 3-21GSP plus diffuse functions on all atoms",
	"4-22++GSP 4-22GSP plus diffuse functions on all atoms",
	"6-31++G 6-31G plus diffuse functions on all atoms",
	"6-311++G 6-311G plus diffuse functions on all atoms",

	"3-21++G*  3-21G + diff. + 1 pol. non-hydrogens",
	"3-21++GSP* 3-21GSP + diff. + 1 pol. non-hydrogens",
	"4-22++GSP* 4-22GSP + diff. + 1 pol. non-hydrogens",
	"6-31++G* 6-31G + diff. + 1 pol. non-hydrogens",
	"6-311++G* 6-311G + diff. + 1 pol. non-hydrogens",

	"3-21++G**  3-21G + diff. + 1 pol. ",
	"3-21++GSP** 3-21GSP + diff. + 1 pol. ",
	"4-22++GSP** 4-22GSP + diff. + 1 pol. ",
	"6-31++G** 6-31G + diff. + 1 pol. ",
	"6-311++G** 6-311G + diff. + 1 pol. ",

	"3-21++G(2d)  3-21G + diff. + 2 pol. non-hydrogens",
	"3-21++GSP(2d) 3-21GSP + diff. + 2 pol. non-hydrogens",
	"4-22++GSP(2d) 4-22GSP + diff. + 2 pol. non-hydrogens",
	"6-31++G(2d) 6-31G + diff. + 2 pol. non-hydrogens",
	"6-311++G(2d) 6-311G + diff. + 2 pol. non-hydrogens",

	"3-21++G(2d,2p)  3-21G + diff. + 2 pol.",
	"3-21++GSP(2d,2p) 3-21GSP + diff. + 2 pol.",
	"4-22++GSP(2d,2p) 4-22GSP + diff. + 2 pol.",
	"6-31++G(2d,2p) 6-31G + diff. + 2 pol.",
	"6-311++G(2d,2p) 6-311G + diff. + 2 pol.",

	"3-21++G(2df)  3-21G + diff. + 3 pol. non-hydrogens",
	"3-21++GSP(2df) 3-21GSP + diff. + 3 pol. non-hydrogens",
	"4-22++GSP(2df) 4-22GSP + diff. + 3 pol. non-hydrogens",
	"6-31++G(2df) 6-31G + diff. + 3 pol. non-hydrogens",
	"6-311++G(2df) 6-311G + diff. + 3 pol. non-hydrogens",

	"3-21++G(2df,2pd)  3-21G + diff. + 3 pol.",
	"3-21++GSP(2df,2pd) 3-21GSP + diff. + 3 pol.",
	"4-22++GSP(2df,2pd) 4-22GSP + diff. + 3 pol.",
	"6-31++G(2df,2pd) 6-31G + diff. + 3 pol.",
	"6-311++G(2df,2pd) 6-311G + diff. + 3 pol.",

	"3-21++G(3df)  3-21G + diff. + 4 pol. non-hydrogens",
	"3-21++GSP(3df) 3-21GSP + diff. + 4 pol. non-hydrogens",
	"4-22++GSP(3df) 4-22GSP + diff. + 4 pol. non-hydrogens",
	"6-31++G(3df) 6-31G + diff. + 4 pol. non-hydrogens",
	"6-311++G(3df) 6-311G + diff. + 4 pol. non-hydrogens",

	"3-21++G(3df,3pd)  3-21G + diff. + 4 pol.",
	"3-21++GSP(3df,3pd) 3-21GSP + diff. + 4 pol.",
	"4-22++GSP(3df,3pd) 4-22GSP + diff. + 4 pol.",
	"6-31++G(3df,3pd) 6-31G + diff. + 4 pol.",
	"6-311++G(3df,3pd) 6-311G + diff. + 4 pol.",

};
static gchar* listPopleDiffuseAllAtomsBasisReal[] = 
{ 
	"3-21++G",
	"3-21++GSP",
	"4-22++GSP",
	"6-31++G",
	"6-311++G",

	"3-21++G*",
	"3-21++GSP*",
	"4-22++GSP*",
	"6-31++G*",
	"6-311++G*",

	"3-21++G**",
	"3-21++GSP**",
	"4-22++GSP**",
	"6-31++G**",
	"6-311++G**",

	"3-21++G(2d)",
	"3-21++GSP(2d)",
	"4-22++GSP(2d)",
	"6-31++G(2d)",
	"6-311++G(2d)",

	"3-21++G(2d,2p)",
	"3-21++GSP(2d,2p)",
	"4-22++GSP(2d,2p)",
	"6-31++G(2d,2p)",
	"6-311++G(2d,2p)",

	"3-21++G(2df)",
	"3-21++GSP(2df)",
	"4-22++GSP(2df)",
	"6-31++G(2df)",
	"6-311++G(2df)",

	"3-21++G(2df,2pd)",
	"3-21++GSP(2df,2pd)",
	"4-22++GSP(2df,2pd)",
	"6-31++G(2df,2pd)",
	"6-311++G(2df,2pd)",

	"3-21++G(3df)",
	"3-21++GSP(3df)",
	"4-22++GSP(3df)",
	"6-31++G(3df)",
	"6-311++G(3df)",

	"3-21++G(3df,3pd)",
	"3-21++GSP(3df,3pd)",
	"4-22++GSP(3df,3pd)",
	"6-31++G(3df,3pd)",
	"6-311++G(3df,3pd)",
};
static guint numberOfPopleDiffuseAllAtomsBasis = G_N_ELEMENTS (listPopleDiffuseAllAtomsBasisView);
/*************************************************************************************************************/
static gchar* listDunningBasisView[] = 
{ 
	"cc-pVDZ Dunning correlation concisistent polarized double zeta",
	"cc-(p)VDZ Same but no polarization on hydrogens",
	"Aug-cc-pVDZ Same but including diffuse functions",
	"cc-pVTZ Dunning correlation concisistent polarized triple zeta",
	"cc-(p)VTZ Same but no polarization on hydrogen",
	"Aug-cc-pVTZ Same but including diffuse functions(g-functions deleted!)",
	"cc-pVQZ Dunning correlation concisistent polarized quadruple zeta",
	"Aug-cc-pVQZ with diffuse functions",
	"cc-pV5Z Dunning correlation concisistent polarized quintuple zeta",
	"Aug-cc-pV5Z with diffuse functions",
	"cc-pV6Z Dunning correlation concisistent polarized sextuple zeta",
	"Aug-cc-pV6Z ... with diffuse functions",
	"cc-pCVDZ Core-polarized double-zeta correlation consistent basis set",
	"cc-pCVTZ Same for triple zeta",
	"cc-pCVQZ Same for quadruple zeta",
	"cc-pCV5Z Same for quintuple zeta",
	"cc-pV6Z Same for sextuple zeta",
	"Aug-pCVDZ Same double zeta with diffuse functions augmented",
	"Aug-pCVTZ Same for triple zeta",
	"Aug-pCVQZ Same for quadruple zeta",
	"Aug-pCV5Z Same for quintuple zeta",
	"Aug-cc-pV6Z Same for sextuple zeta",
	"DUNNING-DZP Dunning's original double zeta basis set",
};
static gchar* listDunningBasisReal[] = 
{ 
	"cc-pVDZ",
	"cc-(p)VDZ",
	"Aug-cc-pVDZ",
	"cc-pVTZ",
	"cc-(p)VTZ",
	"Aug-cc-pVTZ",
	"cc-pVQZ",
	"Aug-cc-pVQZ",
	"cc-pV5Z",
	"Aug-cc-pV5Z",
	"cc-pV6Z",
	"Aug-cc-pV6Z",
	"cc-pCVDZ",
	"cc-pCVTZ",
	"cc-pCVQZ",
	"cc-pCV5Z",
	"cc-pV6Z",
	"Aug-pCVDZ",
	"Aug-pCVTZ",
	"Aug-pCVQZ",
	"Aug-pCV5Z",
	"Aug-cc-pV6Z",
};
static guint numberOfDunningBasis = G_N_ELEMENTS (listDunningBasisView);
/*************************************************************************************************************/
static gchar* listAhlrichsBasisView[] = 
{ 
	"SV Ahlrichs split valence basis set",
	"VDZ Ahlrichs split valence basis set",
	"VTZ Ahlrichs Valence triple zeta basis set",
	"TZV Ahlrichs triple-zeta valence basis set. NOT identical to VTZ",
	"QZVP Ahlrichs quadruple-zeta basis set. P is already polarized",
	"DZ Ahlrichs double zeta basis set",
	"QZVPP(-g,-f) QZVPP with highest polarization functions deleted",

	"SV(P) SV + One polar set on all non-hydrogens atoms",
	"VDZ(P) VDZ + One polar set on all non-hydrogens atoms",
	"VTZ(P) VTZ  + One polar set on all non-hydrogens atoms",
	"TZV(P) TZV  + One polar set on all non-hydrogens atoms",
	"DZ(P)  DZ   + One polar set on all non-hydrogens atoms",

	"SVP SV + One polar set on all atoms",
	"VDZP VDZ + One polar set on all atoms",
	"VTZP VTZ  + One polar set on all atoms",
	"TZVP TZV  + One polar set on all atoms",
	"DZP  DZ   + One polar set on all atoms",

	"SV(2D) SV + Two polar set on all non-hydrogens atoms",
	"VDZ(2D) VDZ + Two polar set on all non-hydrogens atoms",
	"VTZ(2D) VTZ  + Two polar set on all non-hydrogens atoms",
	"TZV(2D) TZV  + Two polar set on all non-hydrogens atoms",
	"DZ(2D)  DZ   + Two polar set on all non-hydrogens atoms",

	"SV(2D,2P) SV + Two polar set on all atoms",
	"VDZ(2D,2P) VDZ + Two polar set on all atoms",
	"VTZ(2D,2P) VTZ  + Two polar set on all atoms",
	"TZV(2D,2P) TZV  + Two polar set on all atoms",
	"DZ(2D,2P)  DZ   + Two polar set on all atoms",

	"SV(2df) SV + Three polar set on all non-hydrogens atoms",
	"VDZ(2df) VDZ + Three polar set on all non-hydrogens atoms",
	"VTZ(2df) VTZ  + Three polar set on all non-hydrogens atoms",
	"TZV(2df) TZV  + Three polar set on all non-hydrogens atoms",
	"DZ(2df)  DZ   + Three polar set on all non-hydrogens atoms",

	"SV(2df,2pd) SV + Three polar set on all atoms",
	"VDZ(2df,2pd) VDZ + Three polar set on all atoms",
	"VTZ(2df,2pd) VTZ  + Three polar set on all atoms",
	"TZV(2df,2pd) TZV  + Three polar set on all atoms",
	"DZ(2df,2pd)  DZ   + Three polar set on all atoms",

	"SV(PP) SV + Three polar set on all non-hydrogens atoms",
	"VDZ(PP) VDZ + Three polar set on all non-hydrogens atoms",
	"VTZ(PP) VTZ  + Three polar set on all non-hydrogens atoms",
	"TZV(PP) TZV  + Three polar set on all non-hydrogens atoms",
	"DZ(PP)  DZ   + Three polar set on all non-hydrogens atoms",

	"SVPP SV + Three polar set on all atoms",
	"VDZPP VDZ + Three polar set on all atoms",
	"VTZPP VTZ  + Three polar set on all atoms",
	"TZVPP TZV  + Three polar set on all atoms",
	"DZPP  DZ   + Three polar set on all atoms",

	"SV(P)+ SV plus Pople diff. func. + 1 polar on non-hydrogens",
	"VDZ(P)+ VDZ plus Pople diff. func. + 1 polar on non-hydrogens",
	"VTZ(P)+ VTZ plus Pople diff. func. + 1 polar on non-hydrogens",
	"TZV(P)+ TZV  plus Pople diff. func. + 1 polar on non-hydrogens",

	"SVP++ SV plus Pople diff. func. + One polar set on all atoms",
	"TZVP++ TZV  plus Pople diff. func. + One polar set on all atoms",
	"TZV(2D) TZV  lus Pople diff. func. + One polar set on all atoms",

	"aug-SV(P) SV plus Dunning diff. One polar set on all non-hydrogens atoms",
	"aug-VDZ(P) VDZ plus Dunning diff. One polar set on all non-hydrogens atoms",
	"aug-VTZ(P) VTZ  plus Dunning diff. One polar set on all non-hydrogens atoms",
	"aug-TZV(P) TZV  plus Dunning diff. One polar set on all non-hydrogens atoms",
	"aug-DZ(P)  DZ   plus Dunning diff. One polar set on all non-hydrogens atoms",

	"aug-SVP SV plus Dunning diff. One polar set on all atoms",
	"aug-VDZP VDZ plus Dunning diff. One polar set on all atoms",
	"aug-VTZP VTZ  plus Dunning diff. One polar set on all atoms",
	"aug-TZVP TZV  plus Dunning diff. One polar set on all atoms",
	"aug-DZP  DZ   plus Dunning diff. One polar set on all atoms",

	"aug-SV(2D) SV plus Dunning diff. Two polar set on all non-hydrogens atoms",
	"aug-VDZ(2D) VDZ plus Dunning diff. Two polar set on all non-hydrogens atoms",
	"aug-VTZ(2D) VTZ  plus Dunning diff. Two polar set on all non-hydrogens atoms",
	"aug-TZV(2D) TZV  plus Dunning diff. Two polar set on all non-hydrogens atoms",
	"aug-DZ(2D)  DZ   plus Dunning diff. Two polar set on all non-hydrogens atoms",

	"aug-SV(2D,2P) SV plus Dunning diff. Two polar set on all atoms",
	"aug-VDZ(2D,2P) VDZ plus Dunning diff. Two polar set on all atoms",
	"aug-VTZ(2D,2P) VTZ  plus Dunning diff. Two polar set on all atoms",
	"aug-TZV(2D,2P) TZV  plus Dunning diff. Two polar set on all atoms",
	"aug-DZ(2D,2P)  DZ   plus Dunning diff. Two polar set on all atoms",

	"aug-SV(2df) SV plus Dunning diff. Three polar set on all non-hydrogens atoms",
	"aug-VDZ(2df) VDZ plus Dunning diff. Three polar set on all non-hydrogens atoms",
	"aug-VTZ(2df) VTZ  plus Dunning diff. Three polar set on all non-hydrogens atoms",
	"aug-TZV(2df) TZV  plus Dunning diff. Three polar set on all non-hydrogens atoms",
	"aug-DZ(2df)  DZ   plus Dunning diff. Three polar set on all non-hydrogens atoms",

	"aug-SV(2df,2pd) SV plus Dunning diff. Three polar set on all atoms",
	"aug-VDZ(2df,2pd) VDZ plus Dunning diff. Three polar set on all atoms",
	"aug-VTZ(2df,2pd) VTZ  plus Dunning diff. Three polar set on all atoms",
	"aug-TZV(2df,2pd) TZV  plus Dunning diff. Three polar set on all atoms",
	"aug-DZ(2df,2pd)  DZ   plus Dunning diff. Three polar set on all atoms",

	"aug-SV(PP) SV plus Dunning diff. Three polar set on all non-hydrogens atoms",
	"aug-VDZ(PP) VDZ plus Dunning diff. Three polar set on all non-hydrogens atoms",
	"aug-VTZ(PP) VTZ  plus Dunning diff. Three polar set on all non-hydrogens atoms",
	"aug-TZV(PP) TZV  plus Dunning diff. Three polar set on all non-hydrogens atoms",
	"aug-DZ(PP)  DZ   plus Dunning diff. Three polar set on all non-hydrogens atoms",

	"aug-SVPP SV plus Dunning diff. Three polar set on all atoms",
	"aug-VDZPP VDZ plus Dunning diff. Three polar set on all atoms",
	"aug-VTZPP VTZ  plus Dunning diff. Three polar set on all atoms",
	"aug-TZVPP TZV  plus Dunning diff. Three polar set on all atoms",
	"aug-DZPP  DZ   plus Dunning diff. Three polar set on all atoms",
};
static gchar* listAhlrichsBasisReal[] = 
{ 
	"SV",
	"VDZ",
	"VTZ",
	"TZV",
	"QZVP",
	"DZ",
	"QZVPP(-g,-f)",

	"SV(P)",
	"VDZ(P)",
	"VTZ(P)",
	"TZV(P)",
	"DZ(P)",

	"SVP",
	"VDZP",
	"VTZP ",
	"TZVP ",
	"DZP",

	"SV(2D)",
	"VDZ(2D)",
	"VTZ(2D)",
	"TZV(2D)",
	"DZ(2D)",

	"SV(2D,2P)",
	"VDZ(2D,2P)",
	"VTZ(2D,2P)",
	"TZV(2D,2P)",
	"DZ(2D,2P)",

	"SV(2df)",
	"VDZ(2df)",
	"VTZ(2df)",
	"TZV(2df)",
	"DZ(2df)",

	"SV(2df,2pd)",
	"VDZ(2df,2pd)",
	"VTZ(2df,2pd)",
	"TZV(2df,2pd)",
	"DZ(2df,2pd)",

	"SV(PP)",
	"VDZ(PP)",
	"VTZ(PP)",
	"TZV(PP)",
	"DZ(PP)",

	"SVPP",
	"VDZPP",
	"VTZPP",
	"TZVPP",
	"DZPP",

	"SV(P)+",
	"VDZ(P)+",
	"VTZ(P)+",
	"TZV(P)+",

	"SVP++",
	"TZVP++",
	"TZV(2D)",

	"aug-SV(P)",
	"aug-VDZ(P)",
	"aug-VTZ(P)",
	"aug-TZV(P)",
	"aug-DZ(P)",

	"aug-SVP",
	"aug-VDZP",
	"aug-VTZP",
	"aug-TZVP",
	"aug-DZP",

	"aug-SV(2D)",
	"aug-VDZ(2D)",
	"aug-VTZ(2D)",
	"aug-TZV(2D)",
	"aug-DZ(2D)",

	"aug-SV(2D,2P)",
	"aug-VDZ(2D,2P)",
	"aug-VTZ(2D,2P)",
	"aug-TZV(2D,2P)",
	"aug-DZ(2D,2P)",

	"aug-SV(2df)",
	"aug-VDZ(2df)",
	"aug-VTZ(2df)",
	"aug-TZV(2df)",
	"aug-DZ(2df)",

	"aug-SV(2df,2pd)",
	"aug-VDZ(2df,2pd)",
	"aug-VTZ(2df,2pd)",
	"aug-TZV(2df,2pd)",
	"aug-DZ(2df,2pd)",

	"aug-SV(PP)",
	"aug-VDZ(PP)",
	"aug-VTZ(PP)",
	"aug-TZV(PP)",
	"aug-DZ(PP)",

	"aug-SVPP",
	"aug-VDZPP",
	"aug-VTZPP",
	"aug-TZVPP",
	"aug-DZPP",

};
static guint numberOfAhlrichsBasis = G_N_ELEMENTS (listAhlrichsBasisView);
/*************************************************************************************************************/
static gchar* listDef2AhlrichsBasisView[] = 
{ 
	"Def2-SV(P) SV basis set with 'new' polarization functions",
	"Def2-SVP",
	"Def2-TZVP TZVP basis set with 'new' polarization functions",
	"Def2-TZVP(-f) Delete the f-polarization functions from def2-TZVP",
	"Def2-TZVP(-df) delete the double d-function and replace it by the older single d-function.",
	"Def2-TZVPP TZVPP basis set with 'new' polarization functions",
	"Def2-aug-TZVPP Same but with diffuse functions from aug-cc-pVTZ",
	"Def2-QZVPP Very accurate quadruple-zeta basis.",
	"Def2-QZVPP(-g,-f) highest angular momentum polarization functions deleted",
};
static gchar* listDef2AhlrichsBasisReal[] = 
{ 
	"Def2-SV(P)",
	"Def2-SVP",
	"Def2-TZVP",
	"Def2-TZVP(-f)",
	"Def2-TZVP(-df)",
	"Def2-TZVPP",
	"Def2-aug-TZVPP",
	"Def2-QZVPP",
	"Def2-QZVPP(-g,-f)",
};
static guint numberOfDef2AhlrichsBasis = G_N_ELEMENTS (listDef2AhlrichsBasisView);
/*************************************************************************************************************/
static gchar* listJensenBasisView[] = 
{ 
	"PC-1 Polarization consistent basis sets (H-Ar) optimized for DFT",
	"PC-2 double zeta polarization consistent basis sets (H-Ar) optimized for DFT",
	"PC-3 triple zeta polarization consistent basis sets (H-Ar) optimized for DFT",
	"PC-4 quadruple zeta polarization consistent basis sets (H-Ar) optimized for DFT",
	"Aug-PC-1 PC-1 with augmentations by diffuse functions",
	"Aug-PC-2 PC-2 with augmentations by diffuse functions",
	"Aug-PC-3 PC-3 with augmentations by diffuse functions",
	"Aug-PC-4 PC-4 with augmentations by diffuse functions",
};
static gchar* listJensenBasisReal[] = 
{ 
	"PC-1",
	"PC-2",
	"PC-3",
	"PC-4",
	"Aug-PC-1",
	"Aug-PC-2",
	"Aug-PC-3",
	"Aug-PC-4",
};
static guint numberOfJensenBasis = G_N_ELEMENTS (listJensenBasisView);
/*************************************************************************************************************/
static gchar* listANOBasisView[] = 
{ 
	"ano-pVDZ better than the cc-pVDZ (but much larger number of primitives of course)",
	"ano-pVTZ",
	"ano-pVQZ",
	"ano-pV5Z",
	"saug-ano-pVDZ ano-pVDZ augmentation with a single set of s,p functions.",
	"saug-ano-pVTZ ano-pVTZ augmentation with a single set of s,p functions.",
	"saug-ano-pVQZ ano-pVQZ augmentation with a single set of s,p functions. ",
	"saug-ano-pV5Z ano-pV5Z augmentation with a single set of s,p functions.",
	"aug-ano-pVDZ ano-pVDZ full augmentation with spd",
	"aug-ano-pVTZ ano-pVTZ full augmentation with spdf",
	"aug-ano-pVQZ ano-pVQZ full augmentation with spdfg",
	"BNANO-DZP (Bonn-ANO-DZP), small DZP type ANO basis set from the Bonn group",
	"BNANO-TZ2P (Bonn-ANO-TZ2P), slightly larger triple-zeta ANO with two pol. Bonn group",
	"BNANO-TZ3P Same but with a contracted set of f-polarization functions on the heavy atoms",
	"NASA-AMES-ANO The original NASA/AMES ANO basis set (quadruple-zeta type)",
	"BAUSCHLICHER ANO First row transition metal ANO sets",
	"ROOS-ANO-DZP A fairly large DZP basis set from Roos, same size as aug-ano-pVDZ",
	"ROOS-ANO-TZP A fairly large TZP basis from Roos, same size as aug-ano-pVTZ",

};
static gchar* listANOBasisReal[] = 
{ 
	"ano-pVDZ",
	"ano-pVTZ",
	"ano-pVQZ",
	"ano-pV5Z",
	"saug-ano-pVDZ",
	"saug-ano-pVTZ",
	"saug-ano-pVQZ",
	"saug-ano-pV5Z",
	"aug-ano-pVDZ",
	"aug-ano-pVTZ",
	"aug-ano-pVQZ",
	"BNANO-DZP",
	"BNANO-TZ2P",
	"BNANO-TZ3P",
	"NASA-AMES-ANO",
	"BAUSCHLICHER",
	"ROOS-ANO-DZP",
	"ROOS-ANO-TZP",
};
static guint numberOfANOBasis = G_N_ELEMENTS (listANOBasisView);
/*************************************************************************************************************/
static gchar* listMiscellenousBasisView[] = 
{ 
	"DGAUSS DGauss polarized valence double zeta basis set",
	"DZVP-DFT DGauss polarized valence double zeta basis set",
	"SADLEJ-PVTZ Sadlej's polarized triple zeta basis for poarlizability and related calculations",
	"EPR-II Barone's Basis set for EPR calculations (double zeta)",
	"EPR-III Barone's Basis set for EPR calculations (triple-zeta)",
	"IGLO-II Kutzelniggs basis set for NMR and EPR calculations",
	"IGLO-III Kutzelniggs basis set for NMR and EPR calculations (accurate)",
	"Partridge-1 Accurate uncontracted basis set",
	"Partridge-2 Accurate uncontracted basis set",
	"Partridge-3 Accurate uncontracted basis set",
	"Wachters Good first row transition metal basis set",

};
static gchar* listMiscellenousBasisReal[] = 
{ 
	"DGAUSS",
	"DZVP-DFT",
	"SADLEJ-PVTZ",
	"EPR-II",
	"EPR-III",
	"IGLO-II",
	"IGLO-III",
	"Partridge-1",
	"Partridge-2",
	"Partridge-3",
	"Wachters",
};
static guint numberOfMiscellenousBasis = G_N_ELEMENTS (listMiscellenousBasisView);
/*************************************************************************************************************/
static gchar* listAuxBasisView[] = 
{ 
	"AutoAux Automatic construction of a general purpose fitting basis",
	"DEMON/J The DeMon/J Coulomb fitting basis",
	"DGAUSS/J The DGauss A1 Coulomb fitting basis",
	"SV/J (=VDZ/J) Ahlrichs Coulomb fitting basis for the SVP basis",
	"TZV/J (=VTZ/J) Ahlrichs Coulomb fitting basis for the TZV or TZVP basis",
	"QZVPP/J Ahlrichs Coulomb fitting for the QZVPP basis",
	"Def2-SVP/J Ahlrichs Coulomb fitting for def-SVP",
	"Def2-TZVPP/J Ahlrichs Coulomb fitting for def2-TZVPP/J",
	"Def2-QZVPP/J Ahlrichs Coulomb fitting for def2-QZVPP/J",
	"SV/J(-f) Same as SV/J but with the highest angular momentum aux-function deleted",
	"TZV/J(-f) Same as TZV/J but with the highest angular momentum aux-function deleted",
	"SV/C (=VDZ/C) The Ahlrichs correlation fitting basis for MP2-RI with SVP",
	"TZV/C (=VTZ/C) The Ahlrichs correlation fitting basis for MP2-RI with TZVP",
	"TZVPP/C (=VTZPP/C) The Ahlrichs correlation fitting basis for MP2-RI with extended triple-z bases",
	"QZVP/C Correlation fitting for the QZVP basis",
	"QZVPP/C Correlation fitting for the QZVPP basis",
	"Def2-SVP/C Correlation fitting for the def2-SVP basis",
	"Def2-TZVP/C Correlation fitting for the def2-TZVP basis",
	"Def2-TZVPP/C Correlation fitting for the def2-TZVPP basis",
	"Def2-QZVPP/C Correlation fitting for the def2-QZVPP basis",
	"cc-pVDZ/C Aux-basis for the cc-pVDZ orbital basis",
	"cc-pVTZ/C Aux-basis for the cc-pVTZ orbital basis",
	"cc-pVQZ/C Aux-basis for the cc-pVQZ orbital basis",
	"cc-pV5Z/C Aux-basis for the cc-pV5Z orbital basis",
	"cc-pV6Z/C Aux-basis for the cc-pV6Z orbital basis,",
	"Aug-cc-pVDZ/C Aux-basis for the aug-cc-pVDZ orbital basis",
	"Aug-cc-pVTZ/C Aux-basis for the aug-cc-pVTZ orbital basis",
	"Aug-SV/C Aux basis for SVP and related bases but with diffuse functions",
	"Aug-TZV/C Aux basis for TZVP and related bases but with diffuse functions",
	"Aug-TZVPP/C Aux basis for TZVPP and related bases but with diffuse functions",
	"SVP/JK Coulomb+Exchange fitting for SVP",
	"TZVPP/JK Coulomb+Exchange fitting for TZVPP",
	"QZVPP/JK Coulomb+Exchange fitting for QZVPP",
	"Def2-SVP/JK Coulomb+Exchange fitting for def2-SVP",
	"Def2-TZVPP/JK Coulomb+Exchange fitting for def2-TZVPP",
	"Def2-QZVPP/JK Coulomb+Exchange fitting for def2-QZVPP",
	"cc-pVDZ/JK Coulomb+Exchange fitting for cc-pVDZ",
	"cc-pVTZ/JK Coulomb+Exchange fitting for cc-pVTZ",
	"cc-pVQZ/JK Coulomb+Exchange fitting for cc-pVQZ",
	"cc-pV5Z/JK Coulomb+Exchange fitting for cc-pV5Z",
	"cc-pV6Z/JK Coulomb+Exchange fitting for cc-pV6Z",
};
static gchar* listAuxBasisReal[] = 
{ 
	"AutoAux",
	"DEMON/J",
	"DGAUSS/J",
	"SV/J",
	"TZV/J",
	"QZVPP/J",
	"Def2-SVP/J",
	"Def2-TZVPP/J",
	"Def2-QZVPP/J",
	"SV/J(-f)",
	"TZV/J(-f)",
	"SV/C (=VDZ/C)",
	"TZV/C (=VTZ/C)",
	"TZVPP/C (=VTZPP/C)",
	"QZVP/C",
	"QZVPP/C",
	"Def2-SVP/C",
	"Def2-TZVP/C",
	"Def2-TZVPP/C",
	"Def2-QZVPP/C",
	"cc-pVDZ/C",
	"cc-pVTZ/C",
	"cc-pVQZ/C",
	"cc-pV5Z/C",
	"cc-pV6Z/C",
	"Aug-cc-pVDZ/C",
	"Aug-cc-pVTZ/C",
	"Aug-SV/C",
	"Aug-TZV/C",
	"Aug-TZVPP/C",
	"SVP/JK",
	"TZVPP/JK",
	"QZVPP/JK",
	"Def2-SVP/JK",
	"Def2-TZVPP/JK",
	"Def2-QZVPP/JK",
	"cc-pVDZ/JK",
	"cc-pVTZ/JK",
	"cc-pVQZ/JK",
	"cc-pV5Z/JK",
	"cc-pV6Z/JK",
};
static guint numberOfAuxBasis = G_N_ELEMENTS (listAuxBasisView);
/*************************************************************************************************************/
static void setOrcaBasis(GtkWidget* comboBasis)
{
	gint i;

	if(strcmp(selectedTypeBasis, "Pople")==0)
	{
		listBasisView = listPopleBasisView;
		listBasisReal = listPopleBasisReal;
		numberOfBasis = numberOfPopleBasis;
	}
	else if(strcmp(selectedTypeBasis, "PopleDiffuseNonHydrogen")==0)
	{
		listBasisView = listPopleDiffuseNonHydrogenBasisView;
		listBasisReal = listPopleDiffuseNonHydrogenBasisReal;
		numberOfBasis = numberOfPopleDiffuseNonHydrogenBasis;
	}
	else if(strcmp(selectedTypeBasis, "PopleDiffuseAllAtoms")==0)
	{
		listBasisView = listPopleDiffuseAllAtomsBasisView;
		listBasisReal = listPopleDiffuseAllAtomsBasisReal;
		numberOfBasis = numberOfPopleDiffuseAllAtomsBasis;
	}
	else if(strcmp(selectedTypeBasis, "Dunning")==0)
	{
		listBasisView = listDunningBasisView;
		listBasisReal = listDunningBasisReal;
		numberOfBasis = numberOfDunningBasis;
	}
	else if(strcmp(selectedTypeBasis, "Ahlrichs")==0)
	{
		listBasisView = listAhlrichsBasisView;
		listBasisReal = listAhlrichsBasisReal;
		numberOfBasis = numberOfAhlrichsBasis;
	}
	else if(strcmp(selectedTypeBasis, "Def2Ahlrichs")==0)
	{
		listBasisView = listDef2AhlrichsBasisView;
		listBasisReal = listDef2AhlrichsBasisReal;
		numberOfBasis = numberOfDef2AhlrichsBasis;
	}
	else if(strcmp(selectedTypeBasis, "Jensen")==0)
	{
		listBasisView = listJensenBasisView;
		listBasisReal = listJensenBasisReal;
		numberOfBasis = numberOfJensenBasis;
	}
	else if(strcmp(selectedTypeBasis, "ANO")==0)
	{
		listBasisView = listANOBasisView;
		listBasisReal = listANOBasisReal;
		numberOfBasis = numberOfANOBasis;
	}
	else 
	{
		listBasisView = listMiscellenousBasisView;
		listBasisReal = listMiscellenousBasisReal;
		numberOfBasis = numberOfMiscellenousBasis;
	}
	
	if (comboBasis && GTK_IS_COMBO_BOX(comboBasis))
	{
		GtkTreeModel * model = NULL;
		model = gtk_combo_box_get_model(GTK_COMBO_BOX(comboBasis));
		gtk_list_store_clear(GTK_LIST_STORE(model));

		for(i=0;i<numberOfBasis;i++)
  			gtk_combo_box_append_text (GTK_COMBO_BOX (comboBasis), listBasisView[i]);
  		gtk_combo_box_set_active(GTK_COMBO_BOX (comboBasis), 0);
	}

}
/************************************************************************************************************/
static void traitementTypeBasis (GtkComboBox *combobox, gpointer d)
{
	GtkTreeIter iter;
	gchar* data = NULL;
	gchar* res = NULL;
	gint i;
	GtkWidget* comboBasis = NULL;
	/* gchar* s;*/
	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	else return;
	for(i=0;i<numberOfTypeBasis;i++)
	{
		if(strcmp((gchar*)data,listTypeBasisView[i])==0) res = listTypeBasisReal[i];
	}
	if(res) sprintf(selectedTypeBasis,"%s",res);
	comboBasis = g_object_get_data(G_OBJECT (combobox), "ComboBasis");

	if(comboBasis) setOrcaBasis(comboBasis);
}
/********************************************************************************************************/
static GtkWidget *create_list_type_basis(GtkWidget *comboBasis)
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfTypeBasis;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listTypeBasisView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	g_object_set_data(G_OBJECT (combobox), "ComboBasis",comboBasis);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementTypeBasis), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
void initOrcaBasis()
{
	orcaBasis.name = g_strdup("3-21G");
	orcaBasis.molecule = g_strdup("$:molecule");
	orcaBasis.numberOfBasisTypes=0;
	orcaBasis.basisNames = NULL;
}
/************************************************************************************************************/
void freeOrcaBasis()
{
	if(orcaBasis.name ) g_free(orcaBasis.name);
	orcaBasis.name = NULL;
	if(orcaBasis.molecule ) g_free(orcaBasis.molecule);
	orcaBasis.molecule = NULL;
	if(orcaBasis.basisNames)
	{
		gint i;
		for(i=0;i<orcaBasis.numberOfBasisTypes;i++)
			if(orcaBasis.basisNames[i])g_free(orcaBasis.basisNames[i]);

		if(orcaBasis.basisNames)g_free(orcaBasis.basisNames);
	}
	orcaBasis.numberOfBasisTypes=0;
	orcaBasis.basisNames = NULL;
}
/************************************************************************************************************/
static void changedEntryBasis(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	gchar* res = NULL;
	gint i;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	if(orcaBasis.name) g_free(orcaBasis.name);
	for(i=0;i<numberOfBasis;i++)
	{
		if(strcmp((gchar*)entryText,listBasisView[i])==0) res = listBasisReal[i];
	}
	if(res) orcaBasis.name=g_strdup(res);
}
/************************************************************************************************************/
static void changedEntryAuxBasis(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	gint i;
	gchar* res = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	if(orcaMolecule.auxBasisName) g_free(orcaMolecule.auxBasisName);
	for(i=0;i<numberOfAuxBasis;i++)
	{
		if(strcmp((gchar*)entryText,listAuxBasisView[i])==0) res = listAuxBasisReal[i];
	}
	if(res) orcaMolecule.auxBasisName=g_strdup(res);
}
/***********************************************************************************************/
static GtkWidget* addComboListToATable(GtkWidget* table,
		gchar** list, gint nlist, gint i, gint j, gint k)
{
	GtkWidget *entry = NULL;
	GtkWidget *combo = NULL;

	combo = create_combo_box_entry(list, nlist, TRUE, -1, -1);

	gtk_table_attach(GTK_TABLE(table),combo,j,j+k,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);
	entry = GTK_BIN (combo)->child;
	g_object_set_data(G_OBJECT (entry), "Combo",combo);

	return entry;
}
/***********************************************************************************************/
void addOrcaTypeBasisToTable(GtkWidget *table, gint i, GtkWidget* comboBasis)
{
	GtkWidget* combo = NULL;

	add_label_table(table,_("Type"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	combo  = create_list_type_basis(comboBasis);
	gtk_table_attach(GTK_TABLE(table),combo,2,2+1,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);
}
/***********************************************************************************************/
GtkWidget* addOrcaBasisToTable(GtkWidget *table, gint i)
{
	GtkWidget* entryBasis = NULL;
	GtkWidget* comboBasis = NULL;
	gint nlistBasis = 1;
	gchar* listBasis[] = {" "};


	add_label_table(table,_("Basis"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entryBasis = addComboListToATable(table, listBasis, nlistBasis, i, 2, 1);
	comboBasis  = g_object_get_data(G_OBJECT (entryBasis), "Combo");
	gtk_widget_set_sensitive(entryBasis, TRUE);
	gtk_editable_set_editable(GTK_EDITABLE(entryBasis),FALSE);

	g_signal_connect(G_OBJECT(entryBasis),"changed", G_CALLBACK(changedEntryBasis),NULL);
	return comboBasis;
}
/***********************************************************************************************/
void addOrcaAuxBasisToTable(GtkWidget *table, gint i, GtkWidget* comboMethod, GtkWidget* comboExcited)
{
	GtkWidget* entryAuxBasis = NULL;
	GtkWidget* comboAuxBasis = NULL;
	GtkWidget* label = NULL;

	label = add_label_table(table,_("Auxiliary basis"),(gushort)i,0);
	if(comboMethod) g_object_set_data(G_OBJECT (comboMethod), "LabelAuxBasis1", label);
	if(comboExcited) g_object_set_data(G_OBJECT (comboExcited), "LabelAuxBasis1", label);
	label = add_label_table(table,":",(gushort)i,1);
	if(comboMethod) g_object_set_data(G_OBJECT (comboMethod), "LabelAuxBasis2", label);
	if(comboExcited) g_object_set_data(G_OBJECT (comboExcited), "LabelAuxBasis2", label);
	entryAuxBasis = addComboListToATable(table, listAuxBasisView, numberOfAuxBasis, i, 2, 1);
	comboAuxBasis  = g_object_get_data(G_OBJECT (entryAuxBasis), "Combo");
	gtk_widget_set_sensitive(entryAuxBasis, TRUE);
	gtk_editable_set_editable(GTK_EDITABLE(entryAuxBasis),FALSE);
	if(comboMethod) g_object_set_data(G_OBJECT (comboMethod), "ComboAuxBasis", comboAuxBasis);
	if(comboExcited) g_object_set_data(G_OBJECT (comboExcited), "ComboAuxBasis", comboAuxBasis);

	g_signal_connect(G_OBJECT(entryAuxBasis),"changed", G_CALLBACK(changedEntryAuxBasis),NULL);
	orcaMolecule.auxBasisName=g_strdup(listAuxBasisReal[0]);
}
/***********************************************************************************************/
static void putOrcaBasisInTextEditor()
{
        gchar buffer[BSIZE];

	sprintf(buffer," %s ",orcaBasis.name);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

}
/***********************************************************************************************/
void putOrcaAuxBasisInTextEditor()
{
        gchar buffer[BSIZE];

	if(!orcaMolecule.auxBasisName) return;
	sprintf(buffer," %s",orcaMolecule.auxBasisName);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
}
/***********************************************************************************************/
void putOrcaBasisInfoInTextEditor()
{
	putOrcaBasisInTextEditor();
}
