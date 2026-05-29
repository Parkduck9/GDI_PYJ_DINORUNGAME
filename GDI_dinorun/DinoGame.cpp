#include "INC_Windows.h"
#include "DinoGame.h"
#include "GameTimer.h"
#include "Collider.h"
#include "GameObject.h"
#include "RenderHelp.h"
#include <iostream>
#include <assert.h>

using namespace learning;

bool DinoGame::Initialize()
{
    // 1. 타이머 생성
    m_pGameTimer = new GameTimer();
    m_pGameTimer->Reset();

    const wchar_t* className = L"DinoGame";
    const wchar_t* windowName = L"DinoGame";

    if (false == __super::Create(className, windowName, 1024, 720))
    {
        return false;
    }

    RECT rcClient = {};
    GetClientRect(m_hWnd, &rcClient);
    m_width = rcClient.right - rcClient.left;
    m_height = rcClient.bottom - rcClient.top;

    m_hFrontDC = GetDC(m_hWnd);
    m_hBackDC = CreateCompatibleDC(m_hFrontDC);
    m_hBackBitmap = CreateCompatibleBitmap(m_hFrontDC, m_width, m_height);

    m_hDefaultBitmap = (HBITMAP)SelectObject(m_hBackDC, m_hBackBitmap);


    // 필요한 리소스를 로드해볼까요
#pragma region resource
    m_pPlayerBitmapInfo = renderHelp::CreateBitmapInfo(L"./Resource/GUGARUN.png");
    m_pJumpBitmapInfo = renderHelp::CreateBitmapInfo(L"./Resource/JumpGuGuGaGa.png");
#pragma region endregion
  
    CreatePlayer();

    return true;
}

    // 2. 창 생성 (Create 호출)
void DinoGame::Run()
{
    MSG msg = { 0 };
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_LBUTTONDOWN)
            {
                DinoGame::OnLButtonDown(LOWORD(msg.lParam), HIWORD(msg.lParam));
            }
            //우클릭은 삭제함
            else if (msg.message == WM_MOUSEMOVE)
            {
                DinoGame::OnMouseMove(LOWORD(msg.lParam), HIWORD(msg.lParam));
            }
            else if (msg.message == WM_LBUTTONUP) {
                DinoGame::OnLButtonUp(LOWORD(msg.lParam), HIWORD(msg.lParam));
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            Update();
            Render();
        }
    }
}

void DinoGame::Finalize()
{
    delete m_pGameTimer;
    m_pGameTimer = nullptr;

    __super::Destroy();
}


void DinoGame::FixedUpdate()
{

}

void DinoGame::LogicUpdate()
{
    
    UpdateDinoInfo();
    UpdateWallInfo();
    
    if (m_pDino) m_pDino->Update(m_fDeltaTime);
}

void DinoGame::CreatePlayer()
{
    GameObject* pNewObject = new GameObject(ObjectType::PLAYER);

    pNewObject->SetName("Player");
    pNewObject->SetPosition(300.0f, 500.0f);
    pNewObject->SetSpeed(0.15f); // 일단, 임의로 설정  

    pNewObject->SetWidth(200); // 일단, 임의로 설정
    pNewObject->SetHeight(113); // 일단, 임의로 설정

    pNewObject->SetColliderBox(40.0f, 90.0f); // 일단, 임의로 설정. 오브젝트 설정할 거 다 하고 나서 하자.
    pNewObject->SetBitmapInfo(m_pPlayerBitmapInfo); // 여러분은 따로 빼지 않아서 그냥, 하면 됩니다.

    m_pDino = pNewObject;
}

void DinoGame::CreateEnemy() {
    GameObject* pNewObject = new GameObject(ObjectType::ENEMY);
    pNewObject->SetName("Enemy");

    //float x = m_EnemySpawnPos.x;
    //float y = m_EnemySpawnPos.y;

   // m_EnemySpawnPos = { 0, 0 };

    pNewObject->SetSpeed(1.0f); // 일단, 임의로 설정   


    pNewObject->SetWidth(100); // 일단, 임의로 설정
    pNewObject->SetHeight(100); // 일단, 임의로 설정

    pNewObject->SetColliderCircle(50.0f); // 일단, 임의로 설정. 오브젝트 설정할 거 다 하고 나서 하자.
    //pNewObject->SetBitmapInfo(m_pEnemyWallInfo);
    int i = 0;

}

void DinoGame::CreateWall()
{
    GameObject* pNewObject = new GameObject(ObjectType::Wall);
    pNewObject->SetName("Wall");

    //float x = m_EnemySpawnPos.x;
    //float y = m_EnemySpawnPos.y;

   // m_EnemySpawnPos = { 0, 0 };

    pNewObject->SetSpeed(1.0f); // 일단, 임의로 설정   


    pNewObject->SetWidth(100); // 일단, 임의로 설정
    pNewObject->SetHeight(100); // 일단, 임의로 설정

    pNewObject->SetColliderCircle(50.0f); // 일단, 임의로 설정. 오브젝트 설정할 거 다 하고 나서 하자.
    //pNewObject->SetBitmapInfo(m_pEnemyWallInfo);
    int i = 0;
    

}

