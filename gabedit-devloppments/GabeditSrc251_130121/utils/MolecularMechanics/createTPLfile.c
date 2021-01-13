#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/**********************************************************************/
int getAmberTypes(char* gromacsType, char* amberType)
{
	char dump[1024];
	int len = 1024;
	int n;
	char t1[100];
	FILE* file;

	file = fopen("ffamber99nb.itp","r");
	if(file == NULL)
	{
		printf("file ffamber99nb.itp not found\n");
		return 0;
	}

	fgets(dump,len,file);
	n = 0;
	while(!feof(file))
	{
		if(fgets(dump,len,file))
		{
			if(strstr(dump,"[ atomtypes ]")) continue;
			if(dump[0]==';') continue;
			if(dump[0]=='[') continue;
		}
		sscanf(dump,"%s %s",t1,amberType);
		if(!strcmp(gromacsType,t1)) 
		{
			fclose(file);
			return 1;
		}
	}
	return 0;
}
/**********************************************************************/
void delete_fl(char* str)
{

	int j;
	for(j=0;j<strlen(str)-1;j++)
	{
		str[j] = str[j+1];
	}
	for(j=0;j<strlen(str);j++)
	{
		if(str[j]=='\"')
		{
			str[j] = '\0';
			break;
		}
	}
}
/**********************************************************************/
void printLines()
{
	int i;
	printf("/");
	for(i=0;i<60;i++)
		printf("*");
	printf("/\n");
}
/**********************************************************************/
void createIncludes()
{
	printf("#include <stdlib.h>\n");
	printf("#include <stdio.h>\n");
	printf("#include <string.h>\n");
	printf("#include <math.h>\n");
}
/**********************************************************************/
void printTitle()
{
	printf("void createTitleResidueTpl(FILE* fout)\n");
	printf("{\n");
	printf("\tfprintf(fout,\"Begin Title\\n\");\n");
	printf("\tfprintf(fout,\"\tResidue        : PDB type atom  Amber type atom  charge of atom\\n\");\n");
	printf("\tfprintf(fout,\"End\\n\");\n");
	printf("}\n");
}
/**********************************************************************/
char** printListResidue(FILE* file,int*nResidue, int ifNucleic)
{
	char** t = (char**)malloc(sizeof(char*));
	
	char dump[1024];
	int len = 1024;
	int i;
	int j;
	int n;

	fgets(dump,len,file);
	n = 0;
	while(!feof(file))
	{
		if(fgets(dump,len,file))
		{
			if(strstr(dump,"[ bondedtypes ]")) continue;
			if(!(dump[0]=='[' && dump[1]==' ')) continue;
		} else break;
		t = realloc(t,(n+1)*sizeof(char*));
		t[n] = (char*)malloc(100*sizeof(char*));
		sscanf(dump,"[ %s",t[n]);
		n++;
	}
	if(n==0) free(t);
	printf("void createListResidueTpl(FILE* fout)\n");
	printf("{\n");
	printf("\tfprintf(fout,\"%s\\n\");\n","Begin Residue List");

	for(i=0;i<n;i++)
	{
		printf("\tfprintf(fout,\"%s\\n\");\n",t[i]);
	}
	printf("\tfprintf(fout,\"End\\n\");\n");
	printf("}\n");
	*nResidue = n;

	return t;
			

}
/**********************************************************************/
void printTPLResidue(FILE* file,char* residueName)
{
	char dump[1024];
	char t1[1024];
	char t2[1024];
	char t3[1024];
	char t4[1024];
	char t5[1024];
	char t6[1024];
	char t7[1024];
	char t8[1024];
	int len = 1024;
	int i;
	int j;
	int n;
	char title[1024];
	int Ok = 0;

	sprintf(title,"%s",residueName);

	fseek(file, 0L, SEEK_SET);
	/* Search Begin INPUT FOR  ATOM TYPES */ 
	while(!feof(file))
	{
		if(fgets(dump,len,file))
		{
			if(strstr(dump,title) && dump[0]=='[' && dump[1]==' ')
			{
				if(fgets(dump,len,file)) Ok = 1;
				break;
			}
		}
	}
	if(!Ok)
		return;
	printf("\n");
	printf("\tfprintf(fout,\"Begin %s Residue\\n\");\n",residueName);
	n = 0;
	while(!feof(file))
	{
		if(fgets(dump,len,file))
		{
			if(strstr(dump,"[")) break;
		}
		sscanf(dump,"%s %s %s %s",t1,t2, t3, t4);
		if(getAmberTypes(t2, t4)!=0)
		printf("\tfprintf(fout,\"%-6s %-6s %-6s\\n\");\n",t1,t4,t3);
		else
		printf("\tfprintf(fout,\"%-6s %-6s %-6s\\n\");\n",t1,t2,t3);
		n++;
	}
	printf("\tfprintf(fout,\"End\\n\");\n");
	return ;



}
/**********************************************************************/
void printFileResidueTpl()
{
	printf("int main()\n");
	printf("{\n");
	printf("\tFILE* fout = fopen(\"TEST.TPL\",\"w\");\n");
	printf("\tif(!fout)\n");
	printf("\t\treturn 1;\n");
	printf("\tcreateTitleResidueTpl(fout);\n");
	printf("\tcreateListResidueTpl(fout);\n");
	printf("\tcreateResidueTpl(fout);\n");
	printf("\treturn 0;\n");
	printf("}\n");
}
/**********************************************************************/
int main()
{
	int nResidue;
	char** listResidue = NULL;
	FILE* file;
	int i;
	int ifNucleic = 0;
	
	/*
	ifNucleic = 0;
	file = fopen("amber_amino.tpl","r");
	 */
	ifNucleic = 1;
	file = fopen("ffamber99.rtp","r");
	if(file == NULL)
	{
		printf("file ffamber99.rtp not found\n");
		return 1;
	}

	createIncludes();
	printLines();
	printTitle();
	printLines();
	listResidue = printListResidue(file,&nResidue,ifNucleic);
	printLines();
	printf("void createResidueTpl(FILE* fout)\n");
	printf("{\n");
	for(i=0;i<nResidue;i++)
		printTPLResidue(file,listResidue[i]);
	printf("}\n");
	printLines();
	printFileResidueTpl();
 
	return 0;
	
}
