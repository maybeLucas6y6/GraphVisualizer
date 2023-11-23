#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <string>
#include <vector>
#include <iostream>

struct AppState {
    float windowWidth;
    float windowHeight;

    Vector2 mPos;

    Vector2 addingEdge;
    Vector3 editingEdge;
    Vector2 deletingEdge;
    int hovered;
    int selected;
    bool locked;
    std::vector<std::vector<bool>> undirected;
    std::vector<std::vector<bool>> directed;
    std::vector<std::vector<bool>> editingCopy;
    std::vector<Vector2> verticesPos;
    std::vector<Color> culori;
    Color undirectedColor;
    float radius;
    Rectangle settinsPanel;
    Rectangle graphTypePanel;
    Rectangle colorPickerPanel;
    Rectangle lockPanel;
    std::vector<Rectangle> UISpace;
    int actionCombo;
    int objectCombo;
    int typeCombo;
};

float Dist(Vector2 a, Vector2 b) {
    return sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2));
}
bool IsInsideBounds(std::vector<Rectangle> bounds, Vector2 pos) {
    for (int i = 0; i < bounds.size(); i++) {
        if ((pos.x >= bounds[i].x && pos.x <= bounds[i].x + bounds[i].width) && (pos.y >= bounds[i].y && pos.y <= bounds[i].y + bounds[i].height)) {
            return true;
        }
    }
    return false;
}
bool IsInsideScreen(Vector2 pos, AppState* state) {
    return (pos.x >= 0 && pos.x <= state->windowWidth) && (pos.y >= 0 && pos.y <= state->windowHeight);
}
int GetClickedVertex(Vector2 pos, AppState* state) {
    for (int i = 0; i < state->undirected.size(); i++) {
        if (Dist(state->verticesPos[i], pos) <= 20) {
            return i;
        }
    }
    return -1;
}
int GetHoveredVertex(AppState* state) {
    for (int i = 0; i < state->undirected.size(); i++) {
        if (Dist(state->verticesPos[i], state->mPos) <= 20) {
            return i;
        }
    }
    return -1;
}
float GetNearestVertexDist(Vector2 pos, AppState* state, int except = -1) {
    if (state->verticesPos.empty()) return 2.0f * state->radius + 1;
    float minDist = -1;
    for (int i = 0; i < state->undirected.size(); i++) {
        if (i == except) continue;
        if (minDist == -1) {
            minDist = Dist(state->verticesPos[i], pos);
        }
        else if (Dist(state->verticesPos[i], pos) <= minDist) {
            minDist = Dist(state->verticesPos[i], pos);
        }
    }
    return minDist;
}
Vector2 GetControlPoint(Vector2 start, Vector2 end) {
    Vector2 control = { 0,0 };
    float d = Dist(start, end);
    float h = 0.3f * d;
    float Ax = start.x;
    float Ay = start.y;
    float Bx = end.x;
    float By = end.y;
    float T = (Ax * Ax - Bx * Bx + Ay * Ay - By * By) / 2;
    float G = (h * d - Bx * Ay + By * Ax);
    control.x = (T * (Ax - Bx) - G * (Ay - By)) / ((Ay - By) * (Ay - By) + (Ax - Bx) * (Ax - Bx));
    control.y = (T - control.x * (Ax - Bx)) / (Ay - By);
    return control;
}

