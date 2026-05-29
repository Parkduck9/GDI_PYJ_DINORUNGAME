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

void DinoGame::UpdateDinoInfo() {
    if (!m_pDino) return;


    Vector2f dinoPos = m_pDino->GetPosition();
    float targetX = (float)m_MousePos.x;
    float dinoX = (float)dinoPos.x;

    float dir = (targetX - dinoX);


    if (abs(dir) > 5.0f) {
        dir = (dir > 0) ? 1.0f : -1.0f; //방향 추출
        m_pDino->SetDirection(Vector2f(dir, 0.0f));
    }
    if (!m_isOnGround) {
        m_jumpX = dir;
    }
    else {
        m_pDino->SetDirection(Vector2f(0.0f, 0.0f));
    } 
    // 여기까지 마우스x를 이용해서 좌우 이동

    const float GROUND_Y = 500.0f;

    if (!m_isOnGround)
    {

        m_jumpTime += m_fDeltaTime;

        float t = m_jumpTime / m_jumpDuration;

        if (t >= 1.0f)
        {
            t = 1.0f;
            m_isOnGround = true;
            m_jumpCount = 0;
            dinoPos.y = GROUND_Y;
        }
        else
        {
            if (t < 0.5f)
            {
                // 올라가는 구간: ground -> target
                float upT = t / 0.5f;
                dinoPos.y = GROUND_Y + (m_jumpTarget - GROUND_Y) * upT;
            }
            else
            {
                // 내려오는 구간: target -> ground
                float downT = (t - 0.5f) / 0.5f;
                dinoPos.y = m_jumpTarget + (GROUND_Y - m_jumpTarget) * downT;
            }
        }
    }

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
    


    if (m_jumpCount < 2) {
        Vector2f dinoPos = m_pDino->GetPosition();
        m_PlayerTargetPos.x = x;
        m_PlayerTargetPos.y = y;

        m_jumpStartY = dinoPos.y;
        m_jumpTarget = (float)y;
        m_jumpTime = 1000.0f;

        

        m_jumpDuration = 3500.0f;// *(m_jumpTarget - m_PlayerTargetPos.y);

        m_jumpCount++;
        m_isOnGround = false;
   }
}

    // 3. 더블버퍼링 셋업
    // 4. 리소스 로딩 (png)
    // 5. 플레이어 생성
