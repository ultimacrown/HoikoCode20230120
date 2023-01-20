#include "TetrizAI_Expand.h"

#include <cstring>
#include <algorithm>

#include "TetrizAI_Common.h"

int ExpandAI::ExpandNode(const GameState *parentGame, Node *childNode) {
	// なんやかんややって設置パターンを列挙
	// 変更予定なので消した

	int columnHeight[10] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	parentGame->board.GetColumnHeight(columnHeight);

	int childCount = 0;
	this->babyNode->fallingMino = parentGame->fallingMino;
	this->babyNode->hold = parentGame->hold;
	this->babyNode->nextIt = parentGame->nextIt;
	this->babyNode->commands.Init();
	++childCount;

	childCount = ExpandBaseNode(this->babyNode, childCount, &parentGame->board);

	childCount = ExpandDeriveNode(this->babyNode, childCount, &parentGame->board, columnHeight);

	childCount = RegisterChildNode(this->babyNode, childCount, parentGame, &childNode[0]);

	if (parentGame->fallingMino.m == parentGame->hold.m) {

		return std::min(childCount, MAX_CHILDREN);
	}

	int prevChildCount = childCount;

	childCount = 0;
	this->babyNode->fallingMino = parentGame->fallingMino;
	this->babyNode->hold = parentGame->hold;
	this->babyNode->nextIt = parentGame->nextIt;
	this->babyNode->commands.Init();
	if (this->babyNode->commands.SwapHold(&this->babyNode->fallingMino, &this->babyNode->hold, &parentGame->board, &this->babyNode->nextIt) == 0) {
		return std::min(prevChildCount, MAX_CHILDREN);
	}
	++childCount;

	childCount = ExpandBaseNode(this->babyNode, childCount, &parentGame->board);

	childCount = ExpandDeriveNode(this->babyNode, childCount, &parentGame->board, columnHeight);

	childCount = RegisterChildNode(this->babyNode, childCount, parentGame, &childNode[prevChildCount]);

	return std::min(prevChildCount + childCount, MAX_CHILDREN);
}
