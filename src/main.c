#include "kmeans.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>

#define N_CLUSTERS 2
#define N_FEATURES 3

const char* DISPLAY_NAMES[] = {
	"Original",
	"Threshold",
};

typedef enum {
	SHOW_ORIGINAL,
	SHOW_THRESHOLD
} DisplayMode;

struct Segmenter {
    KMeans cluster;
	SDL_Surface* image;
	SDL_Surface* threshold;
};

struct Display {
	SDL_Window* window;
	SDL_Renderer* renderer;
};
	
struct ImageProcessor {
	DisplayMode display_mode;
	struct Segmenter* segmenter;
	struct Display* display;
};

void die( struct ImageProcessor* ip, const char* message );
void initialize_sdl( void );
void handle_events( struct ImageProcessor* ip, int *quit );
void terminate_sdl( void );
Uint32 get_pixel( SDL_Surface *surface, int x, int y );
void set_pixel( SDL_Surface *surface, int x, int y, Uint32 pixel );
Uint32 to_greyscale( SDL_PixelFormat* format, Uint32 r, Uint32 g, Uint32 b, Uint32 a );
void explode( SDL_PixelFormat* format, Uint32 pixel, Uint32* r, Uint32* g, Uint32* b, Uint32* a );
Uint32 compress( SDL_PixelFormat* format, Uint32 r, Uint32 g, Uint32 b, Uint32 a );

struct ImageProcessor* ImageProcessor_create( void );
void ImageProcessor_load( struct ImageProcessor* ip, char* filename );
void ImageProcessor_set_display_mode ( struct ImageProcessor* ip, DisplayMode dm );
void ImageProcessor_update_title ( struct ImageProcessor* ip );
void ImageProcessor_detect( struct ImageProcessor* ip );
void ImageProcessor_show( struct ImageProcessor* ip );
void ImageProcessor_destroy( struct ImageProcessor* ip );

struct Segmenter* Segmenter_create( void );
void Segmenter_threshold( struct Segmenter* s );
void Segmenter_destroy( struct Segmenter* ed );

struct Display* Display_create( void );
void Display_destroy( struct Display *d );

int main( int argc, char *argv[] ) {
	
	if( argc < 2 ) {
		die(NULL, "USAGE: kmeans <image>");
	}

	initialize_sdl();

	char* filename = argv[1];
	struct ImageProcessor* ip = ImageProcessor_create( );
    
    srand(time(NULL));
	
    ImageProcessor_load(ip, filename);
	ImageProcessor_detect(ip);
	ImageProcessor_update_title ( ip );
	ImageProcessor_show(ip);
	
	int quit = 0;
	while(!quit) {
		handle_events(ip, &quit);
	}
	
	ImageProcessor_destroy( ip );
	terminate_sdl();
	
	return EXIT_SUCCESS;
}

void die( struct ImageProcessor* ip, const char* message ) {
	
	printf("ERROR: %s\n", message);
	
	ImageProcessor_destroy(ip);
	terminate_sdl();
	exit(1);
}

void initialize_sdl( void ) {
	/* only need to set up the display elements. We're not doing
	 * anything fancy with sound effects or music etc. */
	if( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		die( NULL, SDL_GetError() );
	}

	/* going to allow the user to load as many different image types as
	 * possible */
	if( IMG_Init(IMG_INIT_TIF | IMG_INIT_PNG | IMG_INIT_JPG) < 0 ) {
		die( NULL, IMG_GetError() );
	}
}

void handle_events ( struct ImageProcessor* ip, int *quit ) {
	SDL_Event e;
	SDL_Scancode key;
	
	while( SDL_PollEvent(&e) ) {
		switch(e.type) {
		case SDL_QUIT:
			*quit = 1;
			break;
		case SDL_KEYDOWN:
			key = e.key.keysym.sym;
			if(key == 'o' || key == 't') {
				ImageProcessor_set_display_mode( ip, (key == 'o')? SHOW_ORIGINAL : SHOW_THRESHOLD );
				ImageProcessor_update_title ( ip );
				ImageProcessor_show( ip );
            }
			break;
		default:
			break;
		}
	}
}

