/*
In the program, I'll use the following abreviations :

 * Cl:	catalog object	(#0)
 * Pr:	page tree root	(#1)
 * Pg:	page object	(#PG_BASE ~ #PG_BASE + NPAGE - 1)
 * Pc:	page content	(#PC_BASE ~ #PC_BASE + NPAGE - 1)
 * Cs:	content stream
 * Cl:	stream length	(#SL_BASE ~ #SL_BASE + NPAGE - 1)
 
 Also, the explanations for this project can be found here : 
 https://www.oreilly.com/library/view/pdf-explained/9781449321581/ch04.html
 
 and also here : 
 https://pypdf2.readthedocs.io/en/3.0.0/dev/pdf-format.html
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define NPAGE 32768 //maximum number of pages
#define PG_BASE 32768
#define PC_BASE (PG_BASE + NPAGE)
#define SL_BASE (PC_BASE + NPAGE)
#define NCOL 80 //at most 80 colums and 66 rows, standard printer dimensions
#define NROW 66

int nByte;
int nPage;
int clPos;
int prPos;
int pgPos[NPAGE];
int pcPos[NPAGE];
int slPos[NPAGE];
int xrefPos;


int escape(char *s,FILE* out){
	int n = 0;
	while (*s) {
		switch (*s) {
            case '(':
            case ')':
            case '\\':{
                fprintf(out,"%d",'\\'); //since (,) and \\ are special char, we need to escape them
                n++;
            }
		    default:{
                fprintf(out,"%s",s);
                n++;
            }
		}
		s++;
	}
	return n;
}

int genCs(FILE *in,FILE* out){
	char line[NCOL + 1];
	int n, nl, eol, ch;
	n = 0;
	n += fprintf(out,"BT\n");
	n += fprintf(out,"11 TL\n");
	n += fprintf(out,"34 784 Td\n");
	n += fprintf(out,"/F0 11 Tf\n");
	nl = 0;
	while (fgets(line, sizeof(line), in)) {
		eol = strcspn(line, "\n");
		if (!line[eol] && (ch = fgetc(in)) != '\n')
			ungetc(ch, in);
		line[eol] = '\0'; //because null terminated strings
		n += fprintf(out,"T* (");
		n += escape(line,out);
		n += fprintf(out,") Tj\n");
		if (++nl == NROW)
			break;
	}
	n += fprintf(out,"ET");
	nByte += n;
	return n;
}

void genHead(void) {
	nByte += printf("%%PDF-1.4\n");
}

void genCl(FILE* out){
	clPos = nByte;
	nByte += fprintf(out,"1 0 obj\n");
	nByte += fprintf(out,"<< /Type /Catalog /Pages 2 0 R >>\n");
	nByte += fprintf(out,"endobj\n");
}

int genPc(FILE* in,FILE* out){
	int sl;
	pcPos[nPage] = nByte;
	nByte += fprintf(out,"%d 0 obj\n", PC_BASE + nPage);
	nByte += fprintf(out,"<< /Length %d 0 R >>\n", SL_BASE + nPage);
	nByte += fprintf(out,"stream\n");
	sl = genCs(in,out);
	nByte += fprintf(out,"\nendstream\n");
	nByte += fprintf(out,"endobj\n");
	return sl;
}


void genSl(int n,FILE* out){
	slPos[nPage] = nByte;
	nByte += fprintf(out,"%d 0 obj\n", SL_BASE + nPage);
	nByte += fprintf(out,"%d\n", n);
	nByte += fprintf(out,"endobj\n");
}

void genPg(FILE* out) {
	pgPos[nPage] = nByte;
	nByte += fprintf(out,"%d 0 obj\n", PG_BASE + nPage);
	nByte += fprintf(out,"<< /Type /Page\n");
	nByte += fprintf(out,"/Parent 2 0 R\n");
	nByte += fprintf(out,"/Contents %d 0 R >>\n", PC_BASE + nPage);
	nByte += fprintf(out,"endobj\n");
}

void genPr(FILE* out){
	prPos = nByte;
	nByte += fprintf(out,"2 0 obj\n");
	nByte += fprintf(out,"<< /Type /Pages\n");
	nByte += fprintf(out,"/Kids [\n");
	for (int i = 0; i < nPage; i++)
		nByte += fprintf(out,"%d 0 R\n", PG_BASE + i);
	nByte += fprintf(out,"]\n");
	nByte += fprintf(out,"/Count %d\n", nPage);
	nByte += fprintf(out,"/MediaBox [0 0 595 842]\n");
	nByte += fprintf(out,"/Resources << /Font << /F0 <<\n");
	nByte += fprintf(out,"/Type /Font\n");
	nByte += fprintf(out,"/Subtype /Type1\n");
	nByte += fprintf(out,"/BaseFont /Courier\n");
	nByte += fprintf(out,">> >> >>\n");	/* end of resources dict */
	nByte += fprintf(out,">>\n");
	nByte += fprintf(out,"endobj\n");
}

