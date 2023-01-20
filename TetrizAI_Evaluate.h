#pragma once
#include "TetrizAI_Common.h"

class EvaluateAI {
public:
	static void Alloc();
	static void ReadConfig();
	static void Dealloc();

	static void SetTrustRate(int depth);
	static void Evaluate(Node *node);
	static void EvaluateFirst(Node *node);
	static void CorrectScore(Node *node);

	static void ReadEvalWeight();
	static void TransitionWeight(RootNode *searching);
	static void EvenOrOdd(RootNode *searching);
	static bool IsEvenBoard();
	static bool UseS4W();
	static void UseS4W(bool set);

private:
	class EvalWeight;
	static inline EvalWeight *weightList;
	static inline EvalWeight *weight;

	static inline double trustRate;
	static inline bool isEvenBoard;
	static inline bool useS4W;

	static inline PlayStyleList playStyle;

	static int EvalHeight(GameState *game, int columnHeight[]);
	static int EvalWellColumn(GameState *game, int columnHeight[], int garbageHeight, int *wellColumn, int *wellDepth);
	static int EvalUpDown(GameState *game, int columnHeight[], int wellColumn, int wellDepth);
	static int EvalUnderground(GameState *game, int columnHeight[], int garbageHeight);
	static int EvalGround(GameState *game, int columnHeight[], int y0);
	static int EvalResource(GameState *game, int columnHeight[], int garbageHeight);

	static int EvalTsd(GameState *game, int columnHeight[], int y0);
	static int EvalTst(GameState *game, int columnHeight[], int y0, int garbageHeight);
	static int EvalDTCannon(GameState *game, int columnHeight[], int y0);

	static int EvalPcBoard(GameState *game, int garbageHeight);
	static int EvalPrePcBoard(GameState *game, int garbageHeight);

	static int EvalContBonus(GameState *game);
	static int EvalFinalAction(GameState *game);

	enum class W;

};

class VersusAI {
public:
	static bool MultiEvaluation();
	static void MultiEvaluation(bool set);

	static void OpponentIsPCBoard(Board *opponentBd);
	static bool OpponentIsPCBoard();
	static void SetOpponentsCombo(int combo);
	static int GetOpponentsCombo();
	static void OpponentIsPowerfulBoard(Board *opponentBd, int combo, int garbage);
	static bool OpponentIsPowerfulBoard();
private:
	static inline bool multiEvaluation;

	static inline bool opponentIsPCBoard;
	static inline bool opponentIsPowerfulBoard;
	static inline int opponentsCombo;
};

class OffsetAI {
public:
	static void UseTimingOffset(bool set);
	static bool UseTimingOffset();
	static bool CanWaitGarbage(int garbage);
	static void DecisionOffset(RootNode *searched);

	static void ResetOpponentsAtk();
	static void CalcOpponentsAtk(int line, int btb, int btbBuff, int combo);
	static int GetOpponentsAtk();
	static void SetRemainingTime(ClearAct act);
	static bool IsInCoolTime(int frame);
	static void SetCoolTimeAfterClearing(int frame, int sendingGarbage, int lock);
	static bool OpponentIsNewLockDown(int lock);
	static void InvalidateOpponentsAtk();
	static bool IsFirstInvalidLockDown();
	static inline int id;

private:

	static inline bool useTimingOffset;

	class OffsetData {
	public:
		bool canWaitGarbage;
		int center4Height;
		int garbageHeight;
		int atk;
		bool pcChance;
		bool nearOffset;
		//bool holdOffset;
	};

	static inline OffsetData ofsData;

	class GarbageData {
	public:
		int opponentsAtk;
		int coolTime;
		int remainingTime;
		int opponentsLockCount;
		bool firstInvalidLock;
	};

	static inline GarbageData gbData;
};
