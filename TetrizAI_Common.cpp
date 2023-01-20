#include "TetrizAI_Common.h"

#include <cstring>
#include <algorithm>

#include "distribution.h"

int NextMino::viewNext = VIEW_NEXT;

void NextMino::Init(const MinoType *nx) {
	viewNext = ConfigAI::ViewNext();

	auto remainder = [](const int n) {
		int r = 7;
		while (r <= n) {
			r += 7;
		}
		return r - n;
	};

	this->predictRemains = remainder(viewNext);
	this->predictRange = this->predictRemains + viewNext;
	this->visibleRange = viewNext;
	if (this->predictRemains == 1) {
		this->visibleRange += 1;
	}

	this->bag.Reset();
	for (int i = 0; i < NextMino::SIZE; ++i) {
		this->m[i] = this->bag.Pick();
	}

	Compare(0, nx);
};
void NextMino::ClearArray() {
	for (int i = 0; i < NextMino::SIZE; ++i) {
		this->m[i] = MinoType::None;
	}
}
void NextMino::Compare(const int cmpFirst, const MinoType *nx) {
	for (int i = 0; i < ViewNext(); ++i) {
		if (this->m[i + cmpFirst] != nx[i]) {
			Swap(i + cmpFirst, nx[i]);
		}
	}
	auto trustRange = [this]() {
		switch (this->bag.Remains()) {
		case 7:
			return 2;
		case 6:
			return 1;
		case 5:
			return 0;
		case 4:
			return 0;
		case 3:
			return 0;
		case 2:
			return 0;
		case 1:
			return 3;
		default:
			return 0;
		}
	};
	this->trustEnd = (ViewNext() - 1) + trustRange();
}
void NextMino::Swap(const int cmpedIndex, const MinoType nx) {
	for (int i = cmpedIndex + 1; i < NextMino::SIZE; ++i) {
		//if (nx == this->m[i]) {
		//	std::swap(this->m[cmpedIndex], this->m[i]);
		//	return;
		//}
		if (nx == this->m[i]) {
			for (; i > cmpedIndex; --i) {
				this->m[i] = this->m[i - 1];
			}
			this->m[cmpedIndex] = nx;
			return;
		}
	}
	this->m[cmpedIndex] = nx;
}
void NextMino::Advance() {
	for (int i = 0; i < NextMino::SIZE - 1; ++i) {
		this->m[i] = this->m[i + 1];
	}
	this->m[NextMino::SIZE - 1] = this->bag.Pick();

	this->predictRemains -= 1;
	if (this->predictRemains <= 0) {
		this->predictRemains = 7;
	}

	this->predictRange = this->predictRemains + viewNext;
	this->visibleRange = viewNext;

	if (this->predictRemains == 1) {
		this->visibleRange += 1;
	}
}
bool NextMino::IsOriginNext(bool includeHold) {
	constexpr auto openingRemains = []() {
		int r = 7;
		for (int i = 0; i < NextMino::SIZE; ++i) {
			if (--r <= 0) r = 7;
		}
		return r;
	};
	constexpr int REMAINS = openingRemains();
	if (includeHold) {
		return this->bag.Remains() == (REMAINS > 1 ? REMAINS - 1 : 7);
	}
	return this->bag.Remains() == REMAINS;
}
void NextMino::OpeningPredictionSwap() {
	// next5なら、std::swap([5],[6]);
	std::swap(this->m[ViewNext()], this->m[ViewNext() + 1]);
}
bool NextMino::OpeningPredictionHit(const MinoType *nx) {
	// next5なら、nx[4]とthis->m[5]を比較
	return nx[ViewNext() - 1] == this->m[ViewNext()];
}
int NextMino::ViewNext() {
	return viewNext;
}
void NextMino::ViewNext(int set) {
	viewNext = ConfigAI::ViewNext();
}

