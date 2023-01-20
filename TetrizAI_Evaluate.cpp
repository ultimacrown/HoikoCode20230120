#include "TetrizAI_Evaluate.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>

//#include "TetrizAI_Common.h"


enum class EvaluateAI::W {
	MaxHeight,
	Over10,
	Over15,
	UpDown2,
	DonateCover,
	Cover,
	Roof,
	BadRoof,
	Pierce,
	Anabara,
	WellHint,
	WellDepth,
	WellPeak,
	Column,
	PcStack,
	PcChance,
	Resource,
	ResourceMax,
	TsdHole,
	TsdSpinable,
	TsdClearable,
	TsdOffensive,
	TstHole,
	TstSpinable,
	TstClearable,
	TstHint,
	TstOffensive,
	TDHole,
	TDHint,
	MoveDelay,
	HoldI,
	HoldT,
	WasteI,
	WasteT,
	Single,
	Double,
	Triple,
	Quad,
	Tss,
	Tsd,
	Tst,
	Btb,
	Pc,
	ComboBtb,
	ComboAtk,
	ComboDebt,
	Nexus,
};

class EvaluateAI::EvalWeight {
	int weight[50];
	int column[10];
	int comboTable[5];
	double nexus;
	void ZeroClear() {
		memset(this, 0, sizeof(*this));
	}
public:
	int GetWeight(W w) {
		return this->weight[static_cast<int>(w)];
	}
	int GetColumn(int x) {
		return this->column[x];
	}
	int GetComboTable(int rank) {
		return this->comboTable[rank];
	}
	double GetNexus() {
		return this->nexus;
	}

	void ReadWeight(const std::string &path) {
		std::string rowBuf;
		std::string columnBuf;
		std::ifstream ifs;
		ifs.open(path);
		if (!ifs.is_open()) {
			ZeroClear();
			return;
		}

		int correct = -1;
		int i = 0;

		try {
			for (; std::getline(ifs, rowBuf); ++i) {
				std::istringstream ist(rowBuf);
				if (i == static_cast<int>(W::Column)) {
					for (int column = 1; std::getline(ist, columnBuf, ','); ++column) {
						if (column == 1) continue;
						this->column[column - 2] = std::stoi(columnBuf);
						if (column >= 11) {
							correct += 1;
							break;
						}
					}
					continue;
				}
				if (i == static_cast<int>(W::ComboAtk)) {
					int correctTable = 0;
					for (int column = 1; std::getline(ist, columnBuf, ','); ++column) {
						if (column == 1) continue;
						if (columnBuf[0] == '\0') break;
						this->comboTable[column - 2] = std::stoi(columnBuf);
						correctTable += 1;
						if (column >= 6) {
							break;
						}
					}
					if (correctTable > 0) {
						correct += 1;
						if (correctTable < 5) {
							this->comboTable[1] = this->comboTable[0] * 2;
							this->comboTable[2] = this->comboTable[0] * 3;
							this->comboTable[3] = this->comboTable[0] * 4;
							this->comboTable[4] = this->comboTable[0] * 5;
						}
					}
					continue;
				}
				for (int column = 1; std::getline(ist, columnBuf, ','); ++column) {
					if (column == 1) continue;
					this->weight[i] = std::stoi(columnBuf);
					if (column >= 2) {
						correct += 1;
						break;
					}
				}
				if (i == static_cast<int>(W::Nexus)) {
					this->nexus = std::clamp(this->weight[i], 0, 100);
					this->nexus *= 0.01;
					break;
				}
			}
		}
		catch (...) {
			correct = -1;
		}

		if (i != correct) {
			ZeroClear();
		}

		ifs.close();
	}
};

void EvaluateAI::Alloc() {
	if (weightList != nullptr) return;
	weightList = new EvalWeight[4];
}
void EvaluateAI::ReadConfig() {
	ReadEvalWeight();

	isEvenBoard = true;
	useS4W = ConfigAI::UseS4W();
	VersusAI::MultiEvaluation(false);
}
void EvaluateAI::Dealloc() {
	if (weightList == nullptr) return;
	delete[] weightList;
	weightList = nullptr;
}

