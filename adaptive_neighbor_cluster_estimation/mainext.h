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

#define NEED_PARA_NUM 11							/// ### self-define ###: legal number of parameters
bool _input_parameters(int argc, char *argv[], char *dsname, int *K, int *h, int *nq, int *k){
									/// ### self-define ###: parameter list
	printf("%d", argc);
	/* check parameter numbers */
	if(argc != NEED_PARA_NUM){
		printf("get parameter error: insufficient parameter numbers \n");
		printf("hb: \n\t-ds dsname \n\t-K number of clusters \n\t-h clustering depth \n\t-nq number of queries \n\t-k [k]nn\n");
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
				return false;
			}
									/// ### self-define ###: get the parameter values
			if(strcmp(pname, "ds") == 0){			strcpy(dsname, argv[i]);	}
			else if(strcmp(pname, "K") == 0){		*K = atoi(argv[i]);	}
			else if(strcmp(pname, "h") == 0){		*h = atoi(argv[i]);	}
			else if(strcmp(pname, "nq") == 0){		*nq = atoi(argv[i]);	}
			else if(strcmp(pname, "k") == 0){		*k = atoi(argv[i]);	}

			i++;
		}else{
			printf("parameter format invalid: not expect a value before a parameter name\n");
			return false;
		}
	}
	return true;
}

#endif
