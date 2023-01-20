#pragma once

//#include "TetrizAI_Common.h"
//#include "TetrizAI_Expand.h"
#include "TetrizAI_Evaluate.h"
#include "Timer.h"

class ExpandAI;

class SearchAI {
public:
	static void Search(RootNode *rootNode, RootNode *another);
	static void Search(RootNode *rootNode);
	static void ReadConfig();
	static void QuickSearch(RootNode *rootNode);
	static bool IsSearching();
	static void IsSearching(bool set);
	static void MemoryClear();
private:
	static void Alloc(int threadNum);
	static void Dealloc();

	static void WriteResult(RootNode *rootNode, Node *resultNode, int node, int depth);

	class SearchParams {
	public:
		int targetDepth;
		int upperDepthLimit;
		int lowerDepthLimit;

		int basicBeamSize;
		int correctedBeamSize;

		int threadCount;
		ChronoTimer timer;

		void AdjustParams(RootNode *rootNode);
		static constexpr double CalcBeamScaleFactorFromThread(double th, double thMax) {
			if (th == 1.0) return 1.0;

			auto scale = (1.0 - 0.6) / (thMax);

			return th * (1.0 - th * scale);
		};
	};

	static inline SearchParams params;

	class Tree;
	//static inline Tree *tree = nullptr;

	class ParallelSearchAI;
	static inline ParallelSearchAI *parallelSearchAI;

	static inline bool isSearching;
};
