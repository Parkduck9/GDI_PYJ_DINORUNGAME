#include "INC_Windows.h"

#include "Collider.h"
#include "GameObject.h"
#include "RenderHelp.h"
#include <assert.h>

GameObject::~GameObject()
{
	if (m_pColliderCircle)
	{
		delete m_pColliderCircle;
		m_pColliderCircle = nullptr;
	}

	if (m_pColliderBox)
	{
		delete m_pColliderBox;
		m_pColliderBox = nullptr;
	}
}

void GameObject::SetBitmapInfo(BitmapInfo* bitmapInfo) // 기본 스프라이트 이미지 설정
{
	assert(m_pBitmapInfo == nullptr && "BitmapInfo must be null!");

	m_pBitmapInfo = bitmapInfo;

	// 스프라이트 정보는 일단은 하드코딩해요. 
	// 일단, 프레임 크기와 시간이 같다고 가정합니다.
	m_frameWidth = m_pBitmapInfo->GetWidth() / 20;
	m_frameHeight = m_pBitmapInfo->GetHeight();
	//m_frameIndex = 0;
	m_frameCount = 20;
	for (int i = 0; i < 20; ++i)
	{
		m_frameXY[i].x = i * m_frameWidth;
		m_frameXY[i].y = 0;
	}
}
void GameObject::SetBitmapInfo(BitmapInfo* bitmapInfo, int frameCount)
{   // 프레임이 1인 것들을 위해서 제작했으나 나중에 조절가능하게 사용해도될듯합니다요?
	assert(m_pBitmapInfo == nullptr && "BitmapInfo must be null!");

	m_pBitmapInfo = bitmapInfo;
	m_frameCount = frameCount;
	m_frameWidth = m_pBitmapInfo->GetWidth() / frameCount;
	m_frameHeight = m_pBitmapInfo->GetHeight();
	// 각프레임 시작 좌표 저장
	for (int i = 0; i < frameCount; ++i)
	{
		m_frameXY[i].x = i * m_frameWidth;
		m_frameXY[i].y = 0;
	}
}
void GameObject::ChangeBitmapInfo(BitmapInfo* bitmapInfo, int frameCount, float m_frameJumpDuration)
{  //현재 오브젝트의 애니메이션 이미지를 다른 이미지로 교체해보자!
	m_pBitmapInfo = bitmapInfo;
	m_frameCount = frameCount;
	m_frameDuration = m_frameJumpDuration;
	m_frameWidth = m_pBitmapInfo->GetWidth() / frameCount;
	m_frameHeight = m_pBitmapInfo->GetHeight();
	//m_frameIndex = 0;
	//m_frameTime = 0.0f;
	if (m_frameIndex >= frameCount) { 
		// 만약 현재 프레임 번호가 새 프레임 수보다 크면 0으로 초기화!
		m_frameIndex = 0;
	}
	for (int i = 0; i < frameCount; ++i)
	{
		m_frameXY[i].x = i * m_frameWidth;
		m_frameXY[i].y = 0;
	}
}

void GameObject::Update(float deltaTime) 
{ // 매 프레임 호출되는 업데이트 함수
	UpdateFrame(deltaTime); //프레임 갱신!

	Move(deltaTime); // 위치 이동!

	// Collider 업데이트 (콜라이더 주심을 오브젝트 위치와 동기화)
	if (m_pColliderCircle)
	{
		m_pColliderCircle->center = m_pos;
	}
	if (m_pColliderBox)
	{
		m_pColliderBox->center = m_pos;
	}
}

void GameObject::Render(HDC hdc)
{
	DrawBitmap(hdc); //이미지 출력
	//DrawCollider(hdc); // 충돌 박스 출력(게임 상 안보이게 주석처리)
}

