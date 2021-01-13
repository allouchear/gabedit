arithmetic.o: arithmetic.c arithmetic.h debug.h
cell.o: cell.c cell.h mathfunc.h debug.h
debug.o: debug.c
delaunay.o: delaunay.c delaunay.h mathfunc.h debug.h
determination.o: determination.c cell.h mathfunc.h determination.h \
 primitive.h symmetry.h refinement.h spacegroup.h debug.h
hall_symbol.o: hall_symbol.c hall_symbol.h spacegroup.h cell.h mathfunc.h \
 primitive.h symmetry.h spg_database.h
kgrid.o: kgrid.c kgrid.h
kpoint.o: kpoint.c mathfunc.h kpoint.h kgrid.h
mathfunc.o: mathfunc.c mathfunc.h debug.h
niggli.o: niggli.c niggli.h
overlap.o: overlap.c overlap.h mathfunc.h cell.h debug.h
pointgroup.o: pointgroup.c pointgroup.h mathfunc.h symmetry.h cell.h \
 debug.h
primitive.o: primitive.c cell.h mathfunc.h delaunay.h primitive.h \
 symmetry.h debug.h
refinement.o: refinement.c refinement.h cell.h mathfunc.h symmetry.h \
 spacegroup.h primitive.h pointgroup.h spg_database.h site_symmetry.h \
 debug.h
sitesym_database.o: sitesym_database.c sitesym_database.h
site_symmetry.o: site_symmetry.c cell.h mathfunc.h symmetry.h \
 site_symmetry.h sitesym_database.h debug.h
spacegroup.o: spacegroup.c cell.h mathfunc.h delaunay.h hall_symbol.h \
 spacegroup.h primitive.h symmetry.h niggli.h pointgroup.h spg_database.h \
 debug.h
spg_database.o: spg_database.c spg_database.h spacegroup.h cell.h \
 mathfunc.h primitive.h symmetry.h
spglib.o: spglib.c arithmetic.h cell.h mathfunc.h debug.h delaunay.h \
 determination.h primitive.h symmetry.h refinement.h spacegroup.h kgrid.h \
 kpoint.h niggli.h pointgroup.h spglib.h spg_database.h spin.h version.h
spglib_f.o: spglib_f.c spglib.h
spin.o: spin.c mathfunc.h symmetry.h cell.h spin.h debug.h
symmetry.o: symmetry.c cell.h mathfunc.h delaunay.h symmetry.h overlap.h \
 debug.h
tetrahedron_method.o: tetrahedron_method.c tetrahedron_method.h kgrid.h
