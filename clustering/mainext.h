/* -----------------------------------------------------------------
 * mainext:	define some extension contents for the main.c file
 *      - analysis argv: _input_parameters()
 *      -
 * author: xkfeng
 * time: 2017-3-16
 * -----------------------------------------------------------------*/
#ifndef	MAINEXT_H
#define	MAINEXT_H
#include "common.h"

/*  Analysis the input and extract the parameters from argv[]:
 *
 *  This can be self-define.
 */

/// ### self-define ###: usage string
#define USAGE_STR       "(-log 1 to report)\n\t\t-ds\t[dsname]\n\t\t-K\t[no. of clusters]\n\t\t-h\thierarchical layers\n\t\t-mk\tk-NN number for member points"

/// ### self-define ###: parameter list
bool _input_parameters(int argc, char *argv[], char *dsname, int *K, int *h, int *mk, int *logOn){

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
			else if(strcmp(pname, "K") == 0){		*K = atoi(argv[i]);	}
			else if(strcmp(pname, "h") == 0){		*h = atoi(argv[i]);	}
			else if(strcmp(pname, "mk") == 0){		*mk = atoi(argv[i]);	}
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
