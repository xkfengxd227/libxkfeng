//
//  main.cpp
//  fanng
//
//  Created by 冯小康 on 2016/11/4.
//  Copyright © 2016年 cui1001. All rights reserved.
//

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "common.h"
#include "fanng.h"
#include "/media/xikafe/dataset/dsbconfig.h"
#include "mainext.h"
extern "C"{
	#include <yael/vector.h>
}
using namespace std;

int main(int argc, const char * argv[]) {
    /* config */
    DSBConfig dscfg;
    char 	nngfile[255] = "fanng.txt";
    char	clusterfile[255] = "/media/xikafe/dataset/color/bin/color_cluster500.centroid.fvecs";
    int 	K = 500;

    // accept parameters
    _input_parameters(argc, argv, dscfg.dsname);
    config_dataset(dscfg.dsname, &dscfg);
    int 	nq = dscfg.nq;
    int 	d = dscfg.d;

    /* load data */
    float *cent = fvec_new(K*d);
    fvecs_read(clusterfile, d, K, cent);
    
    
    /* nng construction */
    FANNG fanng = FANNG(K, d);
    fanng.generate_nng(cent);
    fanng.nng_into_files(nngfile);

    /* free */
    FREE(cent);
    
    return 0;
}
