#include <sstream>
#include <math.h>
#include <ctime>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

using namespace std;

SDL_Window *window = NULL;
SDL_Surface *surface = NULL;
TTF_Font *TextFont = NULL;
SDL_Surface *LeftText = NULL;
SDL_Surface *RightText = NULL;
SDL_Surface *EndText = NULL;
Mix_Chunk *ScoreSound = NULL;
Mix_Chunk *WallHitSound = NULL;
Mix_Chunk *PaddleHitSound = NULL;

SDL_DisplayMode res;
unsigned int WindowWidth;
unsigned int WindowHeight;

const int White = 0xff;
const int Black = 0x00;
SDL_Color fg = {0xff, 0xff, 0xff};
SDL_Color bg = {0x00, 0x00, 0x00};
const int CharSize = 30;

struct Sound{
	Sound(){
	Mix_Init(0);
	Mix_OpenAudio(15000, MIX_DEFAULT_FORMAT, 2, 1024);
	ScoreSound = Mix_LoadWAV("PointGot.wav");
	WallHitSound = Mix_LoadWAV("WallHit.wav");
	PaddleHitSound = Mix_LoadWAV("PaddleHit.wav");
	}
	void Score()	{Mix_PlayChannel(-1, ScoreSound, 0);};
	void Paddle()	{Mix_PlayChannel(-1, PaddleHitSound, 0);};
	void Wall()		{Mix_PlayChannel(-1, WallHitSound, 0);};
};

void DrawPix(int x, int y, int Col){
	Uint8* pixel_ptr = (Uint8*)surface->pixels + (y * WindowWidth + x) * 4;
	*(pixel_ptr + 2) = Col;
	*(pixel_ptr + 1) = Col;
	*(pixel_ptr) = Col;
}

