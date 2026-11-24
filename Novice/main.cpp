#include <Novice.h>
#include <cmath>
#include <memory>

const char kWindowTitle[] = "学籍番号";

// 弾の構造体
struct Bullet {
	float x = 0.0f;
	float y = 0.0f;
	float radius = 8.0f;
	float speed = 12.0f;
	bool isShot = false;
};

class Player;

// 自機の状態の基底クラス
class PlayerState {
public:
	virtual ~PlayerState() = default;

	// 状態に入ったときの処理
	virtual void Enter(Player&) {}

	// 状態ごとの更新処理
	virtual void Update(Player& player, const char* keys, const char* preKeys) = 0;

	// 状態ごとの描画色を取得
	virtual unsigned int GetColor() const = 0;

	// 状態名を取得
	virtual const char* GetName() const = 0;
};

// 待機状態クラス
class PlayerIdleState : public PlayerState {
public:
	void Update(Player& player, const char* keys, const char* preKeys) override;
	unsigned int GetColor() const override { return WHITE; }
	const char* GetName() const override { return "IDLE"; }
};

// 移動状態クラス
class PlayerMoveState : public PlayerState {
public:
	void Update(Player& player, const char* keys, const char* preKeys) override;
	unsigned int GetColor() const override { return 0x88FFFFFF; }
	const char* GetName() const override { return "MOVE"; }
};

// 攻撃状態クラス
class PlayerAttackState : public PlayerState {
public:
	void Enter(Player& player) override;
	void Update(Player& player, const char* keys, const char*) override;
	unsigned int GetColor() const override { return 0xFF8800FF; }
	const char* GetName() const override { return "ATTACK"; }

private:
	// 攻撃状態の継続フレーム数
	int attackTimer_ = 0;
};

// 自機クラス
class Player {
public:
	Player(Bullet* bullet) : bullet_(bullet) {
		// 初期状態を「待機」に設定
		ChangeState(std::make_unique<PlayerIdleState>());
	}

	// 状態を切り替える
	void ChangeState(std::unique_ptr<PlayerState> newState) {
		state_ = std::move(newState);
		if (state_) {
			state_->Enter(*this);
		}
	}

	// 更新処理
	void Update(const char* keys, const char* preKeys) {
		if (state_) {
			state_->Update(*this, keys, preKeys);
		}
	}

	// 描画処理
	void Draw(int textureHandle) const {
		unsigned int color = state_ ? state_->GetColor() : WHITE;
		Novice::DrawSprite(static_cast<int>(x_ - radius_), static_cast<int>(y_ - radius_), textureHandle, radius_ * 2.0f, radius_ * 2.0f, 0.0f, color);
	}

	// 移動と画面外制限
	void Move(float dx, float dy) {
		x_ += dx * speed_;
		y_ += dy * speed_;

		// 画面外に出ないように制限
		if (x_ < radius_) {
			x_ = radius_;
		}
		if (x_ > 1280.0f - radius_) {
			x_ = 1280.0f - radius_;
		}
		if (y_ < radius_) {
			y_ = radius_;
		}
		if (y_ > 720.0f - radius_) {
			y_ = 720.0f - radius_;
		}
	}

	// 弾を発射できるか確認
	bool CanShoot() const { return bullet_ && !bullet_->isShot; }

	// 弾を発射する
	void Shoot() {
		if (CanShoot()) {
			bullet_->isShot = true;
			bullet_->x = x_;
			bullet_->y = y_;
		}
	}

	// 現在の状態名を取得
	const char* GetStateName() const { return state_ ? state_->GetName() : "NONE"; }

private:
	float x_ = 640.0f;
	float y_ = 600.0f;
	float radius_ = 20.0f;
	float speed_ = 6.0f;

	Bullet* bullet_ = nullptr;
	std::unique_ptr<PlayerState> state_;
};

// 待機状態の更新処理
void PlayerIdleState::Update(Player& player, const char* keys, const char* preKeys) {
	// SPACEキーが押されたら攻撃状態へ遷移
	if (player.CanShoot() && preKeys[DIK_SPACE] == 0 && keys[DIK_SPACE] != 0) {
		player.ChangeState(std::make_unique<PlayerAttackState>());
		return;
	}

	// WASDのいずれかが押されたら移動状態へ遷移
	if (keys[DIK_W] != 0 || keys[DIK_S] != 0 || keys[DIK_A] != 0 || keys[DIK_D] != 0) {
		player.ChangeState(std::make_unique<PlayerMoveState>());
		return;
	}
}

