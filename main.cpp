#include <stdio.h>

#include <SDL.h>

// Game of Life
// Rules
// 1. Any live cell with two or three neighbors survives
// 2. Any dead cell with three live neighbors becomes a live cell
// 3. All other live cells die

enum class Action {
	NONE, PLAYPAUSE, STEP, CLEAR, RESET, SHOW, MOUSE, GRID
};

struct
{
	SDL_Window* window;
	SDL_Renderer* renderer;
	bool running;
	SDL_Event event_handler;

	bool currentCells[80 * 80];
	bool nextCells[80 * 80];

	bool changedCells[80 * 80];

	Action nextAction = Action::NONE;
	bool play;
	bool show;
	bool grid;

	bool stable;
	long steps;

	bool dragMode;
	int currentX;
	int currentY;

} Game;

void init(const char* title, int width, int height)
{
	SDL_Init(SDL_INIT_VIDEO);
	Game.window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
	Game.renderer = SDL_CreateRenderer(Game.window, -1, SDL_RENDERER_ACCELERATED);
	Game.running = true;
	Game.play = false;
	Game.show = true;
	Game.grid = true;
	Game.stable = false;
	Game.steps = 0;

	Game.currentX = -1;
	Game.currentY = -1;

	for (int x = 0; x < 800; x++)
	{
		for (int y = 0; y < 800; y++)
		{
			SDL_SetRenderDrawColor(Game.renderer, 0, 0, 0, 0xff);
			SDL_RenderDrawPoint(Game.renderer, x, y);
		}
	}

	for (int i = 0; i < 80; i++)
	{
		Game.changedCells[i] = false;
		Game.currentCells[i] = false;
	}

	SDL_RenderPresent(Game.renderer);
}

bool update_event()
{
	while (SDL_PollEvent(&Game.event_handler))
	{
		if (Game.event_handler.type == SDL_QUIT)
		{
			Game.running = false;
			break;
		}
		else if (Game.event_handler.type == SDL_KEYDOWN)
		{
			switch (Game.event_handler.key.keysym.sym)
			{
			case SDLK_g:
				Game.nextAction = Action::GRID;
				break;
			case SDLK_s:
				Game.nextAction = Action::SHOW;
				break;
			case SDLK_SPACE:	// pause/play
				Game.nextAction = Action::PLAYPAUSE;
				break;
			case SDLK_p:		// step
				Game.nextAction = Action::STEP;
				break;
			case SDLK_c:		// clear
				Game.nextAction = Action::CLEAR;
				break;
			case SDLK_r:		// reset
				Game.nextAction = Action::RESET;
				break;
			}
		}
		else if (!Game.play && Game.event_handler.type == SDL_MOUSEBUTTONDOWN)
		{
			int px = Game.event_handler.button.x / 10;
			int py = Game.event_handler.button.y / 10;

			if (px >= 0 && px < 80 && py >= 0 && py < 80)
			{
				Game.currentX = px;
				Game.currentY = py;

				Game.currentCells[px + py * 80] = Game.dragMode = !Game.currentCells[px + py * 80];
				Game.changedCells[px + py * 80] = true;
			}
			else
			{
				Game.currentX = -1;
				Game.currentY = -1;
			}
		}
		else if (!Game.play && Game.event_handler.type == SDL_MOUSEBUTTONUP)
		{
			Game.currentX = -1;
			Game.currentY = -1;
		}
		else if (!Game.play && Game.event_handler.type == SDL_MOUSEMOTION)
		{
			if (Game.currentX != -1)
			{
				int px = Game.event_handler.button.x / 10;
				int py = Game.event_handler.button.y / 10;

				if (px >= 0 && px < 80 && py >= 0 && py < 80)
				{
					if (Game.currentX != px || Game.currentY != py)
					{
						if (Game.currentCells[px + py * 80] != Game.dragMode)
						{
							Game.currentCells[px + py * 80] = !Game.currentCells[px + py * 80];
							Game.changedCells[px + py * 80] = true;
						}
						Game.currentX = px;
						Game.currentY = py;
					}
				}
			}
		}
	}

	return Game.running;
}

int get_live_neighbors(int x, int y)
{
	int count = 0;
	for (int xx = x-1; xx <= x+1; xx++)
	{
		for (int yy = y-1; yy <= y+1; yy++)
		{
			if (xx == x && yy == y) continue;
			if (xx < 0 || xx >= 80 || yy < 0 || yy >= 80) continue;

			if (Game.currentCells[xx + yy * 80]) count++;
		}
	}

	return count;
}

