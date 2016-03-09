/**
 *  an declaration of basic data structs and operations for VA-File
 */
#ifndef VAFILE_H
#define VAFILE_H
#include "common.h"
#include "heap.h"
#include "cost.h"

/// Data element in VA-File
typedef struct
{
    int         pid;            // id of point
    int         *code;          // code array of the point
    float       ub;             // upper bound
    float       lb;             // lower bound
}VAElem;

/// config of VA-File
typedef struct
{
    int         nbit;           // bit per dimension
    int         n;
    int         d;
    float       *bound;         // boundaries of all dimension
    float       *inter_len;     // inverval length of each dimension
    int         *code;          // code for all data points

	/* some working variables */
	float		**inter_alldiff_len_s;	// square lengths of all possible internal diff in each dimension: diff=0;1,2,...2^b-1;
}VAConfig;

/// initialize a VAConfig
void VA_Init(VAConfig *cfg, int bi);
/// destroy a VAConfig structure
void VA_Destroy(VAConfig *cfg);
/// dealing with a dataset, figure out the cfgrations
void VA_Config(VAConfig *cfg, const fDataSet *ds);
/// generate a va-file code for all data points in a dataset
void VA_Encode(VAConfig *cfg, const fDataSet *ds);
/// write a va-file index structure into files
void VA_IndexIntoFile(const VAConfig *cfg, const char *indexfolder, const char *dsname);
/** \brief check if exists index (i.e., va codes)
 *	return status
 */
bool VA_CheckExistsIndex(const char *indexfolder, const char *dsname, int b);
/// check and load a va-file index structure from files
bool VA_IndexFromFile(VAConfig *cfg, const char *indexfolder, const char *dsname, int b);
/// search knn via vafile
void VA_Search(const VAConfig *cfg, const fDataSet *qds, int nk, const char *folder, DoubleIndex **knn, Cost *cost);
/** \brief prepare square interval lengths in each dimension */
void VA_SquareInternalDiffLength(VAConfig *cfg);

/************************************* basic operations ************************************/
/// figure out the lower and upper distance between query and the i-th code
void VA_LowerBoundDistanceAll(const VAConfig *cfg, const float *query, float **sdisqb, const float **bound, VAElem *elem);
void VA_BoundDistanceAll(const VAConfig *cfg, const float *query, float **sdisqb, float **bound, VAElem *elem);
void VA_LowerBoundDistance(const VAConfig *cfg, const float *query, float **sdisqb, const float **bound, int i, VAElem *elem);
void VA_UpperBoundDistance(const VAConfig *cfg, const float *query, float **sdisqb, const float **bound, int i, VAElem *elem);
/**
 *	\brief calculate the square lower bound distance between two codes
 *	@param	cfg		the VAConfig object
 *	@param	ione	the first code's id
 *	@param	iother	the other code's id
 *	@return	square ldb
 */
float VA_LowerBoundDistance_Codes_S(const VAConfig *cfg, int ione, int iother);
/** brief [to be realized] square upper bound distance and square lower + upper distance bounds 
 *	VA_UpperBoundDistance_Codes_S()
 *	VA_LUBoundsDistance_Codes_S()
 */
#endif
