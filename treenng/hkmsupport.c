#include "hkmsupport.h"
#include "common.h"
#include <string.h>
#include <yael/hkm.h>
#include <yael/vector.h>

bool hkm_check_exists(const char *folder, int datatype){
	ASSERTINFO(folder == NULL, "IPP");
	char	filename[255];
	bool	status = true;

	// index bingo
	if(IndexData_Index & datatype){
		sprintf(filename, "%s/index.hkm", folder);
		status = file_exists(filename);
	}
	if(IndexData_Data & datatype){
		sprintf(filename, "%s/base_assign.ivecs", folder);
		status = file_exists(filename);
	}
	return status;
}

bool hkm_from_file(hkm_t **phkm, int **passign, const char *folder, int n, int datatype){
	ASSERTINFO(phkm == NULL || passign == NULL || folder == NULL || n <= 0, "IPP");
	bool 	status = hkm_check_exists(folder, datatype);
	char	filename[255];

	if(status){
		if(IndexData_Index & datatype){	// load index
			sprintf(filename, "%s/index.hkm", folder);
			*phkm = hkm_read(filename);
		}
		if(IndexData_Data & datatype){	// load data assign
			sprintf(filename, "%s/base_assign.ivecs", folder);
			*passign = ivec_new(n);
			ivecs_read(filename, 1, n, *passign);
		}
	}

	return status;
}