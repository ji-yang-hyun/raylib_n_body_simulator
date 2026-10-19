#include <iostream>
#include <raylib.h>
#include<vector>
#include <cstdlib>
using namespace std;

float realG = 6.6743 * pow(10,-11);


float vectorSize(Vector2 v){
    return sqrt(pow(v.x,2) + pow(v.y,2));
}
Vector2 vectorTime(Vector2 v, float k){
    return {v.x*k, v.y*k};
}
float vectorDot(Vector2 v1, Vector2 v2){
    return v1.x*v2.x + v1.y*v2.y;
}
Vector2 vectorSum(Vector2 v1, Vector2 v2){
    return {v1.x + v2.x, v1.y + v2.y};
}

float growSpeed = 100;
float screenSpeed = 100;
float G = 50000;
float accelerationVisualScale = 1.0;
float circleSize = 30;
unsigned char alpha = 177;
float screenW = 1300;
float screenH = 1000;

float dt, playTime = 0;
float clickTime, clickStartTime, clickEndTime;
int isClicking = 0;
bool isPlaying = false;
RenderTexture2D traceCanvas;
RenderTexture2D mapCanvas;

// 맵 설정
Vector2 playerInitPosition = {400, 500};
Vector2 playerInitVelocity = {0,0};
float playerInitMass = 30;
float playerInitSize = 30;
Color playerInitColor = {0,0,255,alpha};

struct Rect{
    vector<Vector2> pointPositoins = {}; //ABCD순서. 노트에 있음
    Color color;
};

vector<Vector2> goalPositionRect = {{0,0}, {0,100}, {100,100}, {100,0}};
Color goalColor = {0,0,255,255};

vector<Rect> map = {{goalPositionRect, goalColor}};

void drawRect(Rect rect){
    DrawTriangle(rect.pointPositoins[0],rect.pointPositoins[1],rect.pointPositoins[3],rect.color);
    DrawTriangle(rect.pointPositoins[1],rect.pointPositoins[2],rect.pointPositoins[3],rect.color);
}

struct Ball{
    Vector2 position;
    Vector2 velocity;
    vector<Vector2> forces;

    float mass;
    bool isAnchor;
    float size;

    Color color;
};

vector<Ball> balls = {
    {
        playerInitPosition, playerInitVelocity, {{0,0}}, playerInitMass, false, playerInitSize, playerInitColor
    }
};

void mouseLeft(){
    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
        Vector2 clickPoint = GetMousePosition();
        if(!isClicking){
            Vector2 newMassPos = clickPoint;
            unsigned char  randNumR = char(rand()%256);
            unsigned char randNumG = char(rand()%256);
            unsigned char randNumB = char(rand()%100 + 155);
            isClicking = 1;
            clickStartTime = GetTime();

            balls.push_back(
                {
                newMassPos,
                {0,0},
                {{0,0}},
                0.0,
                false,
                0.0,
                {randNumR,randNumG,randNumB,alpha}
                }
            );
        }
        if(isClicking){
            balls[balls.size() - 1].size += dt*growSpeed;
            balls[balls.size() - 1].mass += dt*growSpeed;
            balls[balls.size() - 1].velocity.x = clickPoint.x - balls[balls.size() - 1].position.x;
            balls[balls.size() - 1].velocity.y = clickPoint.y - balls[balls.size() - 1].position.y;
        }
    }
    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
        Vector2 clickPoint = GetMousePosition();
        isClicking = 0;
        clickEndTime = GetTime();
    }
}

void mouseRight(){
    if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
        Vector2 clickPoint = GetMousePosition();
        if(!isClicking){
            Vector2 newMassPos = clickPoint;
            unsigned char  randNumR = char(rand()%256);
            unsigned char randNumG = char(rand()%256);
            unsigned char randNumB = char(rand()%100 + 155);
            isClicking = 1;
            clickStartTime = GetTime();
            balls.push_back(
                {
                newMassPos,
                {0,0},
                {{0,0}},
                circleSize,
                true,
                circleSize,
                {randNumR,randNumG,randNumB,alpha}
                }
            );
        }
        if(isClicking){
            
        }
    }
    if(IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)){
        Vector2 clickPoint = GetMousePosition();
        isClicking = 0;
        clickEndTime = GetTime();
    }
}

void keySpace(){
    if(IsKeyPressed(KEY_SPACE)){
        isPlaying = !isPlaying;
    }
}

