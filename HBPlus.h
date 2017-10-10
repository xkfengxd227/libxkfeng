/**
 * \file hbplus.h
 * \brief Declarations of basic operations for the [HBPlus] class
 */
#ifndef HBPLUS_H
#define HBPLUS_H
#include "common.h"
#include "clustering.h"

class HBPlus : public Clustering{
public:
    /**
     *  inner lb distance for all points in each cluster, [K*...]
     *      stored in an inverted index of [K * ...] size
     *      in each cluster, the lb sorted in ascending order, therefore
     *      the first lb of each cluster innerLB[][0] is the lb of the cluster
     */
    DoubleIndex             **innerLB;

    HBPlus(int _nc, int _d, int _n) : Clustering(_nc, _d, _n){
        innerLB = NULL;
    }
    ~HBPlus();

    

    /// figure out the inner distance for points in each cluster
    void inner_lb_distance_OnePerPoint(const fDataSet *ds);

    /**
     * \brief Store the index of HBPlus into file
     * @param	indexfolder		the path to store the index
     */
    void index_into_file(const char *indexfolder);

    /** \brief  check if exists index 
     * @param   folder  folder stores index files
     * @return  check status
     */
    bool index_exists(const char *folder);

    /** \brief load index from file
     * @param   folder  the index folder
     * @param   return  load status
     */
    void load_index(const char *folder);

    /**
 * \brief Load data points from cluster file, with a limitation for LB
 * @param   filename    the cluster file name
 * @param   num         number of data points to load
 * @param   d           dimension
 * @param   set         to store loaded data points
 * @param   set_num     to store loaded id of data points
 * @param   lblimit     another stop flag of lowerbound when load data points
 */
int HBPlus_ClusterFromFile(const char *filename, int num, int d, float *set, int *set_num, double lblimit);



    /**
     * \brief excute knn search based on HBPlus
     * @param	qds the query point set
     * @param	m	
     * @param	alpha	
     * @param	R 
     * @param	r_centroid 
     * @param	indexfolder		
     * @param	nk	number of nns
     * @param	knnset	the knn set
     * @param	cost	the cost
     */
    void search(int m, float alpha, float *R, float *r_centroid, char *indexfolder, int nk, DoubleIndex **knnset, Cost *cost);

    /**
     * \brief Search k-NN for all queries in queryset
     * @param   baseset     the base vector set
     * @param   queryset    the query data set
     * @param   folder      the cluster structure path
     * @param   m           the reducted dimension
     * @param   alpha       
     * @param   R           the rotated matrix for dimension reduction
     * @param   nk  number of nns
     * @param   knnset  to store knn
     * @param   cost    the cost performance
     * @param   whether use [true lower bound]
     */
    void search(const fDataSet *baseset, const fDataSet *queryset, const char *folder, 
        int m, float alpha, float *R,
        int nk, DoubleIndex **knnset, Cost *cost, int lb_type);


    /**
     * \brief calculate all the lower bounds between query and all clusters
     *      note that: lowerbound = max(dis(q,ci)) + innerLB[ci]
     * @param   lb          to store the final lower bounds
     * @param   query       the query point
     * @param   r_c         the rotated centroids
     * @param   centroid    the centroids
     * @param   m           the reducted dimension
     * @param   alpha       
     * @param   sortflag    whether to sort the lowerbounds in ascending order
     */
    void lowerbound(DoubleIndex *lb, const float *query, const float *r_c, const float *centroid,
        int m, float alpha, bool sortflag);


    /********************* basic operations *******************/
/**
 * \brief Figure out the LB for each centroid, using R to estimate, and alpha to filter seperate hyperplane bound
 * @param   lowerbound  the lowerbound value (DoubleIndex)
 * @param   query       the query point
 * @param   r_c         the rotated centroid
 * @param   centroid    the centroids
 * @param   innerLB     the inner lowerbounds
 * @param   ncenter     the number of clusters
 * @param   d           the dimension
 * @param   m           the reduced dimension
 * @param   alpha       a parameter when evaluating lowerbounds
 */
void HBPlus_LowerBound(DoubleIndex *lowerbound, const float *query, const float *r_c, 
    const float *centroid, DoubleIndex **innerLB, int ncenter, int d, int m, float alpha);


    /**
     * \brief Generate the rotate matrix
     * @param   R   the rotated matrix
     * @param   m   the reduced dimension
     * @param   d   the dimension
     */
    void generate_rotation(float *R, int m, int d);

    /**
     * \brief  Rotate centroids with R
     * @param   r_c     the rotated centers
     * @param   c       the original centers
     * @param   R       the rotation matrix
     * @param   k       the number of nns
     * @param   d       the dimension
     * @param   m       the reduced dimension
     */ 
    void rotate_centroid(float *r_c, float *c, float *R, int k, int d, int m);

#endif

