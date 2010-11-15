#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#define MAXLINELEN 65536
#define PATTERNNUM 4096

const char gsVersion[] = "0.1.7";

void help(void) {
	printf("Search for PATTERN in each FASTA FILE or standard input.\n");
	printf("Usage: fastafgrep [-h] [-v] [-c] [-1] -p pattern | -f pattern_file [-i FASTA_file]\n");
	printf("\n");
	printf("-v  Select non-matching lines\n");
	printf("-c  Ignore case distinctions\n");
	printf("-V  print version information and exit\n");
	printf("-1  Select 1st matching sequence only\n");
}

void ver(void) {
	printf("fastafgrep %s\n", gsVersion);
}

void rtrim(char *sLine) {
	int i;
	
	for (i = (strlen(sLine) - 1); i >= 0; i--) {
		if (isspace(*(sLine + i))) *(sLine + i) = '\0';
		else break;
	}
}

int main(int argc, char *argv[]) {
	char *spFileName = NULL;
	char *spPatternFileName = NULL;
	char sLine[MAXLINELEN];
	char **spPatterns;
	int iPatternBufferSize = PATTERNNUM;
	int iPatternNum = 0;
	int iMatchNum = 0;
	int iMatchFlag = 0;
	int iInvert = 0;
	int iOpt;
	int i;
	int iOne = 0;
	int iReturn = 1;
	FILE *FSeqFile;
	FILE *FPatternFile;
	char *(*comp)(const char *, const char *);

	void rtrim(char *sLine);

	spPatterns = (char **)malloc(sizeof(char *) * PATTERNNUM);

	comp = strstr;

	while((iOpt = getopt(argc, argv, "1vcVp:f:i:h")) != -1) {
		switch(iOpt) {
		case 'h':
			help();
			exit(0);
			break;
		case '1':
			iOne = 1;
			break;
		case 'p':
			*spPatterns = (char *)malloc(strlen(optarg) * sizeof(char));
			strcpy(*spPatterns, optarg);
			/* *spPatterns = optarg; Version 0.1.5에서 수정 */
			iPatternNum++;
			break;
		case 'f':
			spPatternFileName = optarg;
			break;
		case 'i':
			spFileName = optarg;
			break;
		case 'v':
			iInvert = 1;
			break;
		case 'c':
			comp = strcasestr;
			break;
        case 'V':
			ver();
			exit(0);
/*		case '?':
			printf("unknown option: %c\n", iOpt);
			exit(0);*/
		default:
			help();
			exit(0);
		}
	}

	if (iPatternNum < 1 && spPatternFileName == '\0') {
		help();
		exit(0);
	}
	
	if (iPatternNum >= 1 && spPatternFileName != '\0') {
		help();
		exit(0);
	}

	if (spFileName == '\0') {
		FSeqFile = stdin;
	} else {
		if (access(spFileName, F_OK) != 0) {
			perror(spFileName);
			exit(1);
		} else {
			FSeqFile = fopen(spFileName, "r");
		}
	}

	if (spPatternFileName != '\0') {
		if (access(spPatternFileName, F_OK) != 0) {
			perror(spPatternFileName);
			exit(1);
		} else {
			FPatternFile = fopen(spPatternFileName, "r");
		}
		while(fgets(sLine, MAXLINELEN, FPatternFile)) {
			rtrim(sLine);
			*(spPatterns + iPatternNum) = (char *)malloc((strlen(sLine) + 1) * sizeof(char));
			strcpy(*(spPatterns + iPatternNum), sLine);
			iPatternNum++;
			if (iPatternBufferSize < iPatternNum) {
				spPatterns = (char **)realloc(spPatterns, sizeof(char *) * (PATTERNNUM + iPatternBufferSize));
				iPatternBufferSize += PATTERNNUM;
			}
		}
	}
	
	while(fgets(sLine, MAXLINELEN, FSeqFile)) {
		if (sLine[0] == '>') {
			iMatchNum = 0;
			if (iMatchFlag == 1 && iOne == 1 && iInvert == 1) break;
			iMatchFlag = 0;
			for (i = 0; i < iPatternNum; i++) {
				if (*(spPatterns + i) != NULL && ((*comp)(sLine, *(spPatterns + i)) != NULL)) {
					iMatchNum++;
					if (iOne == 1 && iInvert == 0) {
						free(*(spPatterns + i));
						*(spPatterns + i) = NULL;
					}
				}
			}
			if (iInvert == 0) {
				if (iMatchNum > 0) iMatchFlag = 1;
			} else {
				if (iMatchNum == 0)  iMatchFlag = 1;
			}
		}
		if (iMatchFlag == 1) {
			iReturn = 0;
			printf("%s", sLine);
		}
	}

	for (i = 0; i < iPatternNum; i++) if (*(spPatterns + i) != NULL) free(*(spPatterns + i));
	free(spPatterns);
	
	return iReturn;
}
