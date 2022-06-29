#pragma once
#include <bits/stdc++.h>
#include <SDL.h>
#include <SDL_ttf.h>

using namespace std;

SDL_Window *window = NULL;
SDL_Surface *surface = NULL;
TTF_Font *ScoreFont = NULL;
TTF_Font *EndFont = NULL;
SDL_Surface *TextScore = NULL;
SDL_Surface *TextEnd = NULL;
SDL_DisplayMode res;
unsigned int WindowWidth;
unsigned int WindowHeight;
const SDL_Color White = {0xff, 0xff, 0xff};
const SDL_Color Black = {0x00, 0x00, 0x00};
const SDL_Color DropColor = {0x88, 0xcc, 0xff};
const SDL_Color CatcherColor = {0xff, 0xaa, 0x99};	
const int CharSize = 36;
const int DropSize = 15;

/*
It could be optimized (?):
DrawDrop -> Draw a circle and a bunch of lines that goes from a setted top to every point of the upper part of the circumference
(l'ho fatto un po' più ottimizzato rispetto a prima ma credo che si possa fare meglio)
CheckDrop -> Per il checkDrop basta solo un pixel esterno della goccia (fai con cerchio esterno e linee esterne(triangolo)
(l'ho fatto un po' più ottimizzato rispetto a prima ma credo che si possa fare meglio)
DeleteCircle -> disegnare solo i pixel esterni del verso opposto in cui va il cerchio (non va bene pk aggiorna troppo smerdo)
*/

void DrawPix(unsigned int x, unsigned int y, SDL_Color Col){
	Uint8* pixel_ptr = (Uint8*)surface->pixels + (y * WindowWidth + x) * 4;
	*(pixel_ptr + 2) = Col.r;
	*(pixel_ptr + 1) = Col.g;
	*(pixel_ptr) = Col.b;
}

void DrawLine(int x0, int y0, int x1, int y1, SDL_Color Col){
    double x = x1 - x0; 
	double y = y1 - y0; 
	double length = sqrt(x * x + y * y); 
	double addx = x / length; 
	double addy = y / length; 
	x = x0; 
	y = y0; 	
	for (int i = 0; i < length; i++){
		DrawPix(x, y, Col);
		x += addx;
		y += addy;
	}
}

void DrawCircle(int x, int y, int radius, SDL_Color Col){ 
    for (int w = 0; w < 2 * radius; w++){
        for (int h = 0; h < 2 * radius ; h++){
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) < (radius * radius)){
                DrawPix(x + dx, y - dy, Col);
			}
        }
    }
}

void DrawDrop(int x, int y, int radius, SDL_Color Col){ 
  	for (int w = 0; w < 2 * radius; w++){
        for (int h = radius / 2; h < 2 * radius ; h++){
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) < (radius * radius)){
                DrawLine(x + dx, y - dy, x, y - 2 * radius, Col);
			}
        }
    }
}

void DeleteDrop(int x, int y, int radius, SDL_Color Col){ 
        for (int h = 0; h < radius ; h++){
            int dx = radius - 2 * radius + 1;
            int dy = radius - h;
            DrawLine(x - dx, y - dy, x, y - 2 * radius, Col);
            DrawLine(x + dx, y - dy, x, y - 2 * radius, Col);
    }
}

bool CheckPix(unsigned int x, unsigned int y, SDL_Color Col){
    Uint8* pixel_ptr = (Uint8*)surface->pixels + (y * WindowWidth + x) * 4;
    if(*(pixel_ptr + 2) == Col.r && *(pixel_ptr + 1) == Col.g && *pixel_ptr == Col.b)
    	return true;
    else
    	return false;
}

bool CheckLine(int x0, int y0, int x1, int y1, SDL_Color Col){
	double x = x1 - x0; 
	double y = y1 - y0; 
	double length = sqrt(x * x + y * y); 
	double addx = x / length; 
	double addy = y / length; 
	x = x0; 
	y = y0; 	
	for (int i = 0; i < length; i++){ 
		if(CheckPix(x, y, Col))
			return true;
		else
			return false;
		x += addx; 
		y += addy; 		
	}
}

bool CheckDrop(int x, int y, int radius, SDL_Color Col){
	for (int w = 0; w < 2 * radius; w++){
        for (int h = radius / 2; h < 2 * radius ; h++){
            int dx = radius - w;
            int dy = radius - h;
            if((dx * dx + dy * dy) < (radius * radius)){
            	if(CheckLine(x + dx, y - dy, x, y - 2 * radius, Col))
            		return true;            
			}
        }
    }
    return false;
}

