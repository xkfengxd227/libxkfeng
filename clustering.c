/**
 *	\file clustering.c
 *	\brief implementation of opeartions of the Clustering class
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clustering.h"
#include "common.h"
#include <yael/vector.h>
#include <yael/kmeans.h>
#include <yael/machinedeps.h>

void C_Init(Clustering *c, int nc, int niter, int nth, int seed, int nredo)
{
	c->ncenter = nc;
	c->niter = niter;
	c->nthread = nth;
	c->seed = seed;
	c->nredo = nredo;
	c->centroid = NULL;
	c->assign = NULL;
	c->nassign = NULL;
	c->member = NULL;
	c->innerLB = NULL;
}

void C_Delete(Clustering *c)
{
	int i;
    if(c->centroid != NULL){
        free(c->centroid);  c->centroid = NULL;
    }
    if(c->member != NULL){
        for(i = 0; i < c->ncenter; i++){
            if(c->member[i] != NULL){
                free(c->member[i]); c->member[i] = NULL;
            }
        }
        free(c->member);    c->member = NULL;
    }
    if(c->innerLB != NULL){
        printf("%d, ", i);
        for(i = 0; i < c->ncenter; i++){
            if(c->innerLB[i] != NULL){
                free(c->innerLB[i]); c->innerLB[i] = NULL;
            }
        }
        free(c->innerLB);   c->innerLB = NULL;
    }
    if(c->assign != NULL){
        free(c->assign); c->assign = NULL;
    }
    if(c->nassign != NULL){
        free(c->nassign); c->nassign = NULL;
    }
}

void C_Clustering(Clustering *c, fDataSet *ds, fDataSet *lds)
{
    int i, j;
    int d = ds->d;
    int n = ds->n;
    int n_l = lds->n;

	/// allocate storage space

	printf("--------------before allocate space-------------\n");

	c->centroid = fvec_new_set(c->ncenter*d, 0);
	c->member = (int**)malloc(sizeof(int*)*c->ncenter);
	c->assign = ivec_new_set(n, -1);
    c->innerLB = (DoubleIndex **)malloc(sizeof(DoubleIndex*)*c->ncenter);
    for(i = 0; i < c->ncenter; i++){
        c->member[i] = NULL;
        c->innerLB[i] = NULL;
    }

	/// generate clusters via kmeans

	printf("--------------before kmeans-------------\n");

	float quantierror = kmeans(d, n_l, c->ncenter, c->niter, lds->data, c->nthread, c->seed, c->nredo, c->centroid, NULL, NULL, NULL);
	printf(">>> clustering, quantization error: %f\n", quantierror);
	fvecs_write_txt("centroid.txt", d, c->ncenter, c->centroid);

    // update assign according to the lastest centroids
    double min_id = -1;
    double min_dis = FLOAT_MAX;
    double temp_dis;
    float *mydis = fvec_new(n);
    float *vector = fvec_new(d);
    float *centroid = fvec_new(d);
    for(i = 0; i < n; i++)
    {
        min_id = -1;
        min_dis = FLOAT_MAX;
        memcpy(vector, ds->data+i*d, sizeof(float)*d);
        for(j = 0; j < c->ncenter; j++)
        {
            memcpy(centroid, c->centroid+d*j, sizeof(float)*d);
            temp_dis = odistance(vector, centroid, d);

            if(f_bigger(min_dis, temp_dis))
            {
                min_dis = temp_dis;
                min_id = j;
            }
        }
        c->assign[i] = min_id;
        mydis[i] = min_dis;
    }
    c->nassign = ivec_new_0(c->ncenter);
    for(i = 0; i < n; i++)
    {
        c->nassign[c->assign[i]] ++;
    }

    /// test assign

    printf("--------------test assign-------------\n");

    FILE *fp = fopen("assign.txt", "w");
    for(i = 0; i < n; i++)
    {
        fprintf(fp, "%d\t%f\n", c->assign[i], mydis[i]);
    }
    fclose(fp);

	/// extract clustering members

	printf("--------------before extract members-------------\n");

	extract_members(c->assign, c->nassign, c->member, n, c->ncenter);
    /// figure out inner distance and sort
    C_InnerLBDistance(c, ds);

    printf("--------------after inner-lbdistance-------------\n");

    free(vector); vector = NULL;
    free(centroid); centroid = NULL;
    free(mydis); mydis = NULL;
}

void C_InnerLBDistance(Clustering *c, fDataSet *ds)
{
    int i, j, nci, otheri;
    int d = ds->d;
    float dis = 0;
    float *xcenter = fvec_new(d);
    float *ocenter = fvec_new(d);
    float *x = fvec_new(d);
    float *centroid_dis_map = fvec_new_0(c->ncenter*c->ncenter);

    /// prepare distances between each two centroids
    for(i = 0; i < c->ncenter; i++)
    {
        memcpy(xcenter, c->centroid+i*d, sizeof(float)*d);
        for(j = 0; j <= i; j++)
        {
            memcpy(ocenter, c->centroid+j*d, sizeof(float)*d);
            dis = odistance(xcenter, ocenter, d);
            centroid_dis_map[i*c->ncenter+j] = dis;
            if(i != j)
            {
                centroid_dis_map[j*c->ncenter+i] = dis;
            }
        }
    }

    // initialize the storing space for inner distance
    for(nci = 0; nci < c->ncenter; nci++)
    {
        c->innerLB[nci] = (DoubleIndex*)malloc(sizeof(DoubleIndex) * c->nassign[nci]);
        for(i = 0; i < c->nassign[nci]; i++)
        {
            c->innerLB[nci][i].id = -1;
            c->innerLB[nci][i].val = FLOAT_MAX;
        }
    }

    for(nci = 0; nci < c->ncenter; nci++)
    {// the nci-th centroid
        memcpy(xcenter, c->centroid+nci*d, sizeof(float)*d);
        for(otheri = 0; otheri < c->ncenter; otheri++)
        {// choose another centroid
            memcpy(ocenter, c->centroid+otheri*d, sizeof(float)*d);
            if(otheri != nci)
            {
                for(i = 0; i < c->nassign[nci]; i++)
                {
                    memcpy(x, ds->data+c->member[nci][i]*d, sizeof(float)*d);
                    dis = (odistance_square(x, ocenter, d) - odistance_square(x, xcenter, d)) / (2*centroid_dis_map[nci*c->ncenter+otheri]);
                    if(f_bigger(c->innerLB[nci][i].val, dis))
                    {// update using smaller distance
                        c->innerLB[nci][i].val = dis;
                        c->innerLB[nci][i].id = c->member[nci][i];          // id is the data point
                    }
                }
            }
        }
        // sort member data points along the innerLB distance in the nci-th cluster
        DI_MergeSort(c->innerLB[nci], 0, c->nassign[nci]-1);
    }

    free(centroid_dis_map); centroid_dis_map = NULL;
    free(ocenter); ocenter = NULL;
    free(xcenter); xcenter = NULL;
    free(x); x = NULL;
}

