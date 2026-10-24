#include "SampleKeyEventHandler.h"

#include "debug.h"
#include "Game.h"

#include "Mario.h"
#include "PlayScene.h"

void CSampleKeyHandler::OnKeyDown(int KeyCode)
{
	CMario* ship = (CMario*)((LPPLAYSCENE)CGame::GetInstance()->GetCurrentScene())->GetPlayer();
	if (!ship) return;

	switch (KeyCode)
	{
	case DIK_0:
		ship->SetState(SHIP_STATE_DIE);
		break;
	case DIK_J:
	case DIK_SPACE:
		ship->Shoot();
		break;
	}
}

void CSampleKeyHandler::OnKeyUp(int KeyCode)
{
}

void CSampleKeyHandler::KeyState(BYTE* states)
{
	LPGAME game = CGame::GetInstance();
	CMario* ship = (CMario*)((LPPLAYSCENE)CGame::GetInstance()->GetCurrentScene())->GetPlayer();

	if (!ship || ship->GetState() == SHIP_STATE_DIE) return;

	float vx = 0.0f;
	float vy = 0.0f;

	if (game->IsKeyDown(DIK_RIGHT)) vx += SHIP_FLYING_SPEED;
	if (game->IsKeyDown(DIK_LEFT))  vx -= SHIP_FLYING_SPEED;
	if (game->IsKeyDown(DIK_UP))    vy -= SHIP_FLYING_SPEED;
	if (game->IsKeyDown(DIK_DOWN))  vy += SHIP_FLYING_SPEED;

	ship->SetSpeed(vx, vy);

	if (vy < 0)
		ship->SetState(SHIP_STATE_FLY_UP);
	else if (vy > 0)
		ship->SetState(SHIP_STATE_FLY_DOWN);
	else
		ship->SetState(SHIP_STATE_IDLE);

	// Giữ phím Space hoặc J để tự động xả loạt 5 viên khi reload xong
	if (game->IsKeyDown(DIK_SPACE) || game->IsKeyDown(DIK_J))
	{
		ship->Shoot();
	}
}
