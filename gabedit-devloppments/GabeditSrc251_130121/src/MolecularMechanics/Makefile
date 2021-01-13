OBJECTS = Atom.o Molecule.o ForceField.o MolecularMechanics.o ConjugateGradient.o SteepestDescent.o QuasiNewton.o MolecularMechanicsDlg.o CreateMolecularMechanicsFile.o CreatePersonalMMFile.o LoadMMParameters.o SetMMParameters.o CreateDefaultPDBTpl.o LoadPDBTemplate.o PDBTemplate.o SetPDBTemplate.o SavePDBTemplate.o CalculTypesAmber.o MolecularDynamics.o 

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