// 移動状態の更新処理
void PlayerMoveState::Update(Player& player, const char* keys, const char* preKeys) {
	// SPACEキーが押されたら攻撃状態へ遷移
	if (player.CanShoot() && preKeys[DIK_SPACE] == 0 && keys[DIK_SPACE] != 0) {
		player.ChangeState(std::make_unique<PlayerAttackState>());
		return;
	}

	// 移動入力の計算（WASD）
	float dx = 0.0f;
	float dy = 0.0f;
	if (keys[DIK_W] != 0) {
		dy -= 1.0f;
	}
	if (keys[DIK_S] != 0) {
		dy += 1.0f;
	}
	if (keys[DIK_A] != 0) {
		dx -= 1.0f;
	}
	if (keys[DIK_D] != 0) {
		dx += 1.0f;
	}

	// 入力がなくなったら待機状態へ遷移
	if (dx == 0.0f && dy == 0.0f) {
		player.ChangeState(std::make_unique<PlayerIdleState>());
		return;
	}

	// 自機の移動を実行
	player.Move(dx, dy);
}

// 攻撃状態の開始処理
void PlayerAttackState::Enter(Player& player) {
	attackTimer_ = 10;
	player.Shoot();
}

// 攻撃状態の更新処理
void PlayerAttackState::Update(Player& player, const char* keys, const char*) {
	// 攻撃硬直タイマーを減らす
	attackTimer_--;

	// 攻撃時間が終了したら次の状態へ遷移
	if (attackTimer_ <= 0) {
		if (keys[DIK_W] != 0 || keys[DIK_S] != 0 || keys[DIK_A] != 0 || keys[DIK_D] != 0) {
			player.ChangeState(std::make_unique<PlayerMoveState>());
		} else {
			player.ChangeState(std::make_unique<PlayerIdleState>());
		}
		return;
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	// 2Dテクスチャの読み込み
	// ※環境によってフルパスが必要な場合は "C:/PG3_4/Novice/Resources/white1x1.png" に変更してください
	int whiteTextureHandle = Novice::LoadTexture("white1x1.png");

	// 弾の変数
	Bullet bullet;

	// 自機の生成
	Player player(&bullet);

	// 敵の変数
	float enemyX = 640.0f;
	float enemyY = 150.0f;
	float enemyRadius = 25.0f;
	float enemySpeed = 4.0f;
	bool isEnemyAlive = true;

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		// 自機の更新処理
		player.Update(keys, preKeys);

		// 弾の移動処理
		if (bullet.isShot) {
			bullet.y -= bullet.speed;
			// 画面上端に出たら発射状態を解除
			if (bullet.y < -bullet.radius) {
				bullet.isShot = false;
			}
		}

		// 敵の移動処理
		if (isEnemyAlive) {
			enemyX += enemySpeed;
			// 画面端で反転
			if (enemyX < enemyRadius || enemyX > 1280.0f - enemyRadius) {
				enemySpeed = -enemySpeed;
			}
		} else {
			// Rキーで敵を再出現
			if (preKeys[DIK_R] == 0 && keys[DIK_R] != 0) {
				isEnemyAlive = true;
				enemyX = 640.0f;
			}
		}

		// 弾と敵の当たり判定
		if (bullet.isShot && isEnemyAlive) {
			float dx = bullet.x - enemyX;
			float dy = bullet.y - enemyY;
			float distance = std::sqrtf(dx * dx + dy * dy);
			if (distance <= bullet.radius + enemyRadius) {
				bullet.isShot = false;
				isEnemyAlive = false;
			}
		}

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		// 敵の描画（赤色）
		if (isEnemyAlive) {
			Novice::DrawSprite(static_cast<int>(enemyX - enemyRadius), static_cast<int>(enemyY - enemyRadius), whiteTextureHandle, enemyRadius * 2.0f, enemyRadius * 2.0f, 0.0f, RED);
		}

		// 弾の描画（黄色）
		if (bullet.isShot) {
			Novice::DrawSprite(
			    static_cast<int>(bullet.x - bullet.radius), static_cast<int>(bullet.y - bullet.radius), whiteTextureHandle, bullet.radius * 2.0f, bullet.radius * 2.0f, 0.0f, 0xFFFF00FF);
		}

		// 自機の描画
		player.Draw(whiteTextureHandle);

		// 操作説明と現在の状態の描画
		Novice::ScreenPrintf(20, 20, "WASD: Move / SPACE: Shot / R: Respawn Enemy");
		Novice::ScreenPrintf(20, 40, "Player State: %s", player.GetStateName());

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}