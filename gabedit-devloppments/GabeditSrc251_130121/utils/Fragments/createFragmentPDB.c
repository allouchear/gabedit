/* Program for read a pdb file and create c function for create a Fragment */
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
static int read_atom_pdb_file(char* line,char* listFields[])
{
	int i;
	int k = 0;
	if(strlen(line)<54)
		return FALSE;

	/* 0 -> Atom Type */
	k = 0;
	for(i=0;i<MAXATOMTYPE;i++)
		listFields[k][i] = line[13+i-1];
	listFields[k][MAXATOMTYPE] = '\0';
	if(isdigit(listFields[k][0]))
	{
		char c0 = listFields[k][0];
		for(i=0;i<MAXATOMTYPE-1;i++)
			listFields[k][i] = listFields[k][i+1];
		listFields[k][MAXATOMTYPE-1] = c0;
	}
	/* 1-> Residue Name */
	k = 1;
	for(i=0;i<MAXRESIDUENAME;i++)
		listFields[k][i] = line[17+i-1];
	listFields[k][MAXRESIDUENAME] = '\0';

	/* 2-> Residue Number */
	k = 2;
	for(i=0;i<4;i++)
		listFields[k][i] = line[23+i-1];
	listFields[k][4] = '\0';
	/* 3-> x */
	k = 3;
	for(i=0;i<8;i++)
		listFields[k][i] = line[31+i-1];
	listFields[k][8] = '\0';

	/* 4-> y */
	k = 4;
	for(i=0;i<8;i++)
		listFields[k][i] = line[39+i-1];
	listFields[k][8] = '\0';

	/* 5-> z */
	k = 5;
	for(i=0;i<8;i++)
		listFields[k][i] = line[47+i-1];
	listFields[k][8] = '\0';

	/* 6-> Symbol */
	k = 6;
	if(strlen(line)>=78)
	{
		for(i=0;i<2;i++)
		{
			listFields[k][i] = line[76+i];
		}
		listFields[k][2] = '\0';
		if(listFields[k][1]==' ')
			listFields[k][1] = '\0';
		if(listFields[k][0]==' ')
			listFields[k][0] = '\0';
	}
	else
		listFields[k][0] = '\0';
	/* 7-> Charge */
	k = 7;
	if(strlen(line)>=80)
	{
		for(i=0;i<(int)strlen(line)-79+1;i++)
			listFields[k][i] = line[79+i-1];

		listFields[k][strlen(line)-79+1] = '\0';

		if(listFields[k][strlen(line)-79]=='\n')
			listFields[k][strlen(line)-79]='\0';

	}
	else
		listFields[k][0] = '\0';

	for(i=0;i<8;i++)
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
 char pdb[10];
 char mm[10];
 char t[BSIZE];
 char* filename = NULL;
 int Natoms = 0;
 float C[3];
 float charge;
 int i;
 char name[100];
	char *listFields[8];
 

 if(argc<2) filename = strdup("p.pdb");
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
	 if(strstr(t,"ATOM"))Natoms++;

 }
 fclose(fin);
 if(Natoms<=0)
	{
		printf("Error : Natoms <=0\n");
		return 1;
	}
 printf("Natoms = %d\n",Natoms);
 fin = fopen(filename,"r");
 if(!fin)
 {
	printf("I can not open %s\n",filename);
	return 1;
 }
 sprintf(name,"ToChange");
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
	if(!read_atom_pdb_file(t,listFields)) continue;
		/* 0 -> Atom Type  1-> Residue Name  2-> Residue Number 
		 * 3-> x  4-> y  5-> z  6-> Symbol 7-> Charge */
	printf("t=%s\n",t);
	i++;
	sprintf(pdb,"%s",listFields[0]);
	C[0] = atof(listFields[3]);
	C[1] = atof(listFields[4]);
	C[2] = atof(listFields[5]);
	charge = atof(listFields[7]);
 	fprintf(fout,"\t\tSetAtom(&F.Atoms[ %d ] , \"%s\",%0.6ff,%0.6ff,%0.6ff,%0.6ff);\n",i,pdb,C[0],C[1],C[2],charge);
 }
 fprintf(fout,"\t\tF.atomToDelete =%d;\n",1);
 fprintf(fout,"\t\tF.atomToBondTo =%d;\n",2);
 fprintf(fout,"\t\tF.angleAtom    =%d;\n",3);
 fprintf(fout,"\t}\n");

 fclose(fin);
 fclose(fout);
 

 return 0;
	
}