void terminate_sdl ( void ) {
	IMG_Quit();
	SDL_Quit();
}

Uint32 get_pixel( SDL_Surface *surface, int x, int y ) {
	Uint32 *target_pixel = (Uint32 *) (surface->pixels + (y * surface->pitch + x * surface->format->BytesPerPixel));
	return *target_pixel;
}

void set_pixel( SDL_Surface *surface, int x, int y, Uint32 pixel ) {
	Uint32 *target_pixel = (Uint32 *) (surface->pixels + (y * surface->pitch + x * surface->format->BytesPerPixel));
	*target_pixel = pixel;
}

Uint32 to_greyscale( SDL_PixelFormat* format, Uint32 r, Uint32 g, Uint32 b, Uint32 a ) {
	return (r+g+b)/3;
}

void explode( SDL_PixelFormat* format, Uint32 pixel, Uint32* r, Uint32* g, Uint32* b, Uint32* a ) {
	*r = (pixel & format->Rmask) >> format->Rshift;
	*g = (pixel & format->Gmask) >> format->Gshift;
	*b = (pixel & format->Bmask) >> format->Bshift;
	*a = (pixel & format->Amask) >> format->Ashift;
}

Uint32 compress( SDL_PixelFormat* format, Uint32 r, Uint32 g, Uint32 b, Uint32 a ) {
	Uint32 pixel;
	pixel = 0;

	pixel |= ( r << format->Rshift );
	pixel |= ( g << format->Gshift );
	pixel |= ( b << format->Bshift );
	pixel |= ( a << format->Ashift );
	
	return pixel;
}

struct ImageProcessor* ImageProcessor_create( void ) {
	struct ImageProcessor* ip = malloc(sizeof(struct ImageProcessor));
	if(!ip) {
		die(ip, "Memory error");
	}

	ip->segmenter = Segmenter_create( );	
	if(!ip->segmenter) {
		die(ip, "Memory error");
	}
	
	ip->display = Display_create();
	if(!ip->display) {
		die(ip, "Memory error");
	}
	
	return ip;
}

void ImageProcessor_set_display_mode ( struct ImageProcessor* ip, DisplayMode dm ) {
	ip->display_mode = dm;
}

void ImageProcessor_update_title ( struct ImageProcessor* ip ) {
	char buf[128] = {0};

	sprintf(buf, "%s", DISPLAY_NAMES[ip->display_mode]);

	SDL_SetWindowTitle(ip->display->window, buf);
}

void ImageProcessor_load( struct ImageProcessor* ip, char* filename ) {
	ip->segmenter->image = IMG_Load(filename);
	if(!ip->segmenter->image) {
		die(ip, IMG_GetError());
	}
	
	SDL_Surface* image = ip->segmenter->image;
	ip->segmenter->threshold = SDL_CreateRGBSurface( 0,
												 image->w,
												 image->h,
												 32,
												 image->format->Rmask,
												 image->format->Gmask,
												 image->format->Bmask,
												 image->format->Amask	); 
	if(!ip->segmenter->threshold) {
		die(ip, SDL_GetError());
	}

	if( SDL_CreateWindowAndRenderer(image->w, image->h, 0,
						&ip->display->window, &ip->display->renderer) < 0 ) {
		die(ip, SDL_GetError());
	}
	
	ImageProcessor_set_display_mode( ip, SHOW_THRESHOLD );
}

void ImageProcessor_detect( struct ImageProcessor* ip ) {

    Segmenter_threshold(ip->segmenter);
}

void ImageProcessor_show( struct ImageProcessor* ip ) {
	SDL_Surface *img;
	
	switch(ip->display_mode) {
	case SHOW_THRESHOLD:
		img = ip->segmenter->threshold;
		break;
	case SHOW_ORIGINAL:
		img = ip->segmenter->image;
		break;
	default:
		return;
	}

	SDL_Texture *texture = SDL_CreateTextureFromSurface(ip->display->renderer, img);

	SDL_RenderClear(ip->display->renderer); 
	SDL_RenderCopy(ip->display->renderer, texture, NULL, NULL);
	SDL_RenderPresent(ip->display->renderer);
	
	SDL_DestroyTexture(texture);
}

