#pragma once
#include <vector>
#include <cmath>
#include "GameObject.h"
#include "Animation.h"
#include "Animations.h"
#include "Game.h"
#include "PlayScene.h"

// ID Animation trong mario.txt
#define ID_ANI_ENEMY_DRONE      2000
#define ID_ANI_ENEMY_EXPLOSION  1099

#define ENEMY_DEFAULT_SPEED     0.08f
#define ENEMY_BBOX_WIDTH        14
#define ENEMY_BBOX_HEIGHT       14

#define ENEMY_STATE_ALIVE       0
#define ENEMY_STATE_DIE         1

// ========================================================
// 1. LỚP QUÁI KHÔNG GIAN (SPACE ENEMY)
// ========================================================
enum EnemyPattern
{
	PATTERN_STRAIGHT = 0,       // Bay thẳng sang trái
	PATTERN_SINE,               // Lượn sóng hình Sin
	PATTERN_DIAGONAL_DOWN,      // Bay chéo xuống
	PATTERN_DIAGONAL_UP,        // Bay chéo lên
	PATTERN_ZIGZAG              // Đổi hướng Zigzag
};

class CEnemy : public CGameObject
{
protected:
	EnemyPattern pattern;
	float startY;
	float totalTime;
	float waveAmplitude;
	float waveFrequency;
	ULONGLONG dieStart;

public:
	CEnemy(float x, float y, EnemyPattern pat = PATTERN_STRAIGHT, float speed = ENEMY_DEFAULT_SPEED)
		: CGameObject(x, y)
	{
		this->pattern = pat;
		this->startY = y;
		this->totalTime = 0.0f;
		this->waveAmplitude = 25.0f;
		this->waveFrequency = 0.005f;
		this->state = ENEMY_STATE_ALIVE;
		this->dieStart = 0;

		this->vx = -speed;
		this->vy = 0.0f;

		if (pattern == PATTERN_DIAGONAL_DOWN)
			this->vy = speed * 0.5f;
		else if (pattern == PATTERN_DIAGONAL_UP)
			this->vy = -speed * 0.5f;
	}

	void SetWaveParams(float amplitude, float frequency)
	{
		this->waveAmplitude = amplitude;
		this->waveFrequency = frequency;
	}

	virtual void Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects = NULL)
	{
		// Nếu quái bị tiêu diệt: chạy animation nổ 300ms rồi tự hủy
		if (state == ENEMY_STATE_DIE)
		{
			if (GetTickCount64() - dieStart > 300)
			{
				isDeleted = true;
			}
			return;
		}

		totalTime += dt;
		x += vx * dt;

		if (pattern == PATTERN_SINE)
		{
			y = startY + waveAmplitude * sin(waveFrequency * totalTime);
		}
		else if (pattern == PATTERN_ZIGZAG)
		{
			vy = (sin(0.003f * totalTime) > 0) ? 0.05f : -0.05f;
			y += vy * dt;
		}
		else
		{
			y += vy * dt;
		}

		// Tự hủy an toàn khi bay vượt khỏi mép trái màn hình
		CGame* game = CGame::GetInstance();
		float cx, cy;
		game->GetCamPos(cx, cy);

		if (x < cx - 40.0f)
		{
			isDeleted = true;
		}
	}

	virtual void Render()
	{
		CAnimations* animations = CAnimations::GetInstance();
		int aniId = (state == ENEMY_STATE_DIE) ? ID_ANI_ENEMY_EXPLOSION : ID_ANI_ENEMY_DRONE;

		LPANIMATION ani = animations->Get(aniId);
		if (ani != NULL)
			ani->Render(x, y);
	}

	virtual void GetBoundingBox(float& left, float& top, float& right, float& bottom)
	{
		left = x - ENEMY_BBOX_WIDTH / 2.0f;
		top = y - ENEMY_BBOX_HEIGHT / 2.0f;
		right = left + ENEMY_BBOX_WIDTH;
		bottom = top + ENEMY_BBOX_HEIGHT;
	}

	virtual void OnDie()
	{
		if (state == ENEMY_STATE_DIE) return;
		state = ENEMY_STATE_DIE;
		vx = 0;
		vy = 0;
		dieStart = GetTickCount64();
	}

	virtual int IsBlocking() { return 0; }
	virtual int IsCollidable() { return (state != ENEMY_STATE_DIE); }
};