void genXref(FILE* out){
	xrefPos = nByte;
	fprintf(out,"xref");
	fprintf(out,"0 3");
	fprintf(out,"0000000000 65535 f ");
	fprintf(out,"%010d 00000 n \n", clPos);
	fprintf(out,"%010d 00000 n \n", prPos);
	fprintf(out,"%d %d\n", PG_BASE, nPage);
	for (int i = 0; i < nPage; i++)
		fprintf(out,"%010d 00000 n \n", pgPos[i]);
	fprintf(out,"%d %d\n", PC_BASE, nPage);
	for (int i = 0; i < nPage; i++)
		fprintf(out,"%010d 00000 n \n", pcPos[i]);
	fprintf(out,"%d %d\n", SL_BASE, nPage);
	for (int i = 0; i < nPage; i++)
		fprintf(out,"%010d 00000 n \n", slPos[i]);
}

void genTail(FILE* out){
    fprintf(out,"trailer\n<< /Size %d /Root 1 0 R >>\n", SL_BASE + nPage);
	fprintf(out,"startxref");
	fprintf(out,"%d\n", xrefPos);
	fprintf(out,"%%EOF");
}



void genPdf(FILE* in,FILE* out){
	int sl;
	nByte = nPage = 0;
	genHead();
	genCl(out);
	do {
		genPg(out);
		sl = genPc(in,out);
		genSl(sl,out);
		nPage++;
	} while (nPage < NPAGE && !feof(in));
	if (!feof(in)) {
		fprintf(stderr, "text-to-pdf couldn't generate pdf : Too many pages.\n");
		exit(-1);
	}
	genPr(out);
	genXref(out);
	genTail(out);
}


int main(int argc, char* argv[]){
	FILE *in;
	if (argc > 2) {
		fprintf(stderr, "Here's how to use the program : ./bin/text-to-pdf [input file]\n");
		exit(-1);
	}
	argv++;
	if (*argv && !strcmp(*argv, "--help")) {
		printf("Here's how to use the program : ./bin/text-to-pdf [input ]\n\n");
		exit(0);
	}
	if (*argv && !strcmp(*argv, "--version")) {
		printf("%s %s\n%s\n", "text-to-pdf", "0.1", "Esteban795");
		exit(0);
	}
	if (*argv) {
		if (!(in=fopen(*argv, "r"))) {
			perror("text-to-pdf");
			exit(-1);
		}
	} else
		in = stdin;
	int len = strlen(*argv);
	char* outfile = malloc(sizeof(char) * len);
	outfile[len - 4] = 'p';
	outfile[len - 3] = 'd';
	outfile[len - 2] = 'f';

	printf("%s",outfile);
	//FILE* out = fopen(outfile, "w");
	//genPdf(in,out);
	fclose(in);
	//fclose(outfile);
	free(outfile);
	return 0;
}