void EvaluateAI::SetTrustRate(int depth) {
	trustRate = 1.0;
	// <変更> 確定していないnextの点をさげる
	if (depth > NextMino::ViewNext() + 2) {
		trustRate = 0.9;
	}
}
void EvaluateAI::Evaluate(Node *node) {
	GameState *game = &node->game;

	int columnHeight[Board::WIDTH] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	int garbageHeight = game->board.GetColumnHeight(columnHeight);

	int boardScore = 0;

	boardScore += EvalHeight(game, columnHeight);
	int wellColumn = -1;
	int wellDepth = 0;
	boardScore += EvalWellColumn(game, columnHeight, garbageHeight, &wellColumn, &wellDepth);
	boardScore += EvalUpDown(game, columnHeight, wellColumn, wellDepth);
	boardScore += EvalUnderground(game, columnHeight, garbageHeight);
	boardScore += EvalGround(game, columnHeight, wellDepth + garbageHeight);
	boardScore += EvalResource(game, columnHeight, garbageHeight);
	boardScore += EvalTsd(game, columnHeight, garbageHeight + wellDepth);
	boardScore += EvalTst(game, columnHeight, garbageHeight + wellDepth, garbageHeight);

	boardScore += EvalPcBoard(game, garbageHeight);
	boardScore += EvalContBonus(game);

	int actionScore = EvalFinalAction(game);
	// 前借りしたスコアを返却
	actionScore -= node->parentNode->pcScore;
	// パフェ直前地形を加算&記憶
	node->pcScore = EvalPrePcBoard(game, garbageHeight);
	actionScore += node->pcScore;

	{
		// 地形スコアは0~100%引き継ぐ（今は50%）
		// 火力スコアはそのまま引き継ぐ
		int fs = actionScore + static_cast<int>(boardScore * weight->GetNexus());

		int s = actionScore + boardScore;

		node->familyScore = static_cast<int>(fs * trustRate);
		node->score = static_cast<int>(s * trustRate);

		node->familyScore += node->parentNode->familyScore;
		node->score += node->parentNode->familyScore;
	}
}
void EvaluateAI::EvaluateFirst(Node *node) {
	GameState *game = &node->game;

	int columnHeight[Board::WIDTH] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	int garbageHeight = game->board.GetColumnHeight(columnHeight);

	int boardScore = 0;

	boardScore += EvalHeight(game, columnHeight);
	int wellColumn = -1;
	int wellDepth = 0;
	boardScore += EvalWellColumn(game, columnHeight, garbageHeight, &wellColumn, &wellDepth);
	boardScore += EvalUpDown(game, columnHeight, wellColumn, wellDepth);
	boardScore += EvalUnderground(game, columnHeight, garbageHeight);
	boardScore += EvalGround(game, columnHeight, wellDepth + garbageHeight);
	boardScore += EvalResource(game, columnHeight, garbageHeight);
	boardScore += EvalTsd(game, columnHeight, garbageHeight + wellDepth);
	boardScore += EvalTst(game, columnHeight, garbageHeight + wellDepth, garbageHeight);

	boardScore += EvalPcBoard(game, garbageHeight);
	boardScore += EvalContBonus(game);

	int actionScore = EvalFinalAction(game);
	// パフェ直前地形を加算&記憶
	node->pcScore = EvalPrePcBoard(game, garbageHeight);
	actionScore += node->pcScore;

	{
		int fs = actionScore + static_cast<int>(boardScore * weight->GetNexus());

		node->familyScore = static_cast<int>(fs * trustRate);
	}
}
void EvaluateAI::CorrectScore(Node *node) {
	GameState *game = &node->game;

	int score = 0;

	if (game->others.combo > 0) {
		score = game->others.combo * weight->GetWeight(W::ComboDebt);
	}

	// 余剰スコアを削除
	node->score -= score;
	node->score -= node->pcScore;
}

// 高さ
int EvaluateAI::EvalHeight(GameState *game, int columnHeight[]) {

	int score = (game->board.stackHeight + 1) * weight->GetWeight(W::MaxHeight);
	if (game->board.stackHeight > 9) {
#if 0
		score += (game->board.stackHeight + 1 - 10) * weight->GetWeight(W::Over10);
		if (game->board.stackHeight > 14) {
			score += (game->board.stackHeight + 1 - 15) * weight->GetWeight(W::Over15);
		}
#else
		int center4Height = std::max(columnHeight[3], columnHeight[4]);
		center4Height = std::max(columnHeight[5], center4Height);
		center4Height = std::max(columnHeight[6], center4Height);
		center4Height = std::max(10, center4Height);
		score += (center4Height + 1 - 10) * weight->GetWeight(W::Over10);
		if (center4Height > 14) {
			score += (center4Height + 1 - 15) * weight->GetWeight(W::Over15);
		}
#endif
}

	return score;
}

// 直列
int EvaluateAI::EvalWellColumn(GameState *game, int columnHeight[], int garbageHeight, int *wellColumn, int *wellDepth) {

	auto well = [](int line) {
		if (Bit::PopCount(line) != 9) return -1;
		line += 1;
		return Bit::PopCount(line) - 1;
	};

	int line = game->board.line[garbageHeight + 1];

	int column = well(line);
	if (column == -1) return 0;
	*wellColumn = column;

	int depth = 1;
	for (int y = garbageHeight + 2; y < Board::HEIGHT; ++y) {
		if (game->board.line[y] != line) {
			break;
		}
		++depth;
	}
	*wellDepth = depth;

	int hint = 1;
	line = (0b1'01000'00000 >> column) & 0b11111'11111;
	for (int y = garbageHeight + 2; y < Board::HEIGHT; ++y) {
		if ((game->board.line[y] & line) != line) {
			break;
		}
		++hint;
	}

	auto wellPeak = [](int dep) {
		if (dep == 2) return 1;
		return std::min(dep, weight->GetWeight(W::WellPeak));
	};

	return wellPeak(hint) * weight->GetWeight(W::WellHint)
		+ wellPeak(depth) * weight->GetWeight(W::WellDepth)
		+ weight->GetColumn(column);
}

