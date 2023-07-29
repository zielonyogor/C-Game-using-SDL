#define _CRT_SECURE_NO_WARNINGS

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 680;

const int FRAMES_PER_SECOND = 30;
unsigned int SPAWN_INTERVAL = 1600;
unsigned int BOOST_INTERVAL = 10000;
unsigned int BOOST_TIME = 6000;
unsigned int BOOST_MUL = 2;
unsigned int COLOUR_INTERVAL = 400;
const int SPEED = 10;
unsigned int ENEMY_SPEED = 8;
const int OFFSET = 40;

bool is_Boosted = false;
unsigned int lastBoostTime = 0;
unsigned int lastUpdateTime = 0;
unsigned int lastSpawnTime = 0;
unsigned int lastColourTime = 0;

SDL_Event event;

//Player "class"
struct Player {
	int height, width, pos_x, pos_y, vel_x;
	SDL_Texture* image;
	SDL_Rect rectangle;
};

void create_player(struct Player* self, SDL_Renderer* rend, int x, int y) {
	self->image = NULL;
	self->image = IMG_LoadTexture(rend, "images/player.png");
	if (self->image == NULL) {
		printf("error ups\n");
		return;
	}
	self->vel_x = 0;
	self->pos_x = x; self->pos_y = y;
	SDL_QueryTexture(self->image, NULL, NULL, &self->width, &self->height);
	self->rectangle.w = 2 * self->width; self->rectangle.h = 2 * self->height;
	self->rectangle.x = x; self->rectangle.y = y;
}

void handle_input(struct Player* self) {
	if (event.type == SDL_KEYDOWN)
	{
		switch (event.key.keysym.sym)
		{
		case SDLK_LEFT: self->vel_x = -SPEED; break;
		case SDLK_RIGHT: self->vel_x = SPEED; break;
		}
	}
	else if (event.type == SDL_KEYUP) //need to make it smoother
	{
		switch (event.key.keysym.sym)
		{
		case SDLK_LEFT: if(self->vel_x == -SPEED) self->vel_x = 0; break;
		case SDLK_RIGHT: if (self->vel_x == SPEED) self->vel_x = 0; break;
		}
	}
	
}

void move(struct Player* self, SDL_Renderer* rend) {
	self->pos_x += self->vel_x;
	self->rectangle.x += self->vel_x;

	if ((self->rectangle.x < 1) || (self->rectangle.x + self->rectangle.w >= SCREEN_WIDTH)){
		self->pos_x -= self->vel_x;
		self->rectangle.x -= self->vel_x;
	}
	SDL_RenderCopy(rend, self->image, NULL, &self->rectangle);
}

//Object "class"
struct Object {
	int width, height;
	bool is_Enemy;
	SDL_Texture* image;
	SDL_Rect rectangle;
};

void create_object(struct Object* self, SDL_Renderer* rend, int x, int y, bool is_Enemy) {
	if (is_Enemy) {
		self->image = IMG_LoadTexture(rend, "images/can.png");
	} else self->image = IMG_LoadTexture(rend, "images/leek.png");
	SDL_QueryTexture(self->image, NULL, NULL, &self->width, &self->height);
	self->rectangle.w = 2 * self->width; self->rectangle.h = 2 * self->height;
	self->rectangle.x = x; self->rectangle.y = y;
	self->is_Enemy = is_Enemy;
}

bool move_object(struct Object* self, SDL_Renderer* rend) {
	self->rectangle.y += ENEMY_SPEED;
	SDL_RenderCopy(rend, self->image, NULL, &self->rectangle);
	return (self->rectangle.y > SCREEN_HEIGHT);
}

bool check_collision(struct Player* self, struct Object* other) {
	//collision and little offset on Y axis
	return (SDL_HasIntersection(&self->rectangle, &other->rectangle) && ( other->rectangle.y + other->rectangle.h < self->pos_y + self->rectangle.h - 12));
}

//Text "class"
struct Text{
	char* text;
	int fontsize;
	SDL_Color text_color;
	SDL_Rect rectangle;
	TTF_Font* font;
	SDL_Surface* surface;
	SDL_Texture* texture;
};

