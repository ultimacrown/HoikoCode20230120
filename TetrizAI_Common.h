#pragma once

#include "Tetriz_Common.h"
#include <array>

class NextMino {
public:
	static constexpr int SIZE = 28;
	MinoType m[NextMino::SIZE];
	int trustEnd;
	int predictRemains;
	int predictRange;
	int visibleRange;
	void Init(const MinoType *nx);
	void ClearArray();

	void Compare(const int cmpFirst, const MinoType *nx);
	void Advance();

	bool IsOriginNext(bool includeHold);
	void OpeningPredictionSwap();
	bool OpeningPredictionHit(const MinoType *nx);

	static int ViewNext();
	static void ViewNext(int set);
private:
	static int viewNext;

	void Swap(const int cmpedIndex, const MinoType nx);
	class Bag {
		int remains;
	public:
		void Reset();
		MinoType Pick();
		int Remains() const;
	};

	Bag bag;
};

#define OUR

class NextIterator {
public:
	const MinoType Get() const;
	const MinoType Get(int i) const;
	void Advance();
	void SetIterator(const NextMino *nx);
	bool IsTrustNext(const int i, const int trustRange) const;
	bool IsVisibleNext(const int i, const int trustRange) const;
private:
#ifndef OUR
	const MinoType *m;
	const MinoType *trustEnd;
#else
	const NextMino *ourNext;
	int myIndex;
#endif
};

// 全てのノードがCommandの情報を持っている必要がなかったため
// その代わりをつとめるクラス
#if 0
class MinoTrack {
public:
	int moveDelay;
	int firstHold;
	int softDropDelay;
	void Init();
	int HardDrop(FallMino *mino, const Board *bd);
	int SoftDrop(FallMino *mino, const Board *bd);
	int SoftDropOnce(FallMino *mino, const Board *bd);
	int MoveLeft(FallMino *mino, const Board *bd);
	int MoveRight(FallMino *mino, const Board *bd);
	int MoveLeftMost(FallMino *mino, const Board *bd);
	int MoveRightMost(FallMino *mino, const Board *bd);
	int RotateA(FallMino *mino, const Board *bd);
	int RotateB(FallMino *mino, const Board *bd);
	int SwapHold(FallMino *mino, HoldMino *hld, const Board *bd, NextIterator *nxIt);
	bool IsFirstHold();
};
#endif

class Command {
public:
	static constexpr int LENGTH = 48;
	char s[Command::LENGTH];
	int moveDelay;
	void Init();
	int HardDrop(FallMino *mino, const Board *bd);
	int SoftDrop(FallMino *mino, const Board *bd);
	int SoftDropOnce(FallMino *mino, const Board *bd);
	int MoveLeft(FallMino *mino, const Board *bd);
	int MoveRight(FallMino *mino, const Board *bd);
	int MoveLeftMost(FallMino *mino, const Board *bd);
	int MoveRightMost(FallMino *mino, const Board *bd);
	int RotateA(FallMino *mino, const Board *bd);
	int RotateB(FallMino *mino, const Board *bd);
	int SwapHold(FallMino *mino, HoldMino *hld, const Board *bd, NextIterator *nxIt);
	bool IsFirstHold();
	void AddCommands(const char *cmds);
	void CorrectDelay();
private:
	int len;
	void AddCommand(const char cmd);
	void AddCommandSoftDrop(const char cmd, const int y);
};

class GameState {
public:
	NextIterator nextIt;
	HoldMino hold;

	FallMino fallingMino;
	FallMino lockedMino;

	Board board;

	Command commands;

	Others others;

	int LockDown();
	bool IsSafe() const;
};

constexpr int BEAMSIZE_MAX_PERTHREAD = 500;
constexpr int BEAMSIZE_MIN = 50;
constexpr int MAX_DEPTH = 14;
static_assert(NextMino::SIZE >= MAX_DEPTH + 2, "");

class Plans {
public:
	FallMino mino[MAX_DEPTH];
	ClearAct act[MAX_DEPTH];
	int drawingBreak;
	void Clear();
};

class SearchResult {
public:
	double overTime;
	double searchTime;
	double pptTime;
	int node;
	int depth;
	int ren;
	int basicBeamSize;
	int correctedBeamSize;
	Plans plans;

	void Clear();
private:
};

