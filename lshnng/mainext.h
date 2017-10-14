/* -----------------------------------------------------------------
 * mainext:	define some extension contents for the main.c file
 *      - analysis argv: _input_parameters()
 *      -
 * author: xkfeng
 * time: 2016-3-1
 * -----------------------------------------------------------------*/
#ifndef	MAINEXT_H
#define	MAINEXT_H
#include "common.h"

/*  Analysis the input and extract the parameters from argv[]:
 *
 *  This can be self-define.
 */

/// ### self-define ###: usage string
#define USAGE_STR       "(-log 1 to report)\n\t\t-ds\t[dsname]\n\t\t-m\t[no. lsh functions]\n\t\t-w\t[lsh internal width]\n\t\t-g\t[no. of neighbors]\n\t\t-nc\t[no. of candidates]\n\t\t-nq\t[no. of queries]\n\t\t-k\t[k]-nn"

/// ### self-define ###: parameter list
bool _input_parameters(int argc, char *argv[], char *dsname, int *m, float *w, int *g, int *nc, int *nq, int *k, int *logOn){

	if(argc <= 1){
		printf("get parameter value error: insufficient parameter values\n");
		puts(USAGE_STR);
		return false;
	}

	char pname[255];
	int i = 1;
	while(i < argc){
		/* initialize the parameter name buffer */
		memset(pname, 0, sizeof(char)*255);
		if(PNAME == get_para_type(argv[i])){		/* a parameter name */
			/* get the parameter name */
			extract_para_name(argv[i], pname);

			if(i < argc-1){
				/* move i on to the parameter value */
				i++;
			}else{
				/* no value for some parameter */
				printf("get parameter value error: insufficient parameter values\n");
				puts(USAGE_STR);
				return false;
			}

            /// ### self-define ###: get the parameter values
			if(strcmp(pname, "ds") == 0){			strcpy(dsname, argv[i]);	}
			else if(strcmp(pname, "m") == 0){		*m = atoi(argv[i]);	}
			else if(strcmp(pname, "w") == 0){		*w = atof(argv[i]);	}
			else if(strcmp(pname, "g") == 0){		*g = atoi(argv[i]);	}
			else if(strcmp(pname, "nc") == 0){		*nc = atoi(argv[i]);	}
			else if(strcmp(pname, "nq") == 0){		*nq = atoi(argv[i]);	}
			else if(strcmp(pname, "k") == 0){		*k = atoi(argv[i]);	}
			else if(strcmp(pname, "log") == 0){		*logOn = atoi(argv[i]);	}

			i++;
		}else{
			printf("parameter format invalid: not expect a value before a parameter name\n");
			puts(USAGE_STR);
			return false;
		}
	}
	return true;
}

#endif