void NextMino::Bag::Reset() {
	this->remains = 7;
}
MinoType NextMino::Bag::Pick() {
	auto r = this->remains;

	this->remains -= 1;
	if (this->remains <= 0) this->Reset();

	switch (r) {
	case 7:
		return MinoType::Z;
	case 6:
		return MinoType::S;
	case 5:
		return MinoType::T;
	case 4:
		return MinoType::O;
	case 3:
		return MinoType::J;
	case 2:
		return MinoType::L;
	case 1:
		return MinoType::I;
	default:
		throw;
		return MinoType::None;
	}
}
int NextMino::Bag::Remains() const {
	return this->remains;
}

const MinoType NextIterator::Get() const {
#ifndef OUR
	return this->m[0];
#else
	if (this->myIndex < NextMino::SIZE) {
		return this->ourNext->m[this->myIndex];
	}
	else {
		throw;
		return MinoType::None;
	}
#endif
}
const MinoType NextIterator::Get(int i) const {
#ifndef OUR
	return this->m[i];
#else
	if (this->myIndex + i < NextMino::SIZE) {
		return this->ourNext->m[this->myIndex + i];
	}
	else {
		return MinoType::None;
	}
#endif
}
void NextIterator::Advance() {
#ifndef OUR
	++this->m;
#else
	this->myIndex += 1;
#endif
}
void NextIterator::SetIterator(const NextMino *nx) {
#ifndef OUR
	this->m = nx->m;
	this->trustEnd = nx->m + nx->trustEnd;
#else
	this->ourNext = nx;
	this->myIndex = 0;
#endif
}
bool NextIterator::IsTrustNext(const int i, const int trustRange) const {
#ifndef OUR
	return this->m + i <= this->trustEnd;
#else
	auto end = this->ourNext->predictRange;
	if (this->ourNext->predictRemains > trustRange) {
		end -= this->ourNext->predictRemains;
	}
	bool b = this->myIndex + i < end;

	return b;
#endif
}
bool NextIterator::IsVisibleNext(const int i, const int trustRange) const {
#ifndef OUR
	return true;
#else
	// if (i == 0) throw;

	// nx5 なら 5 or 6
	auto end = this->ourNext->visibleRange;
	// 1手目スタートなら index - i = 1 - 1 = 0;
	auto first = this->myIndex - i;

	return first < end;
#endif
}

