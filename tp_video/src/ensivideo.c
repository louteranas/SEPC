#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <SDL2/SDL.h>

#include "stream_common.h"
#include "oggstream.h"


int main(int argc, char *argv[]) {
    int res;

    if (argc != 2) {
	fprintf(stderr, "Usage: %s FILE", argv[0]);
	exit(EXIT_FAILURE);
    }
    assert(argc == 2);


    // Initialisation de la SDL
    res = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_EVENTS);
    atexit(SDL_Quit);
    assert(res == 0);
    
    // start the two stream readers
    /* on intialise les id des deux threads */ 
    pthread_t idTheora, idVorbis;
    /*on crée les threads */
    pthread_create(&idTheora, NULL, theoraStreamReader, (void *)argv[1]);
    pthread_create(&idVorbis, NULL, vorbisStreamReader, (void *)argv[1]);

    
    // wait audio thread

    // 1 seconde de garde pour le son,
    sleep(1);
    pthread_cancel(idTheora);
    pthread_cancel(idVorbis);

    // tuer les deux threads videos si ils sont bloqués

    // attendre les 2 threads videos

    
    exit(EXIT_SUCCESS);    
}