// ========================================================
// 2. LỚP QUẢN LÝ QUY TẮC SPAWN QUÁI (SPAWN MANAGER)
// ========================================================
class CSpawner
{
private:
	CPlayScene* scene;

	// Thời gian đợt đầu
	ULONGLONG gameStartTime;
	ULONGLONG firstWaveDelayMs;
	bool isStarted;
	bool firstWaveSpawned;

	// Thời gian các đợt liên tục
	ULONGLONG lastSpawnTime;
	ULONGLONG spawnIntervalMs;
	int waveCounter;

	// Hàm lấy tọa độ X vừa xuất hiện ngoài mép phải màn hình
	float GetSpawnX()
	{
		CGame* game = CGame::GetInstance();
		float cx, cy;
		game->GetCamPos(cx, cy);
		return cx + (float)game->GetBackBufferWidth() + 20.0f;
	}

public:
	CSpawner(CPlayScene* playScene)
	{
		this->scene = playScene;
		this->gameStartTime = GetTickCount64();
		this->firstWaveDelayMs = 2000; // Mặc định 2 giây cho đợt đầu
		this->isStarted = false;
		this->firstWaveSpawned = false;

		this->lastSpawnTime = 0;
		this->spawnIntervalMs = 3500;  // Mặc định 3.5 giây/đợt liên tục
		this->waveCounter = 0;
	}

	// ----------------------------------------------------
	// HÀM 1: Chạy lúc bắt đầu game để gọi đợt quái đầu sau X giây
	// ----------------------------------------------------
	void StartFirstWaveAfter(float seconds)
	{
		this->firstWaveDelayMs = (ULONGLONG)(seconds * 1000.0f);
		this->gameStartTime = GetTickCount64();
		this->isStarted = true;
		this->firstWaveSpawned = false;
		this->lastSpawnTime = 0;
		this->waveCounter = 0;
	}

	// Đổi thời gian giãn cách giữa các đợt quái liên tục
	void SetSpawnInterval(float seconds)
	{
		this->spawnIntervalMs = (ULONGLONG)(seconds * 1000.0f);
	}

	// ----------------------------------------------------
	// CÁC HÀM TẠO LAYOUT ĐỘI HÌNH QUÁI
	// ----------------------------------------------------

	// Layout 1: Đội hình hàng ngang nối đuôi nhau
	void SpawnLineFormation(float startY = 100.0f, int count = 5, float spacingX = 35.0f, float speed = 0.08f)
	{
		float spawnX = GetSpawnX();
		for (int i = 0; i < count; i++)
		{
			scene->AddObject(new CEnemy(spawnX + i * spacingX, startY, PATTERN_STRAIGHT, speed));
		}
	}

	// Layout 2: Đội hình chữ V (Mũi tên tấn công)
	void SpawnVShapeFormation(float centerY = 120.0f, int count = 5, float spacingX = 30.0f, float spacingY = 25.0f, float speed = 0.09f)
	{
		float spawnX = GetSpawnX();
		// Con đầu đàn ở giữa
		scene->AddObject(new CEnemy(spawnX, centerY, PATTERN_STRAIGHT, speed));

		int pairs = (count - 1) / 2;
		for (int i = 1; i <= pairs; i++)
		{
			// Cánh trên
			scene->AddObject(new CEnemy(spawnX + i * spacingX, centerY - i * spacingY, PATTERN_STRAIGHT, speed));
			// Cánh dưới
			scene->AddObject(new CEnemy(spawnX + i * spacingX, centerY + i * spacingY, PATTERN_STRAIGHT, speed));
		}
	}

