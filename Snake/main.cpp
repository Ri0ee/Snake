#include <iostream>
#include <map>
#include <deque>
#include <math.h>
#include "SDL.h"
#include "SDL_ttf.h"
using namespace std;

constexpr float pi = 3.14159265;
constexpr float target_fps = 60;
constexpr float target_frame_time = 1000 / target_fps;
constexpr float screen_size_width = 500;
constexpr float screen_size_height = 500;

class SnakeGame {
public:
	SnakeGame() {
		initialized = InitGraphics();
	}
	
	~SnakeGame() {
		SDL_DestroyTexture(texture);
		SDL_DestroyRenderer(renderer);
		SDL_FreeSurface(surface);
		SDL_DestroyWindow(window);
		TTF_CloseFont(font);
		TTF_Quit();
		SDL_Quit();
	}
	
	int Run() {
		if(!initialized)
			return 1;
		
		while(!finishing) {
			Input();

			if(key_states[SDL_SCANCODE_SPACE] == true && !game_running)
				InitializeGame();
			
			Process();
			Render();
			SDL_Delay(target_frame_time);
		}
		
		return 0;
	}
	
private:
	bool initialized = false;
	bool finishing = false;
	bool game_running = false;
	bool fruit_exists = false;
	bool game_over = false;
	
	map<int, bool> key_states;
	
	int score = 0;
	int length_increase = 0;
	
	struct pos {
		float x, y;
	} fruit_pos;
	
	struct params {
		float speed = 2;
		float rotation_speed = pi / 60; // Per frame
		float current_rotation = 0;
	} snake_params;
	
	deque<pos> snake;
	int field[(int)screen_size_width / 100][(int)screen_size_height / 100];
	
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Surface* surface = nullptr;
	SDL_Texture* texture = nullptr;
	
	TTF_Font* font = nullptr;
	
	bool InitGraphics() {
		if(SDL_Init(SDL_INIT_VIDEO) != 0) {
			std::cerr << "Failed to init SDL2, error: " << SDL_GetError() << "\n";
			return false;
		}
		
		if(TTF_Init() != 0) {
			cerr << "Failed to init SDL ttf, error: " << TTF_GetError() << "\n";
			return false;
		}
		
		window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
		if(!window) {
			std::cerr << "Failed to create window, error: " << SDL_GetError() << "\n";
			return false;
		}
		
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
		if(!renderer) {
			std::cerr << "Failed to create renderer, error: " << SDL_GetError() << "\n";
			return false;
		}
		
		surface = SDL_GetWindowSurface(window);
		texture = SDL_CreateTextureFromSurface(renderer, surface);
		font = TTF_OpenFont("OpenSans-Regular.ttf", 24);
		
		return true;
	}
	
	void InitializeGame() {
		snake.clear();
		snake.push_back({100, 100});
		snake.push_back({105, 100});
		snake.push_back({110, 100});
		snake.push_back({115, 100});
		snake.push_back({120, 100});
		
		snake_params.current_rotation = 0;
		score = 0;
		length_increase = 0;
		
		game_running = true;
	}
	
	bool Collision(pos r1, pos r2, float width) {
		pos vert1[4];
		pos vert2[4];
		
		vert1[0] = r1;
		vert1[1] = {r1.x + width, r1.y};
		vert1[2] = {r1.x + width, r1.y + width};
		vert1[3] = {r1.x, r1.y + width};
		
		vert2[0] = r2;
		vert2[1] = {r2.x + width, r2.y};
		vert2[2] = {r2.x + width, r2.y + width};
		vert2[3] = {r2.x, r2.y + width};
		
		for(int i = 0; i < 4; i++) {
			float x = vert1[i].x, y = vert1[i].y;
			
			bool result = true;
			for(int j = 0; j < 3; j++) {
				float d =	(x - vert2[j].x)*(vert2[j+1].y - vert2[j].y) - 
							(y - vert2[j].y)*(vert2[j+1].x - vert2[j].x);
				
				if(d > 0) {
					result = false;
					break;
				}
			}
			
			float d =	(x - vert2[3].x)*(vert2[0].y - vert2[3].y) - 
						(y - vert2[3].y)*(vert2[0].x - vert2[3].x);
			if(d > 0) 
				result = false;
				
			if(result == true)
				return true;
		}
		
		return false;
	}
	
