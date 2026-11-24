#include <Novice.h>
#include <iterator>
#include <list>

const char kWindowTitle[] = "学籍番号";

// 選択モード
enum SelectMode {
	SELECTOR,
	UNIT,
};

class Selector;

// ==================================================
// Unitクラス
// ==================================================
class Unit {
private:
	int mapX_ = 0;
	int mapY_ = 0;
	int mapSize_ = 32;
	int id_ = 0;
	bool isSelected_ = false;

public:
	Unit() = default;
	void Init(int idref);
	void Update();
	void Draw();

	void Move(int x, int y);
	void MoveEnd(Selector* selector);

	// Getter / Setter
	int GetMapX() const { return mapX_; }
	int GetMapY() const { return mapY_; }
	void SetMapPos(int x, int y) {
		mapX_ = x;
		mapY_ = y;
	}
	bool GetIsSelected() const { return isSelected_; }
	void SetIsSelected(bool isSelected) { isSelected_ = isSelected; }
};

// ==================================================
// Selectorクラス
// ==================================================
class Selector {
private:
	int mapX_ = 0;
	int mapY_ = 0;
	int mapSize_ = 32;

	int selectMode_ = SELECTOR;

	Unit* unit_ = nullptr;
	Unit* selectedUnit_ = nullptr;

public:
	static const int kUnitNum = 4;

	Selector();
	~Selector();

	void Init();
	void Update();
	void Draw();

	void Move(int x, int y);
	void SelectUnit();
	void ClearSelectedUnit();
	bool IsOverlapUnit(const Unit* targetUnit) const;

	// Getter / Setter
	int GetSelectMode() const { return selectMode_; }
	void SetSelectMode(int mode) { selectMode_ = mode; }
	Unit* GetSelectedUnitAddress() const { return selectedUnit_; }
	int GetMapX() const { return mapX_; }
	int GetMapY() const { return mapY_; }
	void SetMapPos(int x, int y) {
		mapX_ = x;
		mapY_ = y;
	}
};

// ==================================================
// コマンド基底クラス（インターフェース）
// ==================================================
class IStageSceneCommand {
public:
	virtual ~IStageSceneCommand() = default;
	virtual void Exec() = 0;
	virtual void Undo() {}
};

// ==================================================
// SelectorCommandクラス群
// ==================================================
class SelectorMoveCommand : public IStageSceneCommand {
public:
	SelectorMoveCommand(Selector* selector, int x, int y);
	void Exec() override;

private:
	Selector* selector_ = nullptr;
	int x_ = 0;
	int y_ = 0;
};

class SelectUnitCommand : public IStageSceneCommand {
public:
	SelectUnitCommand(Selector* selector);
	void Exec() override;

private:
	Selector* selector_ = nullptr;
};

// ==================================================
// UnitCommandクラス群
// ==================================================
class UnitMoveCommand : public IStageSceneCommand {
public:
	UnitMoveCommand(Unit* unit, int x, int y);
	void Exec() override;
	void Undo() override;

private:
	Unit* unit_ = nullptr;
	int x_ = 0;
	int y_ = 0;
};

class UnitMoveEndCommand : public IStageSceneCommand {
public:
	UnitMoveEndCommand(Unit* unit, Selector* selector);
	void Exec() override;

private:
	Unit* unit_ = nullptr;
	Selector* selector_ = nullptr;
};

// ==================================================
// StageSceneInputHandlerクラス
// ==================================================
class StageSceneInputHandler {
private:
	char keys_[256] = {0};
	char preKeys_[256] = {0};

	Selector* selector_ = nullptr;
	Unit* selectedUnit_ = nullptr;

public:
	StageSceneInputHandler(Selector* selector = nullptr);
	void UpdateKeyState();

	IStageSceneCommand* SelectorHandleInput(Selector* selector);
	IStageSceneCommand* UnitHandleInput(Unit* unit);

	bool CheckUndoStatus();
	bool IsTriggerEscape() const { return preKeys_[DIK_ESCAPE] == 0 && keys_[DIK_ESCAPE] != 0; }
};

// ==================================================
// StageSceneクラス
// ==================================================
class StageScene {
private:
	StageSceneInputHandler* inputHandler_ = nullptr;
	IStageSceneCommand* command_ = nullptr;
	std::list<IStageSceneCommand*> commandHistory_;
	std::list<IStageSceneCommand*>::iterator commandHistoryItr_;

	Selector* selector_ = nullptr;

	// コマンド履歴の全削除
	void ClearCommandHistory();

public:
	StageScene();
	~StageScene();

