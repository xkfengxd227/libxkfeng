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

/// ### self-define ###: parameter list
bool _input_parameters(int argc, char *argv[], char *dsname,
	char *basefile, char *queryfile, int *n, int *d, int *nq, int *dGT){

	if(argc <= 1){
		printf("get parameter value error: insufficient parameter values\n");
		print_usage();
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
				print_usage();
				return false;
			}

            /// ### self-define ###: get the parameter values
			if(strcmp(pname, "ds") == 0){			strcpy(dsname, argv[i]);	}
			else if(strcmp(pname, "v") == 0){			strcpy(basefile, argv[i]);	}
			else if(strcmp(pname, "q") == 0){			strcpy(queryfile, argv[i]);	}
			else if(strcmp(pname, "n") == 0){		*n = atoi(argv[i]);	}
			else if(strcmp(pname, "d") == 0){		*d = atoi(argv[i]);	}
			else if(strcmp(pname, "nq") == 0){		*nq = atoi(argv[i]);	}
			else if(strcmp(pname, "dGT") == 0){		*dGT = atoi(argv[i]);	}

			i++;
		}else{
			printf("parameter format invalid: not expect a value before a parameter name\n");
			print_usage();
			return false;
		}
	}

	return true;
}

#endif
