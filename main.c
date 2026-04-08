#include <SDL.h>
#include <SDL_image.h>	//包含SDL_Image库
#include <SDL_mixer.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
/*
* PS.为了简便地增加辅助区(搞边缘消除)，所有涉及map[][]处下标均+1。
*
* 带中文的字符串前记得加 u8
*
* Surface 表面 软件加速
* texture 纹理 支持硬件加速(更快)
*/
#define ANIMAL_WIDTH 80			//角色图片宽度
#define ANIMAL_HEIGHT 80		//角色图片高度
#define ANIMAL_NUMBER 30

#define MUSIC_NUMBER 3

#define ROWS 10
#define COLS 10

#define MAIN_WINDOW_WIDTH (ANIMAL_WIDTH * (ROWS + 2))	//主窗口宽
#define MAIN_WINDOW_HEIGHT (ANIMAL_HEIGHT * (COLS + 2))	//主窗口高

#define BLANKSPACE_WIDTH ((MAIN_WINDOW_WIDTH - ANIMAL_WIDTH * COLS) / 2)		//空出来的横像素数(调整以使角色居中)
#define BLANKSPACE_HEIGHT ((MAIN_WINDOW_HEIGHT - ANIMAL_HEIGHT * ROWS) / 2)	//空出来的纵像素数(调整以使角色居中)

#define swap(_a, _b)\
int _temp = _a; \
_a = _b; \
_b = _temp;


SDL_Window** MWindow;
typedef struct coord		//储存点的行、列
{
	int r;
	int c;
}coord;
coord begin = { -2, -2 }, end = { -2,-2 }, line[6][2];		//起止点行列和消除路径的存储
int lineCnt = 0;											//line数组存的点对数量
int elementNum;												//记录当前map里有效元素个数
int musicPlaying;										//记录当前播放的背景音乐是哪首
bool isRecord = false;		//记录是否点击过
bool isRunning = true;
bool isReplays = false;
int map[ROWS + 2][COLS + 2] = { 0 };

//图片
SDL_Texture* tex_bg, * tex_animals[ANIMAL_NUMBER + 1];
//音乐和音效
Mix_Music* bgmusic[MUSIC_NUMBER];
Mix_Chunk* clickWAV, * eliminateWAV, * winWAV;

void initMap()
{
	bool used[ANIMAL_NUMBER + 1] = { 0 };
	for (int i = 0; i < ROWS; i++)
	{
		int tmp;
		do
		{
			tmp = rand() % ANIMAL_NUMBER + 1;
		} while (used[tmp] == true);
		used[tmp] = true;
		for (int j = 0; j < COLS; j++)
		{
			map[i + 1][j + 1] = tmp;
		}
	}

	int r1 = 0, c1 = 0, r2 = 0, c2 = 0;

	for (int i = 0; i < ROWS * COLS * ROWS * COLS * ROWS * COLS; i++)
	{
		r1 = rand() % ROWS;
		c1 = rand() % COLS;
		r2 = rand() % ROWS;
		c2 = rand() % COLS;
		swap(map[r1 + 1][c1 + 1], map[r2 + 1][c2 + 1]);
	}
}

