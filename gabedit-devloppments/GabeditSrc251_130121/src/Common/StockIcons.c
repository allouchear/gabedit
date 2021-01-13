/* StockIcons.c */
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

#include "../../pixmaps/A0d.xpm"
#include "../../pixmaps/A0p.xpm"
#include "../../pixmaps/About.xpm"
#include "../../pixmaps/Apply.xpm"
#include "../../pixmaps/Book_close.xpm"
#include "../../pixmaps/Book_open.xpm"
#include "../../pixmaps/Book_parent.xpm"
#include "../../pixmaps/Camera.xpm"
#include "../../pixmaps/Cancel.xpm"
#include "../../pixmaps/Close.xpm"
#include "../../pixmaps/ConvertUnit.xpm"
#include "../../pixmaps/Copy.xpm"
#include "../../pixmaps/Cut.xpm"
#include "../../pixmaps/DeleteAtom.xpm"
#include "../../pixmaps/Draw.xpm"
#include "../../pixmaps/Driver.xpm"
#include "../../pixmaps/Error.xpm"
#include "../../pixmaps/Exit.xpm"
#include "../../pixmaps/Find.xpm"
#include "../../pixmaps/Gaussian.xpm"
#include "../../pixmaps/Geometry.xpm"
#include "../../pixmaps/Help.xpm"
#include "../../pixmaps/Hide.xpm"
#include "../../pixmaps/AdjustH.xpm"
#include "../../pixmaps/Home.xpm"
#include "../../pixmaps/H.xpm"
#include "../../pixmaps/Gabedit.xpm"
#include "../../pixmaps/Info.xpm"
#include "../../pixmaps/InsertAtom.xpm"
#include "../../pixmaps/InsertFragment.xpm"
#include "../../pixmaps/ChangeBond.xpm"
#include "../../pixmaps/CutBond.xpm"
#include "../../pixmaps/DeleteCut.xpm"
#include "../../pixmaps/AddBond.xpm"
#include "../../pixmaps/Insert.xpm"
#include "../../pixmaps/Measure.xpm"
#include "../../pixmaps/Molcas.xpm"
#include "../../pixmaps/Molden.xpm"
#include "../../pixmaps/Molpro.xpm"
#include "../../pixmaps/Orca.xpm"
#include "../../pixmaps/Vasp.xpm"
#include "../../pixmaps/QChem.xpm"
#include "../../pixmaps/Mopac.xpm"
#include "../../pixmaps/Adf.xpm"
#include "../../pixmaps/Dalton.xpm"
#include "../../pixmaps/DeMon.xpm"
#include "../../pixmaps/Gamess.xpm"
#include "../../pixmaps/FireFly.xpm"
#include "../../pixmaps/MoveAtom.xpm"
#include "../../pixmaps/MPQC.xpm"
#include "../../pixmaps/NWChem.xpm"
#include "../../pixmaps/Psicode.xpm"
#include "../../pixmaps/NBO.xpm"
#include "../../pixmaps/New.xpm"
#include "../../pixmaps/Next.xpm"
#include "../../pixmaps/No.xpm"
#include "../../pixmaps/Ok.xpm"
#include "../../pixmaps/OpenBabel.xpm"
#include "../../pixmaps/Open.xpm"
#include "../../pixmaps/Orbitals.xpm"
#include "../../pixmaps/Ortho.xpm"
#include "../../pixmaps/Page.xpm"
#include "../../pixmaps/Paste.xpm"
#include "../../pixmaps/Perspective.xpm"
#include "../../pixmaps/Play.xpm"
#include "../../pixmaps/Print.xpm"
#include "../../pixmaps/Question.xpm"
#include "../../pixmaps/RenderBallStick.xpm"
#include "../../pixmaps/RenderStick.xpm"
#include "../../pixmaps/RenderWireFrame.xpm"
#include "../../pixmaps/RotateLocal.xpm"
#include "../../pixmaps/Rotate.xpm"
#include "../../pixmaps/RotateX.xpm"
#include "../../pixmaps/RotateY.xpm"
#include "../../pixmaps/RotateZLocal.xpm"
#include "../../pixmaps/RotateZ.xpm"
#include "../../pixmaps/Run.xpm"
#include "../../pixmaps/SaveAs.xpm"
#include "../../pixmaps/SelectAll.xpm"
#include "../../pixmaps/SaveMini.xpm"
#include "../../pixmaps/Save.xpm"
#include "../../pixmaps/ScaleBall.xpm"
#include "../../pixmaps/ScaleDipole.xpm"
#include "../../pixmaps/ScaleStick.xpm"
#include "../../pixmaps/SelectResidue.xpm"
#include "../../pixmaps/Select.xpm"
#include "../../pixmaps/Stop.xpm"
#include "../../pixmaps/Translate.xpm"
#include "../../pixmaps/Warning.xpm"
#include "../../pixmaps/Yes.xpm"
#include "../../pixmaps/Zoom.xpm"
#include "../../pixmaps/Pdb.xpm"
#include "../../pixmaps/O.xpm"
#include "../../pixmaps/AtomToInsert.xpm"
#include "../../pixmaps/Knot.xpm"
#include "StockIcons.h"
#include <gtk/gtk.h>

