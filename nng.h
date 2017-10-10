/**
 *  Declarations of some operations on (g-Nearest Neighbor Graph, g-NNG or g-nng)
 *  specially for matlab
 */
#ifndef NNG_H
#define NNG_H

/* node status */
#define   CleanNode         0       // node has not been visited
#define   CandidateNode     1       // node in the queue
#define   ExpandedNode      2       // node has been expanded
#define   CastNode          3       // node has been cast

/**
 *  for [one] given query, converge to its knns on NNG with init candidates
 *  @param    v       the base dataset
 *  @param    n       base vector number
 *  @param    d       dimensionality
 *  @param    q       the query point,        [d,1]
 *  @param    cand    the candidate set
 *  @param    nc      candidate number
 *  @param    G       the nng
 *  @param    g       neighbor number of nng
 *  @param    k       [k]-nn
 *
 *  @return   knn     knn ids             [k,nq], int
 *  @return   knndis  knn distances       [k,nq], float
 *  @return   status  status of each point
 */
void converge_to_knn( const float *v, int n, int d,
                      const float *q,
                      const int *cand, int nc,
                      const int *G, int g,
                      int k,
                      int *knn, float *knndis, int *status);
/**
 *  for [one] given query, converge to its knns on NNG with init candidates
 *	desp:
 * 		for each candidate, converge to its best
 */
void converge_to_knn_one_by_one( const float *v, int n, int d,
                      const float *q,
                      const int *cand, int nc,
                      const int *G, int g,
                      int k,
                      int *knn, float *knndis, int *status);

#endif
