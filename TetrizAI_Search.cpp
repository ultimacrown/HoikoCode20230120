#include "TetrizAI_Search.h"

#include <cstring>
#include <algorithm>
#include <thread>
#include <atomic>
#include <vector>

#include "TetrizAI_Expand.h"
#include "Mutex.h"

constexpr int CORRECTED_BEAM_SCALEFACTOR = 10;
constexpr int CORRECTEDBEAMSIZE_PERTHREAD = BEAMSIZE_MAX_PERTHREAD * CORRECTED_BEAM_SCALEFACTOR;

class BeamSortNode {
	Node **bsNode;
	int size;
public:
	BeamSortNode() : bsNode(nullptr), size(0) {}
	BeamSortNode(int size) : bsNode(nullptr), size(0) {
		this->Alloc(size);
	}
	~BeamSortNode() {
		this->Dealloc();
	}
	BeamSortNode(const BeamSortNode &) = delete;
	BeamSortNode &operator=(const BeamSortNode &) = delete;
	BeamSortNode(BeamSortNode &&rhs) : bsNode(rhs.bsNode), size(rhs.size) {
		rhs.bsNode = nullptr;
	}
private:
	void Alloc(int size) {
		if (this->bsNode != nullptr) return;
		this->bsNode = new Node * [size * MAX_CHILDREN + MAX_BABYS];
	}
	void Dealloc() {
		if (this->bsNode == nullptr) return;
		delete[] this->bsNode;
		this->bsNode = nullptr;
	}
public:
	void ResizeZero() {
		this->size = 0;
	}
	int Size() {
		return this->size;
	}
	void Push(Node *node) {
		this->bsNode[this->size] = node;
		this->size += 1;
	}
	void Push(BeamSortNode *bsNode) {
		for (int i = 0; i < bsNode->size; ++i) {
			this->Push(&bsNode[i]);
		}
	}
	void PartialSortCopy(int mid, Node *partOfTree) {
		this->PartialSort(mid);
		for (int i = 0; i < mid; ++i) {
			partOfTree[i] = *this->bsNode[i];
			partOfTree[i].parentIndex = i;
		}
	}
	void PartialSort(int mid) {
		// ポインタをソートするだけなのでコピーコストが少なめ
		std::partial_sort(this->bsNode, this->bsNode + mid, this->bsNode + this->size, Node::Cmp());
		this->size = mid;
	}
	Node *MaxElement() {
		if (this->size <= 0) return nullptr;

		Node **result = this->bsNode;
		EvaluateAI::CorrectScore(*result);

		Node **first = result + 1;
		Node **last = result + this->size;
		for (; first != last; ++first) {
			EvaluateAI::CorrectScore(*first);
			if (Node::Cmp()(*first, *result)) {
				result = first;
			}
		}
		return *result;
	}
};

class SearchAI::Tree {
public:
	Node *GetGeneration(int depth) {
		// depthは1以上なので-1する
		return &this->familyTree[(depth - 1) * this->parentSize];
	}
	void SetLeafNode(int depth, int parentSize) {
		Node *parent = GetGeneration(depth);
		this->leafNode = parent;
		this->leafSize = parentSize;
	}
	void SelectLeafNode(Node **parent, int *parentSize) {
		if (params.threadCount <= 1) {
			*parent = this->leafNode;
			*parentSize = this->leafSize;
			this->leafSize = 0;
			return;
		}
		{
			// 最初の8割くらいを等分してもいいかも
			LockGuard lock(&this->mutex);
			*parent = this->leafNode;
			// まとめてノードを持って行ってもらうと排他制御の負荷が減る
			if (this->leafSize > 100) {
				this->leafNode += 10;
				this->leafSize -= 10;
				*parentSize = 10;
				return;
			}
			if (this->leafSize > 50) {
				this->leafNode += 5;
				this->leafSize -= 5;
				*parentSize = 5;
				return;
			}
			if (this->leafSize > 0) {
				this->leafNode += 1;
				this->leafSize -= 1;
				*parentSize = 1;
				return;
			}
		}
		parent = nullptr;
		*parentSize = 0;
	}
	Tree(int size) : familyTree(nullptr) {
		this->Alloc(size);
	}
	~Tree() {
		this->Dealloc();
	}
	Tree(const Tree &) = delete;
	Tree &operator=(const Tree &) = delete;
private:
	void Alloc(int size) {
		if (this->familyTree != nullptr) return;
		this->familyTree = new Node[size * MAX_DEPTH];
		this->parentSize = size;
	}
	void Dealloc() {
		if (this->familyTree == nullptr) return;
		delete[] this->familyTree;
		this->familyTree = nullptr;
	}
	Node *familyTree;
	int parentSize;
	Node *leafNode;
	int leafSize;
	Mutex mutex;
};

