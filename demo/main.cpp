#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <easyx.h>
#include <algorithm>
// 修正缺少的宏定义
#define SCREEN_WIDTH 474
#define SCREEN_HEIGHT 841

enum GameState { MENU, PLAYING, GAME_OVER };

enum ObjectType {
    PLAYER,
    BULLET_PLAYER,
    ENEMY_BIG,
    ENEMY_SMALL
};

struct GameObject {
    int x, y;
    int width, height;
    int speed;
    bool live;
    int hp;
    ObjectType type;
};

// 全局变量
GameObject player;
GameObject bullets[30];
GameObject enemies[10];
IMAGE imgBackground;
IMAGE imgPlayer[2];
IMAGE imgEnemy[2];
IMAGE imgBullet;
int score = 0;
GameState gameState = MENU;

// 函数声明
void loadResources();
void initGame();
void createBullet();
void createEnemy();
void updateGame();
void drawGame();
bool checkCollision(const GameObject& a, const GameObject& b);
void drawMenu();
void drawGameOver();

int main() {
    initgraph(SCREEN_WIDTH, SCREEN_HEIGHT); // 修正缺少的宏定义
    srand((unsigned)time(NULL));
    loadResources();
    
    while (true) {
        switch (gameState) {
            case MENU:
                drawMenu();
                if (_kbhit() && _getch() == '\r') {
                    initGame();
                    gameState = PLAYING;
                }
                break;
            case PLAYING:
                updateGame();
                drawGame();
                Sleep(10);
                break;
            case GAME_OVER:
                drawGameOver();
                if (_kbhit() && _getch() == '\r') {
                    score = 0;
                    gameState = MENU;
                }
                break;
        }
    }
    closegraph();
    return 0; // 修正 main 函数缺少返回值的问题
}

void loadResources() {
    // 加载图片（请确保图片路径正确）
    loadimage(&imgBackground, _T("image/bk.jpg"), SCREEN_WIDTH, SCREEN_HEIGHT);
    loadimage(&imgPlayer[0], _T("image/me.jpg"), 80, 140);
    loadimage(&imgPlayer[1], _T("image/deadme.gif"), 80, 140);
    loadimage(&imgEnemy[0], _T("image/enemyB.png"), 100, 100);
    loadimage(&imgEnemy[1], _T("image/enemyS.png"), 50, 50);
    loadimage(&imgBullet, _T("image/Bullet.png"), 10, 30);
}

void initGame() {
    player = {
        SCREEN_WIDTH / 2 - 40,  
        SCREEN_HEIGHT - 140,    
        80, 140,                
        15,                     
        true,                   
        3,                      
        PLAYER                  
    };

    for (auto& bullet : bullets) bullet.live = false;
    for (auto& enemy : enemies) enemy.live = false;
}

// 新增UI绘制函数
void drawMenu() {
    cleardevice();
    settextcolor(WHITE);
    settextstyle(40, 0, _T("微软雅黑"));
    outtextxy(100, 200, _T("飞机大战"));
    settextstyle(20, 0, _T("微软雅黑"));
    outtextxy(150, 300, _T("按回车开始游戏"));
}

void drawGameOver() {
    cleardevice();
    settextcolor(RED);
    settextstyle(40, 0, _T("微软雅黑"));
    outtextxy(120, 300, _T("游戏结束！"));
    TCHAR scoreText[50];
    swprintf(scoreText, _T("得分：%d  按回车重新开始"), score);
    settextstyle(20, 0, _T("微软雅黑"));
    outtextxy(100, 400, scoreText);
}

void updateGame() {
    if (gameState != PLAYING) return;

    // 玩家控制
    if (_kbhit()) {
        char key = _getch();
        switch (key) {
        case 'w': player.y = max(0, player.y - player.speed); break;
        case 's': player.y = min(SCREEN_HEIGHT - player.height, player.y + player.speed); break;
        case 'a': player.x = max(0, player.x - player.speed); break;
        case 'd': player.x = min(SCREEN_WIDTH - player.width, player.x + player.speed); break;
        case 'j': createBullet(); break;
        }
    }

    // 子弹移动
    for (auto& bullet : bullets) {
        if (bullet.live) {
            bullet.y -= bullet.speed;
            if (bullet.y < -bullet.height) bullet.live = false;
        }
    }

    // 敌机生成
    static int enemyTimer = 0;
    if (++enemyTimer >= 50) {
        createEnemy();
        enemyTimer = 0;
    }

    // 敌机移动
    for (auto& enemy : enemies) {
        if (enemy.live) {
            enemy.y += enemy.speed;
            if (enemy.y > SCREEN_HEIGHT) enemy.live = false;
        }
    }

    // 碰撞检测
    for (auto& bullet : bullets) {
        if (!bullet.live) continue;
        for (auto& enemy : enemies) {
            if (enemy.live && checkCollision(bullet, enemy)) {
                bullet.live = false;
                if (--enemy.hp <= 0) {
                    enemy.live = false;
                    score += (enemy.type == ENEMY_BIG) ? 30 : 10;
                }
            }
        }
    }

    // 玩家与敌机碰撞
    for (auto& enemy : enemies) {
        if (enemy.live && checkCollision(player, enemy)) {
            player.live = false;
            gameState = GAME_OVER;
        }
    }
}

void drawGame() {
    if (gameState != PLAYING) return;

    cleardevice();
    putimage(0, 0, &imgBackground);

    // 绘制分数
    settextcolor(YELLOW);
    settextstyle(20, 0, _T("微软雅黑"));
    TCHAR scoreText[50];
    swprintf(scoreText, _T("得分：%d"), score);
    outtextxy(10, 10, scoreText);

    // 绘制玩家
    if (player.live) {
        putimage(player.x, player.y, &imgPlayer[0]);
    }

    // 绘制子弹
    for (const auto& bullet : bullets) {
        if (bullet.live) {
            putimage(bullet.x, bullet.y, &imgBullet);
        }
    }

    // 绘制敌机
    for (const auto& enemy : enemies) {
        if (enemy.live) {
            const IMAGE* img = (enemy.type == ENEMY_BIG) ? &imgEnemy[0] : &imgEnemy[1];
            putimage(enemy.x, enemy.y, img);
        }
    }
    FlushBatchDraw();
}

void createBullet() {
    for (auto& bullet : bullets) {
        if (!bullet.live) {
            bullet = {
                player.x + player.width/2 - 5,  
                player.y - 30,                  
                10, 30,                        
                20,                            
                true,                          
                1,                             
                BULLET_PLAYER                  
            };
            return;
        }
    }
}

void createEnemy() {
    for (auto& enemy : enemies) {
        if (!enemy.live) {
            if (rand() % 10 < 3) {
                enemy.type = ENEMY_BIG;
                enemy.width = 100;
                enemy.height = 100;
                enemy.hp = 3;
            } else {
                enemy.type = ENEMY_SMALL;
                enemy.width = 50;
                enemy.height = 50;
                enemy.hp = 1;
            }
            
            enemy.x = rand() % (SCREEN_WIDTH - enemy.width);
            enemy.y = -enemy.height;
            enemy.speed = 2;
            enemy.live = true;
            return;
        }
    }
}

bool checkCollision(const GameObject& a, const GameObject& b) {
    return (a.x < b.x + b.width) &&
           (a.x + a.width > b.x) &&
           (a.y < b.y + b.height) &&
           (a.y + a.height > b.y);
}