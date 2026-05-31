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

    //윈도우 생성 
    if (false == __super::Create(className, windowName, 1024, 720))
    {
        return false;
    }

    //클라이언트 영역 크기
    RECT rcClient = {};
    GetClientRect(m_hWnd, &rcClient);
    m_width = rcClient.right - rcClient.left;
    m_height = rcClient.bottom - rcClient.top;

    //더블 버퍼링용 DC 생성
    m_hFrontDC = GetDC(m_hWnd);
    m_hBackDC = CreateCompatibleDC(m_hFrontDC);
    m_hBackBitmap = CreateCompatibleBitmap(m_hFrontDC, m_width, m_height);

    m_hDefaultBitmap = (HBITMAP)SelectObject(m_hBackDC, m_hBackBitmap);


    // 필요한 리소스를 로드해볼까요
#pragma region resource
    m_pReadyBitmapInfo = renderHelp::CreateBitmapInfo(L"./Resource/GameMainMenu.png");
    m_pEndBitmapInfo = renderHelp::CreateBitmapInfo(L"./Resource/EndgameScene.png");
    // 맵(시작화면 종료화면)
    

    // 캐릭터 및 쿠키와 벽 리소스
    m_pPlayerBitmapInfo = renderHelp::CreateBitmapInfo(L"./Resource/GUGARUN.png");
    m_pJumpBitmapInfo = renderHelp::CreateBitmapInfo(L"./Resource/JumpGuGuGaGa.png");
    m_pMapBitmapInfo = renderHelp::CreateBitmapInfo(L"./Resource/antarcticamap.png");
    m_pCookieBitmapInfo = renderHelp::CreateBitmapInfo(L"./Resource/GuGaCookie.png");
    m_pWallBitmapInfo = renderHelp::CreateBitmapInfo(L"./Resource/BearWall.png");
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
{ // 메모리해제!
    for (int i = 0; i < 10; ++i)
    {
        if (m_pObjects[i])
        {
            delete m_pObjects[i];
            m_pObjects[i] = nullptr;
        }
    }

    if (m_pDino)
    {
        delete m_pDino;
        m_pDino = nullptr;
    }

    delete m_pGameTimer;
    m_pGameTimer = nullptr;

    __super::Destroy();
}


void DinoGame::FixedUpdate() //지우면 오류가 나서 일단 살려뒀습니다
{

}

void DinoGame::LogicUpdate()
{
    switch (m_gameState)
    {
    case GameState::Ready:
        ReadyGame();  // 시작화면
        return;

    case GameState::Start:
        StartGame(); // 게임화면
        break;

    case GameState::GameOver:
        return; // 게임 오버 상태에서 업데이트 금지!
    }
    // 게임 진행 중일 때 실행!
    UpdateDinoInfo();
    UpdateMapInfo();
    UpdateWallInfo();
    
    //점수: 시간 경과
    /*if (m_gameTime > 3000.0f)
    {
        m_score += (int)(m_fDeltaTime * 0.001f);
    }*/ // 왜인지 점수가 안올라서 시간제에서 쿠키 먹으면 점수가 오르는것으로 변경

    //충돌 체크
    if (m_pDino)
    {
        learning::ColliderBox* dinoBox = m_pDino->GetColliderBox();

        for (int i = 0; i < 10; ++i)
        {
            if (m_pObjects[i] == nullptr) continue;

            bool hit = false;

            if (m_pObjects[i]->Type() == ObjectType::ITEM)
            {
                // 구구가가와 쿠키 < 거리비교
                learning::ColliderCircle* coinCircle = m_pObjects[i]->GetColliderCircle();
                if (dinoBox && coinCircle)
                {
                    float dx = dinoBox->center.x - coinCircle->center.x;
                    float dy = dinoBox->center.y - coinCircle->center.y;
                    float dist = sqrt(dx * dx + dy * dy);
                    hit = dist < (coinCircle->radius + 43.0f); // 반지름 + 박스 근사값
                }
            }
            else if (m_pObjects[i]->Type() == ObjectType::Wall)
            {
                // 벽 → Box vs Box 충돌 해서 계산!
                learning::ColliderBox* wallBox = m_pObjects[i]->GetColliderBox();
                if (dinoBox && wallBox)
                    hit = learning::Intersect(*dinoBox, *wallBox);
            }
            if (hit)
            {
                if (m_pObjects[i]->Type() == ObjectType::Wall)
                {
                    m_gameState = GameState::GameOver;
                    return;
                }

                if (m_pObjects[i]->Type() == ObjectType::ITEM)
                {
                    m_score += 100;
                    delete m_pObjects[i];
                    m_pObjects[i] = nullptr;
                }
            }
        }
    }

    // 오브젝트 업데이트
    for (int i = 0; i < 10; ++i)
    {
        if (m_pObjects[i]) m_pObjects[i]->Update(m_fDeltaTime);
    }

    if (m_pDino) m_pDino->Update(m_fDeltaTime);
}