void DrawVertices(AppState* state) {
    if (state->actionCombo == 0 && state->objectCombo == 0) {
        if (GetNearestVertexDist(state->mPos, state) > 2.0f * state->radius && !IsInsideBounds(state->UISpace, state->mPos)) {
            DrawCircleV(state->mPos, state->radius, GREEN);
            DrawText("+", state->mPos.x - MeasureText("+", 20) / 2.0f, state->mPos.y - 10, 20, WHITE);
        }
    }
    for (int i = 0; i < state->undirected.size(); i++) {
        std::string tmp = std::to_string(i + 1);
        float size;
        if (state->hovered == i || state->deletingEdge.x == i || state->addingEdge.x == i || state->editingEdge.x == i || state->editingEdge.y == i) {
            size = 1.2f * state->radius;
        }
        else {
            size = state->radius;
        }
        Color col;
        if (state->addingEdge.x == i) {
            col = GREEN;
        }
        else if (state->deletingEdge.x == i) {
            col = RED;
        }
        else if (state->editingEdge.x == i || state->editingEdge.y == i) {
            col = BLUE;
        }
        else if (state->hovered == i && state->addingEdge.x != -1) {
            col = GREEN;
        }
        else if (state->hovered == i && state->actionCombo == 2) {
            col = RED;
        }
        else if (state->hovered == i && state->actionCombo == 1) {
            col = BLUE;
        }
        else {
            col = state->typeCombo == 0 ? state->undirectedColor : state->culori[i];
        }
        DrawCircleV(state->verticesPos[i], size, col);
        DrawText(tmp.c_str(), state->verticesPos[i].x - MeasureText(tmp.c_str(), 10) / 2.0f, state->verticesPos[i].y - 5, 10, WHITE);
    }
}
void DrawArcs(AppState* state) {
    for (int i = 0; i < state->directed.size(); i++) {
        for (int j = 0; j < state->directed.size(); j++) {
            if (state->directed[i][j]) {
                if (state->hovered == j && state->deletingEdge.x == i) {
                    DrawLineBezierQuad(state->verticesPos[i], state->verticesPos[j], GetControlPoint(state->verticesPos[i], state->verticesPos[j]), 3.0f, GRAY);
                }
                else if ((state->hovered == i || state->hovered == j) && (state->actionCombo == 2 && state->objectCombo == 0)) {
                    DrawLineBezierQuad(state->verticesPos[i], state->verticesPos[j], GetControlPoint(state->verticesPos[i], state->verticesPos[j]), 3.0f, GRAY);
                }
                else if (state->hovered == j && state->editingEdge.x == i) {
                    DrawLineBezierQuad(state->verticesPos[i], state->verticesPos[j], GetControlPoint(state->verticesPos[i], state->verticesPos[j]), 3.0f, GRAY);
                }
                else {
                    DrawLineBezierQuad(state->verticesPos[i], state->verticesPos[j], GetControlPoint(state->verticesPos[i], state->verticesPos[j]), 5.0f, state->culori[i]);
                }
            }
            else if (state->editingEdge.x == i && state->editingEdge.y == j) {
                DrawLineBezierQuad(state->verticesPos[i], state->verticesPos[j], GetControlPoint(state->verticesPos[i], state->verticesPos[j]), 3.0f, GRAY);
            }
        }
    }
    if (state->addingEdge.x != -1) {
        if (state->hovered != -1) {
            DrawLineBezierQuad(state->verticesPos[state->addingEdge.x], state->verticesPos[state->hovered], GetControlPoint(state->verticesPos[state->addingEdge.x], state->verticesPos[state->hovered]), 5.0f, GREEN);
        }
        else {
            DrawLineBezierQuad(state->verticesPos[state->addingEdge.x], state->mPos, GetControlPoint(state->verticesPos[state->addingEdge.x], state->mPos), 3.0f, GREEN);
        }
    }
    else if (state->editingEdge.y != -1) {
        if (state->hovered != -1) {
            DrawLineBezierQuad(state->verticesPos[state->editingEdge.x], state->verticesPos[state->hovered], GetControlPoint(state->verticesPos[state->editingEdge.x], state->verticesPos[state->hovered]), 5.0f, BLUE);
        }
        else {
            DrawLineBezierQuad(state->verticesPos[state->editingEdge.x], state->mPos, GetControlPoint(state->verticesPos[state->editingEdge.x], state->mPos), 3.0f, BLUE);
        }
    }
}
void DrawLines(AppState* state) {
    for (int i = 0; i < state->undirected.size(); i++) {
        for (int j = i + 1; j < state->undirected.size(); j++) {
            if (state->undirected[i][j]) {
                if ((state->hovered == i && state->deletingEdge.x == j) || (state->deletingEdge.x == i && state->hovered == j)) {
                    DrawLineEx(state->verticesPos[i], state->verticesPos[j], 3.0f, GRAY);
                }
                else if ((state->hovered == i || state->hovered == j) && (state->actionCombo == 2 && state->objectCombo == 0)) {
                    DrawLineEx(state->verticesPos[i], state->verticesPos[j], 3.0f, GRAY);
                }
                else if ((state->hovered == i && state->editingEdge.x == j) || (state->hovered == j && state->editingEdge.x == i)) {
                    DrawLineEx(state->verticesPos[i], state->verticesPos[j], 3.0f, GRAY);
                }
                else {
                    DrawLineEx(state->verticesPos[i], state->verticesPos[j], 5.0f, state->undirectedColor);
                }
            }
            else if ((state->editingEdge.x == i && state->editingEdge.y == j) || (state->editingEdge.x == j && state->editingEdge.y == i)) {
                DrawLineEx(state->verticesPos[i], state->verticesPos[j], 3.0f, GRAY);
            }
        }
    }
    if (state->addingEdge.x != -1) {
        if (state->hovered != -1) {
            DrawLineEx(state->verticesPos[state->addingEdge.x], state->verticesPos[state->hovered], 5.0f, GREEN);
        }
        else {
            DrawLineEx(state->verticesPos[state->addingEdge.x], state->mPos, 3.0f, GREEN);
        }
    }
    if (state->editingEdge.y != -1) {
        if (state->hovered != -1) {
            DrawLineEx(state->verticesPos[state->editingEdge.x], state->verticesPos[state->hovered], 5.0f, BLUE);
        }
        else {
            DrawLineEx(state->verticesPos[state->editingEdge.x], state->mPos, 3.0f, BLUE);
        }
    }
}