void initialize_text(struct Text* self, SDL_Renderer* rend, int size, int x, int y, char* text[]) {
	self->text = SDL_strdup(text);
	self->fontsize = size;
	self->text_color.r = 255; self->text_color.g = 131; self->text_color.b = 0;
	self->font = TTF_OpenFont("04B_30__.TTF", self->fontsize);
	TTF_SizeText(self->font, self->text, &self->rectangle.w, &self->rectangle.h);
	if (x == -1) { //-1 stands for center
		self->rectangle.x = (SCREEN_WIDTH - self->rectangle.w)/2;
	}else self->rectangle.x = x; 
	self->rectangle.y = y;
	self->surface = TTF_RenderText_Solid(self->font, self->text, self->text_color);
	self->texture = SDL_CreateTextureFromSurface(rend, self->surface);
}

struct MovingText{
	struct Text text;
	int starting_pos, offset, dir;
	float new_pos;
};

void initialize_movingtext(struct MovingText* self, SDL_Renderer* rend, int size, int x, int y, char* text_char[], int offset) {
	initialize_text(&self->text, rend, size, x, y, text_char);
	self->offset = offset; self->starting_pos = y; 
	self->dir = 1; self->new_pos = (float)y;
}

void move_text(struct MovingText* self, SDL_Renderer* rend) {
	self->new_pos += 0.5 * self->dir;
	self->text.rectangle.y = (int)self->new_pos;
	SDL_RenderCopy(rend, self->text.texture, NULL, &self->text.rectangle);
	if (abs(self->starting_pos - self->text.rectangle.y) > self->offset) self->dir *= -1;
}

struct Score {
	struct Text text;
	int score;
};

void initialize_score(struct Score* self, SDL_Renderer* rend, int size, int x, int y) {
	initialize_text(&self->text, rend, size, x, y, "00000");
	self->score = 0;
}

void update_score(struct Score* self, SDL_Renderer* rend) {
	switch (self->score){ //increasing spawn interval based on score
	case(1000):
	case(1100):
		SPAWN_INTERVAL = 1400;
		break;
	case(1800):
	case(1900):
		SPAWN_INTERVAL = 1200;
		break;
	case(3000):
	case(3100):
		SPAWN_INTERVAL = 1000;
		break;
	default:
		break;
	}
	sprintf(self->text.text, "%05d", self->score);
	SDL_FreeSurface(self->text.surface);
	SDL_DestroyTexture(self->text.texture);
	self->text.surface = TTF_RenderText_Solid(self->text.font, self->text.text, self->text.text_color);
	self->text.texture = SDL_CreateTextureFromSurface(rend, self->text.surface);
	TTF_SizeText(self->text.font, self->text.text, &self->text.rectangle.w, &self->text.rectangle.h);
	SDL_RenderCopy(rend, self->text.texture, NULL, &self->text.rectangle);
}

//Boost "Class"
struct Boost {
	int width, height;
	SDL_Texture* image;
	SDL_Rect rectangle;
	bool is_Present;
};

void create_boost(struct Boost* self, SDL_Renderer* rend, int x) {
	self->image = IMG_LoadTexture(rend, "images/note.png");
	SDL_QueryTexture(self->image, NULL, NULL, &self->width, &self->height);
	self->rectangle.w = 3 * self->width; self->rectangle.h = 3 * self->height;
	self->rectangle.x = x; self->rectangle.y = 0;
	self->is_Present = false;
}

bool move_boost(struct Boost* self, SDL_Renderer *rend) {
	self->rectangle.y += ENEMY_SPEED + 1;
	SDL_RenderCopy(rend, self->image, NULL, &self->rectangle);
	return (self->rectangle.y > SCREEN_HEIGHT);
}

bool check_boost(struct Player *self, struct Boost *other) {
	return (SDL_HasIntersection(&self->rectangle, &other->rectangle) && (other->rectangle.y + other->rectangle.h < self->pos_y + self->rectangle.h - 12));
}

struct Picture{
	int width, height;
	SDL_Texture* image;
	SDL_Rect rectangle;
};

void initialize_picture(struct Picture* self, SDL_Renderer* rend,int x, int y, char* path[]) {
	self->image = IMG_LoadTexture(rend, path);
	SDL_QueryTexture(self->image, NULL, NULL, &self->width, &self->height);
	self->rectangle.w = 2 * self->width; self->rectangle.h = 2 * self->height;
	self->rectangle.x = x; self->rectangle.y = y;
}

//tint for fever mode
void apply_tint(SDL_Renderer* renderer, SDL_Color color) {
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 80);
	SDL_Rect rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	SDL_RenderFillRect(renderer, &rect);
}

SDL_Color calculate_tint(SDL_Color *col) {
	switch (col->g)
	{
	case(33): //magenta -> yellow -> cyan because i said so
		col->g = 216; col->b = 0;
		break;
	case(216):
		col->r = 33; col->g = 177; col->b = 255;
		break;
	default:
		col->r = 255; col->g = 33; col->b = 140;
		break;
	}
}

