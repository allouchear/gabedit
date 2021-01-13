OBJECTS = Molcas.o MolcasGateWay.o MolcasSeward.o MolcasScf.o MolcasGridIT.o MolcasOptimization.o MolcasVariables.o MolcasLib.o MolcasBasisLibrary.o MolcasBasis.o MolcasGlobal.o

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

