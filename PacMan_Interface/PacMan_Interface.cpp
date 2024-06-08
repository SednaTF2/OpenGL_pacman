// PacMan_Interface.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "PacMan_Interface.h"
#include <gl/gl.h>
#include <gl/glu.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <cmath>

#define MAX_LOADSTRING 100
#define IDT_TIMER 1
#define PI 3.14159265358979323846

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

////////////////////// sk
HDC hDeviceContext;								// current device context
HGLRC hRenderingContext;						// current rendering context

bool bSetupPixelFormat(HDC hdc);
void Resize(int width, int height);
void DrawScene(HDC MyDC);

void DrawMaze(void);
void DrawScore(float posX, float posY);
void DrawPacman(void);
bool Pacman_Collision(float nextX, float nextY);
void MovePacman(float dx, float dy);
void DrawItems(void);
void CheckItemCollision();
bool DoCollide(float left, float bottom, float right, float top);

int pacman_dir = 0; // 0: right, 1: up, 2: left, 3: down
float pacman_pos[2] = { 0.1f, 0.1f };
bool bRight = true;
float centerPos[1][2] = { { 0.1f, 0.1f } };
float moveDirection[1][2] = { { 0.0f, 0.0f } };

class Ghost {
public:
    float position[2];
    float ghostColor[3];
    float speed;
    bool moveUp;
    bool moveDown;
    bool moveLeft;
    bool moveRight;

    Ghost(float x, float y, float spd, float r, float g, float b)
        : position{ x, y }, ghostColor{ r, g, b }, speed(spd), moveUp(false), moveDown(false), moveLeft(false), moveRight(false) {
    }

    void UpdatePosition() {
        float nextX = position[0];
        float nextY = position[1];

        if (moveUp) {
            nextY += speed;
        }
        else if (moveDown) {
            nextY -= speed;
        }

        if (moveLeft) {
            nextX -= speed;
        }
        else if (moveRight) {
            nextX += speed;
        }

        // 충돌 검사
        if (!CheckCollision(nextX, nextY)) {
            position[0] = nextX;
            position[1] = nextY;
        }
        else {
            // 벽에 부딪혔을 때 방향 변경
            ReverseDirection();
        }
    }

    bool CheckCollision(float nextX, float nextY) {
        float left = nextX - 0.06f;
        float right = nextX + 0.06f;
        float bottom = nextY - 0.06f;
        float top = nextY + 0.06f;

        return DoCollide(left, bottom, right, top);
    }

    void ReverseDirection() {
        if (moveUp) {
            moveUp = false;
            moveDown = true;
        }
        else if (moveDown) {
            moveDown = false;
            moveUp = true;
        }

        if (moveLeft) {
            moveLeft = false;
            moveRight = true;
        }
        else if (moveRight) {
            moveRight = false;
            moveLeft = true;
        }
    }

    void TurnAtCorner() {
        if (moveUp || moveDown) {
            // 상하 이동
            if (moveUp && CheckCollision(position[0], position[1] + speed)) {
                moveUp = false;
                moveDown = true;
            }
            else if (moveDown && CheckCollision(position[0], position[1] - speed)) {
                moveDown = false;
                moveUp = true;
            }
        }
        else if (moveLeft || moveRight) {
            // 좌우 이동
            if (moveLeft && CheckCollision(position[0] - speed, position[1])) {
                moveLeft = false;
                moveRight = true;
            }
            else if (moveRight && CheckCollision(position[0] + speed, position[1])) {
                moveRight = false;
                moveLeft = true;
            }
        }
    }


    void DrawGhost(void) {
        glColor3f(ghostColor[0], ghostColor[1], ghostColor[2]);
        glPushMatrix();
        glTranslatef(position[0], position[1], 0.0f);
        glScalef(0.6f, 0.6f, 1.0f);

        // 반원 (머리)
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0.0f, 0.033f); // 중심점을 적절히 위치
        for (int i = 0; i <= 20; i++) { // 반원을 20개의 세그먼트로 분할
            float theta = PI * float(i) / 20.0f;
            float x = 0.1f * cosf(theta);
            float y = 0.1f * sinf(theta) + 0.033f;
            glVertex2f(x, y);
        }
        glEnd();

