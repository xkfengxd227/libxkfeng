//
//  fanng.h
//  Fast Approximate NNG class
//
//  Created by 冯小康 on 2016/11/4.
//  Copyright © 2016年 cui1001. All rights reserved.
//

#ifndef fanng_h
#define fanng_h
#include <vector>
#include <map>
#include "NeighborGraph.h"
#include "common.h"
using namespace std;

class FANNG : public NeighborGraph{
public:    

    FANNG(int _n, int _d) : NeighborGraph(_n, _d){};

    // 传入数据集，构造近邻图
    void generate_nng(float *data);
    // 查询kNN
    vector<DoubleIndex> knn_search(float *query, int k);
};

#endif /* fanng_h */
