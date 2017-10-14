/**
 * 	\brief	Some common configs
 *	self-define-abbreviations:
 *		IPP:		invalid passing parameters
 */

#ifndef COMMON_H
#define COMMON_H
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include "cost.h"
using namespace std;

/// define two assertion formats
#define ASSERT(error_condition) { \
    if (error_condition) { \
        printf("ASSERT: %s:%d:%s()# \n", __FILE__, __LINE__, __FUNCTION__); \
        exit(EXIT_FAILURE); \
    } \
}

#define ASSERTINFO(error_condition, info) { \
    if (error_condition) { \
        printf("!> assert: %s:%d:%s(): %s\n", __FILE__, __LINE__, __FUNCTION__, info); \
        exit(EXIT_FAILURE); \
    } \
}

#define __assertinfo(error_condition, info) { \
    if (error_condition) { \
        printf("!> assert: %s:%d:%s(): %s\n", __FILE__, __LINE__, __FUNCTION__, info); \
        exit(EXIT_FAILURE); \
    } \
}

#define LOGINFO(log_status, info){ \
	if (log_status){ \
		puts(info); \
	}\
}

#define FREE(p){		\
	if(p != NULL){		\
		free(p);		\
		p = NULL;	\
	}			\
}

#define SAFELY_FREE(p){		\
	if(p != NULL){		\
		free(p);		\
		p = NULL;	\
	}			\
}


#define     INT_MAX 	0x7FFFFFFF			/// max signed integer
#define     INT_DEFAULT	-1				/// a defult value of integer variables
#define     STR_LEN	255				/// default length of a char array
#define     FLOAT_MAX 9999999999.0			/// max float
#define     FLOAT_ZERO  1e-10				/// zero for float
#define     SHAKERANGE  10				/// in case the result knn not at the same position with groundtruth
#define     PAGESIZE    4096				/// page size: 4K
#define     PAGE_RAN_SE 10				/// 10 sequence pages equal 1 random page

#define     INDEX_PARA "index.para"			/// the name of an index file
#define     CLUSTER_POXFIX ".cluster"			/// the postfix of an cluster file

/// kmeans clustering relative
#define     CLUSTERING_NITER    100         	/// clustering iteration times
#define     CLUSTERING_NTHREAD  (count_cpu())   /// number of threads in clustering
#define     CLUSTERING_SEED     (time(NULL))           	/// random seed
#define     CLUSTERING_NREDO    1           	/// clustering run loops

#define		IndexFolder	"index"							/// default index folder
#define     ResultLogFile   "result.log"        /// store results
#define     IndexData_Index     0x1
#define     IndexData_Data      0x2

#define 	Algorithm_Indexing 		0
#define 	Algorithm_Search 		1
#define 	Algorithm_GroundTruth 	2

/**
 *	\brief parameter type
 */
typedef enum {PNAME,PVALUE,PNULL} PARA_TYPE;

/// a DataSet class where the member points are float/int
typedef struct
{
	int 				n;					/// number of data points
	int 				d;					/// data dimension
	float				*data;				/// data points
} fDataSet;
typedef struct
{
	int 				n;
	int 				d;
	int 				*data;
} iDataSet;

/// initialize a float data set
void fDataSet_init(fDataSet *ds, int n, int d);
void iDataSet_init(iDataSet *ds, int n, int d);

/// reset a float data set
void fDataSet_unset(fDataSet *ds);
void iDataSet_unset(iDataSet *ds);

/**
 * \brief extract a subset of the dataset, the range in each dimension is [from, to)
 * @time    2015-9-19 20:57:14
 * @param   ds      the dataset to be extracted
 * @param   from    the begin position (included)
 * @param   to      the ending position (not included)
 * @return  the extracted result sub data set
 */
void fDataSet_subdimset(const fDataSet *ds, int from, int to, fDataSet *subds);
void iDataSet_subdimset(const iDataSet *ds, int from, int to, iDataSet *subds);

