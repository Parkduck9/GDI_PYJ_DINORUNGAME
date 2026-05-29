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

class DinoGame : public NzWndBase
{
public :
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

	void CreatePlayer();
	void CreateWall();
	void CreateEnemy();

	void UpdateDinoInfo();
	void UpdateWallInfo();
	void ResolveWallOverLap();

	//GameObject* GetPlayer() const { return (GameObject*)m_GameObjectPtrTable[0]; }

private :
	HDC m_hFrontDC = nullptr;
	HDC m_hBackDC = nullptr;

	HBITMAP m_hBackBitmap = nullptr;
	HBITMAP m_hDefaultBitmap = nullptr;

	GameTimer* m_pGameTimer = nullptr;
	float m_fDeltaTime = 0.0f;
	float m_fFrameCount = 0.0f;

	float m_jumpX = 0.0f;
	float m_jumpStartY = 0.0f;
	float m_jumpTime = 0.0f;
	float m_jumpDuration = 0.0f;
	float m_jumpTarget = 0.0f;
	int m_jumpCount = 0;

	float m_lastDirNorm = 0.0f;

	float m_jumpPressTime = 0.0f;
	bool  m_isPressingJump = false;

	bool m_isOnGround = true;


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
	GameObject* m_pWalls[10] = {};

	using BitmapInfo = renderHelp::BitmapInfo;

	BitmapInfo* m_pPlayerBitmapInfo = nullptr;
	BitmapInfo* m_pJumpBitmapInfo = nullptr;

	BitmapInfo* m_pWallBitmapInfo = nullptr;
};