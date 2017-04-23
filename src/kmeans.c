/*
 * ============================================================================
 *
 *       Filename:  kmeans.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  22/04/17 21:09:11
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Gary Munnelly (gm), munnellg@tcd.ie
 *        Company:  Adapt Centre, Trinity College Dublin
 *
 * ============================================================================
 */

#include	"kmeans.h"
#include	<stdio.h>

void
KMeans_init( KMeans *km, int n_clusters, int n_features ) {
    if(km) {
        /* pretty self explanatory. Note that km-centroids is one dimensional 
         * to make it easier to allocate and release memory. */ 
        km->n_clusters = n_clusters;
        km->n_features = n_features;
        km->centroids = malloc(sizeof(float)*n_clusters*n_features);
    }
}

void
KMeans_cluster( KMeans *km, float *samples, int n_samples ) {
    
    /* not happy about the malloc here, but hey. Maybe get the user to pass a 
     * labels array? So we won't just train the classifier, we'll tell you 
     * where all your sample data was clustered? scikit-learn does this, so I 
     * don't think it would be too horrible a hack */
    int *labels = malloc( sizeof(int) * n_samples );
 
    cluster_lloyd( km->centroids, samples, km->n_features, km->n_clusters,
            n_samples, labels);

    /* free the hack! */
    free(labels);
}

int
KMeans_classify( KMeans *km, float *sample ) {
    
    /* return the assignment for this sample */
    return find_closest( km->centroids, sample, km->n_features, km->n_clusters, 
            NULL );
}

void
KMeans_free( KMeans *km ) {
    /* always guard against freeing memory that has not been allocated */
    if(km) {
        if(km->centroids) {
            free(km->centroids);
        }
        free(km->centroids);
    }
}
float
euclidean_distance ( float *a, float *b, int dims )
{
    /* test for NULL pointers and return -1 in the event of invalid input */
    if(!a || !b) {
        return -1;
    }

    int i;
    float sum = 0;
    for( i=0; i<dims; i++ ) {
        sum += (a[i]-b[i])*(a[i]-b[i]);
    }

    return sqrt(sum);
}		/* -----  end of function euclidean_distance  ----- */


int
find_closest ( float *points, float *sample, int dims, int n_points, 
        float *dist_pointer )
{
    /* do some error checking just to be safe */
    if(!points || !sample || dims < 1 || n_points < 1 ) {
        return -1;
    }

    /* initialize with the assumption that the first point is the closest */
    int closest = 0;
    float dummy_dist;
    float *distance = (dist_pointer)? dist_pointer : &dummy_dist;
    *distance = euclidean_distance( points, sample, dims );
    /* now check all other points */ 
    int i;
    for( i=1; i<n_points; i++ ) {

        float d = euclidean_distance( &points[i*dims], sample, dims );
        
        /* if we find a point that is closer, update our minimum distance and 
         * the id of the closest point */
        if ( d < *distance ) {
            *distance = d;
            closest = i;
        }
    }

    return closest;
}		/* -----  end of function find_closest  ----- */


void
compute_distances ( float *points, float *sample, float *distances, int dims,
        int n_points )
{
    /* simple error checking */
    if( !points || !sample || !distances || dims < 1 || n_points < 1 ) {
        return;
    }

    /* get all the distances */
    int i;
    for( i=0; i<n_points; i++ ) {
        distances[i] = euclidean_distance( &points[dims*i], sample, dims );
    }
}		/* -----  end of function compute_distances  ----- */


void
lloyd_init_centroids ( float *centroids, float *samples, int dims, 
        int n_centroids, int n_samples )
{
    /* error checking stuff */
    if( !centroids || !samples || dims < 1 || 
            n_centroids < 1 || n_samples < 1 ) {
        return;
    }

    int i;
    for( i=0; i<n_centroids; i++ ) {
        int s = rand()%n_samples;
        memcpy( &centroids[i*dims], &samples[s*dims], sizeof(float) * dims ); 
    }
}		/* -----  end of function lloyd_init_centroids  ----- */