class APM {
	static inline double attack;
	static inline double apm;
public:
	static double AttackPerMinutes() {
		return apm;
	}
	static void Set(double atk, double f) {
		if (f == 0.0) return;
		attack += atk;
		double perMinutes = 60.0 * 60.0 / f;
		apm = attack * perMinutes;
	}
	static void Reset() {
		attack = 0;
		apm = 0;
	}
};

class Node {
public:
	GameState game;

	int childCount;

	int score;
	int familyScore;
	int pcScore;
	//int badFieldScore;

	// indexは使わない
	int parentIndex;
	Node *parentNode;

	class Cmp {
	public:
		bool operator()(const Node &l, const Node &r) const {
			return l.score > r.score;
		}
		bool operator()(const Node *l, const Node *r) const {
			return l->score > r->score;
		}
	};
};

class Mimic {
public:
	Board board;
	MinoType hold;
	NextMino next;
	int combo;
	int btb;
	int missPattern;
	std::array<std::array<int, 40>, 10> pptBoard;
	std::array<int, 5> pptNext;
	std::array<int, 7> pptInvisibleNext;
};

class RootNode {
public:
	NextMino next;

	GameState game;

	int childCount;

	SearchResult result;

	int gameover;

	void Init(const MinoType *nx);
	void GameOver();
	bool IsSameAs(Mimic *mimic);
	void SetPPTData(Mimic *mimic);
	void ResetPPTData(Mimic *mimic);
	void AdvanceMino();
	int GetClearDelay();
	double GetDelay();
};

enum class SpeedLevel {
	S100 = 100,
	S50 = 50,
	S33 = 33,
	S25 = 25,
};
enum class LifespanLevel {
	M1 = 1,
	M3 = 3,
	M5 = 5,
	M10 = 10,
	M30 = 30,
	M60 = 60,
	Inf = 1440,
};
enum class PlayStyleList {
	Vs,
	Ultra,
	Sprint,
};


class ConfigAI {
public:
	//PublicOption
	static void ReadConfig();
	static void WriteConfig();
	static void SetDefaultConfig();

	static void PlayerNo(int set);
	static int PlayerNo();
	static void ViewNext(int set);
	static int ViewNext();
	static void UseHold(bool set);
	static bool UseHold();
	static void BeamSize(int set);
	static int BeamSize();
	static void UseTDOpeners(int set);
	static bool UseTDOpeners();
	static void Speed(int set);
	static void Speed(SpeedLevel set);
	static int Speed_Cast();
	static SpeedLevel Speed();
	static void AutoCharSelect(bool set);
	static bool AutoCharSelect();
	static void PlayStyle(int set);
	static void PlayStyle(PlayStyleList set);
	static int PlayStyle_Cast();
	static PlayStyleList PlayStyle();
	static void UseTimingOffset(bool set);
	static bool UseTimingOffset();
	static void UsePCstack(bool set);
	static bool UsePCstack();
	static void UseS4W(bool set);
	static bool UseS4W();
	static void UseDPC(bool set);
	static bool UseDPC();
	static void LoopTemplate(bool set);
	static bool LoopTemplate();
	static void ThreadCount(int set);
	static int ThreadCount();
	//PrivateOption
	static void LowerDepthLimit(int set);
	static int  LowerDepthLimit();
	static void Lifespan(int set);
	static void Lifespan(LifespanLevel set);
	static int Lifespan_Cast();
	static LifespanLevel Lifespan();
	static void TakeOverTime(double set);
	static double TakeOverTime();

	static void ResetMyCharacter();
	static void MyCharacter1(int set);
	static int MyCharacter1();
	static void MyCharacter2(int set);
	static int MyCharacter2();
	static void UseAltVoice(bool set);
	static bool UseAltVoice();

	static int PhysicalCore();
private:
	//PublicOption
	static inline int playerNo;
	static inline int viewNext;
	static inline bool useHold;
	static inline int beamSize;
	static inline bool useTDOpeners;
	static inline SpeedLevel speed;
	static inline bool autoCharSelect;
	static inline PlayStyleList playStyle;
	static inline bool useTimingOffset;
	static inline bool usePCstack;
	static inline bool useS4W;
	static inline bool useDPC;
	static inline bool loopTemplate;
	static inline int threadCount;
	//PrivateOption
	static inline int lowerDepthLimit;
	static inline LifespanLevel lifespan;
	static inline double takeOverTime;
	static inline int myCharacter1;
	static inline int myCharacter2;
	static inline bool useAltVoice;

	static inline int physicalCore;
};
