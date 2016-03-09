/**
 * \file common.h
 * Some common configs
 */

#ifndef COMMON_H
#define COMMON_H
#include <stdio.h>
#include <sys/time.h>
#include <string.h>

/// define two assertion formats
#define ASSERT(error_condition) { \
    if (error_condition) { \
        printf("ASSERT, LINE:%d, FILE:%s\n", __LINE__, __FILE__); \
        exit(EXIT_FAILURE); \
    } \
}

#define ASSERTINFO(error_condition, info) { \
    if (error_condition) { \
        printf("ASSERT, LINE:%d, FILE:%s\n", __LINE__, __FILE__); \
        printf("INFO: %s\n", info); \
        exit(EXIT_FAILURE); \
    } \
}

#define     INT_MAX 	0x7FFFFFFF			/// max signed integer
#define	    INT_DEFAULT	-1				/// a defult value of integer variables
#define     STR_LEN	255				/// default length of a char array
#define     FLOAT_MAX 9999999999.0			/// max float
#define     FLOAT_ZERO  1e-16				/// zero for float
#define     SHAKERANGE  10				/// in case the result knn not at the same position with groundtruth
#define     PAGESIZE    4096				/// page size: 4K
#define     PAGE_RAN_SE 10				/// 10 sequence pages equal 1 random page

#define     INDEX_PARA "index.para"			/// the name of an index file
#define     CLUSTER_POXFIX ".cluster"			/// the postfix of an cluster file

/// kmeans clustering relative
#define     CLUSTERING_NITER    100         	/// clustering iteration times
#define     CLUSTERING_NTHREAD  (count_cpu())   /// number of threads in clustering
#define     CLUSTERING_SEED     0           	/// random seed
#define     CLUSTERING_NREDO    1           	/// clustering run loops

/** \enum bool Indicating true or false */
typedef enum{false, true} bool;
/**
 *	\brief parameter type
 */
typedef enum {PNAME,PVALUE,PNULL} PARA_TYPE;

/// a DataSet class where the member points are float
typedef struct
{
	int 				n;					/// number of data points
	int 				d;					/// data dimension
	float				*data;				/// data points
} fDataSet;

/// an int-double pair
typedef struct
{
    int                 id;             // index
    double              val;            // value
} DoubleIndex;

/// initialize a float data set
void fDataSet_Init(fDataSet *ds, int n, int d);

/// reset a float data set
void fDataSet_Reset(fDataSet *ds);

/**
 * \brief extract a subset of the dataset, the range in each dimension is [from, to)
 * @time    2015-9-19 20:57:14
 * @param   ds      the dataset to be extracted
 * @param   from    the begin position (included)
 * @param   to      the ending position (not included)
 * @return  the extracted result sub data set
 */
void fDataSet_Subset(const fDataSet *ds, int from, int to, fDataSet *subds);


/// time diff between two timeval
long timediff(struct timeval begin, struct timeval end);

/// <summary>
/// squared Euclidean distance between two d-dim vectors
/// </summary>
/// @param	a	vector a
/// @param	b	vector b
/// @param	d	dimension d
float odistance_square(const float *a, const float *b, int d);

/// <summary>
/// Encludiean distance between two d-dim vectors
/// </summary>
/// @param
float odistance(const float *a, const float *b, int d);

/** \brief Inner product between two vectors
 */
float inner_product(float *a, float *b, int d);

/** \brief extract the blongingness of each data points to the clusters, and calculate the minimum distance between each member and the bound
 */
void extract_members(int *assign, int *nassign, int **member, int n, int ncenter);

/** \brief Compare two float number, return true when a > b
 */
int f_bigger(float a, float b);

/** \brief Open file with given format
 * @param filename
 * @param format
 */
FILE *open_file(const char *filename, const char *format);

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

/** \brief Load the groundtruth
 * @param filename		the groundtruth file name
 * @param dGT			how many groundtruths to load for one query
 * @param nq			the number of queries (q)
 * @param groundtruth	the groundtruth set (DoubleIndex)
 */
void load_groundtruth(char *filename, int dGT, int nq, DoubleIndex *groundtruth);

/** \brief Verify the accuracy of knn
 * @param	knnset		the given knn set
 * @param	groundtruth	the groundtruth set
 * @param	nq			number of query
 * @param	nk			number of nn
 * @param	dGT		how many groundtruths for each query held in groundtruth
 */
float verify_knn(DoubleIndex **knnset, DoubleIndex *groundtruth, int nq, int nk, int dGT);

/** \brief Match knn with groundtruth 
 * @param 	knn		the knn set to be evaluated
 * @param 	gt		groundtruth
 * @param	nq		number of queries
 * @param 	nk 		number of nns
 * @return	the average match rate
 */
float knn_match(DoubleIndex *knn, DoubleIndex *gt, int nq, int nk);


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

/**
 *	\brief judge the parameter type
 */
PARA_TYPE get_para_type(const char *para);

/**
 *	\brief extract the parameter name from the raw parameter,
 *	noticed that we have known the parameter type, i.e. there is a '-' at the head as default 
 */
void extract_para_name(const char *para, char *pname);

#endif
