#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdbool.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 680;

const int FRAMES_PER_SECOND = 30;
const unsigned int SPAWN_INTERVAL = 1800;
const int SPEED = 10;
const int ENEMY_SPEED = 6;
const int OFFSET = 40;

SDL_Event event;

//Player "class"
struct Player {
	int height, width, pos_x, pos_y, vel_x;
	SDL_Texture* image;
	SDL_Rect rectangle;
};

void create_player(struct Player* self, SDL_Renderer* rend, int x, int y) {
	self->image = NULL;
	self->image = IMG_LoadTexture(rend, "player.png");
	if (self->image == NULL) {
		printf("error ups\n");
		return;
	}
	self->vel_x = 0;
	self->pos_x = x; self->pos_y = y;
	SDL_QueryTexture(self->image, NULL, NULL, &self->width, &self->height);
	self->rectangle.w = 4 * self->width; self->rectangle.h = 4 * self->height;
	self->rectangle.x = x; self->rectangle.y = y;
}

void handle_input(struct Player* self) {
	if (event.type == SDL_KEYDOWN)
	{
		//Adjust the velocity
		switch (event.key.keysym.sym)
		{
		case SDLK_LEFT: self->vel_x = -SPEED; break;
		case SDLK_RIGHT: self->vel_x = SPEED; break;
		}
	}
	else if (event.type == SDL_KEYUP)
	{
		//Adjust the velocity
		switch (event.key.keysym.sym)
		{
		case SDLK_LEFT: self->vel_x = 0; break;
		case SDLK_RIGHT: self->vel_x = 0; break;
		}
	}
	
}

void move(struct Player* self, SDL_Renderer* rend) {
	self->pos_x += self->vel_x;
	self->rectangle.x += self->vel_x;

	if ((self->pos_x < 0) || (self->vel_x + self->width > SCREEN_WIDTH))
	{
		self->pos_x -= self->vel_x;
		self->rectangle.x -= self->vel_x;
	}
	SDL_RenderCopy(rend, self->image, NULL, &self->rectangle);
}

struct Object {
	int width, height;
	bool is_Enemy;
	SDL_Texture* image;
	SDL_Rect rectangle;
};

void create_object(struct Object* self, SDL_Renderer* rend, int x, int y, bool is_Enemy) {
	self->image = IMG_LoadTexture(rend, "leek.png");
	SDL_QueryTexture(self->image, NULL, NULL, &self->width, &self->height);
	self->rectangle.w = 4 * self->width; self->rectangle.h = 4 * self->height;
	self->rectangle.x = x; self->rectangle.y = y;
	self->is_Enemy = is_Enemy;
}

bool move_object(struct Object* self, SDL_Renderer* rend) {
	self->rectangle.y += ENEMY_SPEED;
	SDL_RenderCopy(rend, self->image, NULL, &self->rectangle);
	return (self->rectangle.y > SCREEN_HEIGHT);
}

bool check_collision(struct Player* self, struct Object* other) {
	return SDL_HasIntersection(&self->rectangle, &other->rectangle);
}

struct Score{
	int score;

};

int main(int argc, char* args[])
{
	// variable declarations
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	SDL_Texture* img = NULL;

	// Initialize SDL.
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return 1;

	// create the window and renderer
	// note that the renderer is accelerated
	window = SDL_CreateWindow("Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	struct Player p;
	create_player(&p, renderer, 100, SCREEN_HEIGHT - 160);

	int numObjects = 0; struct Object objects_list[10];

	unsigned int lastUpdateTime = 0;
	unsigned int lastSpawnTime = 0;

	while (1) {

		lastUpdateTime = SDL_GetTicks();
		if (SDL_PollEvent(&event)) {
			handle_input(&p);
			if (event.type == SDL_QUIT)
				break;
			else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)
				break;
		}

		if (SDL_GetTicks() - lastSpawnTime >= SPAWN_INTERVAL && numObjects < 9) {

			//some offset so that it will be on screen nicely 
			create_object(&objects_list[numObjects], renderer, OFFSET + rand() % (SCREEN_WIDTH - 3 * OFFSET), 0, ((rand() % 10) < 8) ? 0 : 1);
			numObjects++;
			lastSpawnTime = SDL_GetTicks();
		}

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		SDL_RenderClear(renderer);

		move(&p, renderer);

		for (int i = 0; i < numObjects; i++) {
			if (move_object(&objects_list[i], renderer)) {
				// remove object from the list
				numObjects--;
				objects_list[i] = objects_list[numObjects];
				i--; // move back one index to avoid skipping the next object
			}
			else if(check_collision(&p, &objects_list[i])) {
				printf("kolizja\t");
				if (objects_list[i].is_Enemy == false){
					printf("plus jeden byku\n");
				}
				numObjects--;
				objects_list[i] = objects_list[numObjects];
				i--;
			}
		}

		SDL_RenderPresent(renderer);

		while (SDL_GetTicks() - lastUpdateTime < 1000 / FRAMES_PER_SECOND) {}
	}

	SDL_DestroyTexture(p.image);
	SDL_DestroyTexture(img);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	return 0;
}

//struct Player* new_player;
//new_player->image = IMG_LoadTexture(rend, "player.jpg");
//SDL_QueryTexture(new_player->image, NULL, NULL, new_player->width, new_player->height);
////rectangle
//new_player->rectangle.w = 2 * new_player->width; new_player->rectangle.h = 2 * new_player->height;
//new_player->rectangle.x = x; new_player->rectangle.y = 0;
//return new_player;