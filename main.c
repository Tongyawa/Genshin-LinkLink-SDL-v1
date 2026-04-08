#include <SDL.h>
#include <SDL_image.h>	//包含SDL_Image库
#include <SDL_mixer.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define ANIMAL_WIDTH 56			//动物图片宽度
#define ANIMAL_HEIGHT 56		//动物图片高度

#define ROWS 10
#define COLS 10

#define MAIN_WINDOW_WIDTH (ANIMAL_WIDTH * (ROWS + 2))	//主窗口宽
#define MAIN_WINDOW_HEIGHT (ANIMAL_HEIGHT * (COLS + 6))	//主窗口高

#define BLANKSPACE_WIDTH ((MAIN_WINDOW_WIDTH - ANIMAL_WIDTH * COLS) / 2)		//空出来的横像素数(调整以使动物居中)
#define BLANKSPACE_HEIGHT ((MAIN_WINDOW_HEIGHT - ANIMAL_HEIGHT * ROWS) / 2)	//空出来的纵像素数(调整以使动物居中)

#define swap(_a, _b)\
int _temp = _a; \
_a = _b; \
_b = _temp;

/*
* 带中文的字符串前记得加 u8
*
* Surface 表面 软件加速
* texture 纹理 支持硬件加速(更快)
*/

struct coord		//储存点的行、列
{
	int r;
	int c;
}begin = { -1, -1 }, end = { -1,-1 };
bool isRecord = false;		//记录是否点击过
bool isRunning = true;
int map[ROWS][COLS] = { 0 };

//图片
SDL_Texture* tex_bg, * tex_animals[17];

void initMap()
{
	bool used[17] = { 0 };
	for (int i = 0; i < ROWS; i++)
	{
		int tmp;
		do
		{
			tmp = rand() % 16 + 1;
		} while (used[tmp] == true);
		used[tmp] = true;
		for (int j = 0; j < COLS; j++)
		{
			map[i][j] = tmp;
		}
	}

	int r1 = 0, c1 = 0, r2 = 0, c2 = 0;

	for (int i = 0; i < ROWS * COLS * ROWS * COLS * ROWS * COLS; i++)
	{
		r1 = rand() % ROWS;
		c1 = rand() % COLS;
		r2 = rand() % ROWS;
		c2 = rand() % COLS;
		swap(map[r1][c1], map[r2][c2]);
	}
}

void drawMap(SDL_Renderer* render)
{
	SDL_Rect bgRect = { 0,0,MAIN_WINDOW_WIDTH,MAIN_WINDOW_HEIGHT };
	SDL_RenderCopy(render, tex_bg, NULL, &bgRect);
	for (int r = 0; r < ROWS; r++)//绘制动物
	{
		for (int c = 0; c < COLS; c++)
		{
			SDL_Rect dstRect = { ANIMAL_WIDTH * c + BLANKSPACE_WIDTH,ANIMAL_HEIGHT * r + BLANKSPACE_HEIGHT,ANIMAL_WIDTH,ANIMAL_HEIGHT };
			SDL_RenderCopy(render, tex_animals[map[r][c]], NULL, &dstRect);
		}
	}
	/*for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			printf("%3d", map[i][j]);
		}
		putchar('\n');
	}*/
}

SDL_Texture* loadTexture(SDL_Renderer* render, const char* file)	//封装版给Render加载一张图片
{
	SDL_Surface* sfc = IMG_Load(file);
	if (!sfc)
	{
		SDL_Log("load failed, %s\n", SDL_GetError());
	}
	SDL_Texture* tex = SDL_CreateTextureFromSurface(render, sfc);
	SDL_FreeSurface(sfc);
	return tex;
}

void loadTex(SDL_Renderer* Render)
{
	tex_bg = loadTexture(Render, "assets/texture/bg.png");
	char str[40];
	for (int i = 1; i <= 16; i++)
	{
		SDL_snprintf(str, 40, "assets/texture/%d.png", i);
		tex_animals[i] = loadTexture(Render, str);
	}
}


bool horizontal(int r1, int c1, int r2, int c2)		//水平消除判断
{
	//if (r1 != r2)
	//	return false;		//已在straight函数中进行判断
	if (c1 > c2)
	{
		swap(c1, c2);		//保证从小的遍历到大的
	}
	for (int c = c1 + 1; c < c2; c++)
	{
		if (map[r1][c])
			return false;	//如果有障碍则水平无法消除
	}
	return true;			//能消除
}
bool vertical(int r1, int c1, int r2, int c2)		//竖直消除判断
{
	//if (c1 != c2)
	//	return false;	//已在straight函数中进行判断
	if (r1 > r2)
	{

//
		swap(r1, r2);		//保证从小的遍历到大的(一开始放的swap(c1,c2))
//原来的bug

	
	}
	for (int r = r1 + 1; r < r2; r++)
	{
		if (map[r][c1])
			return false;	//如果有障碍则竖直无法消除
	}
	return true;			//能消除
}
bool straight(int r1, int c1, int r2, int c2)		//封装上两个函数，可以自动判断使用水平判断函数还是垂直判断函数
{
	if (r1 == r2)
		return horizontal(r1, c1, r2, c2);
	if (c1 == c2)
		return vertical(r1, c1, r2, c2);
	else return false;
}

bool turn_once(int r1, int c1, int r2, int c2)		//一次拐弯消除
{
	if (r1 == r2 || c1 == c2)		//如果行相同或者列相同就不判断了，节省计算成本(straight函数里判断过了)
		return false;
	struct coord t1 = { r1,c2 }, t2 = { r2,c1 };
	if (map[t1.r][t1.c] == 0)
	{
		if (straight(t1.r, t1.c, r1, c1) && straight(t1.r, t1.c, r2, c2))
			return true;
	}
	if (map[t2.r][t2.c] == 0)
	{
		if (straight(t2.r, t2.c, r1, c1) && straight(t2.r, t2.c, r2, c2))
			return true;
	}
	return false;			//不能消除
}

