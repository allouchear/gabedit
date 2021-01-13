#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define BSIZE 1024

static char* names[] = {
	"Dummy",
	"hydrogen", "helium", "lithium", "beryllium", "boron",
	"carbon", "nitrogen", "oxygen", "fluorine", "neon",				// 10
	"sodium", "magnesium", "aluminium", "silicon", "phosphorus",
	"sulphur", "chlorine", "argon", "potassium", "calcium",				// 20
	"scandium", "titanium", "vanadium", "chromium", "manganese",
	"iron", "cobalt", "nickel", "copper", "zinc",					// 30
	"gallium", "germanium", "arsenic", "selenium", "bromine",
	"krypton", "rubidium", "strontium", "yttrium", "zirconium",			// 40
	"niobium", "molybdenum", "technetium", "ruthenium", "rhodium",
	"palladium", "silver", "cadmium", "indium", "tin",				// 50
	"antimony", "tellurium", "iodine", "xenon", "cesium",
	"barium", "lanthanum", "cerium", "praseodymium", "neodymium",			// 60
	"promethium", "samarium", "europium", "gadolinium", "terbium",
	"dysprosium", "holmium", "erbium", "thulium", "ytterbium",			// 70
	"lutetium", "hafnium", "tantalum", "tungsten", "rhenium",
	"osmium", "iridium", "platinum", "gold", "mercury",				// 80
	"thallium", "lead", "bismuth", "polonium", "astatine",
	"radon", "francium", " radium", "actinium", "thorium",				// 90
	"protactinium", "uranium", "neptunium", "plutionium", "americium",
	"curium", "berkelium", "californium", "einsteinium", "fermium",			// 100
	"mendelevium", "nobelium", "lawrencium", "rutherfordium", "dubnium",
	"seaborgium", "bohrium", "hassium", "meitnerium", "ununnilium"			// 110
};
int size = sizeof(names)/sizeof(char*);

/**********************************************/
void str_delete_n(char* str)
{
	char *s;

	if(str == NULL)
		return;

	if (!*str)
		return;
	for (s = str + strlen (str) - 1; s >= str && ((unsigned char)*s)=='\n'; s--)
		*s = '\0';
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
	unsigned char *start;
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
/**********************************************/
void add_setProp(char* str)
{
	unsigned char *start;
	int i;
	int l;
	int k = 0;
	char dump[BSIZE];

	if(str == NULL) return;
	if (!*str) return;

	for(i=0, l=0;i<(int)(strlen(str));i++)
	{
		if(str[i]==' ')
		{
			if(k==0)
			{
			       	dump[l]='"';
				l++;
			       	dump[l]=',';
				l++;
			}
			else
			{
			       	dump[l]=',';
				l++;
			}
			k++;
		}
	       	dump[l]=str[i];
		l++;
	}
	dump[l]='\0';
	str[l]='\0';
	for(l=0;l<(int)(strlen(dump));l++) str[l] = dump[l];

}
/**********************************************/
int main(int argc,char* argv[])
{
	FILE* fin;
	FILE* fout;
	char t[BSIZE];
	char* filename = NULL;
	int Z;
	char symb[100];
	float rcov;
	float rbo;
	float rvdw;
	int maxvalance;
	float masse;
	float elNeg;
	float red;
	float green;
	float blue;
	int n = 0;
	int i = 0;


	if(argc<2) filename = strdup("element.txt");
	else filename = strdup(argv[1]);


	printf("Input file = %s\n",filename);
	printf("Output file = %s\n","p.c");

	fin = fopen(filename,"r");
	if(!fin)
	{
		printf("I can not open %s\n",filename);
		return 1;
	}
	fout = fopen("p.c","w");
	if(!fout)
	{
		printf("I can not open p.c\n");
		return 1;
	}
	while(!feof(fin))
	{
		if(!fgets(t,BSIZE,fin)) break;
		if(strstr(t,"# Num")) break;
	}
	n = 0;
	while(!feof(fin))
	{
		if(!fgets(t,BSIZE,fin)) break;
		delete_first_spaces(t);
		delete_last_spaces(t);
		str_delete_n(t);
		i = sscanf(t,"%d %s %f %f %f %d %f %f %f %f %f", &Z, symb, &rcov, &rbo, &rvdw, &maxvalance, &masse, &elNeg, &red, &green, &blue);
		if(i!=11)
		{
			printf("Erreur de lecture \n");
			break;
		}
		if(n<=size-1) 
		{
			char x = names[n][0];
			char* name = strdup(names[n]);
			name[0] = toupper(x);
			printf("%c\n",toupper(x));
			fprintf(fout,"setPropForOneAtom(\"%s\", \"%s\", %d, %f, %f, %f, %d, %f, %f, %f, %f, %f);\n", name, symb, Z, rcov, rbo, rvdw, maxvalance, masse, elNeg, red, green, blue);
		}
		n++;
	}
	printf("n = %d\n", n);
	printf("zise = %d\n", size);
	fclose(fin);
	fclose(fout);
	return 0;
}
