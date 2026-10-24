#include <algorithm>
#include "debug.h"

#include "Mario.h"
#include "Game.h"
#include "Bullet.h"
#include "PlayScene.h"

#include "Goomba.h"
#include "Coin.h"
#include "Portal.h"

#include "Collision.h"

CMario::CMario(float x, float y) : CGameObject(x, y)
{
	isSitting = false;
	maxVx = 0.0f;
	ax = 0.0f;
	ay = 0.0f; // Bỏ hoàn toàn trọng lực để tàu bay tự do

	level = MARIO_LEVEL_BIG;
	untouchable = 0;
	untouchable_start = -1;
	isOnPlatform = false;
	coin = 0;
	state = SHIP_STATE_IDLE;

	// Khởi tạo hệ thống xả đạn & reload timer
	isBursting = false;
	burstBulletCount = 0;
	lastBulletTime = 0;

	isReloading = false;
	reloadStartTime = 0;
	dieStartTime = 0;
}

void CMario::Shoot()
{
	if (state == SHIP_STATE_DIE) return;
	if (isReloading || isBursting) return;

	// Bắt đầu đợt xả đạn 5 viên liên hoàn
	isBursting = true;
	burstBulletCount = 0;
	lastBulletTime = 0; // Để viên đầu tiên bắn ngay tức thì
}

void CMario::Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects)
{
	// Nếu tàu nổ: nổ tại chỗ, không di chuyển, đếm ngược 3 giây rồi đóng game
	if (state == SHIP_STATE_DIE)
	{
		vx = 0.0f;
		vy = 0.0f;

		ULONGLONG elapsed = GetTickCount64() - dieStartTime;
		int secondsLeft = (int)((SHIP_DIE_TIMEOUT - elapsed) / 1000) + 1;
		if (secondsLeft < 0) secondsLeft = 0;

		DebugOutTitle(L"Life Force (NES) | GAME OVER - Closing in %d...", secondsLeft);

		if (elapsed >= SHIP_DIE_TIMEOUT)
		{
			PostQuitMessage(0); // Tự động đóng game sau 3 giây
		}
		return; // Không chạy logic di chuyển, va chạm hay bắn đạn nữa
	}

	// Reset trạng thái bất tử nếu hết thời gian
	if (GetTickCount64() - untouchable_start > MARIO_UNTOUCHABLE_TIME) 
	{
		untouchable_start = 0;
		untouchable = 0;
	}

	// 1. Xử lý xả đạn hàng loạt (Burst firing 5 viên/lần)
	if (isBursting)
	{
		ULONGLONG now = GetTickCount64();
		if (now - lastBulletTime >= BURST_DELAY_TIME)
		{
			LPPLAYSCENE scene = (LPPLAYSCENE)CGame::GetInstance()->GetCurrentScene();
			if (scene != NULL)
			{
				// Bắn viên đạn năng lượng từ mũi tàu
				scene->AddObject(new CBullet(x + 14.0f, y));
			}

			lastBulletTime = now;
			burstBulletCount++;

			// Khi đã xả đủ 5 viên trong loạt
			if (burstBulletCount >= BURST_MAX_BULLETS)
			{
				isBursting = false;
				isReloading = true;
				reloadStartTime = now; // Bắt đầu đếm giờ reload
			}
		}
	}

	// 2. Xử lý Timer Reload
	if (isReloading)
	{
		ULONGLONG elapsed = GetTickCount64() - reloadStartTime;
		if (elapsed >= RELOAD_TIME)
		{
			isReloading = false; // Đã reload xong 5 viên mới!
		}
	}

	// Hiển thị trạng thái đạn & đếm ngược reload lên tiêu đề cửa sổ
	if (isReloading)
	{
		ULONGLONG elapsed = GetTickCount64() - reloadStartTime;
		int timeLeft = (RELOAD_TIME > elapsed) ? (int)(RELOAD_TIME - elapsed) : 0;
		DebugOutTitle(L"Life Force (NES) | RELOADING... [%d ms]", timeLeft);
	}
	else if (isBursting)
	{
		DebugOutTitle(L"Life Force (NES) | FIRING! [%d/%d]", burstBulletCount, BURST_MAX_BULLETS);
	}
	else
	{
		DebugOutTitle(L"Life Force (NES) | AMMO READY [5/5] - Press SPACE to Fire!");
	}

	// 3. Xử lý va chạm
	CCollision::GetInstance()->Process(this, dt, coObjects);

	// Kiểm tra va chạm với Enemy khi Enemy bay chạm vào thân tàu
	if (state != SHIP_STATE_DIE && untouchable == 0 && coObjects != NULL)
	{
		float pL, pT, pR, pB;
		GetBoundingBox(pL, pT, pR, pB);

		for (size_t i = 0; i < coObjects->size(); i++)
		{
			LPGAMEOBJECT obj = (*coObjects)[i];
			if (obj == NULL || obj->IsDeleted()) continue;

			CEnemy* enemy = dynamic_cast<CEnemy*>(obj);
			if (enemy != NULL && enemy->IsCollidable())
			{
				float eL, eT, eR, eB;
				enemy->GetBoundingBox(eL, eT, eR, eB);

				if (!(pR < eL || pL > eR || pB < eT || pT > eB))
				{
					enemy->OnDie();
					SetState(SHIP_STATE_DIE);
					break;
				}
			}
		}
	}

	// 4. Giới hạn viền màn hình (Trái, Trên, Dưới) để tàu không bay xuyên qua
	CGame* game = CGame::GetInstance();
	float cx, cy;
	game->GetCamPos(cx, cy);

	float minX = cx + SHIP_BBOX_WIDTH / 2.0f;
	float minY = SHIP_BBOX_HEIGHT / 2.0f + 2.0f;
	float maxY = (float)game->GetBackBufferHeight() - (SHIP_BBOX_HEIGHT / 2.0f + 2.0f);

	if (x < minX)
	{
		x = minX;
		if (vx < 0) vx = 0;
	}

	if (y < minY)
	{
		y = minY;
		if (vy < 0) vy = 0;
	}
	else if (y > maxY)
	{
		y = maxY;
		if (vy > 0) vy = 0;
	}
}

