//
//  fanng.cpp
//  FANNG类的实现
//
//  Created by 冯小康 on 2016/11/4.
//  Copyright © 2016年 cui1001. All rights reserved.
//

#include "fanng.h"
#include "common.h"
#include <vector>
#include <map>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <algorithm>
using namespace std;

void FANNG::generate_nng(float *data){
	// 需要的变量
	int	 ib, ie, kE;               // 起点/端点的序号
	int	 u, v;                     // 记录两个端点
    double L;                       // length of an edge
	bool	occluded;		   // 边是否被吸收标记
	

	// 构建近邻图
	for(ib = 0; ib < n; ib++){
		/// 对于起点ib结点
		//  首先计算所有b-e边的长度，加入列表
        vector<DoubleIndex> list;   // 以某个点为起点的边-距离列表
		for(ie = 0; ie < n; ie++){
			if(ie == ib){
				continue;
			}
			list.push_back(DoubleIndex(ie, odistance(data+ib*d, data+ie*d, d)));
		}
		// 对所有边从小到大排序
		sort(list.begin(), list.end(), dicomp_asc);
		
		// 检验list所有边，加入edges
		for(ie = 0; ie < list.size(); ie++){
			// 提取一条边
			u = list[ie].id;
			L = list[ie].val;
			// 检验
			occluded = false;
			for(kE = 0; kE < edges[ib].size(); kE++){
				v = edges[ib][kE].id;
				if(odistance(data+u*d, data+v*d, d) < L){
					occluded = true;
				}
			}
			
			if(!occluded){
				// 将通过检验的边ib-ie加入edges
				edges[ib].push_back(DoubleIndex(u, L));
			}
		}
	}
}

vector<DoubleIndex> FANNG::knn_search(float *query, int k){
	vector<DoubleIndex> knn;
	
	// in-complete
	
	return knn;
}