void Command::Init() {
	memset(this->s, 0, sizeof(this->s));
	this->moveDelay = 0;
	this->len = 0;
}
void Command::AddCommands(const char *cmds) {
	memcpy(this->s + this->len, cmds, sizeof(this->s[0]) * 7);
	this->len += static_cast<int>(strlen(cmds));
}
void Command::AddCommand(const char cmd) {
	this->s[this->len] = cmd;
	this->s[this->len + 1] = '\0';
	++this->len;
}
void Command::AddCommandSoftDrop(const char cmd, const int y) {
	this->s[this->len] = cmd;
	this->s[this->len + 1] = 'A' + static_cast<char>(y);
	this->s[this->len + 2] = '\0';
	this->len += 2;
}
int Command::HardDrop(FallMino *mino, const Board *bd) {
	AddCommand('u');
	this->moveDelay += 116;

	int dropCount = bd->CollisionDistance(mino);
	if (dropCount > 0) {
		mino->p.y -= dropCount;
		mino->lastSrs = 0;
	}

	return 1;
}
int Command::SoftDrop(FallMino *mino, const Board *bd) {

	int dropCount = bd->CollisionDistance(mino);

	if (dropCount == 0) return 0;

	mino->p.y -= dropCount;
	mino->lastSrs = 0;
	AddCommandSoftDrop('d', mino->p.y);
	this->moveDelay += dropCount * 33;

	return 1;
}
int Command::SoftDropOnce(FallMino *mino, const Board *bd) {
	if (bd->CollisionBottom(mino) == 1) return 0;

	--mino->p.y;
	mino->lastSrs = 0;
	AddCommandSoftDrop('d', mino->p.y);
	this->moveDelay += 33;

	return 1;
}
int Command::MoveLeft(FallMino *mino, const Board *bd) {
	if (bd->CollisionLeft(mino) == 1) return 0;

	--mino->p.x;
	mino->lastSrs = 0;
	AddCommand('l');
	this->moveDelay += 30;

	if (mino->bugGravity == 1) {
		if (bd->CollisionBottom(mino) == 0) {
			--mino->p.y;
			mino->bugGravity = 0;
			AddCommandSoftDrop('g', mino->p.y);
		}
	}

	return 1;
}
int Command::MoveRight(FallMino *mino, const Board *bd) {
	if (bd->CollisionRight(mino) == 1) return 0;

	++mino->p.x;
	mino->lastSrs = 0;
	AddCommand('r');
	this->moveDelay += 30;

	if (mino->bugGravity == 1) {
		if (bd->CollisionBottom(mino) == 0) {
			--mino->p.y;
			mino->bugGravity = 0;
			AddCommandSoftDrop('g', mino->p.y);
		}
	}

	return 1;
}
int Command::MoveLeftMost(FallMino *mino, const Board *bd) {
	auto x = 0;
	for (;;) {
		if (bd->CollisionLeft(mino) == 1) break;
		mino->p.x -= 1;
		x += 1;
	}
	mino->p.x += x;

	if (x == 0) return 0;
	mino->p.x -= x;
	mino->lastSrs = 0;
	for (auto i = x; i > 0; --i) {
		AddCommand('l');
		this->moveDelay += 30;
	}

	return x;
}
int Command::MoveRightMost(FallMino *mino, const Board *bd) {
	auto x = 0;
	for (;;) {
		if (bd->CollisionRight(mino) == 1) break;
		mino->p.x += 1;
		x += 1;
	}
	mino->p.x -= x;

	if (x == 0) return 0;
	mino->p.x += x;
	mino->lastSrs = 0;
	for (auto i = x; i > 0; --i) {
		AddCommand('r');
		this->moveDelay += 30;
	}

	return x;
}
int Command::RotateA(FallMino *mino, const Board *bd) {
	if (mino->m == MinoType::O) return 0;

	int srsPattern = bd->SrsA(mino);
	if (srsPattern == 0) return 0;
	AddCommand('a');
	this->moveDelay += 16;
	mino->lastSrs = srsPattern;

	if (mino->bugGravity == 1) {
		if (bd->CollisionBottom(mino) == 0) {
			--mino->p.y;
			mino->bugGravity = 0;
			AddCommandSoftDrop('g', mino->p.y);
		}
	}

	return 1;
}
int Command::RotateB(FallMino *mino, const Board *bd) {
	if (mino->m == MinoType::O) return 0;

	int srsPattern = bd->SrsB(mino);
	if (srsPattern == 0) return 0;
	AddCommand('b');
	this->moveDelay += 16;
	mino->lastSrs = srsPattern;

	if (mino->bugGravity == 1) {
		if (bd->CollisionBottom(mino) == 0) {
			--mino->p.y;
			mino->bugGravity = 0;
			AddCommandSoftDrop('g', mino->p.y);
		}
	}

	return 1;
}
int Command::SwapHold(FallMino *mino, HoldMino *hld, const Board *bd, NextIterator *nxIt) {
	// この処理はなくてもいい
	if (hld->swappedHold == 1) return 0;

	// 20段目以外でhold回収した場合（*）、次にhold出現したときは必ず21段目に出現する
	// *厳密には違うが、出現したミノを動かす前にholdすれば問題ない
	int h = hld->abnormalHold;
	hld->abnormalHold = (mino->p.y == FallMino::SPAWN_Y) ? 0 : 1;

	if (hld->m != MinoType::None) {
		mino->m = hld->m;
		hld->m = nxIt->Get();

		AddCommand('h');
		this->moveDelay += 16;
	}
	else {
		hld->m = mino->m;
		nxIt->Advance();
		mino->m = nxIt->Get();

		AddCommand('f');
		//this->moveDelay += 133;
		this->moveDelay += 16;
	}
	mino->p.x = FallMino::SPAWN_X;
	mino->r = 0;
	mino->UpdateShape();

	int spawnHeight = bd->GetSpawnHeight(mino);
	// 通常hold
	if (h == 0) {
		// 20段目に出現
		if (spawnHeight == FallMino::SPAWN_Y) {
			mino->p.y = FallMino::SPAWN_Y;
		}
		// 21段目に出現 or 出現失敗
		else {
			// 必ず21段目に出現
			mino->p.y = FallMino::SPAWN_Y + 1;
			// 座標ずれバグのフラグが立つ
			mino->bugGravity = 1;
		}
	}
	else {
		// 必ず21段目に出現
		mino->p.y = FallMino::SPAWN_Y + 1;
	}
	mino->lastSrs = 0;

	hld->swappedHold = 1;
	if (HoldMino::ForPPT1()) {
		// do nothing;
	}
	else {
		// ppt2では地形にめり込んだミノが横移動で脱出できないため、
		// hold次いでに回転させておく（= Oミノは窒息）
		if (spawnHeight == -1) {
			if (RotateB(mino, bd) == 0) {
				return RotateA(mino, bd);
			}
		}
	}

	return 1;
}
bool Command::IsFirstHold() {
	return this->s[0] == 'f';
}
void Command::CorrectDelay() {
	constexpr double F = 1000.0 / 60.0;

	double delay = 0.0;
	double y = 20.0 + 'A';

	for (int i = 0; i < this->len; ++i) {
		switch (this->s[i]) {
		case 'l':
		case 'r':
			delay += F * 1.0;
			switch (this->s[i + 1]) {
			case 'l':
			case 'r':
				delay += F * 1.0;
				break;
			}
			break;
		case 'b':
		case 'a':
			delay += F * 1.0;
			switch (this->s[i + 1]) {
			case 'b':
			case 'a':
				delay += F * 1.0;
				break;
			case 'u':
				delay += F * 6.0;
				this->moveDelay = (int)delay;
				return;
			}
			break;
		case 'd':
			++i;
			delay += F * 2.0 * (y - this->s[i]);
			y = this->s[i];
			break;
		case 'g':
			++i;
			break;
		case 'u':
			delay += F * 7.0;
			this->moveDelay = (int)delay;
			return;
			break;
		case 'h':
			delay += F * 1.0;
			break;
		case 'f':
			delay += F * (1.0 + 7.0);
			break;
		}
	}
}