	// Layout 3: Đội hình lượn sóng hình Sin
	void SpawnSineWaveFormation(float centerY = 120.0f, int count = 4, float amplitude = 30.0f, float frequency = 0.005f, float speed = 0.07f)
	{
		float spawnX = GetSpawnX();
		for (int i = 0; i < count; i++)
		{
			CEnemy* e = new CEnemy(spawnX + i * 40.0f, centerY, PATTERN_SINE, speed);
			e->SetWaveParams(amplitude, frequency);
			scene->AddObject(e);
		}
	}

	// Layout 4: Đội hình gọng kìm 2 hướng (Trên chéo xuống + Dưới chéo lên)
	void SpawnPincerFormation(int countPerSide = 3, float speed = 0.08f)
	{
		float spawnX = GetSpawnX();
		for (int i = 0; i < countPerSide; i++)
		{
			scene->AddObject(new CEnemy(spawnX + i * 35.0f, 30.0f, PATTERN_DIAGONAL_DOWN, speed));
			scene->AddObject(new CEnemy(spawnX + i * 35.0f, 210.0f, PATTERN_DIAGONAL_UP, speed));
		}
	}

	// Layout 5: Đội hình bậc thang chéo
	void SpawnDiagonalFormation(float startY = 40.0f, int count = 4, bool goUp = false, float speed = 0.08f)
	{
		float spawnX = GetSpawnX();
		float stepY = goUp ? -30.0f : 30.0f;
		for (int i = 0; i < count; i++)
		{
			scene->AddObject(new CEnemy(spawnX + i * 30.0f, startY + i * stepY, PATTERN_STRAIGHT, speed));
		}
	}

	// Hàm tự động luân phiên các Layout theo lượt
	void SpawnWave(int waveIndex = -1)
	{
		int idx = (waveIndex >= 0) ? waveIndex : waveCounter;

		switch (idx % 5)
		{
		case 0:
			// Đợt 1: Hàng ngang ở giữa
			SpawnLineFormation(120.0f, 5, 35.0f, 0.08f);
			break;
		case 1:
			// Đợt 2: Lượn sóng Sin
			SpawnSineWaveFormation(110.0f, 5, 35.0f, 0.005f, 0.08f);
			break;
		case 2:
			// Đợt 3: Đội hình chữ V
			SpawnVShapeFormation(120.0f, 5, 30.0f, 25.0f, 0.09f);
			break;
		case 3:
			// Đợt 4: Gọng kìm trên dưới
			SpawnPincerFormation(3, 0.08f);
			break;
		case 4:
			// Đợt 5: Bậc thang chéo
			SpawnDiagonalFormation(50.0f, 5, false, 0.08f);
			break;
		}

		waveCounter++;
	}

	// ----------------------------------------------------
	// HÀM 2: Gọi quái liên tục chạy sau khi đợt đầu đã kích hoạt
	// ----------------------------------------------------
	void UpdateContinuousSpawn(DWORD dt)
	{
		if (!isStarted) return;

		ULONGLONG now = GetTickCount64();

		// Giai đoạn 1: Chờ đủ X giây để gọi đợt quái đầu tiên
		if (!firstWaveSpawned)
		{
			if (now - gameStartTime >= firstWaveDelayMs)
			{
				firstWaveSpawned = true;
				lastSpawnTime = now;

				// Kích hoạt đợt quái đầu tiên (Đội hình hàng ngang dễ làm quen)
				SpawnWave(0);
			}
		}
		// Giai đoạn 2: Gọi các đợt quái liên tục theo chu kỳ spawnIntervalMs
		else
		{
			if (now - lastSpawnTime >= spawnIntervalMs)
			{
				lastSpawnTime = now;
				SpawnWave(); // Luân phiên các layout tiếp theo
			}
		}
	}

	void Update(DWORD dt)
	{
		UpdateContinuousSpawn(dt);
	}
};
