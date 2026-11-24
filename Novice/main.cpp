#include <Novice.h>
#include <cmath>

const char kWindowTitle[] = "学籍番号";

// 弾の構造体
struct Bullet {
	float x = 0.0f;
	float y = 0.0f;
	float radius = 8.0f;
	float speed = 12.0f;
	bool isShot = false;
};

// 自機クラス
class Player {
private:
	float x_ = 640.0f;
	float y_ = 600.0f;
	float radius_ = 20.0f;
	float speed_ = 6.0f;
	Bullet* bullet_ = nullptr;

public:
	Player(Bullet* bullet);

	void Init();
	void Update();
	void Draw(int textureHandle);

	// メンバ関数（コマンド経由で呼び出す）
	void MoveUp();
	void MoveDown();
	void MoveLeft();
	void MoveRight();
	void Shoot();
};

// コマンドの基底クラス（インターフェース）
class ICommand {
public:
	virtual ~ICommand() = default;
	virtual void Exec(Player& player) = 0;
};

// 上移動コマンドクラス
class MoveUpCommand : public ICommand {
public:
	void Exec(Player& player) override;
};

// 下移動コマンドクラス
class MoveDownCommand : public ICommand {
public:
	void Exec(Player& player) override;
};

// 左移動コマンドクラス
class MoveLeftCommand : public ICommand {
public:
	void Exec(Player& player) override;
};

// 右移動コマンドクラス
class MoveRightCommand : public ICommand {
public:
	void Exec(Player& player) override;
};

// 弾発射コマンドクラス
class ShootCommand : public ICommand {
public:
	void Exec(Player& player) override;
};

// 入力を取りまとめるクラス
class InputHandler {
public:
	~InputHandler();

	ICommand* HandleInput();

	// キーへのコマンド割り当て
	void AssignMoveUpCommand2PressKeyW();
	void AssignMoveDownCommand2PressKeyS();
	void AssignMoveLeftCommand2PressKeyA();
	void AssignMoveRightCommand2PressKeyD();
	void AssignShootCommand2PressKeySpace();

private:
	ICommand* pressKeyW_ = nullptr;
	ICommand* pressKeyS_ = nullptr;
	ICommand* pressKeyA_ = nullptr;
	ICommand* pressKeyD_ = nullptr;
	ICommand* pressKeySpace_ = nullptr;
};

// ==================================================
// Playerクラスの実装
// ==================================================

Player::Player(Bullet* bullet) : bullet_(bullet) {}

void Player::Init() {
	x_ = 640.0f;
	y_ = 600.0f;
}

void Player::Update() {
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

void Player::Draw(int textureHandle) {
	// 自機の描画（白色）
	Novice::DrawSprite(static_cast<int>(x_ - radius_), static_cast<int>(y_ - radius_), textureHandle, radius_ * 2.0f, radius_ * 2.0f, 0.0f, WHITE);
}

void Player::MoveUp() { y_ -= speed_; }

void Player::MoveDown() { y_ += speed_; }

void Player::MoveLeft() { x_ -= speed_; }

void Player::MoveRight() { x_ += speed_; }

void Player::Shoot() {
	if (bullet_ && !bullet_->isShot) {
		bullet_->isShot = true;
		bullet_->x = x_;
		bullet_->y = y_;
	}
}

// ==================================================
// Commandクラスの実装
// ==================================================

void MoveUpCommand::Exec(Player& player) { player.MoveUp(); }

void MoveDownCommand::Exec(Player& player) { player.MoveDown(); }

void MoveLeftCommand::Exec(Player& player) { player.MoveLeft(); }

void MoveRightCommand::Exec(Player& player) { player.MoveRight(); }

void ShootCommand::Exec(Player& player) { player.Shoot(); }

// ==================================================
// InputHandlerクラスの実装
// ==================================================

InputHandler::~InputHandler() {
	delete pressKeyW_;
	delete pressKeyS_;
	delete pressKeyA_;
	delete pressKeyD_;
	delete pressKeySpace_;
}

void InputHandler::AssignMoveUpCommand2PressKeyW() {
	ICommand* command = new MoveUpCommand();
	pressKeyW_ = command;
}

void InputHandler::AssignMoveDownCommand2PressKeyS() {
	ICommand* command = new MoveDownCommand();
	pressKeyS_ = command;
}

void InputHandler::AssignMoveLeftCommand2PressKeyA() {
	ICommand* command = new MoveLeftCommand();
	pressKeyA_ = command;
}

void InputHandler::AssignMoveRightCommand2PressKeyD() {
	ICommand* command = new MoveRightCommand();
	pressKeyD_ = command;
}

void InputHandler::AssignShootCommand2PressKeySpace() {
	ICommand* command = new ShootCommand();
	pressKeySpace_ = command;
}

ICommand* InputHandler::HandleInput() {
	if (Novice::CheckHitKey(DIK_SPACE)) {
		return pressKeySpace_;
	}
	if (Novice::CheckHitKey(DIK_W)) {
		return pressKeyW_;
	}
	if (Novice::CheckHitKey(DIK_S)) {
		return pressKeyS_;
	}
	if (Novice::CheckHitKey(DIK_A)) {
		return pressKeyA_;
	}
	if (Novice::CheckHitKey(DIK_D)) {
		return pressKeyD_;
	}

	return nullptr;
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

	// 入力ハンドラの生成とコマンドの割り当て
	InputHandler* inputHandler = new InputHandler();
	inputHandler->AssignMoveUpCommand2PressKeyW();
	inputHandler->AssignMoveDownCommand2PressKeyS();
	inputHandler->AssignMoveLeftCommand2PressKeyA();
	inputHandler->AssignMoveRightCommand2PressKeyD();
	inputHandler->AssignShootCommand2PressKeySpace();

	// 実行するコマンドのポインタ
	ICommand* iCommand = nullptr;

	// 自機の生成と初期化
	Player* player = new Player(&bullet);
	player->Init();

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

		// 入力からコマンドを取得
		iCommand = inputHandler->HandleInput();

		// コマンドがある場合は対象コマンドを実行
		if (iCommand) {
			iCommand->Exec(*player);
		}

		// 自機の更新処理
		player->Update();

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

		// 自機の描画（白色）
		player->Draw(whiteTextureHandle);

		// 操作説明の描画
		Novice::ScreenPrintf(20, 20, "WASD: Move / SPACE: Shot / R: Respawn Enemy");

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

	// 解放処理
	delete player;
	delete inputHandler;

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}