int GameState::LockDown() {
	this->commands.HardDrop(&this->fallingMino, &this->board);

	int clearedLine = this->board.LockMino(&this->fallingMino);
	if (clearedLine == -1) return 0;

	this->others.CalcAtk(&this->fallingMino, &this->board, clearedLine);

	if (clearedLine > 0) this->board.ClearLine();

	this->hold.swappedHold = 0;

	this->lockedMino = this->fallingMino;

	this->nextIt.Advance();

	return this->fallingMino.Spawn(this->nextIt.Get(), &this->board);
}
bool GameState::IsSafe() const {
	return this->board.stackHeight < this->fallingMino.p.y - 3;
}

void Plans::Clear() {
	for (int i = 0; i < MAX_DEPTH; ++i) {
		this->mino[i].m = MinoType::None;
		this->act[i] = ClearAct::None;
	}
	this->drawingBreak = MAX_DEPTH;
}

void SearchResult::Clear() {
	this->overTime = 0.0;
	this->searchTime = 0.0;
	this->pptTime = 0.0;
	this->node = 0;
	this->depth = 0;
	this->ren = 0;
	this->correctedBeamSize = 0;
	this->plans.Clear();
}

void RootNode::Init(const MinoType *nx) {
	this->game.board.Init();
	this->game.hold.Init();
	this->next.Init(nx);
	AdvanceMino();
	this->game.commands.Init();
	this->game.others.Init();
	this->gameover = 0;
	this->result.Clear();
}

void RootNode::GameOver() {
	this->game.commands.s[0] = '\0';
	this->game.lockedMino.m = MinoType::None;
	this->gameover = 1;
	this->result.Clear();
}