bool turn_twice(int r1, int c1, int r2, int c2)
{



	//原来的bug：同一行不通时2次拐角可能通，但却return false而忽略了
		//if (r1 == r2 || c1 == c2)		//如果行相同或者列相同就不判断了，节省计算成本(straight函数里判断过了)
		//	return false;






	if (r1 == 0 || c1 == 0 || r1 == ROWS - 1 || c1 == COLS - 1 ||
		r2 == 0 || c2 == 0 || r2 == ROWS - 1 || c2 == COLS - 1)		//如果起点或终点在地图边缘则特判
	{

	}
	for (int r = 0; r < ROWS; r++)	//遍历与起止点同行或同列的拐点
	{
		for (int c = 0; c < COLS; c++)
		{
			if ((r == r1 || r == r2 || c == c1 || c == c2) && map[r][c] == 0)	//如果该点与起止点同行或同列并且不是动物则可作拐点
			{
				if (straight(r1, c1, r, c) && turn_once(r2, c2, r, c))
					return true;	//若拐点到起点直线通且一次拐到终点也通则可消除
				if (straight(r2, c2, r, c) && turn_once(r1, c1, r, c))
					return true;	//若拐点到终点直线通且一次拐到起点也通则也可消除
			}
		}
	}
	return false;
}

void clear()
{

	if ((begin.r == end.r && begin.c == end.c) || map[begin.r][begin.c] != map[end.r][end.c])
		return;		//如果起止两个点是同一个点或不同种则无法消除
	//SDL_Log("can judge");

	bool isClear = false;	//记录是否能消除成功

	isClear |= straight(begin.r, begin.c, end.r, end.c);
	isClear |= turn_once(begin.r, begin.c, end.r, end.c);
	isClear |= turn_twice(begin.r, begin.c, end.r, end.c);

	if (isClear)		//能消除成功则将起止点置为0
	{
		map[begin.r][begin.c] = 0;
		map[end.r][end.c] = 0;
	}
}


void mouseEvent(SDL_MouseButtonEvent* ev)
{
	//SDL_Log("%s", __FUNCTION__);
	if (ev->button == SDL_BUTTON_LEFT)
	{
		if (ev->x < BLANKSPACE_WIDTH || ev->y < BLANKSPACE_HEIGHT || ev->x > MAIN_WINDOW_WIDTH - BLANKSPACE_WIDTH || ev->y > MAIN_WINDOW_HEIGHT - BLANKSPACE_HEIGHT)
			return;				//如果点到游戏区外则不处理
		int mc = (ev->x - BLANKSPACE_WIDTH) / ANIMAL_WIDTH;		//计算点击的行(MouseCol)
		int mr = (ev->y - BLANKSPACE_HEIGHT) / ANIMAL_HEIGHT;	//计算点击的列(MouseRow)
		if (map[mr][mc] == 0)
			return;				//如果点的位置没有动物则也不处理
		if (isRecord)
		{
			end.c = mc;
			end.r = mr;
			//SDL_Log("end");
			clear();
		}
		else
		{
			begin.c = mc;
			begin.r = mr;
			//SDL_Log("begin");
		}
		isRecord = !isRecord;		//转换点击状态，实现点击第一个和第二个的分别处理
		//SDL_Log("row:%d col:%d", mr, mc);		//测试：输出所点击的动物的行和列
	}
}


int main(int argc, char* argv[])	//SDL项目中，main函数必须带参数
{
	srand(time(NULL));
	//初始化SDL
	if ((0 != SDL_Init(SDL_INIT_VIDEO))		//初始化视频(或者SDL_INIT_EVERYTHING)
		&& (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) != (IMG_INIT_JPG | IMG_INIT_PNG))		//初始化IMG_image并初始化jpg和png图片类型
		&& (Mix_Init(MIX_INIT_MP3 | MIX_INIT_FLAC | MIX_INIT_OGG) != (MIX_INIT_MP3 | MIX_INIT_FLAC | MIX_INIT_OGG)))	//初始化IMG_mixer并初始化jpg和png图片类型
	{
		SDL_Log("SDL or SDL_image or SDL_mixer init failed, %s\n", SDL_GetError());
		return -1;
	}

	//创建  未  命  名  窗口，在屏幕上的位置为默认，窗口默认为显示状态
	SDL_Window* mainWindow = SDL_CreateWindow(u8"连连看", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	SDL_Renderer* mainRender = SDL_CreateRenderer(mainWindow, -1, 0);	//创建窗口渲染器

	loadTex(mainRender);
GameAgain:
	initMap();
	while (isRunning)
	{
		SDL_Event ev = { 0 };
		if (SDL_PollEvent(&ev))
		{
			switch (ev.type)
			{
			case SDL_QUIT:
				isRunning = false;
				break;
			case SDL_KEYDOWN:
				if (ev.key.keysym.sym == SDLK_ESCAPE)
					SDL_Log("ESC\n");
				break;
			case SDL_MOUSEBUTTONDOWN:
				mouseEvent(&ev.button);
			default:
				break;
			}
		}
		SDL_RenderClear(mainRender);
		drawMap(mainRender);
		SDL_RenderPresent(mainRender);
	}




	Mix_CloseAudio();
	Mix_Quit();
	SDL_DestroyWindow(mainWindow);		//销毁窗口
	SDL_DestroyRenderer(mainRender);
	SDL_Quit();		//退出SDL
	return 0;
}