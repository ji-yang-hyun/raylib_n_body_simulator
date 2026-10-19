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

float dt, playTime = 0;
float clickTime, clickStartTime, clickEndTime;
int isClicking = 0;
bool isPlaying = false;
Vector2 screenPos = {0,0};
RenderTexture2D traceCanvas;

struct Ball{
    Vector2 position;
    Vector2 velocity;
    vector<Vector2> forces;

    float mass;
    bool isAnchor;
    float size;

    Color color;
};

vector<Ball> balls = {};


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
                {newMassPos.x + screenPos.x, newMassPos.y + screenPos.y},
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
            balls[balls.size() - 1].velocity.x = clickPoint.x + screenPos.x - balls[balls.size() - 1].position.x;
            balls[balls.size() - 1].velocity.y = clickPoint.y + screenPos.y - balls[balls.size() - 1].position.y;
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
                {newMassPos.x + screenPos.x, newMassPos.y + screenPos.y},
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

void moveScreen(){
    if(IsKeyDown(KEY_W)){
        screenPos.y -= screenSpeed*dt;
    }

    if(IsKeyDown(KEY_S)){
        screenPos.y += screenSpeed*dt;
    }

    if(IsKeyDown(KEY_A)){
        screenPos.x -= screenSpeed*dt;
    }

    if(IsKeyDown(KEY_D)){
        screenPos.x += screenSpeed*dt;
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
    // BeginTextureMode(traceCanvas);
    // for (int i = 0; i < masses.size(); i++){
    //     Color traceColor = colors[i];
    //     traceColor.a = 255;
    //     DrawCircle(positions[i].x, positions[i].y, 1, traceColor);
    // }
    // EndTextureMode();
    Vector2 drawPosition = {}; // 편의성을 위한 더미 버퍼 벡터
    BeginDrawing();
    ClearBackground(WHITE);
    for (Ball ball : balls){
        drawPosition = vectorSum(ball.position, vectorTime(screenPos, -1));
        Color traceColor = ball.color;
        traceColor.a = 255;
        DrawCircleV(drawPosition, ball.size, ball.color);
        DrawLineEx(drawPosition, vectorSum(drawPosition, ball.velocity), 7.7, traceColor);
        for (int j = 0; j < ball.forces.size() - 1; j++){
            DrawLineEx(drawPosition, vectorSum(drawPosition, vectorTime(ball.forces[j], 1/ball.mass)), 7.0, balls[j].color);
        }
    }
    // DrawTextureRec(traceCanvas.texture, {0, 0, 1300,-1000}, {0,0}, WHITE);
    EndDrawing();
}

int main(){

    srand(2008);

    InitWindow(1300,1000,"hello world");
    SetTargetFPS(120);
    traceCanvas = LoadRenderTexture(1300, 1000);

    while(!WindowShouldClose()){
        dt = GetFrameTime();
        playTime = GetTime();

        mouseLeft();
        mouseRight();
        keySpace();
        moveScreen();

        if(isPlaying){
            physics();
        }

        drawScreen();
    }

    CloseWindow();
    return 0;
}

/*
clang++ main.cpp -std=c++17 -o app $(pkg-config --cflags --libs raylib)
./app
*/