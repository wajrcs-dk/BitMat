/*
 * Copyright 2009-2014 Medha Atre (medha.atre@gmail.com)
 * 
 * This file is part of BitMat.
 * 
 * BitMat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * BitMat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with BitMat.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Report any bugs or feature requests to medha.atre@gmail.com.
 */


#include "bitmat.h"

void load_mappings(char *subjfile, char *predfile, char *objfile)
{
	FILE *fp;
	unsigned int cnt = 0;
	int i;

	printf("Inside load_mapping\n");
	
	for (i = 0; i < 3; i++) {

		if (i == 0)
			fp = fopen64(subjfile, "r");
		else if (i == 1)
			fp = fopen64(predfile, "r");
		else if (i == 2)
			fp = fopen64(objfile, "r");

		if (fp == NULL) {
			printf("Error opening mapping file\n");
			exit(-1);
		}

		char line[3072];
		char digit[15], str[2048];
		unsigned int intmapping = 0;
		char *c=NULL, *c2=NULL;

		memset(line, 0, 3072);

		while (!feof(fp)) {

			if(fgets(line, sizeof(line) - 1, fp) != NULL) {
				c = strchr(line, ' ');
				*c = '\0';
				strncpy(digit, line, 10);
				intmapping = atoi(digit);

				if (intmapping != cnt + 1) {
					printf("****ERROR: count mismatch intmapping %u digit %s cnt %d\n", intmapping, digit, cnt+1);
					exit(-1);
				}

				c2 = strchr(c+1, '\n');
				*c2 = '\0';
				strncpy(str, c+1, 2048);

				if (i == 0) {
					subjmapping[cnt] = (unsigned char *) malloc (strlen(str) + 1);
					strcpy((char *)subjmapping[cnt], str);
				} else if (i == 1) {
					predmapping[cnt] = (unsigned char *) malloc (strlen(str) + 1);
					strcpy((char *)predmapping[cnt], str);
				} else if (i == 2) {
					objmapping[cnt] = (unsigned char *) malloc (strlen(str) + 1);
					strcpy((char *)objmapping[cnt], str);
				}
				cnt++;
			}
		}

		cnt = 0;
		fclose(fp);
	}
	printf("Exiting load_mapping\n");
}
/////////////////////////////////////////
void init_mappings()
{

	subjmapping = (unsigned char **) malloc (gnum_subs * sizeof(unsigned char *));
	predmapping  = (unsigned char **) malloc (gnum_preds * sizeof(unsigned char *));
	objmapping = (unsigned char **) malloc (gnum_objs * sizeof(unsigned char *));

	memset(subjmapping, 0, gnum_subs * sizeof (unsigned char *));
	memset(predmapping, 0, gnum_preds * sizeof (unsigned char *));
	memset(objmapping, 0, gnum_objs * sizeof (unsigned char *));
}