/**
 *	\brief	load a part of vectors from a dataset file
 */
void fDataSet_load_part(const char *filename, int d, int from, int num, float *vec);
void iDataSet_load_part(const char *filename, int d, int from, int num, int *vec);

/**
 *	\brief 	load dataset of part of dimensions
 */
void fDataSet_load_partd(const char *filename, int d, int pd, int num, float *vec);
void iDataSet_load_partd(const char *filename, int d, int pd, int num, int *vec);


/***************** int-double / int-int pair [c style] ***********************/
typedef struct DoubleIndex
{
	int                 id;             // index
	double              val;            // value
	DoubleIndex(){}
	DoubleIndex(int _id, double _val): id(_id), val(_val){}
} DoubleIndex;

/// an int-int (id-value) pair
typedef struct IntIndex
{
	int                 id;
	int                 val;
	IntIndex(){}
	IntIndex(int _id, int _val): id(_id), val(_val){}
} IntIndex;

DoubleIndex *dilist_new(long n);
IntIndex *iilist_new(long n);
void dilist_unset(DoubleIndex *dilist);
void iilist_unset(IntIndex *iilist);

bool iicomp_asc(const IntIndex &va, const IntIndex &vb);
bool iicomp_des(const IntIndex &va, const IntIndex &vb);
bool dicomp_asc(const DoubleIndex &va, const DoubleIndex &vb);
bool dicomp_des(const DoubleIndex &va, const DoubleIndex &vb);


/// time diff between two timeval
long timediff(struct timeval begin, struct timeval end);

/// <summary>
/// squared Euclidean distance between two d-dim vectors
/// </summary>
/// @param	a	vector a
/// @param	b	vector b
/// @param	d	dimension d
double odistance_square(const float *a, const float *b, int d);

/// <summary>
/// Encludiean distance between two d-dim vectors
/// </summary>
/// @param
double odistance(const float *a, const float *b, int d);

/** \brief Inner product between two vectors
 */
float inner_product(float *a, float *b, int d);

/**
 *	\brief	linear scan base vectors, extract the kNN of q
 */
void linear_knn(float *vb, int nb, float *q, int d, int k, int *nn, float *dis);

/** \brief extract the blongingness of each data points to the clusters, and calculate the minimum distance between each member and the bound
 */
void extract_members(int *assign, int *nassign, int **member, int n, int ncenter);

/** \brief Compare two float number, return true when a > b
 */
bool f_bigger(float a, float b);

/**	\brief	judge whether a float value is zero */
bool f_iszero(float val);

/// common operation: min or max between two values
int i_min(int, int);
int i_max(int, int);
float f_min(float, float);
float f_max(float, float);

void ivec_abs(int *vec, long d);
void fvec_abs(float *vec, long d);
double ivec_fmean(const int *vec, long d);
long ivec_norm2sqr(const int *vec, long d);

int ivecs_read(const char *filename, int d, int n, int *data);


/** \brief Open file with given format
 * @param filename
 * @param format
 */
FILE *open_file(const char *filename, const char *format);

/**
 *	\brief 	make sure a folder exists
 *		if not exists, generate, assert error generate
 */
void makesure_folder(const char *folder);

/**
 *	\brief	check whether a folder exists
 */
bool folder_exists(const char *folder);

/**
 *	\brief	check whether a file exists
 */
bool file_exists(const char *filename);

/// <summary>
/// MergeSort: sort a set of DoubleIndex pairs in ascending order
/// </summary>
/// <param name="di">the original DoubleIndex pairs</param>
/// <param name="l">the starting index</param>
/// <param name="r">the ending index (included)</param>
void DI_MergeSort(DoubleIndex *di, int l, int r);

/// <summary>
/// merge the left and right on holding the right order
/// </summary>
/// <param name="di">DoubleIndex pairs</param>
/// <param name="l">left index</param>
/// <param name="m">middle index</param>
/// <param name="r">right index</param>
void DI_Merge(DoubleIndex *di, int l, int m, int r);




