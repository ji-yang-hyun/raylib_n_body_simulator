#include <iostream>
#include <raylib.h>
#include<vector>
#include <cstdlib>
using namespace std;

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

RenderTexture2D traceCanvas;
float growSpeed = 100;
float dt, playTime = 0;
float clickTime, clickStartTime, clickEndTime;
int isClicking = 0;
float realG = 6.6743 * pow(10,-11);
float G = 50000;
float accelerationVisualScale = 1.0;
float circleSize = 30;
unsigned char alpha = 177;
bool isPlaying = false;

vector<Vector2> positions = {};
vector<Vector2> velocities = {};
vector<float> masses = {};
vector<vector<Vector2>> accelerations = {};
vector<vector<Vector2>> forces = {};
vector<Color> colors = {};

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
            positions.push_back({newMassPos.x, newMassPos.y});
            masses.push_back(0.0);
            velocities.push_back({0,0});
            accelerations.push_back({});
            forces.push_back({});
            colors.push_back({randNumR,randNumG,randNumB,alpha});
        }
        if(isClicking){
            masses[masses.size() - 1] += dt*growSpeed;
            velocities[masses.size() - 1].x = clickPoint.x - positions[masses.size() - 1].x;
            velocities[masses.size() - 1].y = clickPoint.y - positions[masses.size() - 1].y;
        }
    }
    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
        Vector2 clickPoint = GetMousePosition();
        isClicking = 0;
        clickEndTime = GetTime();
        velocities[masses.size() - 1].x = clickPoint.x - positions[masses.size() - 1].x;
        velocities[masses.size() - 1].y = clickPoint.y - positions[masses.size() - 1].y;
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
            positions.push_back({newMassPos.x, newMassPos.y});
            masses.push_back(circleSize);
            velocities.push_back({0,0});
            accelerations.push_back({});
            forces.push_back({});
            colors.push_back({randNumR,randNumG,randNumB,alpha});
        }
        if(isClicking){
            velocities[masses.size() - 1].x = clickPoint.x - positions[masses.size() - 1].x;
            velocities[masses.size() - 1].y = clickPoint.y - positions[masses.size() - 1].y;
        }
    }
    if(IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)){
        Vector2 clickPoint = GetMousePosition();
        isClicking = 0;
        clickEndTime = GetTime();
        velocities[masses.size() - 1].x = clickPoint.x - positions[masses.size() - 1].x;
        velocities[masses.size() - 1].y = clickPoint.y - positions[masses.size() - 1].y;
    }
}

void keySpace(){
    if(IsKeyPressed(KEY_SPACE)){
        isPlaying = !isPlaying;
    }
}

void physics(){
    // 완전탄성충돌에 의한 상호간의 힘, 가속도, 속도 업데이트 or 만유인력에 따른 상호간의 힘, 가속도, 속도 업데이트
    for(int i = 0; i < masses.size() - isClicking; i++){
        Vector2 deltaVeocity = {0,0};
        for(int j = 0; j < masses.size() - isClicking; j++){
            if(i == j){
                accelerations[i].push_back({0,0});
                forces[i].push_back({0,0});
            }
            else{
                float minDistance = masses[i] + masses[j];
                Vector2 forceDirection = vectorSum(positions[j], vectorTime(positions[i], -1.0));
                float distance = vectorSize(forceDirection);
                forceDirection = vectorTime(forceDirection, 1/distance);

                float a,f = 0;
                
                if(distance <= minDistance){
                    if(vectorDot(forceDirection, velocities[i]) > 0){ // 물리적으로는 있을 수 없지만 연산 한계로 여러번 카운팅 우려.
                        Vector2 perpendicularVelocity = vectorTime(forceDirection, vectorDot(forceDirection, velocities[i]));
                        float dv = vectorSize(perpendicularVelocity)*(-2);
                        a = dv/dt;
                        f = masses[i] * a;
                    }
                }
                else{
                    f = G * masses[j] * masses[i]/pow(distance,2);
                    a = f/masses[i];
                }
                forces[i].push_back(vectorTime(forceDirection, f));
                accelerations[i].push_back(vectorTime(forceDirection, a));
                deltaVeocity =  vectorSum(deltaVeocity, vectorTime(forceDirection, a*dt));
            }
        }
        velocities[i] = vectorSum(velocities[i], deltaVeocity);
    }

    // 속도에 따른 위치 업데이트
    for(int i = 0; i < masses.size() - isClicking; i++){
        positions[i] = vectorSum(positions[i], vectorTime(velocities[i], dt));
    }
}

void drawScreen(){
    BeginTextureMode(traceCanvas);
    for (int i = 0; i < masses.size(); i++){
        Color traceColor = colors[i];
        traceColor.a = 255;
        DrawCircle(positions[i].x, positions[i].y, 1, traceColor);
    }
    EndTextureMode();
        
    BeginDrawing();
    ClearBackground(WHITE);
    for (int i = 0; i < masses.size(); i++){
        Color traceColor = colors[i];
        traceColor.a = 255;
        DrawCircleV(positions[i], masses[i], colors[i]);
        DrawLineEx(positions[i], vectorSum(positions[i], velocities[i]), 7.7, traceColor);
        for (int j = 0; j < accelerations[i].size(); j++){
            DrawLineEx(positions[i], vectorSum(positions[i], accelerations[i][j]), 7.0, colors[j]);
        }
    }
    DrawTextureRec(traceCanvas.texture, {0, 0, 1300,-1000}, {0,0}, WHITE);
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
        for(int i = 0; i < accelerations.size(); i++){
            accelerations[i] = {};
        }

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