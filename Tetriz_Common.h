#pragma once

#include "Tetriz_Constant.h"

constexpr int VIEW_NEXT = 5;

enum class MinoType {
	I,
	J,
	L,
	O,
	T,
	S,
	Z,
	None,
	Garbage
};

#if 0
enum class ClearAct {
	None,
	Single,
	Double,
	Triple,
	Quad,
	Tss,
	Tsd,
	Tst,
	Tsm,
	Pc,
};
#else
class ClearAct {
	enum class CAType {
		None,
		Single,
		Double,
		Triple,
		Quad,
		Tss,
		Tsd,
		Tst,
		Tsm,
		Pc,
	};
	CAType type;
public:
	static constexpr auto None = CAType::None;
	static constexpr auto Single = CAType::Single;
	static constexpr auto Double = CAType::Double;
	static constexpr auto Triple = CAType::Triple;
	static constexpr auto Quad = CAType::Quad;
	static constexpr auto Tss = CAType::Tss;
	static constexpr auto Tsd = CAType::Tsd;
	static constexpr auto Tst = CAType::Tst;
	static constexpr auto Tsm = CAType::Tsm;
	static constexpr auto Pc = CAType::Pc;
	int GetDelay() const {
		return this->GetDelay(this->type);
	}
	static int GetDelay(CAType t) {
		switch (t) {
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
	int GetAtk() const {
		return this->GetAtk(this->type);
	}
	static int GetAtk(CAType t) {
		switch (t) {
		case ClearAct::None:
		case ClearAct::Single:
		case ClearAct::Tsm:
			return 0;
		case ClearAct::Double:
			return 1;
		case ClearAct::Triple:
		case ClearAct::Tss:
			return 2;
		case ClearAct::Quad:
		case ClearAct::Tsd:
			return 4;
		case ClearAct::Tst:
			return 6;
		case ClearAct::Pc:
			return 10;
		}
		return 0;
	}
	ClearAct &operator=(const CAType &rhs) {
		this->type = rhs;
		return *this;
	}
	bool operator==(const CAType &rhs) const {
		return this->type == rhs;
	}
	bool operator!=(const CAType &rhs) const {
		return !(this->type == rhs);
	}
	operator CAType() const {
		return this->type;
	}
};
#endif

enum class TspinType {
	Tspin,
	NotTspin,
	MiniTspin
};

class FallMino {
public:
	static constexpr int SPAWN_X = 3;
	static constexpr int SPAWN_Y = 20;

	MinoPos p;
	int r;
	MinoType m;
	int lastSrs;
	// ���W����o�O�p�ϐ�
	int bugGravity;
	const MinoShape *shape;

	void Init();
	int Spawn(const MinoType newM, const class Board *bd);
	void SpinA();
	void SpinB();
	void UpdateShape();
	const MinoCoord *GetCoord() const;
	const Srs *GetSrsKickDataA() const;
	const Srs *GetSrsKickDataB() const;
	int Left() const;
	int Top() const;
	int Right() const;
	int Bottom() const;
};
/*
	�EbugGravity�̉��
	20�i�ځi�f�t�H���g���W�j�ɏo������͂��̃~�m���n�`�ɏՓ˂�21�i�ڂɏo������ƃt���O�����B
	�t���O�������Ă����Ԃł͉��ړ����]�����邽�тɋ����I��1�i�\�t�h�������������B
	�\�t�h���ɐ��������Ȃ�t���O�͉��낳���B
	�܂�Ƀ\�t�h������������Ȃ��o�O���N���邽�߁A���̎��͎�����1�i�\�t�h�����܂��傤�B
*/

class HoldMino {
public:
	MinoType m;
	int swappedHold;
	int abnormalHold;
	void Init();
	static void PPT1or2(int set);
	static bool ForPPT1();
	static bool ForPPT2();
private:
	static inline int ppt;
};

class Bit {
public:
	static int PopCount(int bit);
	static int Log2(int bit);
};

class Board {
	// int�̓������T�C�Y�ł��Ȃ�
	// �z���-2�Ԗڂ܂ŃA�N�Z�X����̂�2�ϐ��𑝂₵�Ă����i�������݂͂���Ȃ��j
public:
	static constexpr int HEIGHT = 32;
	static constexpr int WIDTH = 10;
	int line[Board::HEIGHT];
	int stackHeight;
	int lockedBottom;
	void Init();
	int CompareTo(const Board *other) const;
	int LockMino(const FallMino *mino);
	int Collision(const FallMino *mino) const;
	int CollisionDistance(const FallMino *mino) const;
	int CollisionBottom(const FallMino *mino) const;
	int CollisionLeft(const FallMino *mino) const;
	int CollisionRight(const FallMino *mino) const;
	int SrsA(FallMino *mino) const;
	int SrsB(FallMino *mino) const;
	int GetSpawnHeight(const FallMino *mino) const;
	TspinType JudgeTspin(const FallMino *mino) const;
	void ClearLine();
	void UpdateStackHeight();
	int PopCount();
	bool IsEvenPop();
	int GetColumnHeight(int columnHeight[]) const;
};

class Others {
public:
	int combo;
	int btb;
	ClearAct act;

	void Init();
	int GetAtk();
	bool HaveAtk();
	void CalcAtk(const FallMino *mino, const Board *bd, const int clearedLine);
};