	void Init();
	void Update();
	void Draw();

	bool IsEnd() const;
};

// ==================================================
// Unitクラスの実装
// ==================================================
void Unit::Init(int idref) {
	id_ = idref;
	isSelected_ = false;
}

void Unit::Update() {}

void Unit::Draw() {
	// 選択中はピンク色、未選択時は白色で描画
	unsigned int color = isSelected_ ? 0xFF9999FF : WHITE;
	Novice::DrawBox(mapX_ * mapSize_ + 1, mapY_ * mapSize_ + 1, mapSize_ - 1, mapSize_ - 1, 0.0f, color, kFillModeSolid);
}

void Unit::Move(int x, int y) {
	mapX_ += x;
	mapY_ += y;

	// 画面外に出ないように制限
	if (mapX_ < 0) {
		mapX_ = 0;
	}
	if (mapX_ > 39) {
		mapX_ = 39;
	}
	if (mapY_ < 0) {
		mapY_ = 0;
	}
	if (mapY_ > 20) {
		mapY_ = 20;
	}
}

void Unit::MoveEnd(Selector* selector) {
	// 他の確定済みユニットと被っている場合は確定しない
	if (selector && selector->IsOverlapUnit(this)) {
		return;
	}

	isSelected_ = false;
	if (selector) {
		// セレクタの位置を移動後のユニットに合わせる
		selector->SetMapPos(mapX_, mapY_);
		selector->ClearSelectedUnit();
		selector->SetSelectMode(SELECTOR);
	}
}

// ==================================================
// Selectorクラスの実装
// ==================================================
Selector::Selector() { unit_ = new Unit[kUnitNum]; }

Selector::~Selector() { delete[] unit_; }

void Selector::Init() {
	mapX_ = 14;
	mapY_ = 14;
	selectMode_ = SELECTOR;
	selectedUnit_ = nullptr;

	for (int i = 0; i < kUnitNum; i++) {
		unit_[i].Init(i);
	}

	// 各ユニットの初期座標を設定
	unit_[0].SetMapPos(14, 14);
	unit_[1].SetMapPos(18, 4);
	unit_[2].SetMapPos(23, 6);
	unit_[3].SetMapPos(13, 17);
}

void Selector::Update() {
	for (int i = 0; i < kUnitNum; i++) {
		unit_[i].Update();
	}
}

void Selector::Draw() {
	// 確定済み（未選択）ユニットの描画
	for (int i = 0; i < kUnitNum; i++) {
		if (!unit_[i].GetIsSelected()) {
			unit_[i].Draw();
		}
	}

	// 未確定（選択中）ユニットの描画
	if (selectedUnit_) {
		selectedUnit_->Draw();
	}

	// セレクタモード中は赤枠を描画
	if (selectMode_ == SELECTOR) {
		Novice::DrawBox(mapX_ * mapSize_, mapY_ * mapSize_, mapSize_, mapSize_, 0.0f, RED, kFillModeWireFrame);
		Novice::DrawBox(mapX_ * mapSize_ + 1, mapY_ * mapSize_ + 1, mapSize_ - 2, mapSize_ - 2, 0.0f, RED, kFillModeWireFrame);
	}
}

void Selector::Move(int x, int y) {
	mapX_ += x;
	mapY_ += y;

	// 画面外に出ないように制限
	if (mapX_ < 0) {
		mapX_ = 0;
	}
	if (mapX_ > 39) {
		mapX_ = 39;
	}
	if (mapY_ < 0) {
		mapY_ = 0;
	}
	if (mapY_ > 20) {
		mapY_ = 20;
	}
}

void Selector::SelectUnit() {
	for (int i = 0; i < kUnitNum; i++) {
		if (unit_[i].GetMapX() == mapX_ && unit_[i].GetMapY() == mapY_) {
			selectedUnit_ = &unit_[i];
			selectedUnit_->SetIsSelected(true);
			selectMode_ = UNIT;
			break;
		}
	}
}

void Selector::ClearSelectedUnit() { selectedUnit_ = nullptr; }

bool Selector::IsOverlapUnit(const Unit* targetUnit) const {
	if (!targetUnit) {
		return false;
	}
	for (int i = 0; i < kUnitNum; i++) {
		if (&unit_[i] != targetUnit && unit_[i].GetMapX() == targetUnit->GetMapX() && unit_[i].GetMapY() == targetUnit->GetMapY()) {
			return true;
		}
	}
	return false;
}

