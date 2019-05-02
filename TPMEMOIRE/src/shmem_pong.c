/*****************************************************
 * Copyright Grégory Mounié 2016                     *
 *           Frédéric Pétrot 2016                    *
 *                                                   *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <SDL/SDL.h>
#include <assert.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>

const int TAILLE_X = 800;
const int TAILLE_Y = 600;

/* parametres de l'affichage de la balle */
static int x;
static int y;
static int delta_x;
static int delta_y;
static int taille;
static int couleur;

#define handle_error(msg) \
  do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_SDL_error(msg) \
  do { fprintf(stderr, "SDL :" msg ", ligne %d \n", __LINE__); exit(EXIT_FAILURE); } while (0)

static void put_pixel(SDL_Surface *surface, int x, int y, uint32_t pixel)
{
    /*
      Remarque : le nombre d'octets par pixel est toujours 4 dans ce programme,
      ce qui simplifie grandement l'écriture de put_pixel().
     */
    int bpp = surface->format->BytesPerPixel;
    /* p est l'adresse du pixel qu'on veut modifier. */
    uint8_t *p = (uint8_t *) surface->pixels + y * surface->pitch + x * bpp;
    *(uint32_t *) p = pixel;
}

static void draw_ball(SDL_Surface *canvas)
{
    /* fond noir sur l'ancienne position */
    int noir = 0;
    for (int k = 0; k < taille; k++)
        for (int l = 0; l < taille; l++)
            put_pixel(canvas, x + k, y + l, noir);


    /* modification de position */
    x += delta_x;
    y += delta_y;

    /* inversion des directions et ajustement */
    if (x + taille > TAILLE_X) {
        x = TAILLE_X - taille;
        delta_x *= -1;
    }

    if (x < 0) {
        x = 0;
        delta_x *= -1;
    }

    if (y + taille > TAILLE_Y) {
        y = TAILLE_Y - taille;
        delta_y *= -1;
    }

    if (y < 0) {
        y = 0;
        delta_y *= -1;
    }

    /* dessin de la balle */
    for (int k = 0; k < taille; k++)
        for (int l = 0; l < taille; l++)
            put_pixel(canvas, x + k, y + l, couleur);
}


int main(int argc, char **argv)
{
    SDL_Surface *canvas = NULL;
    SDL_Surface *ecran = NULL;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
        handle_SDL_error("Init");

    /* On crée une nouvelle fenêtre (un écran) SDL. */
    ecran = SDL_SetVideoMode(TAILLE_X, TAILLE_Y, 32, SDL_SWSURFACE);
    if (ecran == NULL)
        handle_SDL_error("SetVideoMode");

    /* On donne un nom à la fenêtre SDL. */
    SDL_WM_SetCaption("Mini carrés", NULL);

    /*
       On initialise le générateur de nombres aléatoires en utilisant
       le pid du processus courant comme graine. Ainsi, chaque nouvelle
       exécution du programme donnera des nombres différents.
    */
    srandom(getpid());

    /*
       On initialise les caractéristiques de la balle : position initiale,
       taille, couleur. Ces valeurs sont tirées aléatoirement.
    */
    x = random() % TAILLE_X;
    y = random() % TAILLE_Y;
    delta_x = random() % 5 + 1;
    delta_y = random() % 5 + 1;
    taille = random() % 20 + 10;
    couleur =
        (random() % 256 << 16) | (random() % 256 << 8) | (random() % 255);

  /********************
   * Le tampon a changer pour mettre en place le couplage mémoire
   * entre les différents processus.
   *********************/
  /************ Ancienne version avec calloc *******************/

    //void *tampon = calloc(TAILLE_X * TAILLE_Y, 4);
    //if (tampon == NULL)
    //    handle_error("malloc");

    /************** Nouvelle version avec shMem ******************/


    int fd = shm_open("SharedMem", O_RDWR | O_CREAT , 0777);
    if (fd == -1) {
    fprintf(stderr, "Open failed:%s\n", strerror(errno));
    exit(1);
    }

    int sizeId = ftruncate(fd, TAILLE_X * TAILLE_Y*4);
    if (sizeId == -1) {
    fprintf(stderr, "Open failed:%s\n", strerror(errno));
    exit(1);
    }

    void *tampon = mmap(NULL, TAILLE_X * TAILLE_Y*4, PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);



    /* On associe ce tampon à la zone de dessin. */
    canvas =
        SDL_CreateRGBSurfaceFrom(tampon, TAILLE_X, TAILLE_Y, 32, TAILLE_X * 4,
                                 0x0, 0x0, 0x0, 0x0);
    if (canvas == NULL)
        handle_SDL_error();

    assert(canvas->pixels == tampon);

    /*
       On demande à SDL d'afficher cette zone de dessin
       sur l'écran initialisé plus haut.
    */
    if (SDL_BlitSurface(canvas, NULL, ecran, NULL) == -1)
        handle_SDL_error("BlitSurface");

    if (SDL_Flip(ecran) == -1)
        handle_SDL_error("Flip");

    /* Boucle infinie d'affichage de l'animation de la balle. */
    while (1) {
        /* On attend 20ms entre chaque affichage. */
        SDL_Delay(20);

        /* On dessine la nouvelle position de la balle. */
        draw_ball(canvas);

        /* On affiche la zone de dessin correspondante sur l'écran. */
        if (SDL_BlitSurface(canvas, NULL, ecran, NULL) == -1)
            handle_SDL_error("BlitSurface");

        if (SDL_Flip(ecran) == -1)
            handle_SDL_error("Flip");

        /*
           Si l'utilisateur clique sur la croix pour fermer la fenêtre,
           on s'arrête proprement.
         */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                goto fin;
        }
    }

  fin:
    /* On libère la mémoire avant de s'arrêter. */
    SDL_FreeSurface(canvas);
    munmap(tampon, TAILLE_X * TAILLE_Y*4);
    shm_unlink("SharedMem");
    close(fd);



    //free(tampon);
    SDL_Quit();

    return 0;
}
