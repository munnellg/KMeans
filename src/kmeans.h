/*
 * ============================================================================
 *
 *       Filename:  kmeans.h
 *
 *    Description:  K-Means is a clustering algorithm which takes as input 
 *       vectors of features representing data and attempts to establish groups 
 *       into which the data may be categorized.
 *
 *       This is achieved by iteratively selecting representative center points 
 *       (means) for each cluster of data. By repeatedly labelling data 
 *       according to these means and then adjusting the center point based on 
 *       cluster contents we gradually arrive at a suitable (or so we hope)  
 *       mean for each category.
 *
 *       In this implementation, a feature vector's similarity to a mean is 
 *       based on eulcidean distance, which is a fairly standard metric for 
 *       this algorithm.
 *
 *       K-Means is designed to work on continuous or interval datai. That is 
 *       to say, feature vectors must be comprised of numerical values where 
 *       the difference between two values is meaningful. For example, 
 *       temperature is a suitable feature. An enum representing types of 
 *       gearbox in a car is not.
 *
 *       GIGO should always be borne in mind
 *
 *       In the ensuing documentation, I use the following terminology when 
 *       discussing K-Means:
 *       + data:    a collection of items which we are attempting to categorise
 *                  into groups. For example, the pixels of an image file
 *                  based on their colour
 *
 *       + feature: an individual property of the data being analysed. If we
 *                  are clustering pixels based on colour, for example, then a 
 *                  single feature could be the red channel of a single pixel. 
 *                  A feature must be an interval value
 *
 *       + feature vector: a group of features representing a single datum in 
 *                  the input data. For example the red, blue and green 
 *                  channels for a single pixel
 *
 *       + cluster: a label which can be applied to a number of data items that
 *                  are believed to be similar. For example all red pixels of
 *                  varying shades might belong to a single cluster
 *
 *       I will use the terms cluster, group and category interchangeably
 *
 *        Version:  1.0
 *        Created:  22/04/17 14:21:55
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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

/* A K-means model. Can be trained and then used for classification */
typedef struct KMeans {
    int n_clusters;
    int n_features;
    float *centroids;
} KMeans;

KMeans *KMeans_new ( int n_clusters, int n_features );
void KMeans_init ( KMeans *kmeans, int n_clusters, int n_features );
void KMeans_cluster ( KMeans *kmeans, float *samples, int n_features, int n_iter );
int KMeans_classify ( KMeans *kmeans, float sample[] );
void KMeans_free ( KMeans *kmeans );

#endif   /* ----- #ifndef KMEANS_INC  ----- */