void
kmpp_init_centroids ( float *centroids, float *samples, int dims, 
        int n_centroids, int n_samples )
{
    int s = rand()%n_samples;
    memcpy ( centroids, &samples[s*dims], sizeof(float) * dims );
    int x;
    printf("Choosing %d: ( %5.2f", s, samples[s*dims] );
    for( x=1; x<dims; x++ ) {
        printf(", %5.2f", samples[s*dims + x] );
    }
    printf(" )\n");

    int i;
    for( i=1; i<n_centroids; i++ ) {
        float dist = 0;
        
        /* compute cumulative square distance between each sample point and its
         * closest centroid of those we've selected thus far */
        int j;
        for( j=0; j<n_samples; j++ ) {
            float d = 0;
            find_closest( centroids, &samples[j*dims], dims, i, &d );
            dist += d*d;
            printf("%f %f\n", d, dist);
        }
        
        printf("%5.2f\n", dist);
        float p = rand()*dist;
        for( j=0; j<n_samples; j++ ) {
            float d = 0;
            find_closest( centroids, &samples[j*dims], dims, i, &d );
            p -= d*d;
            if( p<= 0 ) {
                break;
            }
        }
        
        printf("Choosing %d: ( %5.2f", j, samples[j*dims] );
        for( x=1; x<dims; x++ ) {
            printf(", %5.2f", samples[j*dims + x] );
        }
        printf(" )\n");
        memcpy( &centroids[i*dims], &samples[j*dims], sizeof(float) * dims);
    }
}		/* -----  end of function kmpp_init_centroids  ----- */


void
count_cluster_members ( int *labels, int *counts, int n_centroids, 
        int n_samples )
{
    memset( counts, 0, sizeof(float)*n_centroids);
    int i;
    for( i=0; i<n_samples; i++ ) {
        counts[labels[i]]++;
    }
}		/* -----  end of function compute_cluster_sizes  ----- */


void
recompute_centroids ( float *centroids, float *samples, int dims,
        int n_centroids, int n_samples, int *labels, int *counts )
{
    count_cluster_members( labels, counts, n_centroids, n_samples );

    memset( centroids, 0, sizeof(float)*dims*n_centroids );

    /* sum of all samples belonging to each cluster */
    int i, j;
    for( i=0; i<n_samples; i++ ) {
        for( j=0; j<dims; j++ ) {
            centroids[labels[i]*dims + j] += samples[i*dims + j];
        }
    }

    /* divide by number of samples in each cluster */
    for( i=0; i<n_centroids; i++ ) {
        for( j=0; j<dims; j++ ) {
            centroids[i*dims + j] /= ((counts[i])? counts[i] : 1);
        }
    }
}		/* -----  end of function recompute_centroids  ----- */

int
reassign_clusters ( float *centroids, float *samples, int dims,
        int n_centroids, int n_samples, int *labels )
{
    int i, n_changed = 0;
    for( i=0; i<n_samples; i++ ) {
        int l = find_closest( centroids, &samples[i*dims], dims,
                    n_centroids, NULL );
            
        if( labels[i] != l ) {
            labels[i] = l;
            n_changed++;
        }
    }
    
    return n_changed;
}		/* -----  end of function reassign_clusters  ----- */

void
cluster_kmeans ( float *centroids, float *samples, int dims, int n_centroids, 
        int n_samples, int *labels  )
{
    int *counts = malloc( sizeof(int)*n_centroids );

    int reassigned = n_samples;
    while( reassigned > 0 ) {
        reassigned = reassign_clusters( centroids, samples, dims, n_centroids,
               n_samples, labels ) == 0; 

        recompute_centroids( centroids, samples, dims, n_centroids, n_samples,
                labels, counts );
    }

    free(counts);
}		/* -----  end of function cluster_kmeans  ----- */


void
cluster_lloyd ( float *centroids, float *samples, int dims,
        int n_centroids, int n_samples, int *labels  )
{
    lloyd_init_centroids( centroids, samples, dims, n_centroids, n_samples );
    cluster_kmeans( centroids, samples, dims, n_centroids, n_samples, labels );
}		/* -----  end of function cluster_lloyd  ----- */


void
cluster_kmpp ( float *centroids, float *samples, int dims,
        int n_centroids, int n_samples, int *labels  )
{
    kmpp_init_centroids( centroids, samples, dims, n_centroids, n_samples );
    cluster_kmeans( centroids, samples, dims, n_centroids, n_samples, labels );
}		/* -----  end of function cluster_kmpp  ----- */
