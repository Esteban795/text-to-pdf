/*
In the program, I'll use the following abreviations :

 * cl:	catalog object	(#0)
 * pr:	page tree root	(#1)
 * pg:	page object	(#PG_BASE ~ #PG_BASE+NPAGE-1)
 * pc:	page content	(#PC_BASE ~ #PC_BASE+NPAGE-1)
 * cs:	content stream
 * sl:	stream length	(#SL_BASE ~ #SL_BASE+NPAGE-1)
 
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
