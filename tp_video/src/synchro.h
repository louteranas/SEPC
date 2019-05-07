#ifndef SYNCHRO_H
#define SYNCHRO_H

#include <stdbool.h>
#include <pthread.h>
#include "ensitheora.h"

extern bool fini;
extern bool gotDimension;
extern bool allReady;
extern bool enConsomation;
extern bool enDeposition;
extern int textures;

/* les variables pour la synchro, ici */
extern pthread_mutex_t windowLock;
extern pthread_mutex_t texLock;
extern pthread_cond_t condDimension;
extern pthread_cond_t condReady;
extern pthread_cond_t peutConsommer;
extern pthread_cond_t peutDeposer;

/* Les extern des variables pour la synchro ici */


/* Fonctions de synchro à implanter */

void envoiTailleFenetre(th_ycbcr_buffer buffer);
void attendreTailleFenetre();

void attendreFenetreTexture();
void signalerFenetreEtTexturePrete();

void debutConsommerTexture();
void finConsommerTexture();

void debutDeposerTexture();
void finDeposerTexture();

#endif
