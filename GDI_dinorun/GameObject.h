#pragma once

#include "Utillity.h"
#include <algorithm>
// [CHECK]. namespace 포함해서 전방 선언해야 함
namespace learning
{
	struct ColliderCircle;
	struct ColliderBox;
}

namespace renderHelp
{
	class BitmapInfo;
}

enum class ObjectType
{
	PLAYER,
	ENEMY,
	BULLET,
	ITEM,
	BACKGROUND,
	Wall,
};

constexpr int OBJECT_NAME_LEN_MAX = 15;

class GameObjectBase
{
	using Vector2f = learning::Vector2f;
public:
	GameObjectBase() = delete;
	GameObjectBase(const GameObjectBase&) = delete;

	GameObjectBase(ObjectType type) : m_type(type) {}

	virtual ~GameObjectBase() = default;

	virtual void Update(float deltaTime) = 0;
	virtual void Render(HDC hdc) = 0;

	void SetPosition(float x, float y) { m_pos = { x, y }; }
	void SetDirection(Vector2f dir) { m_dir = dir; }
	void SetSpeed(float speed) { m_speed = speed; }
	void SetName(const char* name);

	void SetWidth(int width) { m_width = width; }
	void SetHeight(int height) { m_height = height; }

	ObjectType Type() const { return m_type; }

	const char* GetName() const { return m_name; }

	Vector2f GetPosition() const { return m_pos; }
	Vector2f GetDirection() const { return m_dir; }

	float GetSpeed() const { return m_speed; }

protected:

	void Move(float deltaTime)
	{
		m_pos.x += m_dir.x * m_speed * deltaTime;
		m_pos.y += m_dir.y * m_speed * deltaTime;
	}

protected:
	ObjectType m_type;

	int m_width = 0;
	int m_height = 0;

	Vector2f m_pos = { 0.0f, 0.0f };
	Vector2f m_dir = { 0.0f, 0.0f }; // 방향 (단위 벡터)

	float m_speed = 0.0f; // 속력

	char m_name[OBJECT_NAME_LEN_MAX] = "";
};

class GameObject : public GameObjectBase
{
	using ColliderCircle = learning::ColliderCircle;
	using ColliderBox = learning::ColliderBox;

	using BitmapInfo = renderHelp::BitmapInfo;

public:
	GameObject(const GameObject&) = delete;
	GameObject(ObjectType type) : GameObjectBase(type) {}
	~GameObject() override;

	void SetBitmapInfo(BitmapInfo* bitmapInfo);
	void ChangeBitmapInfo(BitmapInfo* bitmapInfo, int frameCount ,float m_frameDuration);

	void Update(float deltaTime) override;
	void Render(HDC hdc) override;

	void SetColliderCircle(float radius);
	void SetColliderBox(float halfWidth, float halfHeight);
	// public 접근자 추가
	ColliderBox* GetColliderBox() const { return m_pColliderBox; }
	ColliderCircle* GetColliderCircle() const { return m_pColliderCircle; }
	void SetBitmapInfo(BitmapInfo* bitmapInfo, int frameCount); // 프레임 수 지정 버전
protected:
	void DrawCollider(HDC hdc);

	void Move(float deltaTime);

	void DrawBitmap(HDC hdc);
	void UpdateFrame(float deltaTime);

	// Bitmap 정보
	BitmapInfo* m_pBitmapInfo = nullptr;

	// 점진적으로 예쁘게 고쳐 보아요.
	struct FrameFPos
	{
		int x;
		int y;
	};
	// 프레임 정보
	FrameFPos m_frameXY[20] = { { 0, 0 }, };
	int m_frameWidth = 0;
	int m_frameHeight = 0;
	int m_frameIndex = 0;
	int m_frameCount = 20; // 프레임 수

	float m_frameTime = 0.0f;
	float m_frameDuration = 50.0f;
	float m_frameJumpDuration = 10.0f;
	// Collider
	ColliderCircle* m_pColliderCircle = nullptr;
	ColliderBox* m_pColliderBox = nullptr;

};