void AddVertex(AppState* state) {
    if ((GetClickedVertex(state->mPos, state) != -1) || (GetNearestVertexDist(state->mPos, state) <= 2.0f * state->radius)) return;
    state->verticesPos.push_back(state->mPos);
    state->undirected.push_back(std::vector<bool>(state->undirected.size(), false));
    state->directed.push_back(std::vector<bool>(state->directed.size(), false));
    for (int i = 0; i < state->undirected.size(); i++) {
        state->undirected[i].push_back(false);
        state->directed[i].push_back(false);
        Color newColor = { GetRandomValue(0,255), GetRandomValue(0,255) ,GetRandomValue(0,255) , 255 };
        state->culori.push_back(newColor);
    }
}
void AddEdge(AppState* state) {
    if (state->addingEdge.x == -1) {
        state->addingEdge.x = GetClickedVertex(state->mPos, state);
        return;
    }
    else {
        state->addingEdge.y = GetClickedVertex(state->mPos, state);
        if (state->addingEdge.y == -1 || state->addingEdge.y == state->addingEdge.x) {
            state->addingEdge = { -1,-1 };
            return;
        }
        if (state->typeCombo == 0) {
            if (state->undirected[state->addingEdge.x][state->addingEdge.y] || state->undirected[state->addingEdge.y][state->addingEdge.x]) {
                state->addingEdge = { -1,-1 };
                return;
            }
            state->undirected[state->addingEdge.x][state->addingEdge.y] = true;
            state->undirected[state->addingEdge.y][state->addingEdge.x] = true;
        }
        else {
            if (state->directed[state->addingEdge.x][state->addingEdge.y]) {
                state->addingEdge = { -1,-1 };
                return;
            }
            state->directed[state->addingEdge.x][state->addingEdge.y] = true;
        }
        state->addingEdge = { -1,-1 };
    }
}

void EditVertex(AppState* state) {
    if (!state->locked && state->selected != -1) {
        if (GetNearestVertexDist(state->mPos, state, state->selected) > 2.0f * state->radius) {
            state->verticesPos[state->selected] = state->mPos;
        }
        else {
            state->selected = -1;
        }
    }
}
void EditEdge(AppState* state) {
    if (state->editingEdge.x == -1) {
        state->editingEdge.x = GetClickedVertex(state->mPos, state);
        return;
    }
    else if (state->editingEdge.y == -1) {
        state->editingEdge.y = GetClickedVertex(state->mPos, state);
        if (state->editingEdge.y == -1 || state->editingEdge.y == state->editingEdge.x) {
            state->editingEdge = { -1,-1,-1 };
            return;
        }
        if (state->typeCombo == 0) {
            if (state->undirected[state->editingEdge.x][state->editingEdge.y]) {
                state->editingCopy = state->undirected;
                state->undirected[state->editingEdge.x][state->editingEdge.y] = false;
                state->undirected[state->editingEdge.y][state->editingEdge.x] = false;
            }
            else {
                state->editingEdge.y = -1;
                return;
            }
        }
        else {
            if (state->directed[state->editingEdge.x][state->editingEdge.y]) {
                state->editingCopy = state->directed;
                state->directed[state->editingEdge.x][state->editingEdge.y] = false;
            }
            else {
                state->editingEdge.y = -1;
                return;
            }
        }
    }
    else {
        state->editingEdge.z = GetClickedVertex(state->mPos, state);
        if (state->editingEdge.z == -1 || state->editingEdge.z == state->editingEdge.x || state->editingEdge.z == state->editingEdge.y) {
            state->editingEdge = { -1,-1,-1 };
            if (state->typeCombo == 0) {
                state->undirected = state->editingCopy;
            }
            else {
                state->directed = state->editingCopy;
            }
            return;
        }
        if (state->typeCombo == 0) {
            if (state->undirected[state->editingEdge.x][state->editingEdge.z]) {
                state->editingEdge.z = -1;
                return;
            }
            else {
                state->undirected[state->editingEdge.x][state->editingEdge.z] = true;
                state->undirected[state->editingEdge.z][state->editingEdge.x] = true;
                state->editingEdge = { -1,-1,-1 };
                state->editingCopy.clear();
            }
        }
        else {
            if (state->directed[state->editingEdge.x][state->editingEdge.z]) {
                state->editingEdge.z = -1;
                return;
            }
            else {
                state->directed[state->editingEdge.x][state->editingEdge.z] = true;
                state->editingEdge = { -1,-1,-1 };
                state->editingCopy.clear();
            }
        }
    }
}