// でこぼこ
int EvaluateAI::EvalUpDown(GameState *game, int columnHeight[], int wellColumn, int wellDepth) {

	int updown2 = 0;
	int diff = 0;

	auto wellPeak = [](int dep) {
		return std::min(dep, weight->GetWeight(W::WellPeak));
	};

	if (wellColumn >= 0) {
		columnHeight[wellColumn] += wellPeak(wellDepth);
	}

	diff = columnHeight[0] - columnHeight[1];
	updown2 += diff * diff * (diff < 0 ? 2 : 1);
	for (int x = 1; x < Board::WIDTH - 2; ++x) {
		diff = columnHeight[x] - columnHeight[x + 1];
		updown2 += diff * diff;
	}
	diff = columnHeight[8] - columnHeight[9];
	updown2 += diff * diff * (diff > 0 ? 2 : 1);

	if (wellColumn >= 0) {
		columnHeight[wellColumn] -= wellPeak(wellDepth);
	}

	return updown2 * weight->GetWeight(W::UpDown2);
}

// 屋根
int EvaluateAI::EvalGround(GameState *game, int columnHeight[], int y0) {
	// 未完成なので消した
	return 0;
}

// 下穴
int EvaluateAI::EvalUnderground(GameState *game, int columnHeight[], int garbageHeight) {

	if (garbageHeight == -1) return 0;

	int cover = 0;
	int donateCover = 0;
	int prevBit = 0b11111'11111;
	int bit = 0;

	bit = game->board.line[garbageHeight] ^ 0b11111'11111;
	prevBit = bit;
	for (int x = 0; bit != 0; ) {
		x = bit & (-bit);
		int depth = columnHeight[Bit::Log2(x)] - garbageHeight;
		donateCover += std::min(depth, 5);
		bit &= ~x;
	}

	for (int y = garbageHeight - 1; y >= 0; --y) {
		bit = game->board.line[y] ^ 0b11111'11111;

		if ((bit & ~prevBit) != 0) {
			int x = bit & (-bit);
			if ((bit & ~x) == 0) {
				int depth = columnHeight[Bit::Log2(x)] - y;
				cover += std::min(depth, 5);
			}
			else {
				for (;;) {
					int depth = columnHeight[Bit::Log2(x)] - y;
					donateCover += depth;
					bit &= ~x;
					if (bit == 0) break;
					x = bit & (-bit);
				}
			}
		}
		prevBit = game->board.line[y] ^ 0b11111'11111;
	}

	return donateCover * weight->GetWeight(W::DonateCover)
		+ cover * weight->GetWeight(W::Cover);
}