// ==================================================
// SelectorCommandクラスの実装
// ==================================================
SelectorMoveCommand::SelectorMoveCommand(Selector* selector, int x, int y) : selector_(selector), x_(x), y_(y) {}

void SelectorMoveCommand::Exec() {
	if (selector_) {
		selector_->Move(x_, y_);
	}
}

SelectUnitCommand::SelectUnitCommand(Selector* selector) : selector_(selector) {}

void SelectUnitCommand::Exec() {
	if (selector_) {
		selector_->SelectUnit();
	}
}

// ==================================================
// UnitCommandクラスの実装
// ==================================================
UnitMoveCommand::UnitMoveCommand(Unit* unit, int x, int y) : unit_(unit), x_(x), y_(y) {}

void UnitMoveCommand::Exec() {
	if (unit_) {
		unit_->Move(x_, y_);
	}
}

void UnitMoveCommand::Undo() {
	if (unit_) {
		unit_->Move(-x_, -y_);
	}
}

UnitMoveEndCommand::UnitMoveEndCommand(Unit* unit, Selector* selector) : unit_(unit), selector_(selector) {}

void UnitMoveEndCommand::Exec() {
	if (unit_) {
		unit_->MoveEnd(selector_);
	}
}

// ==================================================
// StageSceneInputHandlerクラスの実装
// ==================================================
StageSceneInputHandler::StageSceneInputHandler(Selector* selector) : selector_(selector) {}

void StageSceneInputHandler::UpdateKeyState() {
	memcpy(preKeys_, keys_, 256);
	Novice::GetHitKeyStateAll(keys_);
}

IStageSceneCommand* StageSceneInputHandler::SelectorHandleInput(Selector* selector) {
	selector_ = selector;

	// SPACEキーでユニット選択
	if (preKeys_[DIK_SPACE] == 0 && keys_[DIK_SPACE] != 0) {
		return new SelectUnitCommand(selector);
	}

	// WASDまたは矢印キーでセレクタ移動
	if ((preKeys_[DIK_W] == 0 && keys_[DIK_W] != 0) || (preKeys_[DIK_UP] == 0 && keys_[DIK_UP] != 0)) {
		return new SelectorMoveCommand(selector, 0, -1);
	}
	if ((preKeys_[DIK_S] == 0 && keys_[DIK_S] != 0) || (preKeys_[DIK_DOWN] == 0 && keys_[DIK_DOWN] != 0)) {
		return new SelectorMoveCommand(selector, 0, 1);
	}
	if ((preKeys_[DIK_A] == 0 && keys_[DIK_A] != 0) || (preKeys_[DIK_LEFT] == 0 && keys_[DIK_LEFT] != 0)) {
		return new SelectorMoveCommand(selector, -1, 0);
	}
	if ((preKeys_[DIK_D] == 0 && keys_[DIK_D] != 0) || (preKeys_[DIK_RIGHT] == 0 && keys_[DIK_RIGHT] != 0)) {
		return new SelectorMoveCommand(selector, 1, 0);
	}

	return nullptr;
}

IStageSceneCommand* StageSceneInputHandler::UnitHandleInput(Unit* unit) {
	selectedUnit_ = unit;

	// SPACEキーでユニット移動終了（セレクタモードへ戻る）
	if (preKeys_[DIK_SPACE] == 0 && keys_[DIK_SPACE] != 0) {
		return new UnitMoveEndCommand(unit, selector_);
	}

	// WASDまたは矢印キーでユニット移動
	if ((preKeys_[DIK_W] == 0 && keys_[DIK_W] != 0) || (preKeys_[DIK_UP] == 0 && keys_[DIK_UP] != 0)) {
		return new UnitMoveCommand(unit, 0, -1);
	}
	if ((preKeys_[DIK_S] == 0 && keys_[DIK_S] != 0) || (preKeys_[DIK_DOWN] == 0 && keys_[DIK_DOWN] != 0)) {
		return new UnitMoveCommand(unit, 0, 1);
	}
	if ((preKeys_[DIK_A] == 0 && keys_[DIK_A] != 0) || (preKeys_[DIK_LEFT] == 0 && keys_[DIK_LEFT] != 0)) {
		return new UnitMoveCommand(unit, -1, 0);
	}
	if ((preKeys_[DIK_D] == 0 && keys_[DIK_D] != 0) || (preKeys_[DIK_RIGHT] == 0 && keys_[DIK_RIGHT] != 0)) {
		return new UnitMoveCommand(unit, 1, 0);
	}

	return nullptr;
}