void ScoreUpdate(int &Score){
	Score++;
	string str = "Score: ";
	stringstream ss;
	ss << Score;
	string StrDef = str +  ss.str();
	TextScore = TTF_RenderText_Shaded(ScoreFont, StrDef.c_str(), White, Black);
	SDL_BlitSurface(TextScore, NULL, surface, NULL);
	SDL_FreeSurface(TextScore);
}

void GameOver(){
	string End = "Game Over!";
	TextEnd = TTF_RenderText_Blended(EndFont, End.c_str(), White);
	SDL_Rect dst = {(surface -> w - TextEnd -> w) / 2, (surface -> h - TextEnd -> h) / 2, 0, 0};
	SDL_BlitSurface(TextEnd, NULL, surface, &dst);
	SDL_UpdateWindowSurface(window);
	
	SDL_Event e;
	bool quit = false;
	while (!quit){
		while (SDL_PollEvent(&e) != 0)
			if (e.type == SDL_QUIT) quit = true;
		SDL_Delay(true);
	}
	SDL_FreeSurface(surface);
    SDL_FreeSurface(TextScore);
	SDL_FreeSurface(TextEnd);
    TTF_CloseFont(ScoreFont);
    TTF_CloseFont(EndFont);
    TTF_Quit();
    SDL_Quit();   
	SDL_DestroyWindow(window); 
}

int main(int argc, char* argv[]){
	
	srand(time(NULL));

	SDL_Init(SDL_INIT_VIDEO);
	SDL_GetCurrentDisplayMode(0, &res);
	WindowWidth = (res.w / 4) * 3;
	WindowHeight = (res.h / 4) * 3;
	window = SDL_CreateWindow("Catch the Rain", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WindowWidth, WindowHeight, SDL_WINDOW_SHOWN);
	surface = SDL_GetWindowSurface(window);
	TTF_Init();
    ScoreFont = TTF_OpenFont("VcrOsdMono.ttf", CharSize);
	EndFont = TTF_OpenFont("VcrOsdMono.ttf", CharSize * 2);
	
	vector <pair <int, int> > Drop;	
	int CicleCounter = 50;
	int MouseX, MouseY;
	int SpawnRate = 100;
	int TICK_RATE = 7;
	int Score = -1;
	bool Switch = false;
	
	ScoreUpdate(Score);
	
	SDL_Event e;
	
	bool quit = false;
	while (!quit){
		while (SDL_PollEvent(&e) != 0)
			if (e.type == SDL_QUIT) quit = true;

		DrawCircle(MouseX, WindowHeight - 5 * DropSize, 2 * DropSize, Black);
		
		for(int i = 0; i < Drop.size(); i++){		
			DeleteDrop(Drop[i].first, Drop[i].second, DropSize, Black);
		}
		
		if(Score % 30 == 1)
			Switch = true;
			
		if(Switch && Score % 30 == 0 && SpawnRate >= 30 && TICK_RATE > 0){
			SpawnRate -= 10;
			TICK_RATE--;
			Switch = false;
		}
				
		if(CicleCounter % SpawnRate == 0){
			int x = rand() % (WindowWidth - 2 * DropSize) + DropSize;
			int y = 2 * DropSize;
			if(x <= TextScore -> w)
				y = 4 * DropSize;
			Drop.push_back(make_pair(x, y));
			CicleCounter = 1;			
		}
		
		CicleCounter++;
		
		for(int i = 0; i < Drop.size(); i++){
			Drop[i].second++;
			DrawDrop(Drop[i].first, Drop[i].second, DropSize, DropColor);
		}
				
		SDL_GetMouseState(&MouseX, &MouseY);
		if(MouseX > WindowWidth - 2 * DropSize)
			MouseX = WindowWidth - 2 * DropSize;
		if(MouseX < 2 * DropSize)
			MouseX = 2 * DropSize;
			
		DrawCircle(MouseX, WindowHeight - 5 * DropSize, 2 * DropSize, CatcherColor);
		
		for(int i = 0; i < Drop.size(); i++){
			if(Drop[i].second == WindowHeight - DropSize){
				GameOver();
				for(int j = 0; j < Drop.size(); j++)
					Drop.erase(Drop.begin() + j);
			}
		}
		
		for(int i = 0; i < Drop.size(); i++){
			if(CheckDrop(Drop[i].first, Drop[i].second, DropSize, CatcherColor)){
				DrawDrop(Drop[i].first, Drop[i].second, DropSize, Black);
				Drop.erase(Drop.begin() + i);
				ScoreUpdate(Score);
			}
		}
		
		SDL_UpdateWindowSurface(window);
					
		SDL_Delay(TICK_RATE);
	}
}
