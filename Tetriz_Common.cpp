#include "Tetriz_Common.h"

#include <cstring>
#include <algorithm>

// この関数はなくてもいい
void FallMino::Init() {
	this->p.x = FallMino::SPAWN_X;
	this->p.y = FallMino::SPAWN_Y;
	this->r = 0;
	this->lastSrs = 0;
	this->m = MinoType::None;
}
// ミノを出現させる
int FallMino::Spawn(const MinoType newM, const Board *bd) {
	this->m = newM;
	this->p.x = FallMino::SPAWN_X;
	this->p.y = FallMino::SPAWN_Y;
	this->r = 0;
	UpdateShape();
	this->lastSrs = 0;
	this->bugGravity = 0;

	int h = bd->GetSpawnHeight(this);

	if (h == -1) {
		return 0;
	}

	this->p.y = h;

	return 1;
}
void FallMino::SpinA() {
	if (this->r > 2) {
		this->r = 0;
		this->shape -= 3;
		return;
	}
	++this->r;
	++this->shape;
}
void FallMino::SpinB() {
	if (this->r < 1) {
		this->r = 3;
		this->shape += 3;
		return;
	}
	--this->r;
	--this->shape;
}
void FallMino::UpdateShape() {
	this->shape = MinoConst::GetShape(static_cast<int>(this->m), this->r);
}
const MinoCoord *FallMino::GetCoord() const {
	return &this->shape->coord;
}
const Srs *FallMino::GetSrsKickDataA() const {
	return MinoConst::GetSrsA(static_cast<int>(this->m), this->r);
}
const Srs *FallMino::GetSrsKickDataB() const {
	return MinoConst::GetSrsB(static_cast<int>(this->m), this->r);
}
int FallMino::Left() const {
	return this->p.x + this->shape->rect.left;
}
int FallMino::Top() const {
	return this->p.y - this->shape->rect.top;
}
int FallMino::Right() const {
	return this->p.x + this->shape->rect.right;
}
int FallMino::Bottom() const {
	return this->p.y - this->shape->rect.bottom;
}


void HoldMino::Init() {
	this->m = MinoType::None;
	this->swappedHold = 0;
	this->abnormalHold = 0;
}
void HoldMino::PPT1or2(int set) {
	ppt = std::clamp(set, 1, 2);
}
bool HoldMino::ForPPT1() {
	return ppt == 1;
}
bool HoldMino::ForPPT2() {
	return ppt == 2;
}

int Bit::PopCount(int bit) {
	class PopArray {
		int ar[1024];
	public:
		constexpr auto PopCount(int bit) const {
			return this->ar[bit];
		}
		constexpr PopArray() : ar{} {
			auto popCount = [](int bit) {
				constexpr int B1 = 0x55555555;
				constexpr int B2 = 0x33333333;
				constexpr int B3 = 0x0f0f0f0f;
				constexpr int B4 = 0x00ff00ff;
				constexpr int B5 = 0x0000ffff;
				bit = bit - ((bit >> 1) & B1);
				bit = (bit & B2) + ((bit >> 2) & B2);
				bit = (bit + (bit >> 4)) & B3;
				bit = bit + (bit >> 8);
				// bit = bit + (bit >> 16);
				return bit & 0xff;
			};
			for (int i = 0; i < 1024; ++i) {
				this->ar[i] = popCount(i);
			}
		}
	};

	static constexpr auto popArray = PopArray();

	return popArray.PopCount(bit);
}
int Bit::Log2(int bit) {
	bit -= 1;
	return 9 - PopCount(bit);
}

