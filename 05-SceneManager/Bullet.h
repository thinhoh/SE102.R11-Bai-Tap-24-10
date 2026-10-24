#pragma once
#include "GameObject.h"
#include "Animation.h"
#include "Animations.h"
#include "Game.h"
#include "Spawner.h"

#define BULLET_SPEED        0.45f
#define BULLET_WIDTH        12
#define BULLET_HEIGHT       6

#define ID_ANI_BULLET       1010

class CBullet : public CGameObject
{
public:
	CBullet(float x, float y) : CGameObject(x, y)
	{
		this->vx = BULLET_SPEED;
		this->vy = 0.0f;
	}

	virtual void Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects = NULL)
	{
		x += vx * dt;
		y += vy * dt;

		// Xóa đạn khi bay ra khỏi mép phải màn hình
		CGame* game = CGame::GetInstance();
		float cx, cy;
		game->GetCamPos(cx, cy);

		if (x > cx + game->GetBackBufferWidth() + 30.0f || x < cx - 30.0f)
		{
			isDeleted = true;
			return;
		}

		// Va chạm giữa Đạn và Quái (Enemy)
		if (coObjects != NULL)
		{
			float bL, bT, bR, bB;
			GetBoundingBox(bL, bT, bR, bB);

			for (size_t i = 0; i < coObjects->size(); i++)
			{
				LPGAMEOBJECT obj = (*coObjects)[i];
				if (obj == NULL || obj->IsDeleted()) continue;

				CEnemy* enemy = dynamic_cast<CEnemy*>(obj);
				if (enemy != NULL && enemy->IsCollidable())
				{
					float eL, eT, eR, eB;
					enemy->GetBoundingBox(eL, eT, eR, eB);

					// AABB collision check
					if (!(bR < eL || bL > eR || bB < eT || bT > eB))
					{
						enemy->OnDie();         // Quái nổ tung
						this->isDeleted = true; // Đạn tan biến
						break;
					}
				}
			}
		}
	}

	virtual void Render()
	{
		CAnimations* animations = CAnimations::GetInstance();
		LPANIMATION ani = animations->Get(ID_ANI_BULLET);
		if (ani != NULL)
			ani->Render(x, y);
	}

	virtual void GetBoundingBox(float& left, float& top, float& right, float& bottom)
	{
		left = x - BULLET_WIDTH / 2.0f;
		top = y - BULLET_HEIGHT / 2.0f;
		right = left + BULLET_WIDTH;
		bottom = top + BULLET_HEIGHT;
	}

	virtual int IsBlocking() { return 0; }
	virtual int IsCollidable() { return 1; }
};