void DinoGame::UpdateDinoInfo()
{
    if (!m_pDino) return;

    // 누르고 있는 동안 시간 누적
    if (m_isPressingJump)
        m_jumpPressTime += m_fDeltaTime;

    Vector2f dinoPos = m_pDino->GetPosition();

    // ★ 점프 중이면 X방향 유지, 아니면 마우스 X 따라가기
    if (!m_isOnGround)
    {
        m_pDino->SetDirection(Vector2f(m_jumpX, 0.0f));

    }
    else
    {
        float targetX = (float)m_MousePos.x;
        float dir = targetX - dinoPos.x;

        // UpdateDinoInfo에서
        if (abs(dir) > 5.0f)
        {
            float dirNorm = (dir > 0) ? 1.0f : -1.0f;
            m_pDino->SetDirection(Vector2f(dirNorm, 0.0f));

            if (dirNorm != m_lastDirNorm) // 방향 바뀔 때만
            {
                float speed = (dirNorm < 0) ? 50.0f : 150.0f; // 왼쪽 빠름, 오른쪽 느림
                m_pDino->ChangeBitmapInfo(m_pPlayerBitmapInfo, 20, speed);
                m_lastDirNorm = dirNorm;
            }
        }
    }

    // 점프 Y 계산
    const float GROUND_Y = 500.0f;
    if (!m_isOnGround)
    {
        m_jumpTime += m_fDeltaTime;
        float t = m_jumpTime / m_jumpDuration;

        if (t >= 1.0f)
        {
            m_isOnGround = true;
            m_jumpCount = 0;
            dinoPos.y = GROUND_Y;
            m_pDino->ChangeBitmapInfo(m_pPlayerBitmapInfo,20, 50.0f); // ← 달리기로 복원

        }
        else if (t < 0.5f)
        {
            float upT = t / 0.5f;
            dinoPos.y = GROUND_Y + (m_jumpTarget - GROUND_Y) * upT;
        }
        else
        {
            float downT = (t - 0.5f) / 0.5f;
            dinoPos.y = m_jumpTarget + (GROUND_Y - m_jumpTarget) * downT;
        }
    }

    // ★ 화면 밖 제한
    float halfW = 100.0f; // 캐릭터 절반 너비
    dinoPos.x = dinoPos.x < halfW ? halfW : dinoPos.x;

    m_pDino->SetPosition(dinoPos.x, dinoPos.y);
}
void DinoGame::UpdateWallInfo() {
    
}

void DinoGame::ResolveWallOverLap() { // 적이 안겹치게 하는 함수
    

}

void DinoGame::Update()
{
    m_pGameTimer->Tick();
    LogicUpdate();

    m_fDeltaTime = m_pGameTimer->DeltaTimeMS();
    m_fFrameCount += m_fDeltaTime;

    while (m_fFrameCount >= 200.0f)
    {
        FixedUpdate();
        m_fFrameCount -= 200.0f;
    }
}

void DinoGame::Render()
{
    //Clear the back buffer
    ::PatBlt(m_hBackDC, 0, 0, m_width, m_height, WHITENESS);
    if (m_pDino) m_pDino->Render(m_hBackDC);
    
    //메모리 DC에 그려진 결과를 실제 DC(m_hFrontDC)로 복사
    BitBlt(m_hFrontDC, 0, 0, m_width, m_height, m_hBackDC, 0, 0, SRCCOPY);
}

void DinoGame::OnResize(int width, int height)
{
    std::cout << __FUNCTION__ << std::endl;

    learning::SetScreenSize(width, height);

    __super::OnResize(width, height);

    m_hBackBitmap = CreateCompatibleBitmap(m_hFrontDC, m_width, m_height);

    HANDLE hPrevBitmap = (HBITMAP)SelectObject(m_hBackDC, m_hBackBitmap);

    DeleteObject(hPrevBitmap);
}

void DinoGame::OnClose()
{
    std::cout << __FUNCTION__ << std::endl;

    SelectObject(m_hBackDC, m_hDefaultBitmap);

    DeleteObject(m_hBackBitmap);
    DeleteDC(m_hBackDC);

    ReleaseDC(m_hWnd, m_hFrontDC);
}

void DinoGame::OnMouseMove(int x, int y)
{
    /*   std::cout << __FUNCTION__ << std::endl;
       std::cout << "x: " << x << ", y: " << y << std::endl;*/
    m_MousePosPrev = m_MousePos;
    m_MousePos = { x, y };
}

void DinoGame::OnLButtonDown(int x, int y)
{
    std::cout << __FUNCTION__ << std::endl;
    std::cout << "x: " << x << ", y: " << y << std::endl;


    if (m_jumpCount < 2 && !m_isPressingJump)
    {
        m_isPressingJump = true;
        m_jumpPressTime = 0.0f;
        // 점프 시작할 때 현재 X 방향 저장
        m_jumpX = m_pDino->GetDirection().x;
    }
}

void DinoGame::OnLButtonUp(int x, int y)
{

    if (!m_isPressingJump) return;

    m_pDino->ChangeBitmapInfo(m_pJumpBitmapInfo, 18, 20.0f); 

    m_isPressingJump = false;

    // 누른 시간에 따라 점프 높이/시간 결정
    // 최소 100ms, 최대 500ms 기준
    float pressTime = m_jumpPressTime > 500.0f ? 500.0f : m_jumpPressTime;    
    float ratio = pressTime / 500.0f; // 0.0 ~ 1.0

    float jumpHeight = 150.0f + ratio * 200.0f; // 최소150 ~ 최대350 픽셀
    m_jumpTarget = 500.0f - jumpHeight;      // y가 작을수록 위
    m_jumpDuration = 500.0f + ratio * 500.0f;  // 최소500 ~ 최대1000ms
    m_jumpTime = 0.0f;
    m_jumpCount++;
    m_isOnGround = false;
}


    // 3. 더블버퍼링 셋업
    // 4. 리소스 로딩 (png)
    // 5. 플레이어 생성
