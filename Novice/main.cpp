#include <Novice.h>
#include <cmath>
#include <memory>

const char kWindowTitle[] = "学籍番号";

// シーン番号
enum SCENE {
	TITLE,
	STAGE,
	CLEAR,
};

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
	Player(Bullet* bullet) : bullet_(bullet) { Init(); }

	// 初期化処理
	void Init() {
		x_ = 640.0f;
		y_ = 600.0f;
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

// シーンの基底クラス
class IScene {
protected:
	// シーン番号を管理する変数
	static int sceneNo;

public:
	virtual ~IScene() = default;

	// 初期化処理
	virtual void Init() = 0;

	// 更新処理
	virtual void Update(const char* keys, const char* preKeys) = 0;

	// 描画処理
	virtual void Draw() = 0;

	// シーン番号のゲッター
	int GetSceneNo() const { return sceneNo; }
};

// 静的メンバ変数の実体化（初期シーンはタイトル）
int IScene::sceneNo = TITLE;

// タイトルシーンクラス
class TitleScene : public IScene {
public:
	void Init() override {}

	void Update(const char* keys, const char* preKeys) override {
		// SPACEキーが押されたらステージシーンへ遷移
		if (preKeys[DIK_SPACE] == 0 && keys[DIK_SPACE] != 0) {
			sceneNo = STAGE;
		}
	}

	void Draw() override {
		// タイトル画面の描画
		Novice::DrawBox(0, 0, 1280, 720, 0.0f, 0x1E293BFF, kFillModeSolid);
		Novice::ScreenPrintf(570, 300, "SHOOTING GAME");
		Novice::ScreenPrintf(540, 360, "Press SPACE to Start");
	}
};

// ステージシーンクラス
class StageScene : public IScene {
public:
	StageScene() {
		// 2Dテクスチャの読み込み
		// ※環境によってフルパスが必要な場合は "C:/PG3_4/Novice/Resources/white1x1.png" に変更してください
		whiteTextureHandle_ = Novice::LoadTexture("white1x1.png");
		player_ = std::make_unique<Player>(&bullet_);
	}

	void Init() override {
		// 弾の初期化
		bullet_ = Bullet();

		// 自機の初期化
		player_->Init();

		// 敵の初期化
		enemyX_ = 640.0f;
		enemyY_ = 150.0f;
		enemyRadius_ = 25.0f;
		enemySpeed_ = 4.0f;
		isEnemyAlive_ = true;
	}

	void Update(const char* keys, const char* preKeys) override {
		// 自機の更新処理
		player_->Update(keys, preKeys);

		// 弾の移動処理
		if (bullet_.isShot) {
			bullet_.y -= bullet_.speed;
			// 画面上端に出たら発射状態を解除
			if (bullet_.y < -bullet_.radius) {
				bullet_.isShot = false;
			}
		}

		// 敵の移動処理
		if (isEnemyAlive_) {
			enemyX_ += enemySpeed_;
			// 画面端で反転
			if (enemyX_ < enemyRadius_ || enemyX_ > 1280.0f - enemyRadius_) {
				enemySpeed_ = -enemySpeed_;
			}
		}

		// 弾と敵の当たり判定
		if (bullet_.isShot && isEnemyAlive_) {
			float dx = bullet_.x - enemyX_;
			float dy = bullet_.y - enemyY_;
			float distance = std::sqrtf(dx * dx + dy * dy);
			if (distance <= bullet_.radius + enemyRadius_) {
				bullet_.isShot = false;
				isEnemyAlive_ = false;
			}
		}

		// 敵を撃破したらクリアシーンへ遷移
		if (!isEnemyAlive_) {
			sceneNo = CLEAR;
		}
	}

	void Draw() override {
		// 敵の描画（赤色）
		if (isEnemyAlive_) {
			Novice::DrawSprite(static_cast<int>(enemyX_ - enemyRadius_), static_cast<int>(enemyY_ - enemyRadius_), whiteTextureHandle_, enemyRadius_ * 2.0f, enemyRadius_ * 2.0f, 0.0f, RED);
		}

		// 弾の描画（黄色）
		if (bullet_.isShot) {
			Novice::DrawSprite(
			    static_cast<int>(bullet_.x - bullet_.radius), static_cast<int>(bullet_.y - bullet_.radius), whiteTextureHandle_, bullet_.radius * 2.0f, bullet_.radius * 2.0f, 0.0f, 0xFFFF00FF);
		}

		// 自機の描画
		player_->Draw(whiteTextureHandle_);

		// 操作説明と現在の状態の描画
		Novice::ScreenPrintf(20, 20, "WASD: Move / SPACE: Shot");
		Novice::ScreenPrintf(20, 40, "Player State: %s", player_->GetStateName());
	}

private:
	int whiteTextureHandle_ = 0;

	// 弾の変数
	Bullet bullet_;

	// 自機
	std::unique_ptr<Player> player_;

	// 敵の変数
	float enemyX_ = 640.0f;
	float enemyY_ = 150.0f;
	float enemyRadius_ = 25.0f;
	float enemySpeed_ = 4.0f;
	bool isEnemyAlive_ = true;
};

// クリアシーンクラス
class ClearScene : public IScene {
public:
	void Init() override {}

	void Update(const char* keys, const char* preKeys) override {
		// SPACEキーが押されたらタイトルシーンへ遷移
		if (preKeys[DIK_SPACE] == 0 && keys[DIK_SPACE] != 0) {
			sceneNo = TITLE;
		}
	}

	void Draw() override {
		// クリア画面の描画
		Novice::DrawBox(0, 0, 1280, 720, 0.0f, 0x14532DFF, kFillModeSolid);
		Novice::ScreenPrintf(580, 300, "GAME CLEAR!");
		Novice::ScreenPrintf(540, 360, "Press SPACE to Title");
	}
};

// ゲーム全体を管理するクラス
class GameManager {
private:
	// シーンを保持するメンバ変数
	std::unique_ptr<IScene> sceneArr_[3];

	// どのステージを呼び出すかを管理する変数
	int currentSceneNo_ = TITLE;
	int prevSceneNo_ = TITLE;

public:
	GameManager() {
		// 各シーンの配列を設定
		sceneArr_[TITLE] = std::make_unique<TitleScene>();
		sceneArr_[STAGE] = std::make_unique<StageScene>();
		sceneArr_[CLEAR] = std::make_unique<ClearScene>();

		// 初期シーンの設定と初期化
		currentSceneNo_ = TITLE;
		sceneArr_[currentSceneNo_]->Init();
	}

	// 更新処理
	void Update(const char* keys, const char* preKeys) {
		// シーンのチェック
		prevSceneNo_ = currentSceneNo_;
		currentSceneNo_ = sceneArr_[currentSceneNo_]->GetSceneNo();

		// シーン変更固有の初期化
		if (prevSceneNo_ != currentSceneNo_) {
			sceneArr_[currentSceneNo_]->Init();
		}

		// 現在のシーンの更新処理
		sceneArr_[currentSceneNo_]->Update(keys, preKeys);
	}

	// 描画処理
	void Draw() {
		// 現在のシーンの描画処理
		sceneArr_[currentSceneNo_]->Draw();
	}
};

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	// ゲームマネージャーの生成
	std::unique_ptr<GameManager> gameManager = std::make_unique<GameManager>();

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

		// ゲームマネージャーの更新処理
		gameManager->Update(keys, preKeys);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		// ゲームマネージャーの描画処理
		gameManager->Draw();

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