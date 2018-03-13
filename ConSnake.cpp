#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long long qword;
              
HANDLE hIn   = 0;
HANDLE hOut  = 0;              
bool bQuit   = false;
bool bGOver  = false;

qword time   = 0;

int width    = 36;
int height   = 18;
int maxsize  = 200;

struct Cell
{
  char ch;
  byte color;
};

Cell palette[ ] = {
  { '.', 0x08 },  // empty
  { ' ', 0x88 },  // wall
  { 'o', 0x0D },  // snake
  { 'a', 0x0E }   // the cranberry
};

struct XY
{
  byte x;
  byte y;
};

byte *map    = 0;
XY *snake    = 0;

int posx     = 5;
int posy     = 3;

int velx     = 0;
int vely     = 0;

int nvelx    = 0;
int nvely    = 0;

int length   = 0;
int delta    = 0;
int counter  = 0;

void setColor(byte color)
{
  SetConsoleTextAttribute(hOut, color);
}

void setPos(int x, int y)
{
  COORD coord;
  coord.X = x;
  coord.Y = y;
  SetConsoleCursorPosition(hOut, coord);
}

void write(const char *s)
{
  int num;
  WriteConsole(hOut, s, strlen(s), (PDWORD)&num, 0);
}

void put(char ch)
{
  int num;
  WriteConsole(hOut, &ch, 1, (PDWORD)&num, 0);
}

void cranberry()
{
  int x, y;

  do
  {
    x = 1 + rand() % (width - 2);
    y = 1 + rand() % (height - 2);
  }
  while (map[y * width + x] != 0);

  map[y * width + x] = 3;
}

void render()
{
  if (bGOver)
    return;

  for (int i = 0, index = 0; i < height; i++)
  {
    setPos(posx, posy + i);
    for (int j = 0; j < width; j++, index++)
    {
      setColor(palette[map[index]].color);
      put(palette[map[index]].ch);
    }
  }

  setColor(0x1E);

  setPos(posx + width / 2 - 2, posy - 2);
  write("Snake");

  char buff[20];
  sprintf(buff, "SCORE: %d", length - 1);
  setPos(posx + width + 3, posy + 2);
  write(buff);

  setPos(posx + width + 3, posy + 4);
  write("Use arrow buttons.");

  setPos(posx + width + 3, posy + 5);
  write("Enter  -- restart.");

  setPos(posx + width + 3, posy + 6);
  write("Escape -- exit.");
}                       

void init()
{                
  bGOver = false;

  velx = nvelx = 0;
  vely = nvely = 0;
  length = 1;
  delta = 350;
  counter = 0;

  for (int j = 0; j < width; j++)
    map[j] = map[width * (height - 1) + j] = 1;

  for (int i = 1; i < height - 1; i++)
    map[i * width] = map[(i + 1) * width - 1] = 1;

  for (int i = 1, j, index = width - 1; i < height - 1; i++)
    for (j = 1, index += 2; j < width - 1; j++, index++)
      map[index] = 0;

  time = GetTickCount();
  srand((int)time);

  snake[0].x = 1 + rand() % (width - 2);
  snake[0].y = 1 + rand() % (height - 2);

  map[snake[0].y * width + snake[0].x] = 2;

  cranberry();
  render();
}

void update(int msec)
{
  if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
  {
    bQuit = true;
    return;
  }

  if (GetAsyncKeyState(VK_RETURN) & 0x8000)
  {
    init();
    return;
  }

  if (bGOver)
    return;

  if (GetAsyncKeyState(VK_LEFT) & 0x8000)
    nvelx = -1, nvely = 0;
  else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
    nvelx = +1, nvely = 0;
  else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
    nvelx = 0, nvely = +1;
  else if (GetAsyncKeyState(VK_UP) & 0x8000)
    nvelx = 0, nvely = -1;
  
  counter += msec;

  if (counter < delta)
    return;

  counter -= delta;

  if (nvelx == 0 && nvely == 0)
    return;

  if (nvelx != -velx && nvely != -vely
   || velx == 0 && vely == 0)
  {
    velx = nvelx;
    vely = nvely;
  }

  for (int i = length; i > 0; i--)
    snake[i] = snake[i - 1];

  snake[0].x += velx;
  snake[0].y += vely;

  int index = snake[0].y * width + snake[0].x;             

  if (map[index] == 3)
  {
    map[index] = 2;
    cranberry();
    delta -= 1 + (delta / 25);

    if (length < maxsize)
      length++;
    else
      map[snake[length].y * width + snake[length].x] = 0;

    render();
  }
  else if (map[index] == 0)
  {                                         
    map[snake[length].y * width + snake[length].x] = 0;
    map[index] = 2;
    render();
  }
  else
  {
    bGOver = true;

    render();

    setPos(width / 2 - 5, height / 2);
    write("Game over!");

    char buff[20];
    sprintf(buff, "SCORE: %d", length - 1);
    setColor(0x1E);
    setPos(width / 2 - 5, height / 2 + 2);
    write(buff);
  }        
}

void frame()
{
  int msec = GetTickCount() - time;

  if (msec > 0)
  {                            
    update(msec);

    time += msec;
  }
}

int main()
{
  hIn = GetStdHandle(STD_INPUT_HANDLE);
  hOut = GetStdHandle(STD_OUTPUT_HANDLE);

  CONSOLE_CURSOR_INFO curInfo;
  curInfo.dwSize = 100;
  curInfo.bVisible = 0;
                                      
  SetConsoleCursorInfo(hOut, &curInfo);
  SetConsoleMode(hOut, ENABLE_PROCESSED_OUTPUT);
                                 
  map = new byte[width * height];
  snake = new XY[maxsize + 1];

  setColor(0x17);
  system("cls");

  init();
    
  while (!bQuit)
  {
    frame();
    Sleep(10);
  }

  delete[] map;
  delete[] snake;

  setColor(0x07);
  system("cls");

  return 0;
}