void DeleteVertex(AppState* state) {
    state->deletingEdge = { -1,-1 };
    int vtx = GetClickedVertex(state->mPos, state);
    if (vtx == -1 || state->undirected.empty() || state->directed.empty() || state->culori.empty() || state->verticesPos.empty()) return;
    for (int i = vtx; i < state->undirected.size() - 1; i++) {
        std::swap(state->undirected[i], state->undirected[i + 1]);
        std::swap(state->directed[i], state->directed[i + 1]);
    }
    state->undirected.pop_back();
    state->directed.pop_back();
    for (int i = 0; i < state->undirected.size(); i++) {
        for (int j = vtx; j < state->undirected[i].size() - 1; j++) {
            bool aux = state->undirected[i][j];
            state->undirected[i][j] = state->undirected[i][j + 1];
            state->undirected[i][j + 1] = aux;

            aux = state->directed[i][j];
            state->directed[i][j] = state->directed[i][j + 1];
            state->directed[i][j + 1] = aux;
        }
        state->undirected[i].pop_back();
        state->directed[i].pop_back();
    }

    for (int i = vtx; i < state->verticesPos.size() - 1; i++) {
        std::swap(state->verticesPos[i], state->verticesPos[i + 1]);
        std::swap(state->culori[i], state->culori[i + 1]);
    }
    state->verticesPos.pop_back();
    state->culori.pop_back();
}
void DeleteEdge(AppState* state) {
    if (state->deletingEdge.x == -1) {
        state->deletingEdge.x = GetClickedVertex(state->mPos, state);
        return;
    }
    else {
        state->deletingEdge.y = GetClickedVertex(state->mPos, state);
        if (state->deletingEdge.y == -1 || state->deletingEdge.y == state->deletingEdge.x) {
            state->deletingEdge = { -1,-1 };
            return;
        }
        if (state->typeCombo == 0) {
            state->undirected[state->deletingEdge.x][state->deletingEdge.y] = false;
            state->undirected[state->deletingEdge.y][state->deletingEdge.x] = false;
        }
        else {
            state->directed[state->deletingEdge.x][state->deletingEdge.y] = false;
        }
        state->deletingEdge = { -1,-1 };
    }
}