static void add_one_icon(GtkIconFactory *icon_factory, gchar* name, char** xpm)
{
	GdkPixbuf *pixbuf;
	GtkIconSet *iconset;
	pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)xpm);
       	iconset = gtk_icon_set_new_from_pixbuf(pixbuf);
       	g_object_unref(G_OBJECT(pixbuf));
	gtk_icon_factory_add(icon_factory, name, iconset);
	gtk_icon_set_unref(iconset);
}

void gabedit_gtk_stock_init()
{
	GtkIconFactory *icon_factory;
	static gboolean stock_inited = FALSE;

	if (stock_inited) return;
	stock_inited = TRUE;

	/* Setup the icon factory. */
	icon_factory = gtk_icon_factory_new();
	gtk_icon_factory_add_default(icon_factory);

	add_one_icon(icon_factory, GABEDIT_STOCK_A0D, a0d_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_A0P, a0p_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_ABOUT, about_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_APPLY, apply_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_BOOK_CLOSE, book_close_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_BOOK_OPEN, book_open_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_BOOK_PARENT, book_parent_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_CAMERA, camera_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_CANCEL, cancel_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_CLOSE, close_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_COPY, copy_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_CUT, cut_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_DELETE_ATOM, delete_atom_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_DRAW, draw_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_IFRAG, insert_fragment_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_DRIVER, driver_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_ERROR , error_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_EXIT , exit_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_FIND , find_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_GAUSSIAN, gaussian_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_GEOMETRY, geometry_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_HELP, help_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_HIDE, hide_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_ADJUST_H, adjust_h_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_HOME, home_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_H, H);
	add_one_icon(icon_factory, GABEDIT_STOCK_GABEDIT, gabedit_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_INFO, info_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_INSERT_ATOM, insert_atom_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_CHANGE_BOND, change_bond_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_CUT_BOND, cut_bond_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_DELETE_CUT, delete_cut_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_ADD_BOND, add_bond_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_INSERT, insert_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_MEASURE, measure_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_MOLCAS, molcas_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_MOLDEN, molden_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_MOLPRO, molpro_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_ORCA, orca_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_VASP, vasp_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_QCHEM, qchem_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_MOPAC, mopac_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_ADF, adf_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_DALTON, dalton_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_DEMON, demon_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_GAMESS, gamess_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_NWCHEM, nwchem_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_PSICODE, psicode_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_NBO, nbo_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_FIREFLY, firefly_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_MOVE_ATOM, move_atom_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_MPQC, mpqc_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_NEW, new_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_NEXTW, next_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_NO, no_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_OK, ok_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_OPEN_BABEL,open_babel_xpm );
	add_one_icon(icon_factory, GABEDIT_STOCK_CONVERT_UNIT,convert_unit_xpm );
	add_one_icon(icon_factory, GABEDIT_STOCK_OPEN, open_xpm );
	add_one_icon(icon_factory, GABEDIT_STOCK_ORBITALS, orbitals_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_ORTHO, ortho_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_PAGE, page_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_PASTE, paste_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_PERSPECTIVE, perspective_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_PLAY, play_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_PRINT, print_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_QUESTION, question_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_RENDER_BALL_STICK, render_ballstick_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_RENDER_STICK, render_stick_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_RENDER_WIREFRAME, render_wireframe_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_ROTATION_LOCAL, rotate_local_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_ROTATION, rotate_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_ROTATION_X, rotatex_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_ROTATION_Y, rotatey_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_ROTATION_Z, rotatez_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_ROTATION_Z_LOCAL, rotatez_local_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_RUN, run_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_SAVE_AS, saveas_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_SELECT_ALL, selectall_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_SAVE_MINI, savemini_xpm );
	add_one_icon(icon_factory, GABEDIT_STOCK_SAVE, save_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_SCALE_BALL, scale_ball_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_SCALE_DIPOLE, scale_dipole_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_SCALE_STICK, scale_stick_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_SELECT_RESIDUE, selectresidue_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_SELECT, select_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_STOP, stop_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_TRANSLATE, translate_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_WARNING, warning_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_YES, yes_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_ZOOM, zoom_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_PDB, pdb_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_O, o_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_ATOMTOINSERT, atomtoinsert_xpm);
	add_one_icon(icon_factory, GABEDIT_STOCK_KNOT, knot_xpm);


	g_object_unref(G_OBJECT(icon_factory));
}
