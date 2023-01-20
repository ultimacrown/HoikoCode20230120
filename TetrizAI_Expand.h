#pragma once

class GameState;
class Board;
class FallMino;
class Node;

#include "TetrizAI_Common.h"

constexpr int MAX_CHILDREN = 100;
constexpr int MAX_BABYS = 300;

class ExpandAI {
public:
	[[nodiscard]] int ExpandNode(const GameState *parentGame, Node *childNode);

	ExpandAI();
	~ExpandAI();
	ExpandAI(const ExpandAI &) = delete;
	ExpandAI &operator=(const ExpandAI &) = delete;
private:
	class ExpandAI::BabyNode {
	public:
		FallMino fallingMino;
		Command commands;
		HoldMino hold;
		NextIterator nextIt;
	};

	BabyNode bN[MAX_BABYS];
	BabyNode *babyNode;

	[[nodiscard]] int ExpandBaseNode(BabyNode *babyNode, int childCount, const Board *parentBoard);
	[[nodiscard]] int ExpandDeriveNode(BabyNode *babyNode, int childCount, const Board *parentBoard, const int columnHeight[]);

	[[nodiscard]] int RegisterChildNode(BabyNode *babyNode, int childCount, const GameState *parentGame, Node *childNode);
};
