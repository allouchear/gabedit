include ../CONFIG

OBJECTS = arithmetic.o cell.o debug.o delaunay.o determination.o hall_symbol.o kgrid.o kpoint.o mathfunc.o niggli.o overlap.o pointgroup.o primitive.o refinement.o sitesym_database.o site_symmetry.o spacegroup.o spg_database.o spglib.o spglib_f.o spin.o symmetry.o tetrahedron_method.o

all: compiling

CFLAGS =  $(COMMONCFLAGS) $(OGLCFLAGS) 

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

compiling: $(OBJECTS)

clean:
	@$(RM) *.o

#make dependance
SRCS = $(OBJECTS:%.o=%.c)
dep :
	gcc -MM $(SRCS) > Dep.mk

-include Dep.mk

