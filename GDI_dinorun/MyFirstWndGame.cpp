#include "INC_Windows.h"
#include "MyFirstWndGame.h"
#include "GameTimer.h"
#include "Collider.h"
#include "GameObject.h"
#include "RenderHelp.h"
#include <iostream>
#include <assert.h>

using namespace learning;

constexpr int MAX_GAME_OBJECT_COUNT = 1000;

bool MyFirstWndGame::Initialize()
{
    m_pGameTimer = new GameTimer();
    m_pGameTimer->Reset();

    const wchar_t* className = L"MyFirstWndGame";
    const wchar_t* windowName = L"MyFirstWndGame";

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

    m_GameObjectPtrTable = new GameObjectBase * [MAX_GAME_OBJECT_COUNT];

    for (int i = 0; i < MAX_GAME_OBJECT_COUNT; ++i)
    {
        m_GameObjectPtrTable[i] = nullptr;
    }

    // 필요한 리소스를 로드해볼까요
#pragma region resource

    // 파일의 절대 경로와 상대 경로 구분
    // IDE 에서 인지하는 현재 경로와 실제 실행 파일을 바로 실행했을 때의 경로 기준이 달라요.
	m_pPlayerBitmapInfo = renderHelp::CreateBitmapInfo(L"./Resource/redbird.png");
	m_pEnemyBitmapInfo = renderHelp::CreateBitmapInfo(L"./Resource/graybird.png");

#pragma endregion

    // 리소스를 먼저 로드한 후에 세팅을 합니다.

	// [CHECK]. 첫 번째 게임 오브젝트는 플레이어 캐릭터로 고정!
	CreatePlayer();

    return true;

}