class SearchAI::ParallelSearchAI {
	static constexpr int ALLOC_SIZE = 300 * MAX_CHILDREN; // 大きめに確保するとdelete回数が減る
	static constexpr int ALLOC_BYTE = sizeof(Node) * ALLOC_SIZE;
	static constexpr int ALLOC_KBYTE = ALLOC_BYTE >> 10;
	static constexpr int ALLOC_MBYTE = ALLOC_KBYTE >> 10;
	static constexpr int VECSIZE = 2 * CORRECTEDBEAMSIZE_PERTHREAD * MAX_CHILDREN / ALLOC_SIZE;
public:
	Tree *tree;
	std::thread parallelThread;
	ExpandAI expandAI;
	bool isRunning;
	bool canBreakTask;
	class NodeVector {
		Node *node;
		int size;
	public:
		NodeVector() : node(nullptr), size(0) {}
		~NodeVector() {
			this->Delete();
		}
		Node *Back() {
			return &this->node[this->size];
		}
		Node *Front() {
			return &this->node[0];
		}
		Node *At(int i) {
			return &this->node[i];
		}
		int Size() {
			return this->size;
		}
		bool IsFull() {
			return this->size > std::max(ALLOC_SIZE - MAX_BABYS, 0);
		}
		bool IsEmpty() {
			return this->node == nullptr;
		}
		void AddSize(int addSize) {
			this->size += addSize;
		}
		void Clear() {
			this->size = 0;
		}
		void Alloc() {
			if (this->node != nullptr) return;
			this->node = new Node[ALLOC_SIZE];
		}
		void Delete() {
			if (this->node == nullptr) return;
			delete[] this->node;
			this->node = nullptr;
			this->size = 0;
		}
	};
	class NodeVectorCollection {
		// vecsizeが足りないとやばい
		NodeVector nodeVector[VECSIZE];
		NodeVector *myVector;
		int size;
	public:
		int capacity;
		NodeVectorCollection() : myVector(nullptr), size(0), capacity(0) {}

