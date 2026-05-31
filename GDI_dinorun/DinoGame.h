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
{	
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

	void ReadyGame();
	void StartGame();
	void RestartGame();



	GameTimer* m_pGameTimer = nullptr;
	float m_fDeltaTime = 0.0f;
	float m_fFrameCount = 0.0f;

	float m_jumpX = 0.0f;
	float m_jumpStartY = 0.0f;
	float m_jumpTime = 0.0f;
	float m_jumpDuration = 0.0f;
	float m_jumpTarget = 0.0f;
	int m_jumpCount = 0;

	float m_mapScrollX = 0.0f;      // 현재 스크롤 위치
	float m_mapScrollSpeed = 0.15f; // 달리기 속도와 동일하게

	float m_lastDirNorm = 0.0f;

	float m_jumpPressTime = 0.0f;
	bool  m_isPressingJump = false;

	bool m_isOnGround = true;

	GameState m_gameState = GameState::Ready;

	// 게임 시작 시 버튼
	RECT m_startButton = { 395, 385, 655, 465 };
	RECT m_howButton = { 395, 475, 655, 560 };
	RECT m_exitButton = { 395, 575, 655, 655 };


	// 게임 종료 시 버튼
	RECT m_restartButton = { 15, 615, 260, 700 };
	RECT m_gameExitButton = { 745, 615, 990, 700 };

	int m_score = 0;

	struct Mouse_Pos
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

	GameObject* m_pDino = nullptr;
	GameObject* m_pObjects[10] = {};
	float m_spawnTimer = 0.0f;
	float m_spawnInterval = 2000.0f;
	float m_gameTime = 0.0f;



	using BitmapInfo = renderHelp::BitmapInfo;

	BitmapInfo* m_pPlayerBitmapInfo = nullptr;
	BitmapInfo* m_pJumpBitmapInfo = nullptr;

	BitmapInfo* m_pMapBitmapInfo = nullptr;

	BitmapInfo* m_pCookieBitmapInfo = nullptr;

	BitmapInfo* m_pWallBitmapInfo = nullptr;

	BitmapInfo* m_pReadyBitmapInfo = nullptr;
	BitmapInfo* m_pEndBitmapInfo = nullptr;
};