	bool SnakeCollision(pos position) {
		for(unsigned i = 0; i < snake.size(); i++)
			if(Collision(snake[i], position, 10))
				return true;
		
		return false;
	}
	
	void Process() {
		if(game_running) {
			if(fruit_exists && SnakeCollision(fruit_pos)){
				fruit_exists = false;
				fruit_pos = {0, 0};
				
				score++;
				length_increase += 5;
			}
			
			for(auto it = snake.begin(); it != snake.end(); it++) {
				if(distance(it, snake.end()) > 40)
					if(Collision(snake.back(), *it, 10)) {
						game_running = false;
						game_over = true;
						return;
					}
			}
			
			if(!fruit_exists) {
				int fx = 0, fy = 0;
				
				while(true) {
					fx = 20 + rand() % (int)(screen_size_width - 40);
					fy = 20 + rand() % (int)(screen_size_height - 40);

					if(!SnakeCollision({(float)fx, (float)fy}))
						break;
				}

				fruit_pos = {(float)fx, (float)fy};
				fruit_exists = true;
			}
			
			if(key_states[SDL_SCANCODE_A]) // Rotate to the left
				snake_params.current_rotation -= snake_params.rotation_speed; 
			
			if(key_states[SDL_SCANCODE_D]) // Rotate to the right
				snake_params.current_rotation += snake_params.rotation_speed;
			
			float dir_x = cosf(snake_params.current_rotation);
			float dir_y = sinf(snake_params.current_rotation);
			
			snake.push_back({snake.back().x + snake_params.speed * dir_x, snake.back().y + snake_params.speed * dir_y});
			if(length_increase > 0)
				length_increase--;
			else 
				snake.pop_front();
		}
	}
	
	void Render() {
		SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
		SDL_RenderClear(renderer);
		
		SDL_Rect src_rect = {0, 0, 10, 10};
		
		if(game_over) {
			SDL_Surface* message_surface = TTF_RenderText_Blended(font, "Game Over", {255, 255, 255});
			SDL_Texture* message_texture = SDL_CreateTextureFromSurface(renderer, message_surface);

			int text_width = 0, text_height = 0;
			TTF_SizeText(font, "Game Over", &text_width, &text_height);
			
			SDL_Rect message_rect = {(int)screen_size_width / 2, 
									(int)screen_size_height / 2 - text_height,
									text_width, text_height};
									
			SDL_RenderCopy(renderer, message_texture, nullptr, &message_rect);
			
			if(key_states[SDL_SCANCODE_SPACE])
				game_over = false;
				
			SDL_DestroyTexture(message_texture);
			SDL_FreeSurface(message_surface);
		}
		
		if(score > 0) {
			string text = "score: " + to_string(score);
			
			SDL_Surface* message_surface = TTF_RenderText_Blended(font, text.c_str(), {255, 255, 255});
			SDL_Texture* message_texture = SDL_CreateTextureFromSurface(renderer, message_surface);
			
			int text_width = 0, text_height = 0;
			TTF_SizeText(font, text.c_str(), &text_width, &text_height);
			
			SDL_Rect message_rect = {10, 10, text_width, text_height};
									
			SDL_RenderCopy(renderer, message_texture, nullptr, &message_rect);
			
			SDL_DestroyTexture(message_texture);
			SDL_FreeSurface(message_surface);
		}
		
		if(snake.size() >= 0 && game_running) {
			if(fruit_exists) {
				SDL_Rect dst_rect = {(int)fruit_pos.x, (int)fruit_pos.y, 10, 10};
				SDL_RenderCopy(renderer, texture, &src_rect, &dst_rect);
			}
			
			for(unsigned i = 0; i < snake.size(); i++) {
				SDL_Rect dst_rect = {(int)snake[i].x, (int)snake[i].y, 10, 10};
				SDL_RenderCopy(renderer, texture, &src_rect, &dst_rect);
			}
		}
		
		SDL_RenderPresent(renderer);
	}
	
	void Input() {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_KEYDOWN:
				if(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) 
					finishing = true;
				else
					key_states[event.key.keysym.scancode] = true;
				break;
			
			case SDL_KEYUP:
				key_states[event.key.keysym.scancode] = false;
				break;
			
			case SDL_WINDOWEVENT:
				if(event.window.event == SDL_WINDOWEVENT_CLOSE)
					finishing = true;
				break;
			}
		}
	}
};

int main(int argc, char **argv) {
	SnakeGame snake_game;
	return snake_game.Run();
}