void CMario::OnNoCollision(DWORD dt)
{
	x += vx * dt;
	y += vy * dt;
	isOnPlatform = false;
}

void CMario::OnCollisionWith(LPCOLLISIONEVENT e)
{
	if (e->ny != 0 && e->obj->IsBlocking())
	{
		vy = 0;
		if (e->ny < 0) isOnPlatform = true;
	}
	else if (e->nx != 0 && e->obj->IsBlocking())
	{
		vx = 0;
	}

	if (dynamic_cast<CGoomba*>(e->obj))
		OnCollisionWithGoomba(e);
	else if (dynamic_cast<CCoin*>(e->obj))
		OnCollisionWithCoin(e);
	else if (dynamic_cast<CPortal*>(e->obj))
		OnCollisionWithPortal(e);
}

void CMario::OnCollisionWithGoomba(LPCOLLISIONEVENT e)
{
	CGoomba* goomba = dynamic_cast<CGoomba*>(e->obj);

	if (untouchable == 0 && goomba->GetState() != GOOMBA_STATE_DIE)
	{
		DebugOut(L">>> Ship DIE >>> \n");
		SetState(SHIP_STATE_DIE);
	}
}

void CMario::OnCollisionWithCoin(LPCOLLISIONEVENT e)
{
	e->obj->Delete();
	coin++;
}

void CMario::OnCollisionWithPortal(LPCOLLISIONEVENT e)
{
	CPortal* p = (CPortal*)e->obj;
	CGame::GetInstance()->InitiateSwitchScene(p->GetSceneId());
}

void CMario::Render()
{
	CAnimations* animations = CAnimations::GetInstance();
	int aniId = ID_ANI_SHIP_IDLE;

	if (state == SHIP_STATE_DIE)
		aniId = ID_ANI_SHIP_DIE;
	else if (state == SHIP_STATE_FLY_UP)
		aniId = ID_ANI_SHIP_FLY_UP;
	else if (state == SHIP_STATE_FLY_DOWN)
		aniId = ID_ANI_SHIP_FLY_DOWN;
	else
		aniId = ID_ANI_SHIP_IDLE;

	LPANIMATION ani = animations->Get(aniId);
	if (ani != NULL)
		ani->Render(x, y);

	// RenderBoundingBox();
}

void CMario::SetState(int state)
{
	if (this->state == SHIP_STATE_DIE) return;

	if (state == SHIP_STATE_DIE)
	{
		this->state = SHIP_STATE_DIE;
		this->vx = 0.0f;
		this->vy = 0.0f;
		this->ax = 0.0f;
		this->ay = 0.0f;
		this->dieStartTime = GetTickCount64();
		return;
	}

	this->state = state;
}

void CMario::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
	left = x - SHIP_BBOX_WIDTH / 2;
	top = y - SHIP_BBOX_HEIGHT / 2;
	right = left + SHIP_BBOX_WIDTH;
	bottom = top + SHIP_BBOX_HEIGHT;
}