// 穴バラとかリソースとか
int EvaluateAI::EvalResource(GameState *game, int columnHeight[], int garbageHeight) {

	auto well = [](int line) {
		if (Bit::PopCount(line) != 9) return -1;
		line += 1;
		return Bit::PopCount(line) - 1;
	};

	int anabara = 0;
	int resource = 0;
	int line = game->board.line[0];

	for (int y = 1; y <= garbageHeight; ++y) {
		if ((line | game->board.line[y]) != line) {
			++anabara;
		}
		line = game->board.line[y];
	}

	for (int y = garbageHeight + 1; y <= game->board.stackHeight; ++y) {
		resource += Bit::PopCount(game->board.line[y]);

		if (resource >= weight->GetWeight(W::ResourceMax)) {
			resource = weight->GetWeight(W::ResourceMax);
			break;
		}
	}

	line = 0;
	if (garbageHeight >= 0) {
		if (int x = well(game->board.line[garbageHeight]); x >= 0) {
			line = static_cast<int>(weight->GetColumn(x) * 0.5);
		}
	}

	if (UseS4W()) {
		// 未完成
		auto s4w = [game, garbageHeight]() {

			int h = 0;
			int wide = 0;
			for (int y = game->board.stackHeight, fill = 0; y > garbageHeight; --y) {
				int r = game->board.line[y];
				fill |= r;
				if (r == 0b11111'10000) {
					if (fill & 0b00000'01111) return 0;
					wide = 0b11111'10000;
					h = y;
					break;
				}
				if (r == 0b00001'11111) {
					if (fill & 0b11110'00000) return 0;
					wide = 0b00001'11111;
					h = y;
					break;
				}
				if (r == 0b11100'00111) {
					if (fill & 0b00011'11000) return 0;
					wide = 0b11100'00111;
					h = y;
					break;
				}
			}
			if (h == 0) return 0;
			int goal = wide != 0b11100'00111 ? 13 : 19;
			if (game->board.stackHeight + 1 > goal) return 0;
			{
				int y = h - 1;
				h = 1;
				for (; y > garbageHeight; --y) {
					if (game->board.line[y] != wide) {
						break;
					}
					h += 1;
				}
				int seed = 0;
				for (int bit = ~wide; y > garbageHeight; --y) {
					seed += Bit::PopCount(bit & game->board.line[y]);
				}
				if (seed < 2) return 0;
				if (seed == 5) h -= 1;
				if (h >= 4) {
					if (game->others.combo > 2) {
						h += game->others.combo;
					}
				}
			}
			int score = 0;
			if (wide == 0b11100'00111) {
				score += h * h * weight->GetWeight(W::UpDown2);
				if (garbageHeight < 2) {
					//h += 4;
					h += 4 - (garbageHeight + 1);
				}
				if (garbageHeight <= h) {
					score *= 2;
					if (h >= 4) {
						if (garbageHeight < 4) h -= 1;
						if (h >= 6) {
							if (garbageHeight < 4) h -= 1;
							if (h >= 8) {
								if (h >= 11) {
									if (h >= 12) {
										score += weight->GetComboTable(4);
									}
									score += weight->GetComboTable(3);
								}
								//else if (VersusAI::OpponentIsPowerfulBoard()) return 0;
								score += weight->GetComboTable(2);
							}
							else if (VersusAI::OpponentIsPowerfulBoard()) return 0;
							score += weight->GetComboTable(1);
						}
						else if (VersusAI::OpponentIsPowerfulBoard()) return 0;
						score += weight->GetComboTable(0);
					}
				}
			}
			else {
				if (garbageHeight <= h) {
					if (h >= 4) {
						if (h >= 6) {
							if (h >= 8) {
								if (h >= 11) {
									if (h >= 12) {
										score += weight->GetComboTable(4);
									}
									score += weight->GetComboTable(3);
								}
								else if (VersusAI::OpponentIsPowerfulBoard()) return 0;
								score += weight->GetComboTable(2);
							}
							else if (VersusAI::OpponentIsPowerfulBoard()) return 0;
							score += weight->GetComboTable(1);
						}
						else if (VersusAI::OpponentIsPowerfulBoard()) return 0;
						score += weight->GetComboTable(0);
					}
				}
			}

			return score;
		};

		line += s4w();
	}

	return line
		+ anabara * weight->GetWeight(W::Anabara)
		+ resource * weight->GetWeight(W::Resource);
}

int EvaluateAI::EvalTsd(GameState *game, int columnHeight[], int y0) {
	// 未完成なので消した
	return 0;
}

int EvaluateAI::EvalTst(GameState *game, int columnHeight[], int y0, int garbageHeight) {
	// 未完成なので消した

	int score = 0;

	if (score = EvalDTCannon(game, columnHeight, y0); score != 0) {
		return score;
	}

	return 0;
}

int EvaluateAI::EvalDTCannon(GameState *game, int columnHeight[], int y0) {
	// 未完成なので消した
	return 0;
}

// パフェ積み
int EvaluateAI::EvalPcBoard(GameState *game, int garbageHeight) {
	if (!isEvenBoard) return 0;

	// パフェ地形を意識させる関数
	// パリティ的なやつをざっくり計算したりする
	// 強すぎるので消した
	auto searchPcStack = [game]() {
		return 0;
	};

	if (ConfigAI::UsePCstack()) {
		if (int w = weight->GetWeight(W::PcStack); w != 0) {
			return searchPcStack() * w;
		}
	}

	return 0;
}

// パフェ直前地形
int EvaluateAI::EvalPrePcBoard(GameState *game, int garbageHeight) {
	if (!isEvenBoard) return 0;

	// あと1手でパフェが取れる地形を探す
	auto searchPc = [game](MinoType m) {
		if (m == MinoType::I) {
			if (game->board.stackHeight == 0) {
				for (int x = 0; x < Board::WIDTH - 3; ++x) {
					if ((game->board.line[0] ^ (0b11110'00000 >> x)) == 0b11111'11111) {
						return true;
					}
				}
			}
			else if (game->board.stackHeight == 3) {
				for (int x = 0; x < Board::WIDTH; ++x) {
					if ((game->board.line[3] ^ (0b10000'00000 >> x)) == 0b11111'11111) {
						x = game->board.line[3];
						if (game->board.line[2] != x) break;
						if (game->board.line[1] != x) break;
						if (game->board.line[0] != x) break;
						return true;
					}
				}
			}
		}
		else if (m == MinoType::J) {
			if (game->board.stackHeight == 1) {
				for (int x = 0; x < Board::WIDTH - 2; ++x) {
					if ((game->board.line[1] ^ (0b11100'00000 >> x)) == 0b11111'11111) {
						if ((game->board.line[0] ^ (0b00100'00000 >> x)) != 0b11111'11111) break;
						return true;
					}
				}
				for (int x = 0; x < Board::WIDTH - 2; ++x) {
					if ((game->board.line[0] ^ (0b11100'00000 >> x)) == 0b11111'11111) {
						if ((game->board.line[1] ^ (0b10000'00000 >> x)) != 0b11111'11111) break;
						return true;
					}
				}
			}
			else if (game->board.stackHeight == 2) {
				for (int x = 0; x < Board::WIDTH - 1; ++x) {
					if ((game->board.line[2] ^ (0b11000'00000 >> x)) == 0b11111'11111) {
						if ((game->board.line[1] ^ (0b10000'00000 >> x)) != 0b11111'11111) break;
						if ((game->board.line[0] ^ (0b10000'00000 >> x)) != 0b11111'11111) break;
						return true;
					}
				}
			}
		}
		else if (m == MinoType::L) {
			if (game->board.stackHeight == 1) {
				for (int x = 0; x < Board::WIDTH - 2; ++x) {
					if ((game->board.line[1] ^ (0b11100'00000 >> x)) == 0b11111'11111) {
						if ((game->board.line[0] ^ (0b10000'00000 >> x)) != 0b11111'11111) break;
						return true;
					}
				}
				for (int x = 0; x < Board::WIDTH - 2; ++x) {
					if ((game->board.line[0] ^ (0b11100'00000 >> x)) == 0b11111'11111) {
						if ((game->board.line[1] ^ (0b00100'00000 >> x)) != 0b11111'11111) break;
						return true;
					}
				}
			}
			else if (game->board.stackHeight == 2) {
				for (int x = 0; x < Board::WIDTH - 1; ++x) {
					if ((game->board.line[2] ^ (0b11000'00000 >> x)) == 0b11111'11111) {
						if ((game->board.line[1] ^ (0b01000'00000 >> x)) != 0b11111'11111) break;
						if ((game->board.line[0] ^ (0b01000'00000 >> x)) != 0b11111'11111) break;
						return true;
					}
				}
			}
		}
		else if (m == MinoType::O) {
			if (game->board.stackHeight == 1) {
				for (int x = 0; x < Board::WIDTH - 1; ++x) {
					if ((game->board.line[1] ^ (0b11000'00000 >> x)) == 0b11111'11111) {
						if ((game->board.line[0] ^ (0b11000'00000 >> x)) != 0b11111'11111) break;
						return true;
					}
				}
			}
		}
		else if (m == MinoType::T) {
			if (game->board.stackHeight == 1) {
				for (int x = 0; x < Board::WIDTH - 2; ++x) {
					if ((game->board.line[1] ^ (0b11100'00000 >> x)) == 0b11111'11111) {
						if ((game->board.line[0] ^ (0b01000'00000 >> x)) != 0b11111'11111) break;
						return true;
					}
				}
			}
		}
		else if (m == MinoType::S) {
			if (game->board.stackHeight == 1) {
				for (int x = 0; x < Board::WIDTH - 2; ++x) {
					if ((game->board.line[1] ^ (0b01100'00000 >> x)) == 0b11111'11111) {
						if ((game->board.line[0] ^ (0b11000'00000 >> x)) != 0b11111'11111) break;
						return true;
					}
				}
			}
		}
		else if (m == MinoType::Z) {
			if (game->board.stackHeight == 1) {
				for (int x = 0; x < Board::WIDTH - 2; ++x) {
					if ((game->board.line[1] ^ (0b11000'00000 >> x)) == 0b11111'11111) {
						if ((game->board.line[0] ^ (0b01100'00000 >> x)) != 0b11111'11111) break;
						return true;
					}
				}
			}
		}
		return false;
	};

	if (garbageHeight == -1) {
		if (searchPc(game->fallingMino.m)) return weight->GetWeight(W::PcChance);
		if (game->fallingMino.m != game->hold.m) {
			if (searchPc(game->hold.m)) return weight->GetWeight(W::PcChance);
		}
	}

	return 0;
}

// hold btb
int EvaluateAI::EvalContBonus(GameState *game) {
	int score = 0;

	switch (game->hold.m) {
	case MinoType::I:
		score += weight->GetWeight(W::HoldI);
		break;
	case MinoType::T:
		score += weight->GetWeight(W::HoldT);
		break;
	}

	if (game->others.btb > 0) {
		score += weight->GetWeight(W::Btb);
	}

	return score;
}

// 火力
int EvaluateAI::EvalFinalAction(GameState *game) {
	int score = 0;

	score += (game->commands.moveDelay >> 4) * weight->GetWeight(W::MoveDelay);

	switch (game->others.act) {
	case ClearAct::None:
		if (game->lockedMino.m == MinoType::T) {
			score += weight->GetWeight(W::WasteT);
		}
		else if (game->lockedMino.m == MinoType::I) {
			score += weight->GetWeight(W::WasteI);
		}
		// renが途切れたら前借りしたスコアを返却
		score += game->others.combo * weight->GetWeight(W::ComboDebt);
		return score;

	case ClearAct::Single:
		score += weight->GetWeight(W::Single);
		break;
	case ClearAct::Double:
		score += weight->GetWeight(W::Double);
		break;
	case ClearAct::Triple:
		score += weight->GetWeight(W::Triple);
		break;
	case ClearAct::Quad:
		score += weight->GetWeight(W::Quad);
		break;
	case ClearAct::Tss:
		score += weight->GetWeight(W::Tss);
		break;
	case ClearAct::Tsd:
		score += weight->GetWeight(W::Tsd);
		break;
	case ClearAct::Tst:
		score += weight->GetWeight(W::Tst);
		break;
	case ClearAct::Tsm:
		score += weight->GetWeight(W::Single) + weight->GetWeight(W::WasteT);
		score += -weight->GetWeight(W::Btb);
		break;
	case ClearAct::Pc:
		score += weight->GetWeight(W::Pc);
		score += (game->commands.moveDelay >> 5) * (-weight->GetWeight(W::MoveDelay));
		break;
	}

	if (game->others.btb == 0) {
		if (game->lockedMino.m == MinoType::T) {
			score += weight->GetWeight(W::WasteT);
		}
		else if (game->lockedMino.m == MinoType::I) {
			score += weight->GetWeight(W::WasteI);
		}
	}
	else if (game->others.btb > 1) {
		score += weight->GetWeight(W::ComboBtb);
		// まとめ打ちボーナス
		if (game->others.combo > 1) {
			score += weight->GetWeight(W::ComboBtb);
		}
	}

	// ren継続中は一定のスコアを前借りし続ける
	score += weight->GetWeight(W::ComboDebt);

	if (game->others.act != ClearAct::Pc) {
		auto cmb = game->others.combo;

		switch (cmb) {
		case 0:
		case 1:
		case 2:
			break;
		case 3:
		case 4:
			score += weight->GetComboTable(0);
			break;
		case 5:
		case 6:
			score += weight->GetComboTable(1);
			break;
		case 7:
		case 8:
			score += weight->GetComboTable(2);
			break;
		case 9:
		case 10:
		case 11:
			score += weight->GetComboTable(3);
			break;
		default:
			score += weight->GetComboTable(4);
			break;
		}
	}

	// 受けの立ち回り
	if (weight >= &weightList[2]) {
		int atk = 0;
		switch (game->others.act) {
		case ClearAct::Double:
			atk = 1;
			break;
		case ClearAct::Triple:
		case ClearAct::Tss:
			atk = 2;
			break;
		case ClearAct::Tsd:
		case ClearAct::Quad:
			atk = 4;
			break;
		case ClearAct::Tst:
			atk = 6;
			break;
		}
		if (game->others.btb > 1) {
			atk += 1;
		}
		if (atk > 0) {
			atk = std::min(atk, 5);
			score += weight->GetComboTable(atk - 1);
		}
	}

	return score;
}

void EvaluateAI::ReadEvalWeight() {

	playStyle = ConfigAI::PlayStyle();

	switch (playStyle) {
	case PlayStyleList::Vs:
		weightList[0].ReadWeight("we.dll");
		weightList[1].ReadWeight("wo.dll");
		weightList[2].ReadWeight("wd.dll");
		weightList[3].ReadWeight("wr.dll");
		break;
	case PlayStyleList::Ultra:
		weightList[0].ReadWeight("ultra1.dll");
		//weightList[1].ReadWeight("ultra2.dll");
		weightList[1] = weightList[0];
		break;
	case PlayStyleList::Sprint:
		weightList[0].ReadWeight("sprint.dll");
		weightList[1] = weightList[0];
		break;
	default:
		weightList[0].ReadWeight("we.dll");
		weightList[1].ReadWeight("wo.dll");
		weightList[2].ReadWeight("wd.dll");
		weightList[3].ReadWeight("wr.dll");
		break;
	}

	weight = &weightList[0];
}

void EvaluateAI::TransitionWeight(RootNode *searching) {
	// パラメーターの参照先を変えると動き変わるよね
	auto transitVs = [searching]() {
		int transitionHeight = 15;
		if (weight >= &weightList[2]) {
			transitionHeight = 13;
		}
		int center4Height = searching->game.board.stackHeight;
		int garbageHeight = -1;
		/*if (center4Height + 1 >= transitionHeight + 1)*/
		{
			int column[10] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
			garbageHeight = searching->game.board.GetColumnHeight(column);
			center4Height = std::max(column[3], column[4]);
			center4Height = std::max(column[5], center4Height);
			center4Height = std::max(column[6], center4Height);
		}
		if (center4Height + 1 <= transitionHeight) {
			if (VersusAI::MultiEvaluation() && VersusAI::GetOpponentsCombo() > 8) {
				if (searching->game.board.stackHeight + 1 >= (garbageHeight < 3 ? 11 : 6)) {
					weight = &weightList[3];
					return;
				}
			}
			if (searching->game.board.stackHeight + 1 >= 11) {
				weight = &weightList[0];
				return;
			}
			if (isEvenBoard) {
				if (VersusAI::MultiEvaluation() && (VersusAI::OpponentIsPCBoard() || VersusAI::GetOpponentsCombo() >= 3)) {
					if (weight == &weightList[1]) return;
					if (garbageHeight == -1 && searching->game.board.stackHeight + 1 <= 5) {
						weight = &weightList[1];
						return;
					}
				}
			}
			else {
				if (weight == &weightList[1]) return;
				if (garbageHeight == -1 && searching->game.board.stackHeight + 1 <= 5) {
					weight = &weightList[1];
					return;
				}
			}
			weight = &weightList[0];
		}
		else {
			if (VersusAI::MultiEvaluation() && VersusAI::GetOpponentsCombo() > 8) {
				weight = &weightList[3];
				return;
			}
			weight = &weightList[2];
		}
	};
	auto transitSolo = [searching]() {
		if (searching->game.board.stackHeight + 1 <= 10) {
			weight = &weightList[0];
		}
		else {
			weight = &weightList[1];
		}
	};

	switch (playStyle) {
	case PlayStyleList::Vs:
		transitVs(); return;
	case PlayStyleList::Ultra:
		transitSolo(); return;
	case PlayStyleList::Sprint:
		transitSolo(); return;
	default:
		transitVs(); return;
	}

}
void EvaluateAI::EvenOrOdd(RootNode *searching) {
	isEvenBoard = searching->game.board.IsEvenPop();
	weight = &weightList[2];
}
bool EvaluateAI::IsEvenBoard() {
	return isEvenBoard;
}
bool VersusAI::MultiEvaluation() {
	return multiEvaluation;
}
void VersusAI::MultiEvaluation(bool set) {
	multiEvaluation = set;
	opponentsCombo = 0;
	if (set) {
		opponentIsPCBoard = true;
		opponentIsPowerfulBoard = true;
	}
	else {
		opponentIsPCBoard = false;
		opponentIsPowerfulBoard = false;
	}
}
bool EvaluateAI::UseS4W() {
	return useS4W;
}
void EvaluateAI::UseS4W(bool set) {
	useS4W = set;
}
bool VersusAI::OpponentIsPCBoard() {
	return opponentIsPCBoard;
}
void VersusAI::OpponentIsPCBoard(Board *enemyBd) {
	if (enemyBd->stackHeight + 1 < 10) {
		opponentIsPCBoard = enemyBd->IsEvenPop();
		return;
	}
	opponentIsPCBoard = false;
}
void VersusAI::SetOpponentsCombo(int combo) {
	opponentsCombo = combo;
}
int VersusAI::GetOpponentsCombo() {
	return opponentsCombo;
}
bool VersusAI::OpponentIsPowerfulBoard() {
	return opponentIsPowerfulBoard;
}
void VersusAI::OpponentIsPowerfulBoard(Board *enemyBd, int combo, int garbage) {
	// true is powerful
	{
		if (combo >= 5) {
			opponentIsPowerfulBoard = true;
			return;
		}
		if (garbage >= 6) {
			opponentIsPowerfulBoard = true;
			return;
		}
	}
	enemyBd->UpdateStackHeight();
	int pop = enemyBd->PopCount();
	int line = 0;
	int same = 0;
	int bara = 0;
	int well = 0;
	for (int y = enemyBd->stackHeight; y >= 0; --y) {
		if ((enemyBd->line[y] | line) != enemyBd->line[y]) {
			++bara;
			well += same >= 4 ? same : 0;
			same = 0;
		}
		else if (line == enemyBd->line[y]) {
			same += 1;
		}
		line = enemyBd->line[y];
	}
	well += same >= 4 ? same : 0;
	if ((pop & 1) == 0) {
		if (enemyBd->stackHeight + 1 >= 10) {
			if (bara >= 4) {
				if (well <= 6) {
					opponentIsPowerfulBoard = false;
					return;
				}
			}
		}
	}
	else {
		if (well <= 6) {
			opponentIsPowerfulBoard = false;
			return;
		}
	}
	opponentIsPowerfulBoard = true;
}



void OffsetAI::UseTimingOffset(bool set) {
	useTimingOffset = set;
	ofsData = OffsetData();
	gbData = GarbageData();
}
bool OffsetAI::UseTimingOffset() {
	return useTimingOffset;
}
// ハードロ直前に呼ぶ
// 飛んできたお邪魔の量から相殺外しの判断をする
// こちらに向かっているお邪魔メモリ、もしくは事前に火力計算したお邪魔を使用
bool OffsetAI::CanWaitGarbage(int garbage) {
	// garbage >= 1

	// 相殺外しの準備ができていないならだめ
	if (!ofsData.canWaitGarbage) return false;

	// 死にそうならだめ
	if (ofsData.center4Height + garbage >= 15) return false;

	if (ofsData.garbageHeight >= 3) {
		if (VersusAI::GetOpponentsCombo() >= 9) {
			// 穴バラを受けないならだめ
			if (garbage - ofsData.atk != 1) return false;
			// 地形が高めならだめ
			if (ofsData.garbageHeight >= 7) return false;
		}
	}

	if (EvaluateAI::IsEvenBoard()) {
		if (ofsData.pcChance) {
			// パフェ見えてるならだめ
			if (ofsData.atk >= garbage) return false;
			//if (ofsData.atk > garbage) return false;
		}
	}

	if (ofsData.nearOffset) {
		if (VersusAI::GetOpponentsCombo() <= 8) {
			// 相手の火力が高いなら少し待ってもいい
			if (garbage >= 4) return true;
		}
	}

	// 穴バラ回避のためならよし
	if (ofsData.atk < garbage) return true;

	// カスみたいな火力は無視する
	if (garbage <= 2) return false;

	// それ以外はおけ
	return true;
}
// 探索結果から相殺外しの条件を準備しておく
void OffsetAI::DecisionOffset(RootNode *searched) {
	auto btbBonus = [searched]() {
		return searched->game.others.btb > 0 ? 1 : 0;
	};

	ofsData.canWaitGarbage = false;

	if (!useTimingOffset) return;

	//ofsData.center4Height = 20;
	ofsData.atk = 0;
	ofsData.pcChance = false;
	ofsData.nearOffset = false;
	//ofsData.holdOffset = false;

	// これからライン消去するなら外さない（待ってもいいけど）
	if (searched->result.plans.act[0] != ClearAct::None) {
		return;
	}

	if (EvaluateAI::IsEvenBoard()) {
		// 信頼区間 + hold(+1) までのパフェを調べる
		for (int i = 2, end = std::min(searched->next.trustEnd + 1, searched->result.depth - 1); i <= end; ++i) {
			if (searched->result.plans.act[i] == ClearAct::Pc) {
				ofsData.pcChance = true;
				break;
			}
		}
	}

	switch (searched->result.plans.act[1]) {
	case ClearAct::None:

		switch (searched->result.plans.act[2]) {
			//case ClearAct::None:
			//case ClearAct::Single:
			//case ClearAct::Double:
			//case ClearAct::Triple:
			//case ClearAct::Tsm:
			//case ClearAct::Pc:
			//	return;
		case ClearAct::Tss:
			// holdにもnextにも無いなら待つ必要はない
			// next1,2個目を調べたほうがいい?
			if (searched->game.hold.m != MinoType::T && searched->next.m[2] == MinoType::T) return;
			ofsData.atk = 2;
			break;
		case ClearAct::Quad:
			if (searched->game.hold.m != MinoType::I && searched->next.m[2] == MinoType::I) return;
			ofsData.atk = 4;
			break;
		case ClearAct::Tsd:
			if (searched->game.hold.m != MinoType::T && searched->next.m[2] == MinoType::T) return;
			ofsData.atk = 4;
			break;
		case ClearAct::Tst:
			if (searched->game.hold.m != MinoType::T && searched->next.m[2] == MinoType::T) return;
			ofsData.atk = 6;
			break;
		default:
			return;
		}

		ofsData.atk += btbBonus();
		ofsData.nearOffset = true;
		break;
	case ClearAct::Pc:
		return;
	case ClearAct::Single:
		if (ofsData.pcChance) {
			break;
		}
		return;
	case ClearAct::Double:
		ofsData.atk = 1;
		break;
	case ClearAct::Triple:
		ofsData.atk = 2;
		break;
	case ClearAct::Tss:
		ofsData.atk = 2 + btbBonus();
		break;
	case ClearAct::Quad:
	case ClearAct::Tsd:
		ofsData.atk = 4 + btbBonus();
		break;
	case ClearAct::Tst:
		ofsData.atk = 6 + btbBonus();
		break;
	case ClearAct::Tsm:
		if (btbBonus() == 1) {
			ofsData.atk = 1;
			break;
		}
		return;
	}

	int column[10] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
	ofsData.garbageHeight = searched->game.board.GetColumnHeight(column);
	ofsData.center4Height = std::max(column[3], column[4]);
	ofsData.center4Height = std::max(column[5], ofsData.center4Height);
	ofsData.center4Height = std::max(column[6], ofsData.center4Height);

	ofsData.canWaitGarbage = true;
}

void OffsetAI::ResetOpponentsAtk() {
	gbData.opponentsAtk = 0;
}
// 相手の盤面を見て事前に火力計算しておくもよし
void OffsetAI::CalcOpponentsAtk(int line, int btb, int btbBonus, int combo) {
	if (useTimingOffset == false) return;
	// tsm非対応
	if (line <= 1) return;
	// btb攻撃であるか
	if (btb == 0) return;
	// ren中は火力計算めんどい
	if (combo > 4) return;
	if (line == 4) {
		gbData.opponentsAtk = 4;
	}
	else {
		gbData.opponentsAtk = line * 2;
	}
	gbData.opponentsAtk += btbBonus;
	if (combo > 2)
		gbData.opponentsAtk += 1;
}
int OffsetAI::GetOpponentsAtk() {
	return gbData.opponentsAtk;
}
void OffsetAI::SetRemainingTime(ClearAct act) {
	auto remains = [act]() {
		switch (act) {
		case ClearAct::None:
			return 0;
		case ClearAct::Single:
		case ClearAct::Tss:
		case ClearAct::Tsm:
			return 15;
		case ClearAct::Double:
		case ClearAct::Triple:
		case ClearAct::Tsd:
		case ClearAct::Tst:
			return 10;
		case ClearAct::Quad:
			return 5;
		case ClearAct::Pc:
			return 21;
		}
		return 0;
	};
	gbData.remainingTime = remains();
}
// 新しいミノが出現した時に呼ぶ
// クールタイム中は相殺外しを禁止する
void OffsetAI::SetCoolTimeAfterClearing(int frame, int sendingGarbage, int lock) {
	if (gbData.remainingTime > 0) {
		gbData.coolTime = frame;
		gbData.coolTime += gbData.remainingTime;
		if (sendingGarbage > 0) {
			// 先打ちなら延長は必要なし
		}
		else {
			// 後打ちなら24f到着が遅れる
			gbData.coolTime += 24;
		}
		OpponentIsNewLockDown(lock);
		gbData.firstInvalidLock = true;
		InvalidateOpponentsAtk();
	}
}
// 以下の関数は未完成
bool OffsetAI::OpponentIsNewLockDown(int lock) {
	bool isNewLock = gbData.opponentsLockCount != lock;
	gbData.opponentsLockCount = lock;
	return isNewLock;
}
void OffsetAI::InvalidateOpponentsAtk() {
	gbData.opponentsAtk = -1;
}
bool OffsetAI::IsInCoolTime(int frame) {
	return gbData.coolTime > frame;
}
bool OffsetAI::IsFirstInvalidLockDown() {
	bool isFirst = gbData.firstInvalidLock;
	gbData.firstInvalidLock = false;
	return isFirst;
}