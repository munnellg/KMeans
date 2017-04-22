#include "kmeans.h"

/* 
 * ===  FUNCTION  =============================================================
 *         Name:  euclidean_distance
 *  Description:  Computes the Euclidean distance between two points 
 *     represented by the input feature vectors. Euclidean distance is found
 *     by getting the sum of the square of the differences between each
 *     pair of features in the two vectors, then finding the square root
 *     of the difference.
 *
 *     In LaTeX: \sqrt{\sum^{len}_{i=0}{(a_i-b_i)^{2}}}
 * ============================================================================
 */
static float
euclidean_distance( float *a, float *b, int len ) {
    float sum=0;

    int i;
    for( i=0; i<len; i++ ) {
        sum += (a[i]-b[i]) * (a[i]-b[i]);
    }

    return sqrt(sum);
}

/* 
 * ===  FUNCTION  =============================================================
 *         Name:  compute_centroid
 *  Description:  
 * ============================================================================
 */
static void
compute_centroid( KMeans *km, int label, float *samples, int n_samples, int *labels ) {
    int i;
    int cnt = 0;

    for( i=0; i<n_samples; i++ ) {
        if(label == labels[i]) {
            cnt++;
            int j;
            for( j=0; j<km->n_features; j++ ) {
                km->centroids[label*km->n_features+j] += 
                    samples[i*km->n_features+j];
            }
        }
    }

    if(!cnt) {
        return;
    }

    for( i=0; i<km->n_features; i++ ) {
        km->centroids[label*km->n_features+i]/=cnt;
    }
}

/* 
 * ===  FUNCTION  =============================================================
 *         Name:  random_initial_centroids
 *  Description:  
 * ============================================================================
 */
static void
random_initial_centroids( KMeans *km, float *samples, int n_samples ) {
    int i,j;
    for( i=0; i<km->n_clusters; i++ ) {
        int s = rand()%n_samples;
        printf("Choose %d: ( %5.2f", s, samples[s*km->n_features]);
        for( j=1; j<km->n_features; j++ ) {
            printf(", %5.2f", samples[s*km->n_features+j]);
        }
        printf(" )\n");

        memcpy( 
            &km->centroids[i*km->n_features],
            &samples[s*km->n_features],
            sizeof(*samples) * km->n_features 
        );
    }
}

static void
kmeanspp_initial_centroids( KMeans *km, float *samples, int n_samples ) {

    /* choose the first centroid at random. We'll try to pick all other 
     * centriods as points which are furthest from this starting point */
    int s = rand()%n_samples;   
    memcpy( 
       &km->centroids[0],
       &samples[s*km->n_features],
       sizeof(*samples) * km->n_features 
    );
    printf("Choose %d: ( %5.2f", s, samples[s*km->n_features]);
    int x;
    for( x=1; x<km->n_features; x++ ) {
        printf(", %5.2f", samples[s*km->n_features+x]);
    }
    printf(" )\n");

    /* for all remaining clusters which must be chosen */
    int i;
    for( i=1; i<km->n_clusters; i++ ) {

        /* find the point which is maximally distant from all other points */
        float max_dist = 0;
        int j;
        for( j=0; j<n_samples; j++ ) {
            /* a point is only as good as its distance from the nearest 
             * centroid so find its classification based on currently chosen 
             * centroids and store the distance from that point */
            int k;
            float min_dist = euclidean_distance(
                        &km->centroids[0],
                        &samples[j*km->n_features],
                        km->n_features
                    ); 
            for( k=1; k<i; k++ ) {
                float d =  euclidean_distance(
                        &km->centroids[k*km->n_features],
                        &samples[j*km->n_features],
                        km->n_features
                    ); 
                if( d < min_dist ) {
                    min_dist = d;
                }
            }
            
            /* if this point is further away than any other point that we have
             * previously tested, then mark it as a candidate for a centroid */
            if( min_dist > max_dist ) {
                max_dist = min_dist;
                s = j;
            }
        }
        printf("Choose %d: ( %5.2f", s, samples[s*km->n_features]);
        for( j=1; j<km->n_features; j++ ) {
            printf(", %5.2f", samples[s*km->n_features+j]);
        }
        printf(" )\n");
        /* add our new point to the set of centroids */
        memcpy( 
            &km->centroids[i*km->n_features],
            &samples[s*km->n_features],
            sizeof(*samples) * km->n_features 
        );
    }
}

KMeans*
KMeans_new( int n_clusters, int n_features ) {
    KMeans *km;

    km = malloc( sizeof(KMeans) );

    /* km will be NULL if malloc failed. If malloc succeeded then we can
     * continue with initializing the struct */
    if(km) {
        KMeans_init(km, n_clusters, n_features);
        
        /* if the init function failed to allocate memory for the centroids, 
         * then km->centroids will be NULL. The allocation failed and this 
         * function should return NULL. Release the memory allocated so far 
         * (KMeans_free will handle the case where km points to valid memory 
         * but km->centroids is NULL, so we can just call that function */
        if(!km->centroids) {
            KMeans_free(km);
            km = NULL;
        }
    }

    /* either return the new struct or NULL if this function failed */
    return km;
}

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
KMeans_cluster( KMeans *km, float *samples, int n_samples, int n_iter ) {
    srand(time(NULL));
    /* not happy about the malloc here, but hey. Maybe get the user to pass a 
     * labels array? So we won't just train the classifier, we'll tell you 
     * where all your sample data was clustered? scikit-learn does this, so I 
     * don't think it would be too horrible a hack */
    int *labels = malloc( sizeof(int) * n_samples );
 
    /* choose initial centers */
    kmeanspp_initial_centroids(km, samples, n_samples);

    int stable=0;
    while(!stable) {
        int j;
        
        stable = 1;
        /* assign labels */
        for( j=0; j<n_samples; j++ ) {
            int l = KMeans_classify( km, &samples[j*km->n_features] );
            if( l != labels[j] ) {
                stable = 0;
                labels[j] = l;
            }
        }
        
        /* reassign cluster centers */
        memset( km->centroids, 0, sizeof(float)*km->n_features*km->n_clusters);
        for( j=0; j<km->n_clusters; j++ ) {
            compute_centroid( km, j, samples, n_samples, labels );
        }
    }

    /* free the hack! */
    free(labels);
}

int
KMeans_classify( KMeans *km, float sample[] ) {
    int label;
    float sim;

    /* just start by assigning to the first cluster. We'll replace it pretty
     * quickly if we're wrong */
    label=0;
    sim=euclidean_distance(&km->centroids[0], sample, km->n_features);

    int i;
    for( i=0; i<km->n_clusters; i++) {
        /* compute the distance between cluster i and the input sample */ 
        float d = euclidean_distance( 
                &km->centroids[i*km->n_features], /* centroid feature vector */
                sample,                           /* sample feature vector */
                km->n_features                    /* feature vector length */
            );

        /* if it is closer than the currently stored label, then update our 
         * values accordingly */
        if( d < sim ) {
            label=i;
            sim = d;
        }
    }

    /* return the assignment for this sample */
    return label;
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