/* ---------------------------------------------------------------------------------
 *	the groundtruth block
 * --------------------------------------------------------------------------------- */

/** \brief Load the groundtruth
 * @param gtfile		the groundtruth file name
 * @param gtdisfile		the gt distance file name
 * @param dGT			how many groundtruths to load for one query
 * @param nq			the number of queries (q)
 * @param groundtruth	the groundtruth set (DoubleIndex)
 */
void load_groundtruth(const char *gtfile, const char *gtdisfile, int dGT, int nq, DoubleIndex *groundtruth);

/** \brief Calculate groundtruth for query in data
 * @param	data		data points
 * @param	query		the query point
 * @param 	n		number of points
 * @param	d		dimension
 * @param 	nq		number of queries
 * @param 	nk		number of returned nns
 * @param	groundtruth	the groundtruth vectors
 */
void calculate_groundtruth(float *data, float *query, int n, int d, int nq, int nk, DoubleIndex *groundtruth);

/**	\brief make up the gt distance for the groundtruth
 */
void fullfill_gtdistance(DoubleIndex *gt, int nq, int dGT, const float *data, const float *query, int d);




/* ---------------------------------------------------------------------------------
 *	the accuracy block: make sure ids and distances in knn and gt are prepared
 *	therefore, we set knn and gt be constant
 * --------------------------------------------------------------------------------- */

/** \brief the precision of knn results (indicating: how many real nk-nn have been retrieved)
 *	@param	knn		the given knn set
 *	@param	gt		the groundtruth set
 *	@param	nq		number of query
 *	@param	nk		number of nn
 *	@param	dGT		how many groundtruths for each query held in groundtruth
 *	@return	average precision
 */
float knn_precision(DoubleIndex **knn, const DoubleIndex *gt, int nq, int nk, int dGT);

/**
 *	\brief	calculate the ratio of knn results (average overall ratio)
 *	@param	dGT		how many knns a query holds in gt
 */
float knn_ratio(DoubleIndex **knn, const DoubleIndex *gt, int nq, int nk, int dGT, int verbose);

/**
 *	\brief	calculate the recall@k for the nearest neighbor
 *			i.e., the proportion of queries which the nn is ranked in the first k positions
 */
float knn_recall(DoubleIndex **knn, const DoubleIndex *gt, int nq, int nk, int dGT);

float knn_recall_at(DoubleIndex **knn, const DoubleIndex *gt, int nq, int nk, int dGT, int at);

/**
 *  the accuracy of the knn search results (refered to the groundtruth)
 *  note that dGT >= k
 */
void knn_accuracy(const int *gt, const float *gtdis, const int *knn, const float *knndis, int dGT, int k, int nq, float *ratio, float *recall, float *precision);


/**
 *	\brief figure the sign vector of a vector
 */
float *fvec_sign(const float *vec, int d);

/**
 *	\brief judge the parameter type
 */
PARA_TYPE get_para_type(const char *para);

/**
 *	\brief extract the parameter name from the raw parameter,
 *	noticed that we have known the parameter type, i.e. there is a '-' at the head as default
 */
void extract_para_name(const char *para, char *pname);

/**
 *	\brief report the performance and record them into files
 *	@param 	envstr 		running envirement string, put at the begining of the record
 */
void performance_report(DoubleIndex **knn, int nq, int k, const DoubleIndex *gt, int dGT, Cost cost, const char *envstr);


/**
 * convert an assign list to an inverted index
 * @param K     number of centroid
 *
 */
vector<vector<int> > list2inverted(vector<int> assign, int K);

/**
 * convert an inverted index to a assign list
 * @param n     total elements
 *
 */
vector<int> inverted2list(vector<vector<int> > ivt, int n);


/**
 * store knn into file
 */
void knn_into_file(const char *filename, DoubleIndex **knnset, int nq, int nk);
#endif
