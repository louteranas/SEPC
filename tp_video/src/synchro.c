#include "synchro.h"
#include "ensitheora.h"


bool fini;
bool gotDimension = false;
bool allReady = false;
bool enConsomation = false;
bool enDeposition = false;
int textures = 0;


/* les variables pour la synchro, ici */
pthread_mutex_t windowLock;
pthread_mutex_t texLock;
pthread_cond_t condDimension;
pthread_cond_t condReady;
pthread_cond_t peutConsommer;
pthread_cond_t peutDeposer;


/* l'implantation des fonctions de synchro ici */
void envoiTailleFenetre(th_ycbcr_buffer buffer) {
    pthread_mutex_lock(&windowLock);
    windowsx = buffer->width;
    windowsy = buffer->height;
    gotDimension = true;
    pthread_cond_signal(&condDimension);
    pthread_mutex_unlock(&windowLock);
}

void attendreTailleFenetre() {
    pthread_mutex_lock(&windowLock);
    while(gotDimension!=true){
        pthread_cond_wait(&condDimension, &windowLock);
    }
    pthread_mutex_unlock(&windowLock);
}

void signalerFenetreEtTexturePrete() {
    pthread_mutex_lock(&windowLock);
    allReady = true;
    pthread_cond_signal(&condReady);
    pthread_mutex_unlock(&windowLock);
}

void attendreFenetreTexture() {
    pthread_mutex_lock(&windowLock);
    while(allReady != true){
        pthread_cond_wait(&condReady, &windowLock);
    }
    pthread_mutex_unlock(&windowLock);
}

void debutConsommerTexture() {
    pthread_mutex_lock(&texLock);
    if(textures <= 0 || enDeposition){
        pthread_cond_wait(&peutConsommer, &texLock);
    }
    enConsomation = true;
    textures--;
    pthread_mutex_unlock(&texLock);
}

void finConsommerTexture() {
    pthread_mutex_lock(&texLock);
    enConsomation = false;
    if(!enDeposition){
        pthread_cond_signal(&peutDeposer);
    }
    pthread_mutex_unlock(&texLock);
}


void debutDeposerTexture() {
    pthread_mutex_lock(&texLock);
    while(textures >= NBTEX || enConsomation){
        pthread_cond_wait(&peutDeposer, &texLock);
    }
    enDeposition = true;
    textures++;
    pthread_mutex_unlock(&texLock);
}

void finDeposerTexture() {
    pthread_mutex_lock(&texLock);
    enDeposition = false;
    if(!enConsomation){
        pthread_cond_signal(&peutConsommer);
    }
    pthread_mutex_unlock(&texLock);
}