void DrawPanels(AppState* state) {
    for (int i = 0; i < state->UISpace.size(); i++) {
        GuiPanel(state->UISpace[i], "a");
    }
}
void DebugMatrix(AppState* state) {
    for (int i = 0; i < state->undirected.size(); i++) {
        for (int j = 0; j < state->undirected.size(); j++) {
            std::cout << state->undirected[i][j] << ' ';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
    for (int i = 0; i < state->directed.size(); i++) {
        for (int j = 0; j < state->directed.size(); j++) {
            std::cout << state->directed[i][j] << ' ';
        }
        std::cout << '\n';
    }
}

int main() {
    AppState* state = new AppState;
    state->windowWidth = 1280;
    state->windowHeight = 720;
    state->addingEdge = { -1,-1 };
    state->editingEdge = { -1,-1,-1 };
    state->deletingEdge = { -1,-1 };
    state->hovered = -1;
    state->selected = -1;
    state->locked = false;
    state->undirectedColor = BLACK;
    state->radius = 20.0f;
    state->settinsPanel = { 0,0,160,195 };
    state->graphTypePanel = { state->windowWidth - 160, 0, 160, 90 };
    state->colorPickerPanel = { 0,state->windowHeight - 160,190,160 };
    state->lockPanel = { state->windowWidth - 160, state->windowHeight - 90, 160, 90 };
    state->UISpace.push_back(state->settinsPanel);
    state->UISpace.push_back(state->graphTypePanel);
    state->UISpace.push_back(state->colorPickerPanel);
    state->UISpace.push_back(state->lockPanel);
    state->actionCombo = 0;
    state->objectCombo = 0;
    state->typeCombo = 0;

    InitWindow(state->windowWidth, state->windowHeight, "Graph Visualizer");
    SetWindowState(FLAG_VSYNC_HINT);
    SetTargetFPS(60);

    int oldActionCombo = 0;
    int oldObjectCombo = 0;
    int oldTypeCombo = 0;
    Color uselessColor = RED;

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(RAYWHITE);

        oldActionCombo = state->actionCombo;
        oldObjectCombo = state->objectCombo;
        oldTypeCombo = state->typeCombo;
        state->mPos = GetMousePosition();

        // options UI
        GuiComboBox(Rectangle{ 30,30,100,30 }, "Add;Edit;Delete", &state->actionCombo);
        GuiComboBox(Rectangle{ 30,65,100,30 }, "Vertices;Edges", &state->objectCombo);
        GuiComboBox(Rectangle{ (float)state->windowWidth - 130, 30, 100, 30 }, "Directed;Undirected", &state->typeCombo);
        state->hovered = GetHoveredVertex(state);
        if (oldActionCombo != state->actionCombo || oldObjectCombo != state->objectCombo || state->typeCombo != oldTypeCombo) {
            state->addingEdge = { -1,-1 };
            state->editingEdge = { -1,-1,-1 };
            state->deletingEdge = { -1,-1 };
            state->selected = -1;
        }
        if (GuiButton(Rectangle{ 30, 100, 100, 30 }, "Clear")) {
            state->undirected.clear();
            state->directed.clear();
            state->verticesPos.clear();
            state->culori.clear();
        }
        else if (GuiButton(Rectangle{ 30, 135, 100, 30 }, "Reset edges only")) {
            if (state->typeCombo == 0) {
                std::vector<std::vector<bool>> empty(state->undirected.size(), std::vector<bool>(state->undirected.size(), false));
                state->undirected = empty;
            }
            else {
                std::vector<std::vector<bool>> empty(state->directed.size(), std::vector<bool>(state->directed.size(), false));
                state->directed = empty;
            }
        }
        if (state->typeCombo == 0) {
            GuiColorPicker(Rectangle{ 30, (float)state->windowHeight - 130, 100, 100 }, "color", &state->undirectedColor);
        }
        else {
            if (state->selected != -1) {
                GuiColorPicker(Rectangle{ 30, (float)state->windowHeight - 130, 100, 100 }, "color", &state->culori[state->selected]);
            }
            else {
                GuiColorPicker(Rectangle{ 30, (float)state->windowHeight - 130, 100, 100 }, "color", &uselessColor);
            }
        }
        state->locked = GuiToggle(Rectangle{ (float)state->windowWidth - 130, (float)state->windowHeight - 60, 100, 30 }, "Lock position", &state->locked);

        // changes
        if (IsInsideBounds(state->UISpace, state->mPos) || !IsInsideScreen(state->mPos, state)) {
            state->addingEdge = { -1,-1 };
            state->editingEdge = { -1,-1,-1 };
            state->deletingEdge = { -1,-1 };
        }
        else {
            if (IsMouseButtonPressed(0)) {
                state->selected = GetClickedVertex(state->mPos, state);
            }
            switch (state->actionCombo) {
            case 0:
                if (!IsMouseButtonPressed(0)) break;
                if (state->objectCombo == 0) {
                    AddVertex(state);
                }
                else {
                    AddEdge(state);
                }
                break;
            case 1:
                if (state->objectCombo == 0) {
                    if (IsMouseButtonDown(0)) {
                        EditVertex(state);
                    }
                }
                else {
                    if (IsMouseButtonPressed(0)) {
                        EditEdge(state);
                    }
                }
                break;
            case 2:
                if (!IsMouseButtonPressed(0)) break;
                if (state->objectCombo == 0) {
                    DeleteVertex(state);
                }
                else {
                    DeleteEdge(state);
                }
                break;
            }
        }

        // rendering
        if (state->typeCombo == 0) {
            DrawLines(state);
        }
        else {
            DrawArcs(state);
        }
        DrawVertices(state);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}