void DinoGame::CreatePlayer()
{
    GameObject* pNewObject = new GameObject(ObjectType::PLAYER);

    pNewObject->SetName("Player");
    pNewObject->SetPosition(800.0f, 620.0f);
    pNewObject->SetSpeed(0.22f); // 일단, 임의로 설정  

    pNewObject->SetWidth(200); // 일단, 임의로 설정
    pNewObject->SetHeight(113); // 일단, 임의로 설정

    pNewObject->SetColliderBox(40.0f, 90.0f); // 일단, 임의로 설정. 오브젝트 설정할 거 다 하고 나서 하자.
    pNewObject->SetBitmapInfo(m_pPlayerBitmapInfo); // 여러분은 따로 빼지 않아서 그냥, 하면 됩니다.

    m_pDino = pNewObject;
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

    pNewObject->SetColliderBox(30.0f,10.0f); // 일단, 임의로 설정. 오브젝트 설정할 거 다 하고 나서 하자.
    //pNewObject->SetBitmapInfo(m_pEnemyWallInfo);
    int i = 0;
    

}

void DinoGame::UpdateDinoInfo()
{
    if (!m_pDino) return;

    // 마우스 좌클릭 누르고 있는 동안 시간 누적
    if (m_isPressingJump)
        m_jumpPressTime += m_fDeltaTime;

    Vector2f dinoPos = m_pDino->GetPosition();

    // 점프 중이면 X방향 유지 (변경 없음!)
    if (!m_isOnGround)
    {
        m_pDino->SetDirection(Vector2f(m_jumpX, 0.0f));

    }
    else // 마우스 따라다니기 (땅에 있다고 판단시)
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
                float speed = (dirNorm < 0) ? 50.0f : 100.0f; // 왼쪽 빠름, 오른쪽 느림
                m_pDino->ChangeBitmapInfo(m_pPlayerBitmapInfo, 20, speed);
                m_lastDirNorm = dirNorm;
            }
        }
        else // 목표 위치 근처 도달 시 정지
        {
            m_pDino->SetDirection(Vector2f(0.0f, 0.0f));
            m_lastDirNorm = 0.0f;
        }
    }

    // 점프 y계산
    const float GROUND_Y = 620.0f;
    if (!m_isOnGround)
    {
        m_jumpTime += m_fDeltaTime;
        float t = m_jumpTime / m_jumpDuration;

        // 2단 점프면 시작y가 땅이 아닌 공중
        float startY = (m_jumpCount == 2) ? m_jumpStartY : GROUND_Y;

        if (t >= 1.0f)
        {
            m_isOnGround = true;
            m_jumpCount = 0;
            dinoPos.y = GROUND_Y;
            m_pDino->ChangeBitmapInfo(m_pPlayerBitmapInfo, 20, 50.0f);
        }
        else if (t < 0.5f)
        {
            float upT = t / 0.5f;
            dinoPos.y = startY + (m_jumpTarget - startY) * upT;
        }
        else
        {
            float downT = (t - 0.5f) / 0.5f;
            dinoPos.y = m_jumpTarget + (GROUND_Y - m_jumpTarget) * downT;
        }
    }

    //화면 밖 제한 
    float halfW = 100.0f;

    dinoPos.x = dinoPos.x < halfW ? halfW : dinoPos.x;
    dinoPos.x = dinoPos.x > m_width - halfW ? m_width - halfW : dinoPos.x;

    m_pDino->SetPosition(dinoPos.x, dinoPos.y);
}
void DinoGame::UpdateWallInfo() {
    const float GAME_START_DELAY = 3000.0f; // 대략 3초 후부터 스폰
    const float SCREEN_RIGHT = 1100.0f;     // 화면 오른쪽 경계

    m_gameTime += m_fDeltaTime;

    // 오브젝트 이동 & 화면 밖으로 나가면 삭제
    for (int i = 0; i < 10; ++i)
    {
        if (m_pObjects[i] == nullptr) continue;

        Vector2f pos = m_pObjects[i]->GetPosition();

        // 화면 오른쪽 벗어나면 삭제
        if (pos.x > SCREEN_RIGHT)
        {
            delete m_pObjects[i];
            m_pObjects[i] = nullptr;
            continue;
        }
    }

    // 게임시작 3초 이후부터 스폰 
    if (m_gameTime < GAME_START_DELAY) return;

    m_spawnTimer += m_fDeltaTime;
    if (m_spawnTimer < m_spawnInterval) return;
    m_spawnTimer = 0.0f;
    // 다음 스폰 간격 랜덤: 1.5초 ~ 4초
    m_spawnInterval = 1500.0f + (rand() % 2501);

    // 빈 슬롯 찾기
    int emptySlot = -1;
    for (int i = 0; i < 10; ++i)
    {
        if (m_pObjects[i] == nullptr)
        {
            emptySlot = i;
            break;
        }
    }
    if (emptySlot == -1) return; // 빈 슬롯 없으면 스킵

    // 랜덤으로 벽 or 쿠키 결정 (쿠키40% 벽60% 확률)
    bool isCoin = (rand() % 100 < 40);

    GameObject* pNew = new GameObject(isCoin ? ObjectType::ITEM : ObjectType::Wall);

    // y 위치: 코인은 살짝 위, 벽은 바닥
    float spawnY = 0.0f;
    if (isCoin)
    {
        // 쿠키: 70 ~ 620 전체 랜덤
        spawnY = 100.0f + (rand() % 550); // 100 ~ 650
    }
    else
    {
        spawnY = 620.0f; // 곰돌이는 계속 같은 위치에서 태어나도록
    }
    pNew->SetPosition(-50.0f, spawnY);  // 화면 왼쪽 밖에서 시작
    pNew->SetDirection(Vector2f(1.0f, 0.0f));  // 오른쪽으로
    pNew->SetSpeed(0.15f);  // 맵 스크롤 속도와 동일


    
    if (isCoin)
    {
        pNew->SetWidth(60);
        pNew->SetHeight(60);
        pNew->SetColliderCircle(25.0f);
        if (m_pCookieBitmapInfo)
            pNew->SetBitmapInfo(m_pCookieBitmapInfo, 1);
    }
    else
    {
        int wallHeight = 100 + (rand() % 201);
        int wallWidth = 120;

        pNew->SetWidth(wallWidth);
        pNew->SetHeight(wallHeight);

        float groundY = 620.0f + (113.0f / 2.0f);

        // 곰 이미지 아래 투명 여백 보정
        float footOffset = wallHeight * 0.12f;

        float wallCenterY = groundY - (wallHeight / 2.0f) + footOffset;

        pNew->SetPosition(-50.0f, wallCenterY);

        pNew->SetColliderBox(wallWidth * 0.4f, wallHeight * 0.55f);

        if (m_pWallBitmapInfo) {
            pNew->SetBitmapInfo(m_pWallBitmapInfo, 1);
        }
    }

    m_pObjects[emptySlot] = pNew;  


}

