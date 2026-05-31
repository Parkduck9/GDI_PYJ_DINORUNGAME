#pragma once

#include "NzWndBase.h"
#include "Utillity.h"

class GameTimer;
class GameObjectBase;
class GameObject;

namespace renderHelp
{
	class BitmapInfo;
}

enum class GameState
{	// 메인 메뉴 , 게임 진행 , 게임 종료 화면 상태관리!
	Ready,
	Start,
	GameOver
};

class DinoGame : public NzWndBase
{
public:
	DinoGame() = default;
	~DinoGame() override = default;

	bool Initialize();
	void Run();
	void Finalize();

private:
	void Update();
	void Render();

	void OnResize(int width, int height) override;
	void OnClose() override;

	void OnMouseMove(int x, int y);
	void OnLButtonDown(int x, int y);
	void OnLButtonUp(int x, int y);

	void FixedUpdate();
	void LogicUpdate();

	void UpdateMapInfo();

	void CreatePlayer();
	void CreateWall();
	void CreateEnemy();

	void UpdateDinoInfo();
	void UpdateWallInfo();

	//GameObject* GetPlayer() const { return (GameObject*)m_GameObjectPtrTable[0]; }

private:
	HDC m_hFrontDC = nullptr;
	HDC m_hBackDC = nullptr;

	HBITMAP m_hBackBitmap = nullptr;
	HBITMAP m_hDefaultBitmap = nullptr;
	// 게임 상태 전환 함수
	void ReadyGame();
	void StartGame();
	void RestartGame();


	GameTimer* m_pGameTimer = nullptr;
	float m_fDeltaTime = 0.0f; 
	float m_fFrameCount = 0.0f;
	
	// 점프 관련 변수
	float m_jumpX = 0.0f; // 점프 시작 시 x방향 저장
	float m_jumpStartY = 0.0f; // 2단 점프 시작 높이저장
	float m_jumpTime = 0.0f; // 현재 점프 진행 시간
	float m_jumpDuration = 0.0f; // 점프 총 시간
	float m_jumpTarget = 0.0f; // 점프 목표 높이
	int m_jumpCount = 0; // 점프 횟수 (최대 2번 (0,1))


	// 배경 스크롤 변수
	float m_mapScrollX = 0.0f;      // 현재 스크롤 위치
	float m_mapScrollSpeed = 0.15f; // 배경이동속도

	float m_lastDirNorm = 0.0f;

	// 입력 상태
	float m_jumpPressTime = 0.0f; // 점프 누른 시간
	bool  m_isPressingJump = false; 
	bool m_isOnGround = true; // 땅에 있는 착지 여부

	GameState m_gameState = GameState::Ready;

	// 게임 시작 시 버튼 (UI 버튼 영역)
	RECT m_startButton = { 395, 385, 655, 465 };
	RECT m_howButton = { 395, 475, 655, 560 };
	RECT m_exitButton = { 395, 575, 655, 655 };


	// 게임 종료 시 버튼 (UI 버튼 영역)
	RECT m_restartButton = { 15, 615, 260, 700 };
	RECT m_gameExitButton = { 745, 615, 990, 700 };

	
	
	//게임 진행 정보
	int m_score = 0; // 점수 기록

	struct Mouse_Pos // 마우스 위치
	{
		int x = 0;
		int y = 0;
		bool operator != (const Mouse_Pos& other) const
		{
			return (x != other.x || y != other.y);
		}
	};
	Mouse_Pos m_MousePos = { 0,0 };
	Mouse_Pos m_MousePosPrev = { 0,0 };
	Mouse_Pos m_PlayerTargetPos = { 0, 0 };

	// 게임 오브젝트 
	GameObject* m_pDino = nullptr;
	GameObject* m_pObjects[10] = {};
	float m_spawnTimer = 0.0f;
	float m_spawnInterval = 2000.0f;
	float m_gameTime = 0.0f;



	using BitmapInfo = renderHelp::BitmapInfo;
	// 비트맵 리소스
	BitmapInfo* m_pPlayerBitmapInfo = nullptr; // 플레이어 달리기
	BitmapInfo* m_pJumpBitmapInfo = nullptr; // 플레이어 점프

	BitmapInfo* m_pMapBitmapInfo = nullptr; // 배경

	BitmapInfo* m_pCookieBitmapInfo = nullptr; // 쿠키 (코인)

	BitmapInfo* m_pWallBitmapInfo = nullptr; // 곰 ( 벽)

	BitmapInfo* m_pReadyBitmapInfo = nullptr; // 시작 화면
	BitmapInfo* m_pEndBitmapInfo = nullptr; // 게임 오버 화면
};