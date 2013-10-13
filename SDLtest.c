#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include <stdio.h>
#include <stdlib.h>

#include "model.h"

/* SDL global variables */

SDL_Event input;
int looping = 1;
Uint8* keyPress;
int mouseX, mouseY;
Uint8 mouseButton;

/* OpenGL global variables */

model_t* object;
float angle = 0.f;
float scale = 1.f;
float lightpos[] = {0.f, 2.f, 0.f, 1.f};

/* defined functions */

void init();
void display();
void cleanup();

/* main function (program loop) */

int main(int argc, char* args[]) {
  SDL_Init(SDL_INIT_EVERYTHING);
  keyPress = SDL_GetKeyState(NULL);
  SDL_SetVideoMode(800, 600, 32, SDL_OPENGL);

  init();

  while (looping) {
    while (SDL_PollEvent(&input)) {
      if (input.type == SDL_QUIT) looping = 0;
    }
    mouseButton = SDL_GetMouseState(&mouseX, &mouseY);

    display();

    SDL_GL_SwapBuffers();
    SDL_Delay(30);
    if (keyPress[SDLK_ESCAPE]) looping = 0;
  }

  cleanup();
  SDL_Quit();
  return 0;
}

/* initial setup function */

void init() {
  glClearColor(0.f, 0.f, 0.5f, 1.f);
  glClearDepth(1.f);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_NORMALIZE); /* normals are fine, OpenGL tries to scale them */

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  object = model_load("completeCube.obj");
  //model_calculateNormals(object);
  model_randomColors(object);

  model_center(object);

  scale = 1.f / model_calculateRadius(object);
}

/* looped display function */

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDepthFunc(GL_LEQUAL);

  if (keyPress[SDLK_w]) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  if (keyPress[SDLK_f]) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(70, 1.333, 1, 100);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.f, 0.f, -2.f);
  glRotatef(angle, 0.f, 0.f, 1.f);
  glRotatef(2.f * angle, 0.f, 1.f, 0.f);
  glScalef(scale, scale, scale);

  glColor3f(1.f, 1.f, 1.f);
  model_drawGL(object);

  glLoadIdentity();
  glTranslatef(0.f, 0.f, -2.f);

  glColor3f(1.f, 0.f, 0.f);
  glBegin(GL_LINES);
  glVertex3f(-1.f, 0.f, 0.f);
  glVertex3f(+1.f, 0.f, 0.f);
  glEnd();

  glColor3f(0.f, 1.f, 0.f);
  glBegin(GL_LINES);
  glVertex3f(0.f, -1.f, 0.f);
  glVertex3f(0.f, +1.f, 0.f);
  glEnd();

  ++angle;
}

void cleanup() {
  model_destroy(object);
}
