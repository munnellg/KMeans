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
#include	<time.h>

/* A K-means model. Can be trained and then used for classification */
typedef struct KMeans {
    int n_clusters;
    int n_features;
    float *centroids;
} KMeans;

KMeans *KMeans_new ( int n_clusters, int n_features );

void KMeans_init ( KMeans *kmeans, int n_clusters, int n_features );

void KMeans_cluster ( KMeans *kmeans, float *samples, int n_features );

int KMeans_classify ( KMeans *kmeans, float *sample );

void KMeans_free ( KMeans *kmeans );

float euclidean_distance ( float *a, float *b, int len );

int find_closest ( float *points, float *sample, int dims, int n_points, 
        float *dist_pointer );

void compute_distances ( float *points, float *sample, float *distances,
        int dims, int n_points );

void lloyd_init_centroids ( float *centroids, float *samples, int dims, 
        int n_centroids, int n_samples );

void kmpp_init_centroids ( float *centroids, float *samples, int dims, 
        int n_centroids, int n_samples );

void compute_cluster_sizes ( int *labels, int *counts, int n_centroids, 
        int n_samples );

void recompute_centroids ( float *centroids, float *samples, int dims,
        int n_centroids, int n_samples, int *labels, int *counts );

void cluster_kmeans ( float *centroids, float *samples, int dims,
        int n_centroids, int n_samples, int *labels );

#endif   /* ----- #ifndef KMEANS_INC  ----- */