bool RootNode::IsSameAs(Mimic *mimic) {
	return this->game.board.CompareTo(&mimic->board) == 0 && this->game.hold.m == mimic->hold;
}
void RootNode::SetPPTData(Mimic *mimic) {
	this->next.Advance();

	this->next.Compare(0, mimic->next.m);

	// 初回ホールドならもう一度シフト
	if (this->game.commands.IsFirstHold()) {
		this->next.Advance();
	}

	AdvanceMino();
}
void RootNode::ResetPPTData(Mimic *mimic) {
	// combo btb をそろえる
	if (mimic->btb == 0) {
		this->game.others.btb = 0;
	}
	else {
		if (mimic->combo <= 0) {
			if (this->game.others.combo > 0 && this->game.others.btb > 0) this->game.others.btb -= 1;
		}
	}
	this->game.others.combo = mimic->combo;

	// 高さの更新忘れずに
	this->game.board = mimic->board;
	this->game.board.UpdateStackHeight();

	this->next.Advance();

	// 初回ホールドしたつもり
	if (this->game.commands.IsFirstHold()) {
		// キー抜け
		if (mimic->hold == MinoType::None) {
			this->game.hold.m = MinoType::None;

			mimic->missPattern = 1;
		}
		// 置きミス
		else {
			this->next.Advance();

			mimic->missPattern = 2;
		}
	}
	// 間違えて初回ホールドした
	else if (this->game.hold.m == MinoType::None && mimic->hold != MinoType::None) {
		this->game.hold.m = mimic->hold;

		this->next.Advance();

		mimic->missPattern = 3;
	}
	// その他全対応
	else {
		this->game.hold.m = mimic->hold;

		mimic->missPattern = 4;
	}

	// 比較位置がずれる
	this->next.Compare(1, mimic->next.m);

	AdvanceMino();
}

void RootNode::AdvanceMino() {
	this->game.nextIt.SetIterator(&this->next);
	this->game.fallingMino.Spawn(this->game.nextIt.Get(), &this->game.board);
}

int RootNode::GetClearDelay() {
	switch (this->game.others.act) {
	case ClearAct::None:
		return 0;
	case ClearAct::Single:
	case ClearAct::Tss:
	case ClearAct::Tsm:
		return 36;
	case ClearAct::Double:
	case ClearAct::Triple:
	case ClearAct::Tsd:
	case ClearAct::Tst:
		return 41;
	case ClearAct::Quad:
		return 46;
	case ClearAct::Pc:
		return 1;
	}
	return 0;
}

double RootNode::GetDelay() {
	constexpr double F = 1000.0 / 60.0;
	return GetClearDelay() * F + this->game.commands.moveDelay;
}

#include <fstream>
#include <sstream>
#include <string>

#include "Hardware.h"