bool StageSceneInputHandler::CheckUndoStatus() {
	// Ctrl + Z が押されたか判定
	bool isCtrlPressed = (keys_[DIK_LCONTROL] != 0 || keys_[DIK_RCONTROL] != 0);
	bool isZTriggered = (preKeys_[DIK_Z] == 0 && keys_[DIK_Z] != 0);
	return isCtrlPressed && isZTriggered;
}

// ==================================================
// StageSceneクラスの実装
// ==================================================
StageScene::StageScene() {
	selector_ = new Selector();
	inputHandler_ = new StageSceneInputHandler(selector_);
	commandHistoryItr_ = commandHistory_.end();
}

StageScene::~StageScene() {
	ClearCommandHistory();
	delete inputHandler_;
	delete selector_;
}

void StageScene::ClearCommandHistory() {
	for (auto* cmd : commandHistory_) {
		delete cmd;
	}
	commandHistory_.clear();
	commandHistoryItr_ = commandHistory_.end();
}

void StageScene::Init() {
	selector_->Init();
	ClearCommandHistory();
}

void StageScene::Update() {
	// 入力処理
	inputHandler_->UpdateKeyState();

	// モードに応じたコマンドを取得
	if (selector_->GetSelectMode() == SELECTOR) {
		command_ = inputHandler_->SelectorHandleInput(selector_);
	} else if (selector_->GetSelectMode() == UNIT) {
		// Undo入力のチェック
		if (inputHandler_->CheckUndoStatus()) {
			if (commandHistoryItr_ != commandHistory_.begin()) {
				commandHistoryItr_--;
				(*commandHistoryItr_)->Undo();
			}
			command_ = nullptr;
			return;
		}
		command_ = inputHandler_->UnitHandleInput(selector_->GetSelectedUnitAddress());
	}

	// コマンドの実行と履歴管理
	if (command_) {
		command_->Exec();

		// ユニット移動コマンドのみUndo履歴に保存（無上限）
		if (dynamic_cast<UnitMoveCommand*>(command_) != nullptr) {
			// Undo後に新しい移動を行った場合、それ以降の履歴を削除
			while (commandHistoryItr_ != commandHistory_.end()) {
				delete *commandHistoryItr_;
				commandHistoryItr_ = commandHistory_.erase(commandHistoryItr_);
			}

			commandHistory_.push_back(command_);
			commandHistoryItr_ = commandHistory_.end();
		} else {
			// 選択開始や移動終了時は履歴をリセット
			if (dynamic_cast<UnitMoveEndCommand*>(command_) != nullptr && selector_->GetSelectMode() == SELECTOR) {
				ClearCommandHistory();
			}
			delete command_;
		}
		command_ = nullptr;
	}

	selector_->Update();
}

void StageScene::Draw() {
	// 背景の描画（青色）
	Novice::DrawBox(0, 0, 1280, 720, 0.0f, 0x4A7EC0FF, kFillModeSolid);

	// グリッド線の描画（32x32マス）
	const int kGridSize = 32;
	for (int x = 0; x <= 1280; x += kGridSize) {
		Novice::DrawLine(x, 0, x, 720, WHITE);
	}
	for (int y = 0; y <= 720; y += kGridSize) {
		Novice::DrawLine(0, y, 1280, y, WHITE);
	}

	// セレクタとユニットの描画
	selector_->Draw();

	// 文字背景の黒いバーの描画
	Novice::DrawBox(0, 660, 1280, 60, 0.0f, BLACK, kFillModeSolid);

	// モードごとの操作説明とUndo回数の描画
	if (selector_->GetSelectMode() == SELECTOR) {
		Novice::ScreenPrintf(20, 670, "WASD||arrow keys: move / space key: change unit mode");
		Novice::ScreenPrintf(20, 690, "In Selector Mode, you cannot use the 'Undo' action.");
	} else {
		int undoCount = static_cast<int>(std::distance(commandHistory_.begin(), commandHistoryItr_));
		Novice::ScreenPrintf(20, 670, "WASD||arrow keys: move / space key: change selector mode / ctrl+z: undo");
		Novice::ScreenPrintf(20, 690, "You have %d more 'UnDo' actions available.", undoCount);
	}
}

bool StageScene::IsEnd() const { return inputHandler_->IsTriggerEscape(); }

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// ステージシーンの生成と初期化
	StageScene* stageScene = new StageScene();
	stageScene->Init();

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		///
		/// ↓更新処理ここから
		///

		stageScene->Update();

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		stageScene->Draw();

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (stageScene->IsEnd()) {
			break;
		}
	}

	// 解放処理
	delete stageScene;

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}