/*-------------------------------
---------MAIN FUNCTION-----------
-------------------------------*/

int main(int argc, char* args[])
{
	SDL_Init(SDL_INIT_VIDEO); //some initializations
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	srand(time(NULL));

	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;

	SDL_Texture* ground = NULL;
	SDL_Rect ground_rectangle;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return 1;

	window = SDL_CreateWindow("Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	//create ground, player, list for objects
	ground = IMG_LoadTexture(renderer, "images/ground.png");
	SDL_QueryTexture(ground, NULL, NULL, &ground_rectangle.w, &ground_rectangle.h); 
	ground_rectangle.w *= 2; ground_rectangle.h *= 2;
	ground_rectangle.x = 0; ground_rectangle.y = 0;

	//player
	struct Player p;
	create_player(&p, renderer, 100, SCREEN_HEIGHT - 184);

	//score
	struct Score score;
	initialize_score(&score, renderer, 32, 14, 14);
	
	//texts
	struct Text title;
	initialize_text(&title, renderer, 64, -1, 24, "LEEK FEVER");

	struct Text game_over;
	initialize_text(&game_over, renderer, 64, -1, 40, "GAME OVER");

	struct MovingText continue_text;
	initialize_movingtext(&continue_text, renderer, 20, -1, 100, "PRESS SPACE TO START", 2);

	struct Text how_to;
	initialize_text(&how_to, renderer, 42, -1, 160, "HOW TO PLAY");
	struct Text move_as;
	initialize_text(&move_as, renderer, 26, -1, 240, "USE ARROWS TO MOVE");
	struct Text what_collect;
	initialize_text(&what_collect, renderer, 26, -1, 276, "CATCH LEEK AND NOTE BOOST");
	struct Text what_avoid;
	initialize_text(&what_avoid, renderer, 26, -1, 312, "AVOID CAT FOOD");
	struct Text score_final;
	initialize_text(&score_final, renderer, 26, -1, 240, "YOUR SCORE:");

	//menu pic
	struct Picture main_menu;
	initialize_picture(&main_menu, renderer, 0, 0, "images/menu.png");

	//falling objects
	int numObjects = 0; struct Object objects_list[10];
	
	//boost
	struct Boost b;
	create_boost(&b, renderer, 0);

	//tint
	SDL_Color tint; 
	calculate_tint(&tint);

	//main menu loop
	while (1) {
		lastUpdateTime = SDL_GetTicks();
		if (SDL_PollEvent(&event)) { //handling all input
			if (event.key.keysym.sym == SDLK_SPACE){
				break;
			}
			if (event.type == SDL_QUIT)
				goto end;
			else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)
				goto end;
		}
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, main_menu.image, NULL, &main_menu.rectangle);
		SDL_RenderCopy(renderer, title.texture, NULL, &title.rectangle);
		move_text(&continue_text, renderer);
		SDL_RenderPresent(renderer);

		while (SDL_GetTicks() - lastUpdateTime < 1000 / FRAMES_PER_SECOND) {}
	}
	//wait is so that it won't jump straight to main game
	int wait = 0;
	continue_text.text.rectangle.y = 400; continue_text.starting_pos = 400; continue_text.new_pos = 400.0;
	//how to play intro
	while (1) {
		lastUpdateTime = SDL_GetTicks();
		if (SDL_PollEvent(&event) && wait>20) { //handling all input
			if (event.key.keysym.sym == SDLK_SPACE) {
				break;
			}
			if (event.type == SDL_QUIT)
				goto end;
			else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)
				goto end;
		}
		SDL_SetRenderDrawColor(renderer, 148, 201, 204, 255); //draw background color
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, how_to.texture, NULL, &how_to.rectangle);
		SDL_RenderCopy(renderer, move_as.texture, NULL, &move_as.rectangle);
		SDL_RenderCopy(renderer, what_collect.texture, NULL, &what_collect.rectangle);
		SDL_RenderCopy(renderer, what_avoid.texture, NULL, &what_avoid.rectangle);
		move_text(&continue_text, renderer);
		SDL_RenderPresent(renderer);

		while (SDL_GetTicks() - lastUpdateTime < 1000 / FRAMES_PER_SECOND) {}
		wait++;
	}
	//main game loop
	game:
	lastBoostTime = SDL_GetTicks();
	while (1) {

		lastUpdateTime = SDL_GetTicks(); //get ticks
		if (SDL_PollEvent(&event)) { //handling all input
			handle_input(&p);
			if (event.type == SDL_QUIT)
				goto end;
			else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)
				goto end;
		}

		if (SDL_GetTicks() - lastSpawnTime >= SPAWN_INTERVAL && numObjects < 9) { //spawning items

			//some offset so that it will be on screen nicely, create object with 80% it being good item
			create_object(&objects_list[numObjects], renderer, OFFSET + rand() % (SCREEN_WIDTH - 3 * OFFSET), 0, ((rand() % 10) < 8) ? 0 : 1);
			numObjects++;
			lastSpawnTime = SDL_GetTicks();
		}

		if (SDL_GetTicks() - lastBoostTime >= BOOST_INTERVAL && !b.is_Present && !is_Boosted && rand() % 100 <= 3 ) {
			b.rectangle.y = 0;
			b.rectangle.x = OFFSET + rand() % (SCREEN_WIDTH - 3 * OFFSET);
			b.is_Present = true;
		}
		if (SDL_GetTicks() - lastBoostTime >= BOOST_TIME && is_Boosted){
			is_Boosted = false; 
			ENEMY_SPEED -= BOOST_MUL;
			SPAWN_INTERVAL += 300 * BOOST_MUL;
		}

		SDL_SetRenderDrawColor(renderer, 148, 201, 204, 255); //draw background color

		SDL_RenderClear(renderer);

		SDL_RenderCopy(renderer, ground, NULL, &ground_rectangle); //draw ground


		move(&p, renderer); //move player

		if (b.is_Present) {
			if (move_boost(&b, renderer)){
				b.is_Present = false;
				lastBoostTime = SDL_GetTicks();
			}
			if (check_boost(&p, &b)) {
				lastBoostTime = SDL_GetTicks();
				is_Boosted = true;
				b.is_Present = false;
				ENEMY_SPEED += BOOST_MUL;
				SPAWN_INTERVAL -= 300 * BOOST_MUL;
			}
		}

		for (int i = 0; i < numObjects; i++) {
			if (move_object(&objects_list[i], renderer)) {
				//check if that was leek if so game over
				if (objects_list[i].is_Enemy==false && !is_Boosted){
					goto over;
				}
				// remove object from the list
				numObjects--;
				objects_list[i] = objects_list[numObjects];
				i--; // move back one index to avoid skipping the next object
			}
			else if(check_collision(&p, &objects_list[i])) { //check collisions
				if (objects_list[i].is_Enemy == false){
					if(is_Boosted) score.score += 200;
					else score.score += 100;
				}
				else if (!is_Boosted) { //cant hit you when boosted
					goto over;
				}
				//delete object after collision
				numObjects--;
				objects_list[i] = objects_list[numObjects];
				i--;
			}
		}

		if (is_Boosted) {
			if (SDL_GetTicks() - lastColourTime >= COLOUR_INTERVAL) {
				calculate_tint(&tint);
				lastColourTime = SDL_GetTicks();
			}
			apply_tint(renderer, tint);
		}

		update_score(&score, renderer);

		SDL_RenderPresent(renderer);

		while (SDL_GetTicks() - lastUpdateTime < 1000 / FRAMES_PER_SECOND) {}
	}

	//game over loop
	//
	over:
	score.text.rectangle.x = (SCREEN_WIDTH - score.text.rectangle.w) / 2;
	score.text.rectangle.y = 270;
	while (1)
	{
		if (SDL_PollEvent(&event)) { //handling all input
			if (event.type == SDL_QUIT)
				break;
			else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)
				break;
		}
		SDL_SetRenderDrawColor(renderer, 148, 201, 204, 255); //draw background color
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, game_over.texture, NULL, &game_over.rectangle);
		SDL_RenderCopy(renderer, score_final.texture, NULL, &score_final.rectangle);
		SDL_RenderCopy(renderer, score.text.texture, NULL, &score.text.rectangle);
		SDL_RenderPresent(renderer);

		while (SDL_GetTicks() - lastUpdateTime < 1000 / FRAMES_PER_SECOND) {}
	}
	//add credits?? props to tvchany
	end:
	SDL_DestroyTexture(p.image);
	SDL_FreeSurface(score.text.surface); //need to add more
	SDL_DestroyTexture(score.text.texture);
	SDL_DestroyTexture(b.image);
	SDL_DestroyTexture(main_menu.image);
	SDL_DestroyTexture(ground);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();

	return 0;
}
