//
//  NeighborGraph.cpp
//  FANNG类的实现
//
//  Created by 冯小康 on 2017/7/22.
//  Copyright © 2017年 cui1001. All rights reserved.
//
#include "NeighborGraph.h"
#include <stdio.h>
#include <stdlib.h>

void NeighborGraph::nng_into_files(const char *filename){
    FILE *fp = fopen(filename, "w");
    if(fp == NULL){
        printf("failed to open file\n");
        return;
    }
    
    int ib, ie;
    for(ib = 0; ib < n; ib++){
        for(ie = 0; ie < edges[ib].size(); ie++){
            fprintf(fp, "%d %d %lf\n", ib, edges[ib][ie].id, edges[ib][ie].val);
        }
    }
    fclose(fp);
}
