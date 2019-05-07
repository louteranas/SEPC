#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <SDL2/SDL.h>

#include "stream_common.h"
#include "oggstream.h"
#include "synchro.h"


pthread_t idDraw;

int main(int argc, char *argv[]) {
    int res;
    void** threadReturn = NULL; // je ne comprends pas ce que ca doit prendre mais c'est comme ça que c'est utilisé pthread_join sur internet

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
    pthread_join(idVorbis, threadReturn);
    // 1 seconde de garde pour le son,
    sleep(1);
    

    // tuer les deux threads videos si ils sont bloqués
    pthread_cancel(idTheora);
    pthread_cancel(idDraw);

    // attendre les 2 threads videos
    pthread_join(idTheora, threadReturn);
    pthread_join(idDraw, threadReturn);
    
    exit(EXIT_SUCCESS);    
}