void ConfigAI::ReadConfig() {
	physicalCore = HardwareInfo::Physical();
	if (physicalCore <= 0) {
		physicalCore = std::max(HardwareInfo::Logical(), 1);
	}

	SetDefaultConfig();

	std::string path = "config.dll";
	std::ifstream ifs;
	ifs.open(path);

	if (ifs.fail()) {
		ifs.close();
		SetDefaultConfig();
		return;
	}

	std::string rowBuf;
	std::string columnBuf;
	constexpr int CONFIG_NUM = 16;
	int configArray[CONFIG_NUM] = {};
	auto streamtoi = [](const std::string &buf) {
		if (buf == "true") return 1;
		if (buf == "false") return 0;
		return std::stoi(buf);
	};

	int correct = 0;

	try {
		for (int i = 0; std::getline(ifs, rowBuf); ++i) {
			std::istringstream ist(rowBuf);
			for (int column = 1; std::getline(ist, columnBuf, ','); ++column) {
				if (column == 1) continue;
				configArray[i] = streamtoi(columnBuf);
				if (column >= 2) {
					correct += 1;
					break;
				}
			}
		}
	}
	catch (...) {
		ifs.close();
		SetDefaultConfig();
		return;
	}

	if (correct != CONFIG_NUM) {
		ifs.close();
		SetDefaultConfig();
		return;
	}

	int *cnfIt = configArray;
	--cnfIt;
	PlayerNo(*++cnfIt);
	ViewNext(*++cnfIt);
	UseHold(*++cnfIt);
	BeamSize(*++cnfIt);
	UseTDOpeners(*++cnfIt);
	Speed(*++cnfIt);
	AutoCharSelect(*++cnfIt);
	PlayStyle(*++cnfIt);
	UseTimingOffset(*++cnfIt);
	UsePCstack(*++cnfIt);
	UseS4W(*++cnfIt);
	LoopTemplate(*++cnfIt);
	ThreadCount(*++cnfIt);
	LowerDepthLimit(*++cnfIt);
	Lifespan(*++cnfIt);
	TakeOverTime(*++cnfIt);

	ifs.close();
}
void ConfigAI::WriteConfig() {
	std::string path = "config.dll";
	std::ofstream ofs;
	ofs.open(path);

	auto streamBool = [](bool b) {
		if (b == true) return "true";
		return "false";
	};

	ofs << "player," << PlayerNo() << std::endl;
	ofs << "viewNext," << ViewNext() << std::endl;
	ofs << "useHold," << streamBool(UseHold()) << std::endl;
	ofs << "beamSize," << BeamSize() << std::endl;
	ofs << "useTDOpeners," << streamBool(UseTDOpeners()) << std::endl;
	ofs << "speed," << Speed_Cast() << std::endl;
	ofs << "autoSelect," << streamBool(AutoCharSelect()) << std::endl;
	ofs << "playStyle," << PlayStyle_Cast() << std::endl;
	ofs << "useTimingOffset," << streamBool(UseTimingOffset()) << std::endl;
	ofs << "usePCstack," << streamBool(UsePCstack()) << std::endl;
	ofs << "useS4W," << streamBool(UseS4W()) << std::endl;
	ofs << "loopTemplate," << streamBool(LoopTemplate()) << std::endl;
	ofs << "threadCount," << ThreadCount() << std::endl;
	ofs << "lowerDepthLimit," << LowerDepthLimit() << std::endl;
	ofs << "lifespan," << Lifespan_Cast() << std::endl;
	ofs << "takeOverTime," << TakeOverTime()/* << std::endl*/;

	ofs.close();
}
void ConfigAI::SetDefaultConfig() {
	PlayerNo(0);
	ViewNext(5);
	UseHold(true);
	BeamSize(300);
	UseTDOpeners(false);
	Speed(SpeedLevel::S100);
	AutoCharSelect(true);
	PlayStyle(PlayStyleList::Vs);
	UseTimingOffset(true);
	UsePCstack(false);
	UseS4W(false);
	ThreadCount(1);
	LowerDepthLimit(10);
	Lifespan(LifespanLevel::Inf);
	TakeOverTime(5.0);
	ResetMyCharacter();
}