		NodeVector *Back() {
			if (this->myVector->IsEmpty()) {
				this->myVector->Alloc();
				this->capacity += 1;
				return this->myVector;
			}

			if (this->myVector->IsFull()) {
				++this->myVector;
				this->size += 1;
				return this->Back();
			}

			return this->myVector;
		}
		NodeVector *Front() {
			return &this->nodeVector[0];
		}
		NodeVector *At(int i) {
			return &this->nodeVector[i];
		}
		int Size() {
			return this->size;
		}
		void StartUp() {
			for (int i = 0; i < VECSIZE; ++i) {
				this->nodeVector[i].Clear();
			}
			this->myVector = this->nodeVector;
			this->size = 1;
		}
		void MemClear() {
			for (int i = VECSIZE - 1; i >= 0; --i) {
				this->nodeVector[i].Delete();
			}
			this->myVector = nullptr;
			this->size = 0;
			this->capacity = 0;
		}
	};
	NodeVectorCollection nodeVectorCollection;
	enum class TaskType {
		Rest,
		Search,
		Template,
		Dealloc,
	};
	class TaskMessage {
		std::atomic<TaskType> atomicFlag;
	public:
		bool InTask() {
			return this->atomicFlag.load() != TaskType::Rest;
		}
		TaskType GetMessage() {
			return this->atomicFlag.load();
		}
		void Done() {
			this->atomicFlag = TaskType::Rest;
		}
		void TaskStart(TaskType myTask) {
			this->atomicFlag = myTask;
		}
	};
	TaskMessage taskMessage;
	void PoolLoop() {
		this->isRunning = true;
		while (this->isRunning) {
			if (isSearching == false) {
				// nanoはあかん
				std::this_thread::sleep_for(std::chrono::microseconds(1));
				// continueいるんかね
				continue;
			}
			auto msg = this->taskMessage.GetMessage();
			switch (msg) {
			case TaskType::Search:
				Expand();
				break;
			case TaskType::Dealloc:
				MemClear();
				break;
			}
		}
	}
public:
	ParallelSearchAI() {
		// do nothing
	}
	~ParallelSearchAI() {
		ThreadAbort();
	}
	void ThreadStart(int threadId) {
		this->taskMessage.Done();
		if (threadId == 0) {
			// メインスレッドなのでスレッドは作成しない
			ThreadAbort();
			return;
		}
		if (threadId + 1 > params.threadCount) {
			// threadCountを超えたスレッドは破棄する
			ThreadAbort();
			return;
		}

		if (this->parallelThread.joinable()) return;
		this->parallelThread = std::thread([this]() {this->PoolLoop(); });
	}
	void ThreadAbort() {
		this->isRunning = false;
		if (this->parallelThread.joinable()) {
			this->parallelThread.join();
		}
	}
	void WaitTask() {
		if (params.threadCount <= 1) return;

		for (;;) {
			if (this->taskMessage.InTask()) continue;
			return;
		}
	}
	int ConcatNewBeam(BeamSortNode *beamSortNode) {
		int count = 0;
		for (int i = 0; i < this->nodeVectorCollection.Size(); ++i) {
			auto *childNode = this->nodeVectorCollection.At(i);
			for (int j = 0; j < childNode->Size(); ++j) {
				beamSortNode->Push(childNode->At(j));
				count += 1;
			}
		}
		//return this->childCount;
		return count;
	}
	void SearchStart(Tree *tree, bool canBreak) {
		this->tree = tree;
		this->canBreakTask = canBreak;
		this->taskMessage.TaskStart(TaskType::Search);
		if (this->parallelThread.joinable()) return;

		Expand();
	}
	void DeallocStart() {
		this->taskMessage.TaskStart(TaskType::Dealloc);
		if (this->parallelThread.joinable()) return;

		MemClear();
	}
	ExpandAI *BorrowExpandAI() {
		return &this->expandAI;
	}
private:
	void MemClear() {
		this->nodeVectorCollection.MemClear();
		this->taskMessage.Done();
	}
	void Expand() {
		//this->childCount = 0;
		this->nodeVectorCollection.StartUp();

		for (;;) {
			Node *parentNode;
			int beamSize;
			// 親ノードと人数を取得
			this->tree->SelectLeafNode(&parentNode, &beamSize);

			for (int i = 0; i < beamSize; ++i) {
				// 空きノード配列をもらう
				auto *childNode = this->nodeVectorCollection.Back();

				parentNode[i].childCount = this->expandAI.ExpandNode(&parentNode[i].game, childNode->Back());

				Evaluate(childNode->Back(), &parentNode[i]);

				//this->childCount += parentNode[i].childCount;
				childNode->AddSize(parentNode[i].childCount);

				if (this->canBreakTask && params.timer.TimeOver()) {
					parentNode = nullptr;
					break;
				}
			}
			if (parentNode == nullptr) break;
			if (beamSize == 0) break;
		}
		this->taskMessage.Done();
	}
	void Evaluate(Node *childNode, Node *parentNode) const {
		for (int i = parentNode->childCount - 1; i >= 0; --i) {
			//childNode[i].parentIndex = 0;
			childNode[i].parentNode = parentNode;
			EvaluateAI::Evaluate(&childNode[i]);
		}
	}
};

