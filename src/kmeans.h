/*
 * ============================================================================
 *
 *       Filename:  kmeans.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  22/04/17 21:08:14
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Gary Munnelly (gm), munnellg@tcd.ie
 *        Company:  Adapt Centre, Trinity College Dublin
 *
 * ============================================================================
 */

#ifndef  KMEANS_INC
#define  KMEANS_INC

#include	<math.h>
#include	<stdlib.h>
#include	<string.h>

/* A K-means model. Can be trained and then used for classification */
typedef struct KMeans {
    int n_clusters;
    int n_features;
    double *centroids;
} KMeans;

KMeans *KMeans_new ( int n_clusters, int n_features );

void KMeans_init ( KMeans *kmeans, int n_clusters, int n_features );

void KMeans_cluster ( KMeans *kmeans, double *samples, int n_samples );

int KMeans_classify ( KMeans *kmeans, double *sample );

void KMeans_free ( KMeans *kmeans );

double euclidean_distance ( double *a, double *b, int len );

int find_closest ( double *points, double *sample, int dims, int n_points, 
        double *dist_pointer );

void compute_distances ( double *points, double *sample, double *distances,
        int dims, int n_points );

void lloyd_init_centroids ( double *centroids, double *samples, int dims, 
        int n_centroids, int n_samples );

void kmpp_init_centroids ( double *centroids, double *samples, int dims, 
        int n_centroids, int n_samples );

void count_cluster_members ( int *labels, int *counts, int n_centroids, 
        int n_samples );

void recompute_centroids ( double *centroids, double *samples, int dims,
        int n_centroids, int n_samples, int *labels, int *counts );

int reassign_clusters ( double *centroids, double *samples, int dims,
        int n_centroids, int n_samples, int *labels, int *counts );

void cluster_kmeans ( double *centroids, double *samples, int dims,
        int n_centroids, int n_samples, int *labels );

void cluster_lloyd ( double *centroids, double *samples, int dims,
        int n_centroids, int n_samples, int *labels );

void cluster_kmpp ( double *centroids, double *samples, int dims,
        int n_centroids, int n_samples, int *labels );

#endif   /* ----- #ifndef KMEANS_INC  ----- */
