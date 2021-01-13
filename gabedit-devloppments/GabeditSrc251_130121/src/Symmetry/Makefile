OBJECTS = MoleculeSymmetry.o MoleculeSymmetryInterface.o PolyHedralOperators.o ReducePolyHedralMolecule.o PrincipalAxis.o ReduceMolecule.o SymmetryOperators.o GenerateMolecule.o Element.o SAtom.o SMolecule.o Elements.o SOperation.o SOperations.o HashMapSAtoms.o PointGroupGabedit.o PointGroups.o SymmetryGabedit.o

include ../../CONFIG
CFLAGS =  $(COMMONCFLAGS) $(GTKCFLAGS)

all: compiling 

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

compiling: $(OBJECTS)

clean:
	@$(RM) *.chk p *o gabedit *.com *.out *.log *.test test* *.cpp  *old *.ps .*swp *~ 

#make dependance
SRCS = $(OBJECTS:%.o=%.c)
dep :
	gcc -MM $(SRCS) > Dep.mk

-include Dep.mk