void SearchAI::Alloc(int threadNum) {
	EvaluateAI::Alloc();

	if (parallelSearchAI == nullptr) {
		parallelSearchAI = new ParallelSearchAI[threadNum];
	}
}
void SearchAI::Dealloc() {
	EvaluateAI::Dealloc();

	if (parallelSearchAI != nullptr) {
		delete[] parallelSearchAI;
		parallelSearchAI = nullptr;
	}
}
void SearchAI::ReadConfig() {

	params.timer.SetTimer(1000.0);

	params.basicBeamSize = ConfigAI::BeamSize();
	params.threadCount = ConfigAI::ThreadCount();
	params.lowerDepthLimit = ConfigAI::LowerDepthLimit();

	auto scale = params.CalcBeamScaleFactorFromThread(params.threadCount, ConfigAI::PhysicalCore());
	constexpr auto s = params.CalcBeamScaleFactorFromThread(4, 4);
	params.basicBeamSize = int(params.basicBeamSize * scale);
	params.correctedBeamSize = params.basicBeamSize * 3;

	params.upperDepthLimit = MAX_DEPTH;

	Alloc(params.threadCount);

	int t = params.threadCount;
	for (int i = 1; i < t; ++i) {
		parallelSearchAI[i].ThreadStart(i);
	}
}
void SearchAI::MemoryClear() {
	Dealloc();
}
void SearchAI::QuickSearch(RootNode *rootNode) {
	constexpr double F = 1000.0 / 60.0;
	constexpr double HARF = F * 7.0 * 0.5;
	params.timer.SetTimer(HARF);
	params.correctedBeamSize = static_cast<int>(params.basicBeamSize * 0.5);
	EvaluateAI::EvenOrOdd(rootNode);

	Search(rootNode);
}

bool SearchAI::IsSearching() {
	return isSearching;
}
void SearchAI::IsSearching(bool set) {
	isSearching = true;
	params.timer.Start();
}

void SearchAI::Search(RootNode *rootNode, RootNode *another) {
	// 開幕は2パターンの解を用意する
	// 実質next7なのでテンプレの判断が楽

	*another = *rootNode;
	IsSearching(true);
	Search(rootNode);

	ReadConfig();
	another->next.OpeningPredictionSwap();
	another->AdvanceMino();
	IsSearching(true);
	Search(another);
}


void SearchAI::Search(RootNode *rootNode) {

	Tree tree(params.correctedBeamSize);
	BeamSortNode beamSortNode(params.correctedBeamSize);

	int node = 0;
	{
		auto *expandAI = parallelSearchAI->BorrowExpandAI();
		rootNode->childCount = expandAI->ExpandNode(&rootNode->game, tree.GetGeneration(1));
	}

	if (rootNode->childCount <= 0) {
		rootNode->GameOver();
		isSearching = false;
		return;
	}
	node += rootNode->childCount;

	EvaluateAI::TransitionWeight(rootNode);
	EvaluateAI::SetTrustRate(0);

	{
		Node *firstGen = tree.GetGeneration(1);

		for (int i = 0; i < rootNode->childCount; ++i) {
			// 親ノードはrootNode
			firstGen[i].parentIndex = 0;
			firstGen[i].parentNode = nullptr;
			EvaluateAI::EvaluateFirst(&firstGen[i]);
		}
	}

	int depth = 1;

	for (int nextBeamSize = rootNode->childCount;;) {

		tree.SetLeafNode(depth, nextBeamSize);

		++depth;
		EvaluateAI::SetTrustRate(depth);


		for (int i = 1; i < params.threadCount; ++i) {
			parallelSearchAI[i].SearchStart(&tree, depth >= params.lowerDepthLimit + 1);
		}
		// [0]はメインスレッドで処理する
		parallelSearchAI[0].SearchStart(&tree, depth >= params.lowerDepthLimit + 1);

		beamSortNode.ResizeZero();
		for (int i = 0; i < params.threadCount; ++i) {
			parallelSearchAI[i].WaitTask();

			int dummy = parallelSearchAI[i].ConcatNewBeam(&beamSortNode);
		}


		node += beamSortNode.Size();

		if (depth >= params.upperDepthLimit ||
			(depth >= params.lowerDepthLimit && params.timer.TimeOver()) ||
			beamSortNode.Size() <= 0) {
			break;
		}

		{
			nextBeamSize = params.correctedBeamSize;
			if (depth > 7) nextBeamSize >>= 1;
			//if (depth > 14) nextBeamSize >>= 1;
			//if (depth > 21) nextBeamSize >>= 1;

			nextBeamSize = std::min(nextBeamSize, beamSortNode.Size());
		}

		{
			Node *nextGen = tree.GetGeneration(depth);
			beamSortNode.PartialSortCopy(nextBeamSize, nextGen);
		}
	}

	Node *resultNode = beamSortNode.MaxElement();
	if (resultNode == nullptr) {
		depth -= 1;
		resultNode = tree.GetGeneration(depth);
	}

	WriteResult(rootNode, resultNode, node, depth);

	params.AdjustParams(rootNode);

	for (int i = 1; i < params.threadCount; ++i) {
		parallelSearchAI[i].DeallocStart();
	}
	// [0]はメインスレッドで処理する
	parallelSearchAI[0].DeallocStart();

	for (int i = 0; i < params.threadCount; ++i) {
		// 次の探索で待つのもあり
		parallelSearchAI[i].WaitTask();
	}

	// どこで探索終了にしようかね
	isSearching = false;
}

