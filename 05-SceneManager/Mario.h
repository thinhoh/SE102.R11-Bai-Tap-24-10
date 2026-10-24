#pragma once
#include "GameObject.h"

#include "Animation.h"
#include "Animations.h"

#include "debug.h"

// Vận tốc bay của tàu
#define SHIP_FLYING_SPEED       0.15f
#define SHIP_GRAVITY            0.0f

// Kích thước Bounding Box của thân tàu (24x10 pixel)
#define SHIP_BBOX_WIDTH         24
#define SHIP_BBOX_HEIGHT        10

// Các State của tàu
#define SHIP_STATE_IDLE         0   // Bay thẳng
#define SHIP_STATE_FLY_UP       100 // Nghiêng cánh lên
#define SHIP_STATE_FLY_DOWN     200 // Nghiêng cánh xuống
#define SHIP_STATE_DIE          -10 // Nổ tung
#define SHIP_DIE_TIMEOUT        3000 // Nổ 3 giây rồi đóng game

// Cấu hình xả đạn hàng loạt và nạp đạn (Reload Timer)
#define BURST_MAX_BULLETS       5       // 5 viên mỗi đợt xả
#define BURST_DELAY_TIME        75      // 75ms giữa mỗi viên đạn trong loạt xả
#define RELOAD_TIME             800     // 800ms để reload xong đợt tiếp theo

// ID Animation tương ứng trong file mario.txt
#define ID_ANI_SHIP_IDLE        1000
#define ID_ANI_SHIP_FLY_UP      1001
#define ID_ANI_SHIP_FLY_DOWN    1002
#define ID_ANI_SHIP_DIE         1099

// Giữ lại các define Mario cũ nếu các file khác có tham chiếu
#define MARIO_STATE_DIE         -10
#define MARIO_STATE_IDLE        0
#define MARIO_LEVEL_SMALL       1
#define MARIO_LEVEL_BIG         2

#define MARIO_UNTOUCHABLE_TIME  2500

class CMario : public CGameObject
{
	BOOLEAN isSitting;
	float maxVx;
	float ax;				// acceleration on x 
	float ay;				// acceleration on y 

	int level; 
	int untouchable; 
	ULONGLONG untouchable_start;
	BOOLEAN isOnPlatform;
	int coin; 

	// Biến quản lý xả đạn & reload timer
	bool isBursting;
	int burstBulletCount;
	ULONGLONG lastBulletTime;

	bool isReloading;
	ULONGLONG reloadStartTime;

	ULONGLONG dieStartTime;

	void OnCollisionWithGoomba(LPCOLLISIONEVENT e);
	void OnCollisionWithCoin(LPCOLLISIONEVENT e);
	void OnCollisionWithPortal(LPCOLLISIONEVENT e);

public:
	CMario(float x, float y);
	void Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects);
	void Render();
	void SetState(int state);

	void Shoot();
	bool IsReloading() { return isReloading; }
	bool IsBursting() { return isBursting; }

	int IsCollidable()
	{ 
		return (state != SHIP_STATE_DIE); 
	}

	int IsBlocking() { return (state != SHIP_STATE_DIE && untouchable == 0); }

	void OnNoCollision(DWORD dt);
	void OnCollisionWith(LPCOLLISIONEVENT e);

	void SetLevel(int l) { this->level = l; }
	void StartUntouchable() { untouchable = 1; untouchable_start = GetTickCount64(); }

	void GetBoundingBox(float& left, float& top, float& right, float& bottom);
};