void Board::Init() {
	memset(this->line, 0, sizeof(this->line));
	this->stackHeight = -1;
	this->lockedBottom = -1;
}
int Board::CompareTo(const Board *other) const {
	return memcmp(this->line, other->line, sizeof(this->line));
}
int Board::Collision(const FallMino *mino) const {

	if (mino->Left() < 0) return 1;
	if (mino->Right() >= Board::WIDTH) return 1;
	if (mino->Bottom() < 0) return 1;

	if ((this->line[mino->p.y - 3] & mino->shape->At(mino->p.x, 3)) > 0) return 1;
	if ((this->line[mino->p.y - 2] & mino->shape->At(mino->p.x, 2)) > 0) return 1;
	if ((this->line[mino->p.y - 1] & mino->shape->At(mino->p.x, 1)) > 0) return 1;
	if ((this->line[mino->p.y - 0] & mino->shape->At(mino->p.x, 0)) > 0) return 1;

	return 0;
}
// 呼び出す前にミノの種類、向き、形を更新しておくこと
int Board::GetSpawnHeight(const FallMino *mino) const {

	// 19段以下なら衝突チェックは不要
	if (this->stackHeight + 1 <= 19) return FallMino::SPAWN_Y;

	int collision = 0;

#if 0
	// 上2段だけでいい（＝ 下2段は空白）
	if ((this->line[FallMino::SPAWN_Y - 3] & mino->shape->At(FallMino::SPAWN_X, 3)) > 0) throw;
	if ((this->line[FallMino::SPAWN_Y - 2] & mino->shape->At(FallMino::SPAWN_X, 2)) > 0) throw;
#endif
	if ((this->line[FallMino::SPAWN_Y - 1] & mino->shape->At(FallMino::SPAWN_X, 1)) > 0) ++collision;
	if ((this->line[FallMino::SPAWN_Y - 0] & mino->shape->At(FallMino::SPAWN_X, 0)) > 0) ++collision;

	if (collision == 0) return FallMino::SPAWN_Y;
	collision = 0;

#if 0
	// 上2段だけでいい（＝ 下2段は空白）
	if ((this->line[FallMino::SPAWN_Y + 1 - 3] & mino->shape->At(FallMino::SPAWN_X, 3)) > 0) throw;
	if ((this->line[FallMino::SPAWN_Y + 1 - 2] & mino->shape->At(FallMino::SPAWN_X, 2)) > 0) throw;
#endif
	if ((this->line[FallMino::SPAWN_Y + 1 - 1] & mino->shape->At(FallMino::SPAWN_X, 1)) > 0) ++collision;
	if ((this->line[FallMino::SPAWN_Y + 1 - 0] & mino->shape->At(FallMino::SPAWN_X, 0)) > 0) ++collision;

	if (collision == 0) {
		return FallMino::SPAWN_Y + 1;
	}
	else {
		return -1;
	}
}
int Board::CollisionDistance(const FallMino *mino) const {
	// 要検討

	for (int y = std::min(mino->p.y, this->stackHeight + mino->shape->rect.bottom + 1) - 1; y > 0; --y) {
		if (y - mino->shape->rect.bottom < 0) return mino->p.y - y - 1;

		if ((this->line[y - 3] & mino->shape->At(mino->p.x, 3)) > 0) return mino->p.y - y - 1;
		if ((this->line[y - 2] & mino->shape->At(mino->p.x, 2)) > 0) return mino->p.y - y - 1;
		if ((this->line[y - 1] & mino->shape->At(mino->p.x, 1)) > 0) return mino->p.y - y - 1;
		if ((this->line[y - 0] & mino->shape->At(mino->p.x, 0)) > 0) return mino->p.y - y - 1;
	}

	return mino->p.y - 1;
}
int Board::CollisionBottom(const FallMino *mino) const {

	if (mino->Bottom() < 0 + 1) return 1;

	if ((this->line[mino->p.y - 1 - 3] & mino->shape->At(mino->p.x, 3)) > 0) return 1;
	if ((this->line[mino->p.y - 1 - 2] & mino->shape->At(mino->p.x, 2)) > 0) return 1;
	if ((this->line[mino->p.y - 1 - 1] & mino->shape->At(mino->p.x, 1)) > 0) return 1;
	if ((this->line[mino->p.y - 1 - 0] & mino->shape->At(mino->p.x, 0)) > 0) return 1;

	return 0;
}
int Board::CollisionLeft(const FallMino *mino) const {

	if (mino->Left() < 0 + 1) return 1;

	if ((this->line[mino->p.y - 3] & mino->shape->At(mino->p.x - 1, 3)) > 0) return 1;
	if ((this->line[mino->p.y - 2] & mino->shape->At(mino->p.x - 1, 2)) > 0) return 1;
	if ((this->line[mino->p.y - 1] & mino->shape->At(mino->p.x - 1, 1)) > 0) return 1;
	if ((this->line[mino->p.y - 0] & mino->shape->At(mino->p.x - 1, 0)) > 0) return 1;

	return 0;
}
int Board::CollisionRight(const FallMino *mino) const {

	if (mino->Right() >= Board::WIDTH - 1) return 1;

	if ((this->line[mino->p.y - 3] & mino->shape->At(mino->p.x + 1, 3)) > 0) return 1;
	if ((this->line[mino->p.y - 2] & mino->shape->At(mino->p.x + 1, 2)) > 0) return 1;
	if ((this->line[mino->p.y - 1] & mino->shape->At(mino->p.x + 1, 1)) > 0) return 1;
	if ((this->line[mino->p.y - 0] & mino->shape->At(mino->p.x + 1, 0)) > 0) return 1;

	return 0;
}
int Board::SrsA(FallMino *mino) const {
	mino->SpinA();
	if (Collision(mino) == 0) return 1;
	const Srs *srs = mino->GetSrsKickDataA();

	// ループ展開
	for (int i = 0; i < 4; ++i) {
		mino->p.x += srs->test[i].x;
		mino->p.y += srs->test[i].y;
		if (Collision(mino) == 0) {
			return i + 2;
		}
		// ここの処理は省略できる（誤差）
		mino->p.x -= srs->test[i].x;
		mino->p.y -= srs->test[i].y;
	}
	mino->SpinB();

	return 0;
}
int Board::SrsB(FallMino *mino) const {
	mino->SpinB();
	if (Collision(mino) == 0) return 1;
	const Srs *srs = mino->GetSrsKickDataB();

	// ループ展開
	for (int i = 0; i < 4; ++i) {
		mino->p.x += srs->test[i].x;
		mino->p.y += srs->test[i].y;
		if (Collision(mino) == 0) {
			return i + 2;
		}
		// ここの処理は省略できる（誤差）
		mino->p.x -= srs->test[i].x;
		mino->p.y -= srs->test[i].y;
	}
	mino->SpinA();

	return 0;
}
int Board::LockMino(const FallMino *mino) {

	// 20段以上ならgameover
	if (mino->Bottom() > 19) return -1;

#if 0
	// エリア内に収まっているはずなのでチェックは不要
	if (mino->Left() < 0) return -1;
	if (mino->Bottom() < 0) return -1;
	if (mino->Right() >= Board::WIDTH) return -1;
#endif

	int clearedLine = 0;

#if 0
	// ミノは地形に貫通しうるのでチェックは不要
	if ((this->line[mino->p.y - 3] & mino->shape->At(mino->p.x, 3)) > 0) return -1;
	if ((this->line[mino->p.y - 2] & mino->shape->At(mino->p.x, 2)) > 0) return -1;
	if ((this->line[mino->p.y - 1] & mino->shape->At(mino->p.x, 1)) > 0) return -1;
	if ((this->line[mino->p.y - 0] & mino->shape->At(mino->p.x, 0)) > 0) return -1;
#endif
	this->line[mino->p.y - 3] |= mino->shape->At(mino->p.x, 3);
	this->line[mino->p.y - 2] |= mino->shape->At(mino->p.x, 2);
	this->line[mino->p.y - 1] |= mino->shape->At(mino->p.x, 1);
	this->line[mino->p.y - 0] |= mino->shape->At(mino->p.x, 0);
	if (this->line[mino->p.y - 3] == 0b11111'11111) ++clearedLine;
	if (this->line[mino->p.y - 2] == 0b11111'11111) ++clearedLine;
	if (this->line[mino->p.y - 1] == 0b11111'11111) ++clearedLine;
	if (this->line[mino->p.y - 0] == 0b11111'11111) ++clearedLine;

	if (this->stackHeight < mino->Top()) {
		this->stackHeight = mino->Top();
	}
	this->lockedBottom = mino->Bottom();

	return clearedLine;
}
TspinType Board::JudgeTspin(const FallMino *mino) const {
	if (mino->m != MinoType::T) {
		return TspinType::NotTspin;
	}

	if (mino->lastSrs == 0) {
		return TspinType::NotTspin;
	}
	// 0 t 1
	// t t t
	// 3   2
	int corners[4] = { 0 };
	int fillCount = 0;

	if ((mino->p.x < 0) || ((this->line[mino->p.y] & (0b10000'00000 >> mino->p.x)))) {
		corners[0] = 1;
		++fillCount;
	}
	if ((mino->p.x >= Board::WIDTH - 2) || ((this->line[mino->p.y] & (0b10000'00000 >> (mino->p.x + 2))))) {
		corners[1] = 1;
		++fillCount;
	}
	if ((mino->p.x >= Board::WIDTH - 2) || (mino->p.y - 2 < 0) || ((this->line[mino->p.y - 2] & (0b10000'00000 >> (mino->p.x + 2))))) {
		corners[2] = 1;
		++fillCount;
	}
	if ((mino->p.x < 0) || (mino->p.y - 2 < 0) || ((this->line[mino->p.y - 2] & (0b10000'00000 >> mino->p.x)))) {
		corners[3] = 1;
		++fillCount;
	}

	if (fillCount < 3) {
		return TspinType::NotTspin;
	}

	int r = mino->r;
	if (++r > 3) r = 0;

	if (corners[mino->r] != corners[r]) {
		if (mino->lastSrs != 5) {
			return TspinType::MiniTspin;
		}
	}

	return TspinType::Tspin;

}
void Board::ClearLine() {
	for (int i = this->lockedBottom, end = this->stackHeight; i <= end; ++i) {
		if (this->line[i] == 0b11111'11111) {
			for (int i2 = i; i2 < end; ++i2) {
				this->line[i2] = this->line[i2 + 1];
			}
			this->line[end] = 0;
			--i;
			--end;
			--this->stackHeight;
		}
	}
}
void Board::UpdateStackHeight() {
#if 0
	// 下から
	for (int y = 0; y < BOARD_HEIGHT; ++y) {
		if (this->line[y] == 0) {
			this->stackHeight = y - 1;
			return;
		}
	}
#else
	// 上から
	for (int y = Board::HEIGHT - 1; y >= 0; --y) {
		if (this->line[y] != 0) {
			this->stackHeight = y;
			return;
		}
	}
	this->stackHeight = -1;
#endif
}
// 高さは事前に更新されている必要がある
int Board::PopCount() {
	//UpdateStackHeight();

	int totalPop = 0;
	for (int y = this->stackHeight; y >= 0; --y) {
		totalPop += Bit::PopCount(this->line[y]);
	}

	return totalPop;
}
// 高さは事前に更新されている必要がある
bool Board::IsEvenPop() {
	return (this->PopCount() & 1) == 0;
}
// return garbageHeight
int Board::GetColumnHeight(int columnHeight[]) const {

	int bit = 0;
	int min = 0;

	for (int y = this->stackHeight; y >= 0; --y) {
		bit |= this->line[y];
		if (bit == 0b11111'11111) {
			min = y + 1;
			break;
		}

		int x = -1;
		columnHeight[x] += bool(bit & (0b10000'00000 >> (++x)));
		columnHeight[x] += bool(bit & (0b10000'00000 >> (++x)));
		columnHeight[x] += bool(bit & (0b10000'00000 >> (++x)));
		columnHeight[x] += bool(bit & (0b10000'00000 >> (++x)));
		columnHeight[x] += bool(bit & (0b10000'00000 >> (++x)));
		columnHeight[x] += bool(bit & (0b10000'00000 >> (++x)));
		columnHeight[x] += bool(bit & (0b10000'00000 >> (++x)));
		columnHeight[x] += bool(bit & (0b10000'00000 >> (++x)));
		columnHeight[x] += bool(bit & (0b10000'00000 >> (++x)));
		columnHeight[x] += bool(bit & (0b10000'00000 >> (++x)));
	}
	if (min > 0) {
		int x = -1;
		columnHeight[++x] += min;
		columnHeight[++x] += min;
		columnHeight[++x] += min;
		columnHeight[++x] += min;
		columnHeight[++x] += min;
		columnHeight[++x] += min;
		columnHeight[++x] += min;
		columnHeight[++x] += min;
		columnHeight[++x] += min;
		columnHeight[++x] += min;
	}

	return min - 1;
}

void Others::Init() {
	this->combo = 0;
	this->btb = 0;
	this->act = ClearAct::None;
}
int Others::GetAtk() {
	int atk = 0;
	auto btbBonus = [this]() {
		return this->btb > 1 ? 1 : 0;
	};
	switch (this->act) {
	case ClearAct::None:
		return 0;
	case ClearAct::Single:
	case ClearAct::Tsm:
		break;
	case ClearAct::Double:
		atk += 1;
		break;
	case ClearAct::Triple:
	case ClearAct::Tss:
		atk += 2;
		break;
	case ClearAct::Quad:
	case ClearAct::Tsd:
		atk += 4;
		break;
	case ClearAct::Tst:
		atk += 6;
		break;
	case ClearAct::Pc:
		return 10;
	}

	atk += btbBonus();

	switch (this->combo) {
	case 0:
	case 1:
	case 2:
		break;
	case 3:
	case 4:
		atk += 1;
		break;
	case 5:
	case 6:
		atk += 2;
		break;
	case 7:
	case 8:
		atk += 3;
		break;
	case 9:
	case 10:
	case 11:
		atk += 4;
		break;
	default:
		atk += 5;
		break;
	}
	return atk;
}
bool Others::HaveAtk() {
	switch (this->act) {
	case ClearAct::None:
		return false;
	case ClearAct::Single:
		break;
	case ClearAct::Tsm:
		if (this->btb > 1) return true;
		break;
		//case ClearAct::Double:
		//case ClearAct::Triple:
		//case ClearAct::Tss:
		//case ClearAct::Quad:
		//case ClearAct::Tsd:
		//case ClearAct::Tst:
		//case ClearAct::Pc:
	default:
		return true;
	}
	return this->combo >= 3;
}
void Others::CalcAtk(const FallMino *mino, const Board *bd, const int clearedLine) {

	if (clearedLine == 0) {
		if (this->combo > 0) this->combo *= -1;
		else this->combo = 0;
		this->act = ClearAct::None;
		return;
	}

	if (this->combo < 0) this->combo = 0;
	++this->combo;

	if (bd->line[clearedLine - 1] == 0b11111'11111
		&& bd->line[clearedLine] == 0) {
		if (clearedLine >= 4) {
			this->btb += 1;
		}
		else {
			this->btb = 0;
		}
		this->act = ClearAct::Pc;
		return;
	}

	if (clearedLine >= 4) {
		this->btb += 1;
		this->act = ClearAct::Quad;
	}
	else {
		switch (bd->JudgeTspin(mino)) {
		case TspinType::NotTspin:
			if (clearedLine == 1) {
				this->act = ClearAct::Single;
			}
			else if (clearedLine == 2) {
				this->act = ClearAct::Double;
			}
			else {
				this->act = ClearAct::Triple;
			}
			this->btb = 0;
			break;
		case TspinType::Tspin:
			if (clearedLine == 1) {
				this->act = ClearAct::Tss;
			}
			else if (clearedLine == 2) {
				this->act = ClearAct::Tsd;
			}
			else {
				this->act = ClearAct::Tst;
			}
			this->btb += 1;
			break;
		case TspinType::MiniTspin:
			this->act = ClearAct::Tsm;
			this->btb += 1;
			break;
		}
	}
}

#if 0

switch () {
case ClearAct::None:
case ClearAct::Single:
case ClearAct::Double:
case ClearAct::Triple:
case ClearAct::Quad:
case ClearAct::Tss:
case ClearAct::Tsd:
case ClearAct::Tst:
case ClearAct::Tsm:
case ClearAct::Pc:
}

#endif
