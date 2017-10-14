#include "mainext.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool _input_parameters(int argc, char *argv[], char *dsname, int *bf, int *h, int *g, int *k, int *pk){
	int i = 1;
	char pname[255];
	bool status = true;

	if(argc <= 1){
		status = false;
	}else{
		while(i < argc){
			/* initialize the parameter name buffer */
			memset(pname, 0, sizeof(char)*255);
			if(PNAME == get_para_type(argv[i])){		/* a parameter name */
				/* get the parameter name */
				extract_para_name(argv[i], pname);
				/* move i backward at the parameter value */
				if(i < argc-1){
					i++;
				}else{
					status = false;
					break;
				}
				/* get the parameter values */
				if(strcmp(pname, "ds") == 0){		strcpy(dsname, argv[i]);	}
				else if(strcmp(pname, "bf") == 0){		*bf = atoi(argv[i]);	}
				else if(strcmp(pname, "h") == 0){		*h = atoi(argv[i]);	}
				else if(strcmp(pname, "g") == 0){		*g = atoi(argv[i]);	}
				else if(strcmp(pname, "k") == 0){		*k = atoi(argv[i]);	}
				else if(strcmp(pname, "pk") == 0){		*pk = atoi(argv[i]);	}

				i++;
			}else{
				printf("parameter format invalid: not expect a value before a parameter name\n");
				status = false;
				break;
			}
		}
	}
	if(!status){
		printf("get parameter error, please refer to the right format:\n");
		printf("-ds [dsname] \n-bf [branch factor] \n-h [depth of the hierarchical kmeans tree] \n-g [neighbors in NNG] \n-k [k] \n-pk [prior k <= k]\n");
	}

	return status;
}
