/*
In the program, I'll use the following abreviations :

 * cl:	catalog object	(#0)
 * pr:	page tree root	(#1)
 * pg:	page object	(#PG_BASE ~ #PG_BASE + NPAGE - 1)
 * pc:	page content	(#PC_BASE ~ #PC_BASE + NPAGE - 1)
 * cs:	content stream
 * sl:	stream length	(#SL_BASE ~ #SL_BASE + NPAGE - 1)
 
 Also, the explanations for this project can be found here : 
 https://www.oreilly.com/library/view/pdf-explained/9781449321581/ch04.html
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define NPAGE 32768
#define PG_BASE 32768
#define PC_BASE (PG_BASE + NPAGE)
#define SL_BASE (PC_BASE + NPAGE)
#define NCOL 80 //at most 80 colums and 66 rows
#define NROW 66

int nByte;
int nPage;
int clPos;
int prPos;
int pgPos[NPAGE];
int pcPos[NPAGE];
int slPos[NPAGE];
int xrefPos;



void genHead(void) {
	nByte += printf("%%PDF-1.4\n");
}

void genCl(void){
	clPos = nByte;
	nByte += printf("1 0 obj\n");
	nByte += printf("<< /Type /Catalog /Pages 2 0 R >>\n");
	nByte += printf("endobj\n");
}

void genPg(void) {
	int sl;
	pgPos[nPage] = nByte;
	nByte += printf("%d 0 obj\n", PG_BASE + nPage);
	nByte += printf("<< /Type /Page\n");
	nByte += printf("/Parent 2 0 R\n");
	nByte += printf("/Contents %d 0 R >>\n", PC_BASE + nPage);
	nByte += printf("endobj\n");
}

int genPc(FILE* in){
	int genCs(FILE *in);
	int sl;
	pcPos[nPage] = nByte;
	nByte += printf("%d 0 obj\n", PC_BASE + nPage);
	nByte += printf("<< /Length %d 0 R >>\n", SL_BASE + nPage);
	nByte += printf("stream\n");
	sl = gencs(in);
	nByte += printf("\nendstream\n");
	nByte += printf("endobj\n");
	return sl;
}


void genSl(int n){
	slPos[nPage] = nByte;
	nByte += printf("%d 0 obj\n", SL_BASE + nPage);
	nByte += printf("%d\n", n);
	nByte += printf("endobj\n");
}

void genPr(void){
	prPos = nByte;
	nByte += printf("2 0 obj\n");
	nByte += printf("<< /Type /Pages\n");
	nByte += printf("/Kids [\n");
	for (int i = 0; i < nPage; i++)
		nByte += printf("%d 0 R\n", PG_BASE + i);
	nByte += printf("]\n");
	nByte += printf("/Count %d\n", nPage);
	nByte += printf("/MediaBox [0 0 595 842]\n");
	nByte += printf("/Resources << /Font << /F0 <<\n");
	nByte += printf("/Type /Font\n");
	nByte += printf("/Subtype /Type1\n");
	nByte += printf("/BaseFont /Courier\n");
	nByte += printf(">> >> >>\n");	/* end of resources dict */
	nByte += printf(">>\n");
	nByte += printf("endobj\n");
}


void genXref(void){
	xrefPos = nByte;
	puts("xref");
	puts("0 3");
	puts("0000000000 65535 f ");
	printf("%010d 00000 n \n", clPos);
	printf("%010d 00000 n \n", prPos);
	printf("%d %d\n", PG_BASE, nPage);
	for (int i = 0; i < nPage; i++)
		printf("%010d 00000 n \n", pgPos[i]);
	printf("%d %d\n", PC_BASE, nPage);
	for (int i = 0; i < nPage; i++)
		printf("%010d 00000 n \n", pcPos[i]);
	printf("%d %d\n", SL_BASE, nPage);
	for (int i = 0; i < nPage; i++)
		printf("%010d 00000 n \n", slPos[i]);
}

void genTail(void){
    printf("trailer\n<< /Size %d /Root 1 0 R >>\n", SL_BASE + nPage);
	puts("startxref");
	printf("%d\n", xrefPos);
	puts("%%EOF");
}

int genCs(FILE *in){
	int escape(char *s);
	char line[NCOL+1];
	int n, nl, eol, ch;
	n = 0;
	n += printf("BT\n");
	n += printf("11 TL\n");
	n += printf("34 784 Td\n");
	n += printf("/F0 11 Tf\n");
	nl = 0;
	while (fgets(line, sizeof(line), in)) {
		eol = strcspn(line, "\n");
		if (!line[eol] && (ch = fgetc(in)) != '\n')
			ungetc(ch, in);
		line[eol] = '\0';
		n += printf("T* (");
		n += escape(line);
		n += printf(") Tj\n");
		if (++nl == NROW)
			break;
	}
	n += printf("ET");
	nByte += n;
	return n;
}

int escape(char *s){
	int n = 0;
	while (*s) {
		switch (*s) {
		case '(':
		case ')':
		case '\\':
			putchar('\\');
			n++;
		default:
			putchar(*s);
			n++;
		}
		s++;
	}
	return n;
}

void genPdf(FILE* in){
	int sl;
	nByte = nPage = 0;
	genHead();
	genCl();
	do {
		genPg();
		sl = genPc(in);
		genSl(sl);
		nPage++;
	} while (nPage < NPAGE && !feof(in));
	if (!feof(in)) {
		fprintf(stderr, "text-to-pdf couldn't generate pdf : Too many pages.\n");
		exit(-1);
	}
	genPr();
	genXref();
	genTail();
}


int main(int argc, char **argv){
	FILE *in;
	if (argc > 2) {
		fprintf(stderr, "Here's how to use the program : ./bin/text-to-pdf [filepath]\n");
		exit(-1);
	}
	argv++;
	if (*argv && !strcmp(*argv, "--help")) {
		printf("Here's how to use the program : ./bin/text-to-pdf [filepath]\n\n");
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
	genpdf(in);
	return 0;
}