        // 몸통
        glBegin(GL_QUADS);
        glVertex2f(-0.1f, 0.033f);
        glVertex2f(0.1f, 0.033f);
        glVertex2f(0.1f, -0.033f);
        glVertex2f(-0.1f, -0.033f);
        glEnd();

        // 삼각형 다리
        glBegin(GL_TRIANGLES);
        // 왼쪽 다리
        float y = -0.033f; // 몸통의 가로 길이와 맞도록 y값 조정
        glVertex2f(-0.1f, y);
        glVertex2f(-0.1f, y - 0.05f); // 왼쪽 다리의 끝점
        glVertex2f(-0.05f, y); // 가운데 다리의 시작점

        // 가운데 다리
        glVertex2f(-0.05f, y); // 가운데 다리의 시작점
        glVertex2f(0.0f, y - 0.05f); // 가운데 다리의 끝점
        glVertex2f(0.05f, y); // 가운데 다리의 끝점

        // 오른쪽 다리
        glVertex2f(0.05f, y); // 가운데 다리의 시작점
        glVertex2f(0.1f, y - 0.05f); // 오른쪽 다리의 끝점
        glVertex2f(0.1f, y); // 오른쪽 다리의 끝점
        glEnd();

        glPopMatrix();
    }
};

Ghost ghost1 = Ghost(0.5f, 0.7f, 0.002f, 1.0f, 0.0f, 0.0f);
Ghost ghost2 = Ghost(-0.5f, -0.7f, 0.020f, 0.0f, 1.0f, 0.0f);
Ghost ghost3 = Ghost(0.7f, -0.7f, 0.015f, 0.0f, 0.0f, 1.0f);
Ghost ghost4 = Ghost(-0.1f, 0.3f, 0.003f, 1.0f, 1.0f, 0.0f);
Ghost ghost5 = Ghost(-0.5f, -0.3f, 0.007f, 1.0f, 0.0f, 1.0f);

void CheckGhostCollision(Ghost& g1, Ghost& g2) {
    if (sqrt(pow(g1.position[0] - g2.position[0], 2) + pow(g1.position[1] - g2.position[1], 2)) < 0.2f) {
        g1.ReverseDirection();
        g2.ReverseDirection();
    }
}

void UpdateGhosts() {
    ghost1.UpdatePosition();
    ghost2.UpdatePosition();
    ghost3.UpdatePosition();
    ghost4.UpdatePosition();
    ghost5.UpdatePosition();

    ghost1.TurnAtCorner();
    ghost2.TurnAtCorner();
    ghost3.TurnAtCorner();
    ghost4.TurnAtCorner();
    ghost5.TurnAtCorner();

    // 고스트들 간의 충돌 체크 및 방향 반전
    CheckGhostCollision(ghost1, ghost2);
    CheckGhostCollision(ghost1, ghost3);
    CheckGhostCollision(ghost1, ghost4);
    CheckGhostCollision(ghost2, ghost3);
    CheckGhostCollision(ghost2, ghost4);
    CheckGhostCollision(ghost3, ghost4);
    CheckGhostCollision(ghost1, ghost5);
    CheckGhostCollision(ghost2, ghost5);
    CheckGhostCollision(ghost3, ghost5);
    CheckGhostCollision(ghost4, ghost5);
}

int num_score = 0;
float wall_width = 0.2f;