void SearchAI::WriteResult(RootNode *rootNode, Node *resultNode, int node, int depth) {
	rootNode->result.node = node;
	rootNode->result.depth = depth;

	rootNode->result.plans.Clear();

	depth -= 1;

	int ren = 0;

	for (; depth >= 1; --depth) {
		rootNode->result.plans.mino[depth] = resultNode->game.lockedMino;
		rootNode->result.plans.act[depth] = resultNode->game.others.act;

		if (resultNode->game.others.combo > 0) {
			ren = std::max(ren, resultNode->game.others.combo);
		}
		else {
			ren = 0;
		}

		if (resultNode->game.others.combo < 0) {
			rootNode->result.plans.drawingBreak = depth;
		}

		resultNode = resultNode->parentNode;
	}

	rootNode->game = resultNode->game;
	rootNode->game.commands.CorrectDelay();
	rootNode->result.plans.mino[0] = resultNode->game.lockedMino;
	rootNode->result.plans.act[0] = resultNode->game.others.act;

	{
		int i = rootNode->result.plans.drawingBreak - 1;
		if (rootNode->result.plans.act[i] == ClearAct::Pc) {
			rootNode->result.plans.drawingBreak = i + 1;
		}
		else {
			for (; i < rootNode->result.depth; ++i) {
				if (rootNode->result.plans.act[i] == ClearAct::Pc) {
					rootNode->result.plans.drawingBreak = i + 1;
					break;
				}
			}
		}
	}

	ren = std::max(ren, resultNode->game.others.combo);
	ren = resultNode->game.others.combo > 0 ? ren - 1 : 0;
	rootNode->result.ren = ren;


	double elapsed = params.timer.Elapsed();
	rootNode->result.overTime = elapsed - params.timer.TimeLimit();
	rootNode->result.searchTime = elapsed;
	rootNode->result.basicBeamSize = params.basicBeamSize;
	rootNode->result.correctedBeamSize = params.correctedBeamSize;

}

void SearchAI::SearchParams::AdjustParams(RootNode *rootNode) {
	constexpr double F = 1000.0 / 60.0;
	constexpr double MIN_DELAY = 1.0 / (F * 7.0);

	if (rootNode->result.overTime > 0.0) {
		int diff = MAX_DEPTH - rootNode->result.depth;
		this->basicBeamSize -= diff * 10;
		this->basicBeamSize = std::max(this->basicBeamSize, BEAMSIZE_MIN);
	}
	else if (rootNode->result.overTime < -(F * 2.0) && timer.TimeLimit() < 336.0) {
		int diff = -static_cast<int>((rootNode->result.overTime + F) * (1.0 / F));
		this->basicBeamSize += diff * 10;
		this->basicBeamSize = std::max(this->basicBeamSize, BEAMSIZE_MIN);
	}

	double timeLimit = rootNode->GetDelay() - ConfigAI::TakeOverTime();
	this->timer.SetTimer(timeLimit);
	this->correctedBeamSize = static_cast<int>(this->basicBeamSize * timeLimit * MIN_DELAY);
	this->correctedBeamSize = std::min(this->correctedBeamSize, this->basicBeamSize * CORRECTED_BEAM_SCALEFACTOR);

	if (!rootNode->game.IsSafe()) {
		this->correctedBeamSize = static_cast<int>(this->correctedBeamSize * 0.8);
	}

	this->upperDepthLimit = MAX_DEPTH;
}