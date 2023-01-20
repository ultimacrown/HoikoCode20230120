#pragma once

class MinoPos {
public:
	int x;
	int y;
};
class MinoRect {
public:
	int top;
	int right;
	int bottom;
	int left;
};
class MinoBit {
public:
	int line[4];
};
class MinoCoord {
public:
	MinoPos p[4];
};

class MinoShape {
public:
	const MinoRect rect;
	const MinoBit bit;
	const MinoCoord coord;
	inline int At(const int x, const int y) const {
		// -2 <= x <= 8
		return (this->bit.line[y] << 8) >> (x + 2);
	}
};

class Srs {
public:
	const MinoPos test[4];
};

class MinoConst {
public:
	static const MinoShape *GetShape(int m, int r);
	static const Srs *GetSrsA(int m, int r);
	static const Srs *GetSrsB(int m, int r);
};