void process_step()
{
	Game.stable = true;

	int x,y,i,n;
	for (x = 0; x < 80; x++)
	{
		for (y = 0; y < 80; y++)
		{
			i = x + y * 80;
			n = get_live_neighbors(x, y);

			if ((Game.currentCells[i] && (n < 2 || n > 3)) || (!Game.currentCells[i] && n == 3))
			{
				Game.nextCells[i] = !Game.currentCells[i];
				Game.changedCells[i] = true;
				Game.stable = false;
			}
			else
			{
				Game.nextCells[i] = Game.currentCells[i];
				Game.changedCells[i] = true;
				Game.stable = false;
			}
		}
	}

	for (i = 0; i < 80 * 80; i++)
	{
		Game.currentCells[i] = Game.nextCells[i];
	}

	Game.steps++;
}

void draw_grid()
{
	int x, y, color = 0x2222227f;
	
	SDL_SetRenderDrawColor(Game.renderer, (color >> 24) & 0xff, (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
	for (x = 0; x < 800; x += 10)
	{
		SDL_RenderDrawLine(Game.renderer, x, 0, x, 799);
	}

	for (y = 0; y < 800; y += 10)
	{
		SDL_RenderDrawLine(Game.renderer, 0, y, 799, y);
	}

	SDL_RenderPresent(Game.renderer);
}

void update_graphic()
{
	if (Game.grid) draw_grid();

	int x, y, px, py, color;
	for (x = 0; x < 80; x++)
	{
		for (y = 0; y < 80; y++)
		{
			if (Game.changedCells[x + y * 80])
			{
				color = (Game.currentCells[x + y * 80]) ? 0xffffffff : 0x000000ff;
				for (px = 0; px < 10; px++)
				{
					for (py = 0; py < 10; py++)
					{
						SDL_SetRenderDrawColor(Game.renderer, (color >> 24) & 0xff, (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
						SDL_RenderDrawPoint(Game.renderer, x*10+px, y*10+py);
					}
				}
				Game.changedCells[x + y * 80] = false;
			}
		}
	}

	SDL_RenderPresent(Game.renderer);
}

void update()
{
	while (update_event())
	{
		if (Game.nextAction == Action::PLAYPAUSE)
		{
			Game.play = !Game.play;
			Game.nextAction = Action::NONE;

			if (Game.play)
				SDL_SetWindowTitle(Game.window, "Game of Life");
			else
				SDL_SetWindowTitle(Game.window, "Game of Life (Paused)");
		}
		else if (Game.nextAction == Action::SHOW)
		{
			Game.show = !Game.show;
			Game.nextAction = Action::NONE;
		}
		else if (Game.nextAction == Action::GRID)
		{
			Game.grid = !Game.grid;
			Game.nextAction = Action::NONE;
		}

		if (Game.play)
		{
			process_step();

			if (Game.stable)
			{
				Game.play = false;
				printf("Reached stable point after %ul steps!\n", Game.steps);
			}
		}
		else
		{
			switch (Game.nextAction)
			{
			case Action::STEP:
				process_step();
				break;
			case Action::CLEAR:
				for (int i = 0; i < 80 * 80; i++)
				{
					Game.changedCells[i] = Game.currentCells[i];
					Game.currentCells[i] = false;
				}
				Game.steps = 0;
				break;
			case Action::RESET:
				for (int i = 0; i < 80 * 80; i++)
				{
					Game.changedCells[i] = Game.currentCells[i];
					Game.currentCells[i] = false;
				}

				Game.currentCells[76 + 77 * 80] = Game.changedCells[76 + 77 * 80] = true;
				Game.currentCells[77 + 78 * 80] = Game.changedCells[77 + 78 * 80] = true;
				Game.currentCells[78 + 76 * 80] = Game.changedCells[78 + 76 * 80] = true;
				Game.currentCells[78 + 77 * 80] = Game.changedCells[78 + 77 * 80] = true;
				Game.currentCells[78 + 78 * 80] = Game.changedCells[78 + 78 * 80] = true;
				Game.steps = 0;
				break;
			}

			Game.nextAction = Action::NONE;
		}

		if (Game.show)
		{
			update_graphic();
		}

		//SDL_Delay(10);
	}
}

void exit()
{
	SDL_DestroyRenderer(Game.renderer);
	SDL_DestroyWindow(Game.window);
	SDL_Quit();
}

int main(int argc, char** argv)
{
	init("Game of Life", 800, 800);

	update();
	
	exit();

	return 0;
}