void DrawLine(int x0, int y0, int x1, int y1, int Col){
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

void DrawBall(int x, int y, int radius, int Col){ 
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

class Paddle{
	public:
		Paddle(int);
		void Draw();
		void MoveUp();
		void MoveDown();
		int ReturnX(){return x;};
		int ReturnY(){return y;};
		int ReturnW(){return width;};
		int ReturnH(){return height;};
	private:
		int x, y;
		int width;
		int height;		
};

Paddle::Paddle(int x){
	this->x = x;
	y = WindowHeight / 2;
	width = WindowWidth / 100;
	height = WindowHeight / 10;
}

void Paddle::Draw(){
	int x1 = x + width;
	for(int j = y; j < y + height; j++){
		DrawLine(x, j, x1, j, White);
	}
}

void Paddle::MoveUp(){
	if(y > 1){
		int x1 = x + width;
		DrawLine(x, y + height - 1, x1, y + height - 1, Black);
		DrawLine(x, y + height - 2, x1, y + height - 2, Black);
		y -= 2;
	}
}

void Paddle::MoveDown(){
	if(y + height < WindowHeight - 1){
		int x1 = x + width;
		DrawLine(x, y + 1, x1, y + 1, Black);
		DrawLine(x, y, x1, y, Black);
		y += 2;
	}
}

class LeftScore{
	public:
		LeftScore();
		void ScoreUp();
		void PrintScore();
		int ReturnScore();
	private:
		int score;
};

LeftScore::LeftScore(){
	score = 0;
}

void LeftScore::ScoreUp(){
	score++;
}

void LeftScore::PrintScore(){
	stringstream ss;
	ss << score;
	string StrDef = ss.str();
	LeftText = TTF_RenderText_Shaded(TextFont, StrDef.c_str(), fg, bg);
	SDL_Rect dst = {(surface -> w - LeftText -> w) / 4, 0, 0, 0};
	SDL_BlitSurface(LeftText, NULL, surface, &dst);
	SDL_UpdateWindowSurface(window);
	SDL_FreeSurface(LeftText);
}

int LeftScore::ReturnScore(){
	return score;
}

class RightScore{
	public:
		RightScore();
		void ScoreUp();
		void PrintScore();
		int ReturnScore();
	private:
		int score;
};

RightScore::RightScore(){
	score = 0;
}

void RightScore::ScoreUp(){
	score++;
}

void RightScore::PrintScore(){
	stringstream ss;
	ss << score;
	string StrDef = ss.str();
	RightText = TTF_RenderText_Shaded(TextFont, StrDef.c_str(), fg, bg);
	SDL_Rect dst = {((surface -> w - RightText -> w) / 4) * 3, 0, 0, 0};
	SDL_BlitSurface(RightText, NULL, surface, &dst);
	SDL_UpdateWindowSurface(window);
	SDL_FreeSurface(RightText);
}

int RightScore::ReturnScore(){
	return score;
}

class GameOver: public LeftScore, public RightScore{
	public:
		bool CheckGameOver(LeftScore&, RightScore&);
		void PrintGameOver();		
	private:
		string OverString;
};

bool GameOver::CheckGameOver(LeftScore& LeftScore, RightScore& RightScore){
	if(LeftScore.ReturnScore() == 11){
		OverString = "Player1 wins!";
		LeftScore.PrintScore();
		return true;
	}
	if(RightScore.ReturnScore() == 11){
		OverString = "Player2 wins!";
		RightScore.PrintScore();
		return true;
	}
	return false;
}

void GameOver::PrintGameOver(){
	EndText = TTF_RenderText_Blended(TextFont, OverString.c_str(), fg);
	SDL_Rect dst = {(surface -> w - EndText -> w) / 2, (surface -> h - EndText -> h) / 2, 0, 0};
	SDL_BlitSurface(EndText, NULL, surface, &dst);
	SDL_UpdateWindowSurface(window);
}

class Ball{
	public:
		Ball();
		void Spawn();
		void Move();
		void CheckBorderCollision(Sound&);
		void CheckLeftPaddleCollision(Paddle&, Sound&);
		void CheckRightPaddleCollision(Paddle&, Sound&);
		bool RightWin(Paddle&, Sound&);
		bool LeftWin(Paddle&, Sound&);
		void Live();
		void Die();
	private:
		float x, y;
		float VelX, VelY;
		float Speed;
		int BallRadius;
};

Ball::Ball(){
	x = WindowWidth / 2;
	y = rand() % (WindowHeight - 2 * BallRadius) + BallRadius;
	VelX = 1.0f;
	VelY = 1.0f;
	Speed = 0.5f;
	BallRadius = 10;
}

void Ball::Spawn(){
	x = WindowWidth / 2;
	y = rand() % (WindowHeight - 6 * BallRadius) + 3 * BallRadius;
	Speed = 0.5f;
	VelX = - VelX;
	VelY = 1.0f;
}

void Ball::Move(){
	Ball::Die();
	x += VelX;
	y += VelY;
 	Ball::Live();
}

void Ball::Live(){
	DrawBall((int)x, (int)y, BallRadius, White);
}

void Ball::Die(){
	DrawBall((int)x, (int)y, BallRadius, Black);
}

void Ball::CheckBorderCollision(Sound& Sound){	
 	if (y > WindowHeight - BallRadius - abs(VelY)){ 
 		Ball::Die();
 		y -= abs(VelY);
	   	VelY *= - 1;
		Sound.Wall();
 	}
 	if(y < BallRadius + abs(VelY)){
 		Ball::Die();
 		y += abs(VelY);
	   	VelY *= - 1;
		Sound.Wall();
 	}
}

void Ball::CheckLeftPaddleCollision(Paddle& LeftPaddle, Sound& Sound){
	if(x - BallRadius < LeftPaddle.ReturnX() + LeftPaddle.ReturnW() && x - BallRadius > LeftPaddle.ReturnX() + LeftPaddle.ReturnW() - 2 && y + BallRadius > LeftPaddle.ReturnY() && y - BallRadius < LeftPaddle.ReturnY() + LeftPaddle.ReturnH()){
		float PaddleCenter = LeftPaddle.ReturnY() + (LeftPaddle.ReturnH() / 2);
    	float Direction = PaddleCenter - y;
		VelY = Speed * Direction * - 0.1f;
		VelX *= - 1;
		Speed += 0.1f;
		Sound.Paddle();
	}
}

void Ball::CheckRightPaddleCollision(Paddle& RightPaddle, Sound& Sound){
	if(x + BallRadius > RightPaddle.ReturnX() && x + BallRadius < RightPaddle.ReturnX() + 2 && y > RightPaddle.ReturnY() && y < RightPaddle.ReturnY() + RightPaddle.ReturnH()){
		float PaddleCenter = RightPaddle.ReturnY() + (RightPaddle.ReturnH() / 2);
    	float Direction = PaddleCenter - y;
    	VelY = Speed * Direction * - 0.1f;
		VelX *= - 1;
		Speed += 0.1f;
		Sound.Paddle();
	}
}

bool Ball::RightWin(Paddle& LeftPaddle, Sound& Sound){
	if(x < WindowWidth / 25){
		Sound.Score();
		return true;
	}
	return false;
}

bool Ball::LeftWin(Paddle& RightPaddle, Sound& Sound){
	if(x > WindowWidth - WindowWidth / 25){
		Sound.Score();
		return true;
	}
	return false;
}

int main(int argc, char* argv[]){
	
	srand(time(NULL));

	SDL_Init(SDL_INIT_VIDEO);
	SDL_GetCurrentDisplayMode(0, &res);
	WindowWidth = (res.w / 4) * 3;
	WindowHeight = (res.h / 4) * 3;
	window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WindowWidth, WindowHeight, SDL_WINDOW_SHOWN);
	surface = SDL_GetWindowSurface(window);
	TTF_Init();
	TextFont = TTF_OpenFont("Pixelated.ttf", CharSize * 2);

	Sound Sound;
	
	Paddle LeftPaddle(WindowWidth / 8);
	Paddle RightPaddle((WindowWidth / 8) * 7);
	LeftScore LeftScore;
	RightScore RightScore;
	Ball Ball;
	GameOver GameOver;
	bool IsOver = false;
	
	SDL_Event e;	
	bool quit = false;
	while (!quit){
		while (SDL_PollEvent(&e) != 0)
			if (e.type == SDL_QUIT) quit = true;
				
		const Uint8 *state = SDL_GetKeyboardState(NULL);
		if(state[SDL_SCANCODE_LSHIFT])
			LeftPaddle.MoveUp();
		if(state[SDL_SCANCODE_LCTRL])
			LeftPaddle.MoveDown();
		if(state[SDL_SCANCODE_UP])
			RightPaddle.MoveUp();
		if(state[SDL_SCANCODE_DOWN])
			RightPaddle.MoveDown();
		
		if(!IsOver){
			if(GameOver.CheckGameOver(LeftScore, RightScore)){
				Ball.Die();
				GameOver.PrintGameOver();
				IsOver = true;
			}
		}
		
		if(!IsOver){
			LeftPaddle.Draw();
			RightPaddle.Draw();
			LeftScore.PrintScore();
			RightScore.PrintScore();
			
			Ball.CheckBorderCollision(Sound);
	 		Ball.CheckLeftPaddleCollision(LeftPaddle, Sound);	
			Ball.CheckRightPaddleCollision(RightPaddle, Sound);
					
			Ball.Move();
					
			if(Ball.RightWin(LeftPaddle, Sound)){
				RightScore.ScoreUp();
				Ball.Die();
				Ball.Spawn();
			}
	
			if(Ball.LeftWin(RightPaddle, Sound)){
				LeftScore.ScoreUp();
				Ball.Die();
				Ball.Spawn();			
			}
	
			SDL_UpdateWindowSurface(window);
		}						
	}
	SDL_FreeSurface(surface);
	SDL_FreeSurface(EndText);
	TTF_CloseFont(TextFont);
	TTF_Quit();
	SDL_DestroyWindow(window); 
	SDL_Quit();
	return 0;	
}