//원형 콜라이더 생성
void GameObject::SetColliderCircle(float radius)
{
	if (m_pColliderCircle)
	{
		delete m_pColliderCircle;
		m_pColliderCircle = nullptr;
	}

	m_pColliderCircle = new ColliderCircle;

	assert(m_pColliderCircle != nullptr && "Failed to create ColliderCircle!");

	m_pColliderCircle->radius = radius;
	m_pColliderCircle->center = m_pos;
}

// 박스 콜라이더 생성width height는 전체크기
void GameObject::SetColliderBox(float width, float height)
{
	if (m_pColliderBox)
	{
		delete m_pColliderBox;
		m_pColliderBox = nullptr;
	}

	m_pColliderBox = new ColliderBox;

	assert(m_pColliderBox != nullptr && "Failed to create ColliderBox!");
	// 내부에서 반으로 저장
	m_pColliderBox->center = m_pos;
	m_pColliderBox->halfSize.x = width / 2.0f;
	m_pColliderBox->halfSize.y = height / 2.0f;
}

// 과제: 해당 코드의 문제는 무엇일까요? 어떻게 개선하면 좋을까요?
// 개선 방향에 대해 서로 토론하고 비교해 보세요.
void GameObject::DrawCollider(HDC hdc)
{
	HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

	if (m_pColliderCircle)
	{
		Ellipse(hdc, m_pColliderCircle->center.x - m_pColliderCircle->radius,
			m_pColliderCircle->center.y - m_pColliderCircle->radius,
			m_pColliderCircle->center.x + m_pColliderCircle->radius,
			m_pColliderCircle->center.y + m_pColliderCircle->radius);
	}

	if (m_pColliderBox)
	{
		Rectangle(hdc, m_pColliderBox->center.x - m_pColliderBox->halfSize.x,
			m_pColliderBox->center.y - m_pColliderBox->halfSize.y,
			m_pColliderBox->center.x + m_pColliderBox->halfSize.x,
			m_pColliderBox->center.y + m_pColliderBox->halfSize.y);
	}

	// 이전 객체 복원 및 펜 삭제
	SelectObject(hdc, hOldPen);
	SelectObject(hdc, hOldBrush);
	DeleteObject(hPen);
}

void GameObject::Move(float deltaTime)
{
	GameObjectBase::Move(deltaTime);
}

void GameObject::DrawBitmap(HDC hdc)
{
	if (m_pBitmapInfo == nullptr) return;
	if (m_pBitmapInfo->GetBitmapHandle() == nullptr) return;

	HDC hBitmapDC = CreateCompatibleDC(hdc);

	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hBitmapDC, m_pBitmapInfo->GetBitmapHandle());
	// BLENDFUNCTION 설정 (알파 채널 처리)
	BLENDFUNCTION blend = { 0 };
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = 255;  // 원본 알파 채널 그대로 사용
	blend.AlphaFormat = AC_SRC_ALPHA;

	const int x = m_pos.x - m_width / 2;
	const int y = m_pos.y - m_height / 2;

	const int srcX = m_frameXY[m_frameIndex].x;
	const int srcY = m_frameXY[m_frameIndex].y;

	AlphaBlend(hdc, x, y, m_width, m_height,
		hBitmapDC, srcX, srcY, m_frameWidth, m_frameHeight, blend);

	// 비트맵 핸들 복원
	SelectObject(hBitmapDC, hOldBitmap);
	DeleteDC(hBitmapDC);
}

void GameObject::UpdateFrame(float deltaTime)
{ //일정 시간 지나면 다음 프레임으로 변경
	m_frameTime += deltaTime;
	if (m_frameTime >= m_frameDuration)
	{
		m_frameTime = 0.0f;
		m_frameIndex = (m_frameIndex + 1) % (m_frameCount);
	}
}

void GameObjectBase::SetName(const char* name)
{
	if (name == nullptr) return;

	strncpy_s(m_name, name, OBJECT_NAME_LEN_MAX - 1);
	m_name[OBJECT_NAME_LEN_MAX - 1] = '\0';
}