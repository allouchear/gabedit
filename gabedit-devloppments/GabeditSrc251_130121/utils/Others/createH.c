#include<stdlib.h>
#include<stdio.h>
#include<string.h>

/**********************************************/
int main(int argc,char* argv[])
{
	FILE* p1;
	FILE* p2;
	char* filename = NULL;
	char* p1name = "p1";
	char* p2name = "p2";
	int i;
	if(argc<2)
	{
	 	printf("vous devez fournir un nom de fichier\n");
		return 1;
	}
	else filename = strdup(argv[1]);
	for(i=0;i<strlen(filename);i++)
		filename[i] = toupper(filename[i]);
	for(i=0;i<strlen(filename);i++)
		if(filename[i]=='.')filename[i]='_';

	printf("Header name %s\n",filename);

	p1 = fopen(p1name,"w");
	p2 = fopen(p2name,"w");
	if(!p1 || !p2)
	{
		printf("I can not open %s or %s\n",p1name, p2name);
		return 1;
	}
	fprintf(p1,"#ifndef __GABEDIT_%s__\n", filename);
	fprintf(p1,"#define __GABEDIT_%s__\n", filename);
	fprintf(p2,"#endif /* __GABEDIT_%s__ */\n", filename);
	fclose(p1);
	fclose(p2);
	return 0;
}