void DinoGame::UpdateMapInfo() {
    if (!m_pMapBitmapInfo) return;

    m_mapScrollX -= m_mapScrollSpeed * m_fDeltaTime;

    int mapW = m_pMapBitmapInfo->GetWidth();
    if (m_mapScrollX <= 0)      // 0 이하일 때만 리셋
    {
        m_mapScrollX += mapW;
    }

}

void DinoGame::Update()
{
    m_pGameTimer->Tick();
    m_fDeltaTime = m_pGameTimer->DeltaTimeMS();
    LogicUpdate();
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
    if (m_gameState == GameState::Ready) // 시작화면일 경우
    {
        if (m_pReadyBitmapInfo)
        { // 시작화면 PNG 출력
            HDC hReadyDC = CreateCompatibleDC(m_hBackDC);
            HBITMAP hOld = (HBITMAP)SelectObject(hReadyDC, m_pReadyBitmapInfo->GetBitmapHandle());

            StretchBlt(
                m_hBackDC,
                0, 0, m_width, m_height,
                hReadyDC,
                0, 0,
                m_pReadyBitmapInfo->GetWidth(),
                m_pReadyBitmapInfo->GetHeight(),
                SRCCOPY
            );

            SelectObject(hReadyDC, hOld);
            DeleteDC(hReadyDC);

        }

       
        BitBlt(m_hFrontDC, 0, 0, m_width, m_height, m_hBackDC, 0, 0, SRCCOPY);
        return;
    }
    else if (m_gameState == GameState::GameOver) // 게임 오버 화면
    {
        if (m_pEndBitmapInfo)
        { // 게임 오버화면 출력
            HDC hEndDC = CreateCompatibleDC(m_hBackDC);
            HBITMAP hOld = (HBITMAP)SelectObject(hEndDC, m_pEndBitmapInfo->GetBitmapHandle());

            StretchBlt(
                m_hBackDC,
                0, 0, m_width, m_height,
                hEndDC,
                0, 0,
                m_pEndBitmapInfo->GetWidth(),
                m_pEndBitmapInfo->GetHeight(),
                SRCCOPY
            );

            SelectObject(hEndDC, hOld);
            DeleteDC(hEndDC);
        }
        wchar_t scoreText[64]; // 배경, 오브젝트, 플레이어, 점수등 출력
        swprintf_s(scoreText, L"SCORE : %d", m_score);
        SetBkMode(m_hBackDC, TRANSPARENT);
        SetTextColor(m_hBackDC, RGB(0, 0, 0));
        HFONT hFont = CreateFont(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH, L"Arial");
        HFONT hOldFont = (HFONT)SelectObject(m_hBackDC, hFont);
        TextOut(m_hBackDC, m_width / 2 - 500, 280, scoreText, wcslen(scoreText));
        SelectObject(m_hBackDC, hOldFont);
        DeleteObject(hFont);


        BitBlt(m_hFrontDC, 0, 0, m_width, m_height, m_hBackDC, 0, 0, SRCCOPY);
        return;
    }


    if (m_pMapBitmapInfo)
    {
        HDC hMapDC = CreateCompatibleDC(m_hBackDC);
        HBITMAP hOld = (HBITMAP)SelectObject(hMapDC, m_pMapBitmapInfo->GetBitmapHandle());

        int mapW = m_pMapBitmapInfo->GetWidth();
        int mapH = m_pMapBitmapInfo->GetHeight();
        int srcX = (int)m_mapScrollX;

        // 배경을 화면 높이에 맞게 비율 유지하며 스트레치
        // 화면 너비 기준으로 나눠서 두 조각을 이어붙임
        // 첫 번째 조각: srcX ~ mapW
        int srcW1 = mapW - srcX;
        int dstW1 = (int)((float)srcW1 / mapW * m_width * ((float)mapW / mapH) / ((float)m_width / m_height));

        // 계산이 복잡하니 단순하게: 맵을 화면 높이에 맞춰 전체 스케일 먼저 계산
        float scale = (float)m_height / mapH;
        int scaledMapW = (int)(mapW * scale); // 스케일 적용된 전체 맵 너비

        int dstX1 = -(int)(srcX * scale); // 오른쪽으로 밀기

        // 첫 번째: 맵 전체를 스케일링해서 dstX1 위치에 그림
        StretchBlt(m_hBackDC, dstX1, 0, scaledMapW, m_height,
            hMapDC, 0, 0, mapW, mapH, SRCCOPY);

        // 두 번째: 맵이 끝나는 지점 바로 뒤에 이어붙이기 (무한루프)
        StretchBlt(m_hBackDC, dstX1 + scaledMapW, 0, scaledMapW, m_height,
            hMapDC, 0, 0, mapW, mapH, SRCCOPY);

        SelectObject(hMapDC, hOld);
        DeleteDC(hMapDC);
    }
    for (int i = 0; i < 10; ++i)
        if (m_pObjects[i]) m_pObjects[i]->Render(m_hBackDC);

    if (m_pDino) m_pDino->Render(m_hBackDC);

    //점수판 추가!
    wchar_t scoreText[64];
    swprintf_s(scoreText, L"SCORE: %d", m_score);
    SetBkMode(m_hBackDC, TRANSPARENT);
    SetTextColor(m_hBackDC, RGB(255, 255, 0));
    HFONT hFont = CreateFont(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH, L"Arial");
    HFONT hOldFont = (HFONT)SelectObject(m_hBackDC, hFont);
    TextOut(m_hBackDC, 20, 20, scoreText, wcslen(scoreText));
    SelectObject(m_hBackDC, hOldFont);
    DeleteObject(hFont);
    BitBlt(m_hFrontDC, 0, 0, m_width, m_height, m_hBackDC, 0, 0, SRCCOPY);
    //메모리 DC에 그려진 결과를 실제 DC(m_hFrontDC)로 복사
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
    //std::cout << __FUNCTION__ << std::endl;
    //std::cout << "x: " << x << ", y: " << y << std::endl;
    // 대기 화면 버튼 사용
    if (m_gameState == GameState::Ready)
    {
        POINT pt = { x, y };

        if (PtInRect(&m_startButton, pt))
        {
            m_gameState = GameState::Start;
            return;
        }

        if (PtInRect(&m_howButton, pt))
        {
            MessageBox(m_hWnd, L"                구구가가의 위치보다 \n      앞이나 뒤를 클릭해 두번까지 점프해\n           곰을 피하고 쿠키를 먹으세요!", L"게임 방법", MB_OK);
            return;
        }

        if (PtInRect(&m_exitButton, pt))
        {
            PostQuitMessage(0);
            return;
        }

        return;
    }
    // 게임 종료 화면 버튼 사용 
    if (m_gameState == GameState::GameOver)
    {
        POINT pt = { x, y };

        if (PtInRect(&m_restartButton, pt))
        {
            RestartGame();
            return;
        }

        if (PtInRect(&m_gameExitButton, pt))
        {
            PostQuitMessage(0);
            return;
        }

        return;
    }

    if (m_isPressingJump) return; // 이미 누르는 중이면 무시

    // 1단 땅에 있을 때 꾹 누르기 방식
    if (m_jumpCount == 0)
    {
        m_isPressingJump = true;
        m_jumpPressTime = 0.0f;
        m_jumpX = m_pDino->GetDirection().x;
    }
    // 2단 점프 공중에 있으면 바로 점프한번더!
    else if (m_jumpCount == 1)
    {
        m_pDino->ChangeBitmapInfo(m_pJumpBitmapInfo, 18, 20.0f);

        // 현재 높이에서 바로 점프 (고정 높이)
        Vector2f dinoPos = m_pDino->GetPosition();
        m_jumpTarget = dinoPos.y - 200.0f;   // 현재 위치에서 200 위로
        m_jumpDuration = 800.0f;              // 고정 시간
        m_jumpTime = 0.0f;
        m_jumpCount++;
        m_isOnGround = false;

        // y시작위지치준으로 점프
        m_jumpStartY = dinoPos.y;
    }
}