bool maze[10][10] = { { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
                      { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
                      { 1, 0, 0, 0, 1, 0, 0, 1, 0, 1 },
                      { 1, 0, 0, 1, 0, 1, 1, 0, 0, 1 },
                      { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
                      { 1, 1, 1, 0, 0, 1, 1, 1, 0, 1 },
                      { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
                      { 1, 0, 1, 0, 1, 1, 1, 1, 0, 1 },
                      { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
                      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } };
/*
bool maze[10][10] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },             // mazetest matrix
                      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
*/
bool items[10][10] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                       { 0, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
                       { 0, 1, 0, 0, 0, 1, 1, 0, 1, 0 },
                       { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0 },
                       { 0, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
                       { 0, 0, 0, 1, 1, 0, 0, 0, 1, 0 },
                       { 0, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
                       { 0, 1, 0, 1, 0, 0, 0, 0, 1, 0 },
                       { 0, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
                       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
/*
bool items[10][10] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },            // itemtest matrix
                       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                       { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 },
                       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
*/
int CountItems(bool items[10][10]) {
    int num_items = 0;
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            if (items[i][j]) {
                num_items++;
            }
        }
    }
    return num_items;
}

int total_items = CountItems(items);

GLubyte wall_pattern[] = { 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe,
                           0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe,
                           0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe,
                           0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe,
                           0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe,
                           0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe,
                           0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe,
                           0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00,
                           0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff,
                           0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff,
                           0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff,
                           0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff,
                           0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff,
                           0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff,
                           0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff,
                           0xff, 0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00 };

/////////////////////////////

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PACMANINTERFACE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PACMANINTERFACE));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PACMANINTERFACE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName = 0; // MAKEINTRESOURCEW(IDC_PACMANINTERFACE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    RECT rect;

    switch (message)
    {
    case WM_CREATE:
        // Initialize for the OpenGL rendering
        hDeviceContext = GetDC(hWnd);
        if (!bSetupPixelFormat(hDeviceContext)) {
            MessageBox(hWnd, "Error in setting up pixel format for OpenGL", "Error", MB_OK | MB_ICONERROR);
            DestroyWindow(hWnd);
        }
        hRenderingContext = wglCreateContext(hDeviceContext);
        wglMakeCurrent(hDeviceContext, hRenderingContext);

        // Create a Timer
        SetTimer(hWnd, 1, 10, NULL);

        // Use font bitmaps
        SelectObject(hDeviceContext, GetStockObject(SYSTEM_FONT));
        wglUseFontBitmaps(hDeviceContext, 0, 255, 1000);
        glListBase(1000);
        
        break;
    case WM_TIMER:
        switch (wParam) {
        case IDT_TIMER:
            UpdateGhosts();
            InvalidateRect(hWnd, NULL, false); // 화면 갱신

            ghost1.moveRight = true;
            ghost2.moveRight = true;
            ghost3.moveDown = true;
            ghost4.moveDown = true;
            ghost5.moveRight = true;

            // 게임 오버 조건 : 고스트와 팩맨이 충돌하면
            if (sqrt(pow(centerPos[0][0] - ghost1.position[0], 2) + pow(centerPos[0][1] - ghost1.position[1], 2)) < 0.1f ||
                sqrt(pow(centerPos[0][0] - ghost2.position[0], 2) + pow(centerPos[0][1] - ghost2.position[1], 2)) < 0.1f ||
                sqrt(pow(centerPos[0][0] - ghost3.position[0], 2) + pow(centerPos[0][1] - ghost3.position[1], 2)) < 0.1f ||
                sqrt(pow(centerPos[0][0] - ghost4.position[0], 2) + pow(centerPos[0][1] - ghost4.position[1], 2)) < 0.1f ||
            	sqrt(pow(centerPos[0][0] - ghost5.position[0], 2) + pow(centerPos[0][1] - ghost5.position[1], 2)) < 0.1f){
                KillTimer(hWnd, 1);
                MessageBox(hWnd, _T("You Lose!"), _T("Game Over"), MB_OK | MB_ICONSTOP);
                PostQuitMessage(0);
            }
            // 게임 클리어 조건: 남아 있는 아이템이 없으면
            if (total_items == 0) {
                KillTimer(hWnd, 1);
                MessageBox(hWnd, _T("You Win!"), _T("Congratulations!"), MB_OK | MB_ICONINFORMATION);
                PostQuitMessage(0);
            }
            break;
        }
        break;
/*
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
 */
    case WM_PAINT:
        {
            DrawScene(hDeviceContext);
            ValidateRect(hWnd, NULL);
        }
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &rect);
        Resize(rect.right, rect.bottom);

        InvalidateRect(hWnd, NULL, false);

        break;

    case WM_KEYDOWN:
        switch (wParam) {
            case VK_LEFT:
            case 'A':
                pacman_dir = 2;
                MovePacman(-0.2f, 0.0f);
                break;
            case VK_RIGHT:
            case 'D':
                pacman_dir = 0;
                MovePacman(0.2f, 0.0f);
                break;
            case VK_UP:
            case 'W':
                pacman_dir = 1;
                MovePacman(0.0f, 0.2f);
                break;
            case VK_DOWN:
            case 'S':
                pacman_dir = 3;
                MovePacman(0.0f, -0.2f);
                break;
        }
        InvalidateRect(hWnd, NULL, FALSE); // 화면 갱신
        break;

    case WM_DESTROY:
        if (hRenderingContext)
            wglDeleteContext(hRenderingContext);
        if (hDeviceContext)
            ReleaseDC(hWnd, hDeviceContext);

        // Destory the Timer
        KillTimer(hWnd, 1);

        // Use font bitmaps
        glDeleteLists(1000, 255);

        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

bool bSetupPixelFormat(HDC hdc)
{
    PIXELFORMATDESCRIPTOR pfd;
    int pixelformat;

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.dwLayerMask = PFD_MAIN_PLANE;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.cAccumBits = 0;
    pfd.cStencilBits = 0;

    if ((pixelformat = ChoosePixelFormat(hdc, &pfd)) == 0) {
        MessageBox(NULL, "ChoosePixelFormat() failed!!!", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    if (SetPixelFormat(hdc, pixelformat, &pfd) == false) {
        MessageBox(NULL, "SetPixelFormat() failed!!!", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    return true;
}

void Resize(int width, int height)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    int cx = width;
    int cy = height;
    glViewport(0, 0, cx, cy);

    // 3D orthographic viewing
    if (cx <= cy) {
        float ratio = (float)cy / (float)cx;
        glOrtho(-1.0, 1.0, -ratio, ratio, -1.0, 1.0);
    }
    else {
        float ratio = (float)cx / (float)cy;
        glOrtho(-ratio, ratio, -1.0, 1.0, -1.0, 1.0);
    }

    return;

}

void DrawScene(HDC MyDC)
{
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.0f, 0.0f, 0.0f);
    DrawScore(1.1f, 0.8f);
    DrawMaze();
    DrawPacman();
    DrawItems();
    CheckItemCollision();

    ghost1.DrawGhost();
    ghost2.DrawGhost();
    ghost3.DrawGhost();
    ghost4.DrawGhost();
    ghost5.DrawGhost();

    SwapBuffers(MyDC);

    return;
}

void DrawScore(float posX, float posY)                                                  // Drawing Score
{
    // Use font bitmaps
    char str[256];
    sprintf_s(str, "Score: %2d", num_score);

	glRasterPos2f(posX, posY);
	glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);

    glColor3f(0.75f, 0.75f, 0.75f);
    glRectf(posX - 0.02f, posY - 0.02f, posX + 0.23f, posY + 0.05f);

	return;
}

void DrawPacman(void)                                                                   // Drawing Pacman
{
    glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(centerPos[0][0] - 0.1f, centerPos[0][1] - 0.1f, 0.0f); // 위치 조정
    glScalef(1.0f, 1.0f, 1.0f);
    glTranslatef(pacman_pos[0], pacman_pos[1], 0.0f);

    float angleOffset = 0.0f;
    switch (pacman_dir) {
    case 0: angleOffset = 0.0f; break; // RIGHT
    case 1: angleOffset = 90.0f; break; // UP
    case 2: angleOffset = 180.0f; break; // LEFT
    case 3: angleOffset = 270.0f; break; // DOWN
    }

    glRotatef(angleOffset, 0.0f, 0.0f, 1.0f);
    glTranslatef(-pacman_pos[0], -pacman_pos[1], 0.0f);

    float startAngle = 30.0f * PI / 180.0f; // 30 degrees to radians
    float endAngle = 330.0f * PI / 180.0f; // 330 degrees to radians

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(pacman_pos[0], pacman_pos[1]);
    for (int i = 0; i <= 30; i++) {
        float angle = startAngle + i * (endAngle - startAngle) / 30.0f;
        glVertex2f(pacman_pos[0] + 0.1f * cos(angle), pacman_pos[1] + 0.1f * sin(angle));
    }
    glEnd();

    glPopMatrix();
}

bool Pacman_Collision(float nextX, float nextY) {                                       // Pacman collision detection
    float x0 = -1.0f, y0 = 1.0f; // start from left-top
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 10; col++) {
            if (maze[row][col]) {
                // 팩맨의 크기와 충돌을 검사합니다.
                if (nextX + 0.09f > x0 && 
                    nextX - 0.09f < x0 + wall_width &&
                    nextY + 0.09f > y0 - wall_width && 
                    nextY - 0.09f < y0) {
                    return true;
                }
            }
            x0 += wall_width;
        }
        x0 = -1.0f;
        y0 -= wall_width;
    }
    return false;
}

void MovePacman(float dx, float dy)
{
    // 팩맨의 새로운 위치를 계산합니다.
    float nextX = centerPos[0][0] + dx;
    float nextY = centerPos[0][1] + dy;

    // 충돌 여부를 확인합니다.
    if (!Pacman_Collision(nextX, nextY)) {
        // 충돌하지 않으면 팩맨의 위치를 업데이트합니다.
        centerPos[0][0] = nextX;
        centerPos[0][1] = nextY;
        moveDirection[0][0] = dx;
        moveDirection[0][1] = dy;

        // 아이템 충돌을 확인합니다.
        CheckItemCollision();
    }
}

void DrawItems(void) {																  // Drawing Items by using rray
    float x0 = -1.0f, y0 = 1.0f; // start from left-top
    glColor3f(1.0f, 1.0f, 1.0f); // 아이템 색상 (하얀색)
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 10; col++) {
            if (items[row][col]) {
                float itemRadius = 0.02f; // 아이템 크기
                glBegin(GL_TRIANGLE_FAN);
                glVertex2f(x0 + wall_width / 2, y0 - wall_width / 2); // 중심
                for (int i = 0; i <= 20; i++) {
                    float angle = 2 * PI * i / 20;
                    glVertex2f((x0 + wall_width / 2) + itemRadius * cos(angle),
                        (y0 - wall_width / 2) + itemRadius * sin(angle));
                }
                glEnd();
            }
            x0 += wall_width;
        }
        x0 = -1.0f;
        y0 -= wall_width;
    }
}

void CheckItemCollision() {
    int row = (int)((1.0f - centerPos[0][1]) / wall_width); // y 좌표 변환
    int col = (int)((centerPos[0][0] + 1.0f) / wall_width); // x 좌표 변환

    if (row >= 0 && row < 10 && col >= 0 && col < 10) {
        if (items[row][col]) {
            items[row][col] = false; // 아이템 먹기
            num_score++; // 점수 증가
            total_items = CountItems(items); // 남아 있는 아이템의 수 다시 계산
        }
    }
}

void DrawMaze(void) {  // Drawing Maze        
    glEnable(GL_POLYGON_STIPPLE);
    glPolygonStipple(wall_pattern);

    float x0 = -1.0f, y0 = 1.0f; // start from left-top
    glColor3f(0.5f, 0.0f, 0.0f);
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 10; col++) {
            if (maze[row][col]) {
                glRectf(x0, y0, x0 + wall_width, y0 - wall_width);
            }
            x0 += wall_width;
        }
        x0 = -1.0f;
        y0 -= wall_width;
    }

    glDisable(GL_POLYGON_STIPPLE);
}

bool DoCollide(float left, float bottom, float right, float top) {
    float bound = 1.0f;
    if (left < -bound || right > bound || bottom < -bound || top > bound) {
        return true;
    }

    float x0 = -1.0f, y0 = 1.0f; // start from left-top
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 10; col++) {
            if (maze[row][col]) {
                // 벽의 좌표
                float wallLeft = x0;
                float wallRight = x0 + wall_width;
                float wallBottom = y0 - wall_width;
                float wallTop = y0;

                // 충돌 판정
                if (right > wallLeft && left < wallRight && top > wallBottom && bottom < wallTop) {
                    return true;
                }
            }
            x0 += wall_width;
        }
        x0 = -1.0f;
        y0 -= wall_width;
    }
    return false;
}