void MyFirstWndGame::Run()
{
    MSG msg = { 0 };
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_LBUTTONDOWN)
            {
                MyFirstWndGame::OnLButtonDown(LOWORD(msg.lParam), HIWORD(msg.lParam));
            }
            else if (msg.message == WM_RBUTTONDOWN)
            {
                MyFirstWndGame::OnRButtonDown(LOWORD(msg.lParam), HIWORD(msg.lParam));
            }
            else if (msg.message == WM_MOUSEMOVE)
            {
                MyFirstWndGame::OnMouseMove(LOWORD(msg.lParam), HIWORD(msg.lParam));
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

void MyFirstWndGame::Finalize()
{
    delete m_pGameTimer;
    m_pGameTimer = nullptr;

    if (m_GameObjectPtrTable)
    {
        for (int i = 0; i < MAX_GAME_OBJECT_COUNT; ++i)
        {
            if (m_GameObjectPtrTable[i])
            {
                delete m_GameObjectPtrTable[i];
                m_GameObjectPtrTable[i] = nullptr;
            }
        }
        delete m_GameObjectPtrTable;
    }

    __super::Destroy();
}

void MyFirstWndGame::FixedUpdate()
{
    if (m_EnemySpawnPos.x != 0 && m_EnemySpawnPos.y != 0)
    {
        CreateEnemy();
    }
}

void MyFirstWndGame::LogicUpdate()
{

    UpdatePlayerInfo();
    UpdateEnemyInfo();


    for (int i = 0; i < MAX_GAME_OBJECT_COUNT; ++i)
    {
        if (m_GameObjectPtrTable[i])
        {
            m_GameObjectPtrTable[i]->Update(m_fDeltaTime);
        }
    }
    
    ResolveEnemyOverlap(); // 부딪힌다면 안된다!! 떨어트려잇!!
    
}

void MyFirstWndGame::CreatePlayer()
{
    assert(m_GameObjectPtrTable[0] == nullptr && "Player object already exists!");

    GameObject* pNewObject = new GameObject(ObjectType::PLAYER);

    pNewObject->SetName("Player");
    pNewObject->SetPosition(50.0f, 50.0f); // 일단, 임의로 설정 
    pNewObject->SetSpeed(2.0f); // 일단, 임의로 설정  

    pNewObject->SetWidth(100); // 일단, 임의로 설정
    pNewObject->SetHeight(100); // 일단, 임의로 설정

    pNewObject->SetColliderCircle(50.0f); // 일단, 임의로 설정. 오브젝트 설정할 거 다 하고 나서 하자.
    pNewObject->SetBitmapInfo(m_pPlayerBitmapInfo); // 여러분은 따로 빼지 않아서 그냥, 하면 됩니다.


    m_GameObjectPtrTable[0] = pNewObject;
}

void MyFirstWndGame::CreateEnemy()
{
    GameObject* pNewObject = new GameObject(ObjectType::ENEMY);
    pNewObject->SetName("Enemy");

    float x = m_EnemySpawnPos.x;
    float y = m_EnemySpawnPos.y;

    m_EnemySpawnPos = { 0, 0 };

    pNewObject->SetPosition(x, y);
    pNewObject->SetSpeed(1.0f); // 일단, 임의로 설정   


    pNewObject->SetWidth(100); // 일단, 임의로 설정
    pNewObject->SetHeight(100); // 일단, 임의로 설정

    pNewObject->SetColliderCircle(50.0f); // 일단, 임의로 설정. 오브젝트 설정할 거 다 하고 나서 하자.
    pNewObject->SetBitmapInfo(m_pEnemyBitmapInfo);
    int i = 0;
    while (++i < MAX_GAME_OBJECT_COUNT) //0번째는 언제나 플레이어!
    {
        if (nullptr == m_GameObjectPtrTable[i])
        {
            m_GameObjectPtrTable[i] = pNewObject;
            break;
        }
    }

    if (i == MAX_GAME_OBJECT_COUNT)
    {
        // 게임 오브젝트 테이블이 가득 찼습니다.
        delete pNewObject;
        pNewObject = nullptr;
    }
}

void MyFirstWndGame::UpdatePlayerInfo()
{
    static GameObject* pPlayer = GetPlayer();

    assert(pPlayer != nullptr);

    Vector2f mousePos(m_PlayerTargetPos.x, m_PlayerTargetPos.y);
    Vector2f playerPos = pPlayer->GetPosition();

    Vector2f playerDir = mousePos - playerPos;
    float distance = playerDir.Length(); // 거리 계산

    if (distance > 50.f) //임의로 설정한 거리
    {
        playerDir.Normalize(); // 정규화
        pPlayer->SetDirection(playerDir); // 플레이어 방향 설정
    }
    else
    {
        pPlayer->SetDirection(Vector2f(0, 0)); // 플레이어 정지
    }
}

void MyFirstWndGame::UpdateEnemyInfo() {
    GameObject* pPlayer = GetPlayer(); //플레이어 객체를 가져옵니다
    assert(pPlayer != nullptr); // 없다면 멈춰!

    Vector2f playerDir = pPlayer->GetDirection(); // 플레이어 현재 이동 방향
    if (playerDir.x == 0.f && playerDir.y == 0.f) { // 플레이어가 멈춰있는가
        for (int i = 1;i < MAX_GAME_OBJECT_COUNT; ++i) { // ENEMY숫자만큼
            if (m_GameObjectPtrTable[i] != nullptr) { // i번째까지 nullptr이 아니면
                m_GameObjectPtrTable[i]->SetDirection(Vector2f(0, 0));
                // 플레이어가 멈추면 모든 Enemy방향도 0,0으로 바꾼다
            }
        }
        return;
    }
    // 플레이어가 멈춘 상태가 아니라면
    Vector2f playerPos = pPlayer->GetPosition();  // 플레이어의 위치를 가져와요
    for (int i = 1; i < MAX_GAME_OBJECT_COUNT; ++i) 
    {
        if (m_GameObjectPtrTable[i] != nullptr)
        {
            Vector2f enemyPos = m_GameObjectPtrTable[i]->GetPosition();
            // Enemy의 위치를 가져옵니다요
            Vector2f dir = playerPos - enemyPos;
            // 방향을 설정(플레이어 - enemypos해서) player를 향하도록
            float distance = dir.Length();
            // 거리를 구합니다
            if (distance > 1.f)
            { // 만약 거리가 1.0보다 클경우에만 추저합니다
                dir.Normalize();
                // 방향벡터 1로 만듭니다
                m_GameObjectPtrTable[i]->SetDirection(dir);
                // Enemy의 이동방향을 dir(플레이어쪽)으로 설정합니다
            }
            else
            { // 또는 거의 같은 위치에 존재하는 경우에는 멈출 수 있게합니다
                m_GameObjectPtrTable[i]->SetDirection(Vector2f(0, 0));
            }
        }
    }
}

void MyFirstWndGame::ResolveEnemyOverlap() { // 적이 안겹치게 하는 함수
    const float distanceMin = 100.0f; // 최소한 멀어져야 하는 거리!
    const float distanceSpeed = 3.0f;
    for (int i = 1;i < MAX_GAME_OBJECT_COUNT;++i)
    {
        if (m_GameObjectPtrTable[i] == nullptr)
            continue; // nullptre이면 넘어가용 ㅋ
        for (int j = i + 1;j < MAX_GAME_OBJECT_COUNT;++j) {
            if (m_GameObjectPtrTable[j] == nullptr)
                continue; // nullptr이면 넘어가용


            Vector2f positionA = m_GameObjectPtrTable[i]->GetPosition();
            Vector2f positionB = m_GameObjectPtrTable[j]->GetPosition();
            //두 새 i,j의 위치를 가져와요!!

            Vector2f direction = positionA - positionB; // 방향가지고오기
            float distance = direction.Length(); // 거리(방향의 길이)
            if (distance < distanceMin) {
                if (distance == 0.f) {
                    // 방향도 강제로 설정 (0,0이면) 
                    direction = Vector2f(1.f, 0.f);
                    distance = 1.f;
                    // 같은 위치면 0,0이니까 강제로 distance를 준다에요
                }
                direction.Normalize(); // 노멀라이즈 후 더 빨라지게 3.0f곱함
                positionA.x += direction.x * distanceSpeed;
                positionA.y += direction.y * distanceSpeed;
                // A의 새 위치를 100보다 작으면 더해줘요!
                positionB.x -= direction.x * distanceSpeed;
                positionB.y -= direction.y * distanceSpeed;
                //B의 위치에는 빼줘요 그래야 A <- -> B멀어지니까요!

                m_GameObjectPtrTable[i]->SetPosition(positionA.x, positionA.y);
                m_GameObjectPtrTable[j]->SetPosition(positionB.x, positionB.y);
                // 다시 계산한 위치를 넣어줘요
            }

        }
    }
    
}

void MyFirstWndGame::Update()
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

void MyFirstWndGame::Render()
{
    //Clear the back buffer
    ::PatBlt(m_hBackDC, 0, 0, m_width, m_height, WHITENESS);

    //메모리 DC에 그리기
    for (int i = 0; i < MAX_GAME_OBJECT_COUNT; ++i)
    {
        if (m_GameObjectPtrTable[i])
        {
            m_GameObjectPtrTable[i]->Render(m_hBackDC);
        }
    }

    //메모리 DC에 그려진 결과를 실제 DC(m_hFrontDC)로 복사
    BitBlt(m_hFrontDC, 0, 0, m_width, m_height, m_hBackDC, 0, 0, SRCCOPY);
}

void MyFirstWndGame::OnResize(int width, int height)
{
    std::cout << __FUNCTION__ << std::endl;

    learning::SetScreenSize(width, height);

    __super::OnResize(width, height);

    m_hBackBitmap = CreateCompatibleBitmap(m_hFrontDC, m_width, m_height);

    HANDLE hPrevBitmap = (HBITMAP)SelectObject(m_hBackDC, m_hBackBitmap);

    DeleteObject(hPrevBitmap);
}

void MyFirstWndGame::OnClose()
{
    std::cout << __FUNCTION__ << std::endl;

    SelectObject(m_hBackDC, m_hDefaultBitmap);

    DeleteObject(m_hBackBitmap);
    DeleteDC(m_hBackDC);

    ReleaseDC(m_hWnd, m_hFrontDC);
}

void MyFirstWndGame::OnMouseMove(int x, int y)
{
    /*   std::cout << __FUNCTION__ << std::endl;
       std::cout << "x: " << x << ", y: " << y << std::endl;*/
    m_MousePosPrev = m_MousePos;
    m_MousePos = { x, y };
}

void MyFirstWndGame::OnLButtonDown(int x, int y)
{
	std::cout << __FUNCTION__ << std::endl;
	std::cout << "x: " << x << ", y: " << y << std::endl;

    m_PlayerTargetPos.x = x;
    m_PlayerTargetPos.y = y;
}

void MyFirstWndGame::OnRButtonDown(int x, int y)
{
	std::cout << __FUNCTION__ << std::endl;
	std::cout << "x: " << x << ", y: " << y << std::endl;

    m_EnemySpawnPos.x = x;
    m_EnemySpawnPos.y = y;
}