void physics(){
    // 완전탄성충돌에 의한 상호간의 힘, 가속도, 속도 업데이트 or 만유인력에 따른 상호간의 힘, 가속도, 속도 업데이트

    for(int i = 0; i < balls.size() - isClicking; i++){
        balls[i].forces = {};
    }

    for(int i = 0; i < balls.size() - isClicking; i++){
        Vector2 netForce = {0,0};
        for(int j = 0; j < balls.size() - isClicking; j++){
            if(i == j){
                balls[i].forces.push_back({0,0});
            }
            else{
                float minDistance = balls[i].mass + balls[j].mass;
                Vector2 forceDirection = vectorSum(balls[j].position, vectorTime(balls[i].position, -1.0));
                float distance = vectorSize(forceDirection);
                forceDirection = vectorTime(forceDirection, 1/distance);

                float f = 0;
                
                if(distance <= minDistance){
                    // 물리적으로는 있을 수 없지만 연산 한계로 여러번 카운팅 우려.
                    if(vectorDot(forceDirection, balls[i].velocity) > 0){ 
                        Vector2 perpendicularVelocity = vectorTime(forceDirection, vectorDot(forceDirection, balls[i].velocity));
                        float dv = vectorSize(perpendicularVelocity)*(-2);
                        f = balls[i].mass * dv / dt; 
                        // 여기서 숫자가 너무 커졌다가 다시 작아지면서 오차 발생하는듯. 근데 구조성 유지하려면 쩔수임.
                    }
                }
                else{
                    f = G * balls[i].mass * balls[j].mass/pow(distance,2);
                }

                if(balls[i].isAnchor){
                    f += f*(-1); // 앵커가 공이 받는 힘을 상쇄.
                }

                balls[i].forces.push_back(vectorTime(forceDirection, f));
                netForce =  vectorSum(netForce, vectorTime(forceDirection, f));
            }
        }
        balls[i].velocity = vectorSum(balls[i].velocity, vectorTime(netForce, 1/balls[i].mass*dt));
    }

    // 속도에 따른 위치 업데이트
    for(int i = 0; i < balls.size() - isClicking; i++){
        balls[i].position = vectorSum(balls[i].position, vectorTime(balls[i].velocity, dt));
    }
}

void drawScreen(){
    BeginTextureMode(traceCanvas);
    for (Ball ball : balls){
        Color traceColor = ball.color;
        traceColor.a = 255;
        DrawCircleV(ball.position, 1, traceColor);
    }
    EndTextureMode();
    DrawTextureRec(traceCanvas.texture, {0, 0, screenW, -screenH}, {0,0}, WHITE);
    DrawTextureRec(mapCanvas.texture, {0, 0, screenW, -screenH}, {0,0}, WHITE);

    
    Vector2 drawPosition = {}; // 편의성을 위한 더미 버퍼 벡터
    BeginDrawing();
    ClearBackground(WHITE);
    for (Ball ball : balls){
        Color traceColor = ball.color;
        traceColor.a = 255;
        DrawCircleV(ball.position, ball.size, ball.color);
        DrawLineEx(ball.position, vectorSum(ball.position, ball.velocity), 7.7, traceColor);
        for (int j = 0; j < ball.forces.size() - 1; j++){
            DrawLineEx(ball.position, vectorSum(ball.position, vectorTime(ball.forces[j], 1/ball.mass)), 7.0, balls[j].color);
        }
    }
    EndDrawing();
}

void gameSystem(){

}

void drawMap(){
    BeginTextureMode(mapCanvas);
    for(Rect rect : map){
        drawRect(rect);
    }
    EndTextureMode();
}

int main(){

    srand(2008);

    InitWindow(screenW, screenH,"hello world");
    SetTargetFPS(120);
    traceCanvas = LoadRenderTexture(screenW, screenH);
    mapCanvas = LoadRenderTexture(screenW, screenH);
    drawMap();

    while(!WindowShouldClose()){
        dt = GetFrameTime();
        playTime = GetTime();

        mouseLeft();
        mouseRight();
        keySpace();

        if(isPlaying){
            physics();
        }

        drawScreen();
    }

    CloseWindow();
    return 0;
}

/*
clang++ game_code.cpp -std=c++17 -o game $(pkg-config --cflags --libs raylib)
./game
*/