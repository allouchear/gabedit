/* Program for read a xyz file and create c function for create a Fragment */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NATOMS 120
#define BSIZE 1024

int main(int argc,char* argv[])
{
 FILE* fin;
 FILE* fout;
 char* symb = malloc(2*sizeof(char));
 char t[BSIZE];
 char* filename = NULL;
 int Natoms = 0;
 float C[3];
 int i;
 char* name  = malloc(100*sizeof(char));
 

 if(argc<2)
	 filename = strdup("p.xyz");
 else
	 filename = strdup(argv[1]);

 printf("FileName = %s\n",filename);

 fin = fopen(filename,"r");
 if(!fin)
 {
	printf("I can not open %s\n",filename);
	return 1;
 }
 if(fgets(t,BSIZE,fin))
 {
	if(sscanf(t,"%d",&Natoms) != 1)
	{
		printf("Error : readding of data file\n");
		return 1;
	}
	if(Natoms<=0)
	{
		printf("Error : Natoms <=0\n");
		return 1;
	}
 }
 fgets(t,BSIZE,fin);
 /* sscanf(t,"%s",name);*/
 sscanf(filename,"%s",name);
 fout = fopen("Fragment.cc","a");
 if(!fout)
 {
	printf("I can not open Fragment.cc\n");
	return 1;
 }
 fprintf(fout,"\telse if ( !strcmp(Name, \"%s\" ) )\n",name);
 fprintf(fout,"\t{\n");
 fprintf(fout,"\t\tF.NAtoms = %d;\n",Natoms);
 fprintf(fout,"\t\tF.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));\n");

 i = -1;
 while(!feof(fin))
 {
	if(!fgets(t,BSIZE,fin))
		break;
	if(sscanf(t,"%s %f %f %f",symb,&C[0],&C[1],&C[2]) != 4)
	{
		printf("Error : readding of data file\n");
		printf("Line : %s\n",t);
		break;
	}
	i++;
 	fprintf(fout,"\t\tSetAtom(&F.Atoms[ %d ] , \"%s\",%0.6ff,%0.6ff,%0.6ff);\n",i,symb,C[0],C[1],C[2]);
 }
 fprintf(fout,"\t\tF.atomToDelete =%d;\n",1);
 fprintf(fout,"\t\tF.atomToBondTo =%d;\n",2);
 fprintf(fout,"\t\tF.angleAtom    =%d;\n",3);
 fprintf(fout,"\t}\n");

 fclose(fin);
 fclose(fout);
 

 return 0;
	
}