void drawLine(SDL_Renderer* render, coord* b, coord* e)		//传入画线两元素的行列
{
	SDL_Point p1, p2;
	p1.x = ANIMAL_WIDTH * (b->c + 0.5) + BLANKSPACE_WIDTH;
	p1.y = ANIMAL_HEIGHT * (b->r + 0.5) + BLANKSPACE_HEIGHT;
	p2.x = ANIMAL_WIDTH * (e->c + 0.5) + BLANKSPACE_WIDTH;
	p2.y = ANIMAL_HEIGHT * (e->r + 0.5) + BLANKSPACE_HEIGHT;
	int dr = b->r - e->r;	//delta row 行之差
	int dc = b->c - e->c;	//delta col 列之差
	if (dr)dr /= SDL_abs(dr);//化为+1或-1
	if (dc)dc /= SDL_abs(dc);//化为+1或-1
	if ((b->c == begin.c && b->r == begin.r) || (b->c == end.c && b->r == end.r))
	{
		p1.x -= 0.5 * dc * ANIMAL_WIDTH;
		p1.y -= 0.5 * dr * ANIMAL_HEIGHT;
		//SDL_Log("dr:%d dc:%d", dr, dc);		//测试用：输出行之差和列之差
	}
	dr = -dr;
	dc = -dc;
	if ((e->c == begin.c && e->r == begin.r) || (e->c == end.c && e->r == end.r))
	{
		p2.x -= 0.5 * dc * ANIMAL_WIDTH;
		p2.y -= 0.5 * dr * ANIMAL_HEIGHT;
		//SDL_Log("dr:%d dc:%d", dr, dc);		//测试用：输出行之差和列之差
	}

	SDL_RenderDrawLine(render, p1.x, p1.y, p2.x, p2.y);
}
void drawMap(SDL_Renderer* render)
{
	SDL_Rect bgDstRect = { 0,0,MAIN_WINDOW_WIDTH,MAIN_WINDOW_HEIGHT };
	SDL_Rect bgSrcRect = { 520,38,MAIN_WINDOW_WIDTH,MAIN_WINDOW_HEIGHT };
	SDL_RenderCopy(render, tex_bg, &bgSrcRect, &bgDstRect);
	for (int r = 0; r < ROWS; r++)//绘制角色
	{
		for (int c = 0; c < COLS; c++)
		{
			SDL_Rect dstRect = { ANIMAL_WIDTH * c + BLANKSPACE_WIDTH,ANIMAL_HEIGHT * r + BLANKSPACE_HEIGHT,ANIMAL_WIDTH,ANIMAL_HEIGHT };
			SDL_RenderCopy(render, tex_animals[map[r + 1][c + 1]], NULL, &dstRect);
		}
	}
	SDL_SetRenderDrawColor(render, 244, 13, 3, 255);
	SDL_Rect bRect = { ANIMAL_WIDTH * begin.c + BLANKSPACE_WIDTH,ANIMAL_HEIGHT * begin.r + BLANKSPACE_HEIGHT,ANIMAL_WIDTH,ANIMAL_HEIGHT };
	SDL_RenderDrawRect(render, &bRect);

	SDL_Rect eRect = { ANIMAL_WIDTH * end.c + BLANKSPACE_WIDTH,ANIMAL_HEIGHT * end.r + BLANKSPACE_HEIGHT,ANIMAL_WIDTH,ANIMAL_HEIGHT };
	SDL_RenderDrawRect(render, &eRect);

	for (int i = 1; i <= lineCnt; i++)	//测试
	{
		drawLine(render, &line[i][0], &line[i][1]);
		//printf("br:%d bc:%d  er:%d ec:%d\n", line[i][0].r, line[i][0].c, line[i][1].r, line[i][1].c);
	}
	//printf("\n");
	/*for (int i = 0; i < ROWS; i++)	//测试用
	{
		for (int j = 0; j < COLS; j++)
		{
			printf("%3d", map[i+1][j+1]);
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
	for (int i = 1; i <= ANIMAL_NUMBER; i++)
	{
		SDL_snprintf(str, 40, "assets/texture/%d.png", i);
		tex_animals[i] = loadTexture(Render, str);
	}
}
void loadMusic()
{
	clickWAV = Mix_LoadWAV("assets/audio/Click.wav");
	eliminateWAV = Mix_LoadWAV("assets/audio/Eliminate.wav");
	winWAV = Mix_LoadWAV("assets/audio/Win.wav");
	bgmusic[0] = Mix_LoadMUS(u8"assets/audio/陈致逸,HOYO-MiX - Moonlike Smile 皎洁的笑颜.flac");
	bgmusic[1] = Mix_LoadMUS(u8"assets/audio/陈致逸,HOYO-MiX - Fragile Fantasy 银白的希望.flac");
	bgmusic[2] = Mix_LoadMUS(u8"assets/audio/陈致逸,HOYO-MiX - Reminiscence (Genshin Impact Main Theme Var.) 追忆.flac");
	if (clickWAV == NULL || eliminateWAV == NULL || winWAV == NULL ||
		bgmusic[0] == NULL || bgmusic[1] == NULL || bgmusic[2] == NULL)
	{
		SDL_Log(u8"音乐加载失败，%s", SDL_GetError());
		isRunning = false;
	}
}


void lineClear()
{
	SDL_memset(line, 0, sizeof(line));
	lineCnt = 0;
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
		if (map[r1 + 1][c + 1])
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
		if (map[r + 1][c1 + 1])
			return false;	//如果有障碍则竖直无法消除
	}
	return true;			//能消除
}
bool straight(int r1, int c1, int r2, int c2)		//封装上两个函数，可以自动判断使用水平判断函数还是垂直判断函数
{
	if (r1 == r2 && horizontal(r1, c1, r2, c2))
	{
		lineCnt++;
		line[lineCnt][0] = (coord){ r1, c1 };
		line[lineCnt][1] = (coord){ r2, c2 };
		return true;
	}
	if (c1 == c2 && vertical(r1, c1, r2, c2))
	{
		lineCnt++;
		line[lineCnt][0] = (coord){ r1, c1 };
		line[lineCnt][1] = (coord){ r2, c2 };
		return true;
	}
	else return false;
}
bool turn_once(int r1, int c1, int r2, int c2)		//一次拐弯消除
{
	if (r1 == r2 || c1 == c2)		//如果行相同或者列相同就不判断了，节省计算成本(straight函数里判断过了)
		return false;
	struct coord t1 = { r1,c2 }, t2 = { r2,c1 };
	if (map[t1.r + 1][t1.c + 1] == 0)
	{
		if (straight(t1.r, t1.c, r1, c1) && straight(t1.r, t1.c, r2, c2))
			return true;
		else lineClear();
	}
	if (map[t2.r + 1][t2.c + 1] == 0)
	{
		if (straight(t2.r, t2.c, r1, c1) && straight(t2.r, t2.c, r2, c2))
			return true;
		else lineClear();
	}
	return false;			//不能消除
}
bool turn_twice(int r1, int c1, int r2, int c2)
{



	/*原来的bug：同一行不通时2次拐角可能通，但却return false而忽略了
		if (r1 == r2 || c1 == c2)		//如果行相同或者列相同就不判断了，节省计算成本(straight函数里判断过了)
			return false;*/





			//if (r1 == 0 || c1 == 0 || r1 == ROWS - 1 || c1 == COLS - 1 ||
			//	r2 == 0 || c2 == 0 || r2 == ROWS - 1 || c2 == COLS - 1)		//如果起点或终点在地图边缘则特判
			//{

			//}
	for (int r = -1; r < ROWS + 1; r++)	//遍历与起止点同行或同列的拐点
	{
		for (int c = -1; c < COLS + 1; c++)
		{
			if ((r == r1 || r == r2 || c == c1 || c == c2) && map[r + 1][c + 1] == 0)	//如果该点与起止点同行或同列并且不是角色则可作拐点
			{
				if (turn_once(r2, c2, r, c) && straight(r1, c1, r, c))
					return true;	//若拐点到起点直线通且一次拐到终点也通则可消除
				else lineClear();
				if (turn_once(r1, c1, r, c) && straight(r2, c2, r, c))
					return true;	//若拐点到终点直线通且一次拐到起点也通则也可消除
				else lineClear();
			}
		}
	}
	return false;
}
void clear()
{

	if (begin.r == end.r && begin.c == end.c)
		return;		//如果起止两个点是同一个点或不同种则无法消除
	//SDL_Log("can judge");

	bool isClear = false;	//记录是否能消除成功

	if (straight(begin.r, begin.c, end.r, end.c))
		isClear = true;
	else if (turn_once(begin.r, begin.c, end.r, end.c))
		isClear = true;
	else if (turn_twice(begin.r, begin.c, end.r, end.c))
		isClear = true;

	if (isClear)		//能消除成功则将起止点置为0
	{
		map[begin.r + 1][begin.c + 1] = 0;
		map[end.r + 1][end.c + 1] = 0;
		elementNum -= 2;
		Mix_PlayChannel(-1, eliminateWAV, 0);		//播放消除音效
		/*for (int i = 1; i <= lineCnt; i++)	//测试路径记录用
		{
			SDL_Log("Pair%d: r%2d, c%2d;   r%2d, c%2d", i, line[i][0].r, line[i][0].c, line[i][1].r, line[i][1].c);
		}
		printf("\n");*/
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
		if (map[mr + 1][mc + 1] == 0)
			return;				//如果点的位置没有角色则也不处理
		Mix_PlayChannel(-1, clickWAV, 0);	//播放点击音效
		if (isRecord)
		{
			end.c = mc;
			end.r = mr;
			if (map[begin.r + 1][begin.c + 1] != map[end.r + 1][end.c + 1])
			{
				begin = end;				//若起止点不同种则重新记录，将终点作为新的起点
				end = (coord){ -2,-2 };
				isRecord = false;			//与外部 isRecord = !isRecord 负负得正抵消掉，使下次点击时isRecord仍为true
			}
			else clear();	//两点同种则判断能否消除
			//SDL_Log("end");
		}
		else
		{
			begin.c = mc;
			begin.r = mr;
			//SDL_Log("begin");
		}
		isRecord = !isRecord;		//转换点击状态，实现点击第一个和第二个的分别处理
		//SDL_Log("row:%d col:%d", mr, mc);		//测试：输出所点击的角色的行和列
	}
}
void playNextMusic()	//用于播放下一首音乐
{
	musicPlaying = (musicPlaying + 1) % MUSIC_NUMBER;
	Mix_PlayMusic(bgmusic[musicPlaying], 0);
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
	SDL_Window* mainWindow = SDL_CreateWindow(u8"原神连连看v1.0 - Made by 文瑶", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	SDL_Renderer* mainRender = SDL_CreateRenderer(mainWindow, -1, 0);	//创建窗口渲染器
	MWindow = &mainWindow;
	musicPlaying = rand() % MUSIC_NUMBER;
	Mix_OpenAudio(44100, AUDIO_F32SYS, 2, 2048);
	loadMusic();
	loadTex(mainRender);
	Mix_PlayMusic(bgmusic[musicPlaying], 0);

	double tmp = 0;
	FILE* fp = fopen("assets/Data", "rb+");

	//Uint64 recordTime = 0;
	//Uint64 startTime = 0;
	//Uint64 currentTime = 0, lastTime = 0;

GameAgain:
	initMap();
	isReplays = false;		//默认不重玩
	elementNum = ROWS * COLS;	//初始化地图上剩余有效元素个数(待消除的个数)

	drawMap(mainRender);
	SDL_RenderPresent(mainRender);

	tmp = 0;
	if (fp)
	{
		rewind(fp);
		fread(&tmp, sizeof(double), 1, fp);
		//SDL_Log("tmp: %lf", tmp);
	}
	else
	{
		SDL_Log(u8"无法找到assets/Data文件!请自行创建。");
		return 114514;
	}

	Uint64 recordTime = tmp * 1000;
	Uint64 startTime = SDL_GetTicks64();
	Uint64 currentTime = 0, lastTime = 0;
	//SDL_Log("%lld\n", recordTime);
	//SDL_Delay(1000);

	while (isRunning)
	{
		Mix_HookMusicFinished(playNextMusic);		//播放完当前音乐时触发传入的回调函数，实现轮播音乐
		currentTime = SDL_GetTicks64() - startTime;	//记录当前所用时

#if 0	//X.x秒显示(十分位)
		if (currentTime > lastTime + 100)
		{
			system("cls");
			printf("%.1lf second", (currentTime / 1000.0));
			lastTime = currentTime;
		}
#else	//X秒显示(个位)
		if (currentTime > lastTime + 1000)
		{
			system("cls");
			printf("%lld second", currentTime / 1000);
			lastTime = currentTime;
		}
#endif
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
				{
					isRunning = false;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				mouseEvent(&ev.button);
				SDL_RenderClear(mainRender);
				drawMap(mainRender);
				SDL_RenderPresent(mainRender);

				if (isRecord == false)		//如果已经点了起止点则延迟清空框框并输出地图
				{
					SDL_Delay(200);
					begin.r = begin.c = -2;
					end.r = end.c = -2;
					lineClear();
					drawMap(mainRender);
					SDL_RenderPresent(mainRender);
					if (elementNum == 0)
					{
						Mix_PlayChannel(-1, winWAV, 0);		//播放游戏完成的音效
						if (recordTime == 0)
							recordTime = currentTime;
						else if (currentTime < recordTime)
							recordTime = currentTime;
						char str[100];
						SDL_snprintf(str, 100, u8"用时: %.3lf sec\n当前纪录: %.3lf sec\n(游戏内按Esc退出)\n\nBy 文瑶                                                \n\n", currentTime / 1000.0, recordTime / 1000.0);
						SDL_MessageBoxButtonData Botton[2] = {
							{SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, u8"退出"},
							{SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, u8"再来一把"},
						};
						//SDL_MessageBoxColorScheme ColorScheme = {
						//	{ /* .colors (.r, .g, .b) */
						//		/* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
						//		{  43, 145, 175 },
						//		/* [SDL_MESSAGEBOX_COLOR_TEXT] */
						//		{   0, 255,   0 },
						//		/* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
						//		{ 255, 255,   0 },
						//		/* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
						//		{   0,   0, 255 },
						//		/* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
						//		{ 255,   0, 255 }
						//	}
						//};
						SDL_MessageBoxData victory = { SDL_MESSAGEBOX_INFORMATION,*MWindow,"Congratulation!",str,2,Botton,NULL };
						int Bottonid = 0;
						SDL_ShowMessageBox(&victory, &Bottonid);
						if (Bottonid == 0)
						{
							isReplays = true;
							//fclose(fp);
						}
						if (Bottonid == 1)
						{
							isRunning = false;
						}
						double temp = recordTime / 1000.0;
						rewind(fp);
						fwrite(&temp, sizeof(Uint64), 1, fp);
					}
				}
			default:
				break;
			}
		}
		if (isReplays)goto GameAgain;		//如果要重玩就goto
	}


	if (fp != NULL)fclose(fp);

	Mix_FreeChunk(clickWAV);
	Mix_FreeChunk(eliminateWAV);
	Mix_FreeChunk(winWAV);
	for (int i = 0; i < MUSIC_NUMBER; i++)
	{
		Mix_FreeMusic(bgmusic[i]);
	}

	Mix_CloseAudio();
	Mix_Quit();
	SDL_DestroyWindow(mainWindow);		//销毁窗口
	SDL_DestroyRenderer(mainRender);
	SDL_Quit();		//退出SDL
	return 0;
}