void ConfigAI::PlayerNo(int set) {
	playerNo = std::clamp(set, 0, 3);
}
int ConfigAI::PlayerNo() {
	return playerNo;
}
void ConfigAI::ViewNext(int set) {
	constexpr int MAX_NEXT = DELETE_PC_RELATED == 1 ? 5 : 12;
	viewNext = std::clamp(set, 3, MAX_NEXT);
}
int ConfigAI::ViewNext() {
	return viewNext;
}
void ConfigAI::UseHold(bool set) {
	return;
	useHold = set;
}
bool ConfigAI::UseHold() {
	return true;
	return useHold;
}
void ConfigAI::BeamSize(int set) {
	beamSize = std::clamp(set, BEAMSIZE_MIN, BEAMSIZE_MAX_PERTHREAD);
}
int ConfigAI::BeamSize() {
	return beamSize;
}
void ConfigAI::UseTDOpeners(int set) {
	useTDOpeners = set;
}
bool ConfigAI::UseTDOpeners() {
	return (ViewNext() >= 5 && UseHold()) && useTDOpeners;
}
void ConfigAI::Speed(int set) {
	Speed(static_cast<SpeedLevel>(set));
}
void ConfigAI::Speed(SpeedLevel set) {
	switch (set) {
	case SpeedLevel::S100:
	case SpeedLevel::S50:
	case SpeedLevel::S33:
	case SpeedLevel::S25:
		speed = set; return;
	}
	speed = SpeedLevel::S100;
}
int ConfigAI::Speed_Cast() {
	return static_cast<int>(speed);
}
SpeedLevel ConfigAI::Speed() {
	return speed;
}
void ConfigAI::AutoCharSelect(bool set) {
	autoCharSelect = set;
}
bool ConfigAI::AutoCharSelect() {
	return autoCharSelect;
}
void ConfigAI::PlayStyle(int set) {
	PlayStyle(static_cast<PlayStyleList>(set));
}
void ConfigAI::PlayStyle(PlayStyleList set) {
	switch (set) {
	case PlayStyleList::Vs:
	case PlayStyleList::Ultra:
	case PlayStyleList::Sprint:
		playStyle = set; return;
	}
	playStyle = PlayStyleList::Vs;
}
int ConfigAI::PlayStyle_Cast() {
	return static_cast<int>(playStyle);
}
PlayStyleList ConfigAI::PlayStyle() {

#if DISTRIBUTION
	return PlayStyleList::Vs;
#endif

	return playStyle;
}
void ConfigAI::UseTimingOffset(bool set) {
	useTimingOffset = set;
}
bool ConfigAI::UseTimingOffset() {
	return useTimingOffset;
}
void ConfigAI::UsePCstack(bool set) {
	usePCstack = set;
}
bool ConfigAI::UsePCstack() {

#if DELETE_PC_RELATED
	return false;
#endif

	return usePCstack;
}
void ConfigAI::UseS4W(bool set) {
	useS4W = set;
}
bool ConfigAI::UseS4W() {
	return useS4W;
}
void ConfigAI::UseDPC(bool set) {
	useDPC = set;
}
bool ConfigAI::UseDPC() {
	return (ViewNext() >= 5 && UseHold()) && useDPC;
}
void ConfigAI::LoopTemplate(bool set) {
	loopTemplate = set;
}
bool ConfigAI::LoopTemplate() {
	return (UseTDOpeners() || UseDPC()) && loopTemplate;
}
void ConfigAI::ThreadCount(int set) {
	threadCount = std::clamp(set, 1, PhysicalCore());
}
int ConfigAI::ThreadCount() {
	return threadCount;
}
void ConfigAI::LowerDepthLimit(int set) {
	lowerDepthLimit = std::clamp(set, 0, MAX_DEPTH);
}
int ConfigAI::LowerDepthLimit() {

#if DISTRIBUTION
	return 10;
#endif

	return lowerDepthLimit;
}
void ConfigAI::Lifespan(int set) {
	Lifespan(static_cast<LifespanLevel>(set));
}
void ConfigAI::Lifespan(LifespanLevel set) {
	switch (set) {
	case LifespanLevel::M1:
	case LifespanLevel::M3:
	case LifespanLevel::M5:
	case LifespanLevel::M10:
	case LifespanLevel::M30:
	case LifespanLevel::M60:
	case LifespanLevel::Inf:
		lifespan = set; return;
	}
	lifespan = LifespanLevel::Inf;
}
int ConfigAI::Lifespan_Cast() {
	return static_cast<int>(lifespan);
}
LifespanLevel ConfigAI::Lifespan() {
	return lifespan;
}
void ConfigAI::TakeOverTime(double set) {
	takeOverTime = std::clamp(set, 0.0, 20.0);
}
double ConfigAI::TakeOverTime() {

	return 5.0;
#if DISTRIBUTION
#endif

	return takeOverTime;
}
void ConfigAI::ResetMyCharacter() {
	MyCharacter1(0);
	MyCharacter2(7);
	UseAltVoice(false);
}
void ConfigAI::MyCharacter1(int set) {
	myCharacter1 = std::clamp(set, 0, 23);
}
int ConfigAI::MyCharacter1() {
	return myCharacter1;
}
void ConfigAI::MyCharacter2(int set) {
	myCharacter2 = std::clamp(set, 0, 39);
}
int ConfigAI::MyCharacter2() {
	return myCharacter2;
}
void ConfigAI::UseAltVoice(bool set) {
	useAltVoice = set;
}
bool ConfigAI::UseAltVoice() {
	return useAltVoice;
}

int ConfigAI::PhysicalCore() {
	return physicalCore;
}