void DinoGame::OnLButtonUp(int x, int y)
{

    if (!m_isPressingJump) return;

    m_pDino->ChangeBitmapInfo(m_pJumpBitmapInfo, 18, 20.0f); 

    m_isPressingJump = false;

    // 누른 시간 점프 높이/시간 결정
    float pressTime = m_jumpPressTime > 620.0f ? 620.0f : m_jumpPressTime;    
    float ratio = pressTime / 620.0f; // 0.0 ~ 1.0

    float jumpHeight = 150.0f + ratio * 200.0f; // 최소150 ~ 최대350
    m_jumpTarget = 620.0f - jumpHeight;      // y가 작을수록 위
    m_jumpDuration = 620.0f + ratio * 620.0f;  // 최소500 ~ 최대1000
    m_jumpTime = 0.0f;
    m_jumpCount++;
    m_isOnGround = false;
}

void DinoGame::ReadyGame(){
    BitBlt(m_hFrontDC, 0, 0, m_width, m_height, m_hBackDC, 0, 0, SRCCOPY);
    // 시작 대기 상태
}

void DinoGame::StartGame()
{
    // 게임 진행 상태
}

void DinoGame::RestartGame()
{
    m_gameState = GameState::Start;

    // 점수/타이머 리셋
    m_score = 0;
    m_gameTime = 0.0f;
    m_spawnTimer = 0.0f;
    m_spawnInterval = 2000.0f;

    // 기존 오브젝트 삭제
    for (int i = 0; i < 10; ++i)
    {
        if (m_pObjects[i])
        {
            delete m_pObjects[i];
            m_pObjects[i] = nullptr;
        }
    }

    // 점프 상태 리셋
    m_jumpCount = 0;
    m_isOnGround = true;
    m_isPressingJump = false;
    m_jumpTime = 0.0f;
    m_jumpPressTime = 0.0f;
    m_mapScrollX = 0.0f;

    // 플레이어 위치 리셋
    if (m_pDino)
    {
        m_pDino->SetPosition(800.0f, 620.0f);
        m_pDino->ChangeBitmapInfo(m_pPlayerBitmapInfo, 20, 50.0f);
    }
}
