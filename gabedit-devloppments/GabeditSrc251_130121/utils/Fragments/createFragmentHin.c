/* Program for read a hin file and create c function for create a Fragment */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define NATOMS 120
#define BSIZE 1024

#define MAXNAME 6
#define MAXATOMTYPE 4
#define MAXRESIDUENAME 4
#define MAXSYMBOL 2
#define FALSE 0
#define TRUE 1
/**********************************************/
char* strup (char *str)
{
  register unsigned char *s;

  if( !str) return NULL;

  s = (unsigned char *) str;

  while (*s)
    {
      if (islower (*s))
	*s = toupper (*s);
      s++;
    }

  return (char *) str;
}
/**********************************************/
void delete_last_spaces(char* str)
{
	char *s;

	if(str == NULL)
		return;

	if (!*str)
		return;
	for (s = str + strlen (str) - 1; s >= str && isspace ((unsigned char)*s); s--)
		*s = '\0';
}
/**********************************************/
void delete_first_spaces(char* str)
{
	char *start;
	int i;
	int lenSpace = 0;

	if(str == NULL)
		return;
	if (!*str)
		return;

	for (start = str; *start && isspace (*start); start++)lenSpace++;

	for(i=0;i<(int)(strlen(str)-lenSpace);i++)
		str[i] = str[i+lenSpace];
	str[strlen(str)-lenSpace] = '\0';
}
/*************************************************************************************/
static int read_atom_hin_file(char* t,char* listFields[])
{
	int taille = BSIZE;
	char dump[BSIZE];
	int i;

    	sscanf(t,"%s",dump);
	strup(dump);
	if(strcmp(dump,"ATOM")!=0)
	{
		if(strcmp(dump,"RES")==0)
		{
    			sscanf(t,"%s %s %s",dump,dump,listFields[1]);
			sprintf(listFields[0],"Unknown");
		}
		else
			return FALSE;
	}
	else
	{
		/* 0 -> Atom Type PDB Style*/
		/* 1 -> Atom Symbol*/
		/* 2 -> Atom Type Amber*/
		/* 3 -> Atom Charge*/
		/* 4 -> x*/
		/* 5 -> y*/
		/* 6 -> z*/
    		sscanf(t,"%s %s %s %s %s %s %s %s %s %s",dump,dump,listFields[0],listFields[1],listFields[2],dump,listFields[3],listFields[4],listFields[5],listFields[6]);
	}
	for(i=0;i<6;i++)
	{
		delete_last_spaces(listFields[i]);
		delete_first_spaces(listFields[i]);
	}
	return TRUE;

}
int main(int argc,char* argv[])
{
 FILE* fin;
 FILE* fout;
 char symb[10];
 char** pdb;
 char** mm;
 float* charge;
 char t[BSIZE];
 char dum1[100];
 char dum2[100];
 char dum3[100];
 char* filename = NULL;
 int Natoms = 0;
 float C[3];
 int i;
 char name[100];
 char *listFields[8];
 
 sprintf(name,"ToChange");

 if(argc<2) filename = strdup("p.hin");
 else filename = strdup(argv[1]);

 printf("FileName = %s\n",filename);

 fin = fopen(filename,"r");
 if(!fin)
 {
	printf("I can not open %s\n",filename);
	return 1;
 }
 Natoms = 0;
 while(fgets(t,BSIZE,fin))
 {
	 strup(t);
	 if(strstr(t,"ATOM"))Natoms++;
	 if(strstr(t,"RES") && strstr(name,"ToChange")) 
	 {
		 printf("t = %s\n",t);
		 if(3==sscanf(t,"%s %s %s",dum1,dum3,dum2))
			 sprintf(name,dum2);
	 }

 }
 fclose(fin);
 if(Natoms<=0)
	{
		printf("Error : Natoms <=0\n");
		return 1;
	}
 pdb = (char**)malloc(Natoms*sizeof(char*));
 mm = (char**)malloc(Natoms*sizeof(char*));
 charge = (float*)malloc(Natoms*sizeof(float));
 for(i=0;i<Natoms;i++)
 {
	 pdb[i] = (char*)malloc(100*sizeof(char));
	 mm[i] = (char*)malloc(100*sizeof(char));
 }
 printf("Natoms = %d\n",Natoms);
 fin = fopen(filename,"r");
 if(!fin)
 {
	printf("I can not open %s\n",filename);
	return 1;
 }
 printf("End sprintfName\n");
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
 printf("End fout\n");

 for(i=0;i<8;i++) listFields[i] = (char*)malloc(100*sizeof(char));
 i = -1;
 while(fgets(t,BSIZE,fin))
 {
	if(!strstr(t,"ATOM")) continue;
	if(!read_atom_hin_file(t,listFields))continue;
		/* 0 -> Atom Type PDB Style*/
		/* 1 -> Atom Symbol*/
		/* 2 -> Atom Type Amber*/
		/* 3 -> Atom Charge*/
		/* 4 -> x*/
		/* 5 -> y*/
		/* 6 -> z*/
	printf("t=%s\n",t);
	i++;
	sprintf(pdb[i],"%s",listFields[0]);
	sprintf(mm[i],"%s",listFields[2]);
	C[0] = atof(listFields[4]);
	C[1] = atof(listFields[5]);
	C[2] = atof(listFields[6]);
	charge[i] = atof(listFields[3]);
 	fprintf(fout,"\t\tSetAtom(&F.Atoms[ %d ] , \"%s\",%0.6ff,%0.6ff,%0.6ff,%0.6ff);\n",i,pdb[i],C[0],C[1],C[2],charge[i]);
 }
 /*
 fprintf(fout,"\t\tF.atomToDelete =%d;\n",1);
 fprintf(fout,"\t\tF.atomToBondTo =%d;\n",2);
 fprintf(fout,"\t\tF.angleAtom    =%d;\n",3);
 */
 fprintf(fout,"\t}\n");
 fprintf(fout,"\n\tfprintf(fout,\"%s\\n\")\n\n",name);
 fprintf(fout,"\tfprintf(fout,\"Begin %s Residue\\n\")\n",name);
 for(i=0;i<Natoms;i++)
 	fprintf(fout,"\tfprintf(fout,\"%s \t%s\t%0.6f\\n\")\n",pdb[i],mm[i],charge[i]);
 fprintf(fout,"\tfprintf(fout,\"End\\n\")\n");

 fclose(fin);
 fclose(fout);
 

 return 0;
	
}