void ImageProcessor_destroy( struct ImageProcessor* ip ) {
	if(ip) {
		/* destroy the struct that identifies edges in image */
		Segmenter_destroy(ip->segmenter);

		/* tear down the window and rendering structs */
		Display_destroy( ip->display );
		
		free(ip);
	}
}

struct Segmenter* Segmenter_create( void ) {
	struct Segmenter* s = malloc(sizeof( struct Segmenter ));

	if(s) {
		s->threshold = NULL;
		s->image = NULL;
		KMeans_init(&s->cluster, N_CLUSTERS, N_FEATURES);
	}

	return s;
}

void Segmenter_threshold( struct Segmenter* s ) {
    int i, x, y;
	Uint32 pixel, r,g,b,a;
	double* features;

	features = malloc( sizeof(double) * s->image->w * s->image->h * N_FEATURES );

	if(!features) {
		printf("Unable to allocate memory for computing features!\n");
		return;
	}
	
	/* clear the edge image */
	SDL_FillRect(s->threshold, NULL, 0x000000);

	/* compute the gradient for each point on the image */
	for( i=0; i<s->image->h*s->image->w; i++ ) {
        pixel = get_pixel(s->image, i%s->image->w, i/s->image->w);
        explode( s->image->format, pixel, &r, &g, &b, &a);
        features[i*N_FEATURES] = (float)r;
        features[i*N_FEATURES+1] = (float)g;
        features[i*N_FEATURES+2] = (float)b;
	}

    KMeans_cluster( &s->cluster, features, s->image->w*s->image->h );

    for( i=0; i<s->cluster.n_clusters; i++ ) {
        printf("%d ", to_greyscale(s->image->format, 
                    s->cluster.centroids[i*s->cluster.n_features],
                    s->cluster.centroids[i*s->cluster.n_features + 1],
                    s->cluster.centroids[i*s->cluster.n_features + 2],
                    0
                )
            );
    }

    printf("%f\n", average_silhouette( s->cluster.centroids, features, 
                s->image->w*s->image->h, s->cluster.n_features, 
                s->cluster.n_clusters ) );

	/* compute the gradient for each point on the image */
	for( y=0; y<s->image->h; y++ ) {
		for( x=0; x<s->image->w; x++ ) {
            
            pixel = get_pixel(s->image, x, y);
            explode( s->image->format, pixel, &r, &g, &b, &a);

            features[0] = (float)r;
            features[1] = (float)g;
            features[2] = (float)b;
 
            int label = KMeans_classify( &s->cluster, features );

            r = s->cluster.centroids[label*N_FEATURES];
            g = s->cluster.centroids[label*N_FEATURES+1];
            b = s->cluster.centroids[label*N_FEATURES+2];
            pixel = compress( s->threshold->format, r, g, b, 255 );
            set_pixel( s->threshold, x, y, pixel );
		}
	}

    free(features);
}

void Segmenter_destroy( struct Segmenter* s ) {
	if(s) {
		/* don't need to check if image or edges are null. SDL will do
		 * that for us */
		SDL_FreeSurface(s->image);
		SDL_FreeSurface(s->threshold);
		free(s);
	}
}

struct Display* Display_create( void ) {
	struct Display* d = malloc(sizeof(struct Display));
	if(d) {
		d->window = NULL;
		d->renderer = NULL;
	}
	return d;
}

void Display_destroy( struct Display *d ) {
	if(d) {
		/* SDL does handle these NULL checks, but I've noticed that
		 * SDL_DestroyRenderer and SDL_DestroyWindow cause memory leaks,
		 * so I'll only call them if I need to. */
		if(d->renderer) {
			SDL_DestroyRenderer(d->renderer);
		}
		if(d->window) {
			SDL_DestroyWindow(d->window);
		}
			
		free(d);
	}
}
