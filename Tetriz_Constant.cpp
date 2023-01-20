#include "Tetriz_Constant.h"

#define MINORECT(a, b, c, d) {a, b, c, d}
#define MINOBIT(a, b, c, d) {a, b, c, d}
#define MINOCOORD(a, b, c, d, e, f, g, h) {a , b, c, d, e, f, g, h}

constexpr MinoShape minoShape[7][4] = {
	// I
	{
		{MINORECT(1,3,1,0),
		MINOBIT(
		0b0000,
		0b1111,
		0b0000,
		0b0000),
		MINOCOORD(0,1 ,1,1 ,2,1 ,3,1)
		},
		{MINORECT(0,2,3,2),
		MINOBIT(
		0b0010,
		0b0010,
		0b0010,
		0b0010),
		MINOCOORD(2,0 ,2,1 ,2,2 ,2,3)
		},
		{MINORECT(2,3,2,0),
		MINOBIT(
		0b0000,
		0b0000,
		0b1111,
		0b0000),
		MINOCOORD(0,2 ,1,2 ,2,2 ,3,2)
		},
		{MINORECT(0,1,3,1),
		MINOBIT(
		0b0100,
		0b0100,
		0b0100,
		0b0100),
		MINOCOORD(1,0 ,1,1 ,1,2 ,1,3)
		}
	},
	// J
	{
		{MINORECT(0,2,1,0),
		MINOBIT(
		0b1000,
		0b1110,
		0b0000,
		0b0000),
		MINOCOORD(0,0 ,0,1 ,1,1 ,2,1)
		},
		{MINORECT(0,2,2,1),
		MINOBIT(
		0b0110,
		0b0100,
		0b0100,
		0b0000),
		MINOCOORD(1,0 ,2,0 ,1,1 ,1,2)
		},
		{MINORECT(1,2,2,0),
		MINOBIT(
		0b0000,
		0b1110,
		0b0010,
		0b0000),
		MINOCOORD(0,1 ,1,1 ,2,1 ,2,2)
		},
		{MINORECT(0,1,2,0),
		MINOBIT(
		0b0100,
		0b0100,
		0b1100,
		0b0000),
		MINOCOORD(1,0 ,1,1 ,0,2 ,1,2)
		}
	},
	// L
	{
		{MINORECT(0,2,1,0),
		MINOBIT(
		0b0010,
		0b1110,
		0b0000,
		0b0000),
		MINOCOORD(2,0 ,0,1 ,1,1 ,2,1)
		},
		{MINORECT(0,2,2,1),
		MINOBIT(
		0b0100,
		0b0100,
		0b0110,
		0b0000),
		MINOCOORD(1,0 ,1,1 ,1,2 ,2,2)
		},
		{MINORECT(1,2,2,0),
		MINOBIT(
		0b0000,
		0b1110,
		0b1000,
		0b0000),
		MINOCOORD(0,1 ,1,1 ,2,1 ,0,2)
		},
		{MINORECT(0,1,2,0),
		MINOBIT(
		0b1100,
		0b0100,
		0b0100,
		0b0000),
		MINOCOORD(0,0 ,1,0 ,1,1 ,1,2)
		}
	},
	// O
	{
		{MINORECT(0,2,1,1),
		MINOBIT(
		0b0110,
		0b0110,
		0b0000,
		0b0000),
		MINOCOORD(1,0 ,2,0 ,1,1 ,2,1)
		},
		{MINORECT(0,2,1,1),
		MINOBIT(
		0b0110,
		0b0110,
		0b0000,
		0b0000),
		MINOCOORD(1,0 ,2,0 ,1,1 ,2,1)
		},
		{MINORECT(0,2,1,1),
		MINOBIT(
		0b0110,
		0b0110,
		0b0000,
		0b0000),
		MINOCOORD(1,0 ,2,0 ,1,1 ,2,1)
		},
		{MINORECT(0,2,1,1),
		MINOBIT(
		0b0110,
		0b0110,
		0b0000,
		0b0000),
		MINOCOORD(1,0 ,2,0 ,1,1 ,2,1)
		},
	},
	// T
	{
		{MINORECT(0,2,1,0),
		MINOBIT(
		0b0100,
		0b1110,
		0b0000,
		0b0000),
		MINOCOORD(1,0 ,0,1 ,1,1 ,2,1)
		},
		{MINORECT(0,2,2,1),
		MINOBIT(
		0b0100,
		0b0110,
		0b0100,
		0b0000),
		MINOCOORD(1,0 ,1,1 ,2,1 ,1,2)
		},
		{MINORECT(1,2,2,0),
		MINOBIT(
		0b0000,
		0b1110,
		0b0100,
		0b0000),
		MINOCOORD(0,1 ,1,1 ,2,1 ,1,2)
		},
		{MINORECT(0,1,2,0),
		MINOBIT(
		0b0100,
		0b1100,
		0b0100,
		0b0000),
		MINOCOORD(1,0 ,0,1 ,1,1 ,1,2)
		}
	},
	// S
	{
		{MINORECT(0,2,1,0),
		MINOBIT(
		0b0110,
		0b1100,
		0b0000,
		0b0000),
		MINOCOORD(1,0 ,2,0 ,0,1 ,1,1)
		},
		{MINORECT(0,2,2,1),
		MINOBIT(
		0b0100,
		0b0110,
		0b0010,
		0b0000),
		MINOCOORD(1,0 ,1,1 ,2,1 ,2,2)
		},
		{MINORECT(1,2,2,0),
		MINOBIT(
		0b0000,
		0b0110,
		0b1100,
		0b0000),
		MINOCOORD(1,1 ,2,1 ,0,2 ,1,2)
		},
		{MINORECT(0,1,2,0),
		MINOBIT(
		0b1000,
		0b1100,
		0b0100,
		0b0000),
		MINOCOORD(0,0 ,0,1 ,1,1 ,1,2)
		}
	},
	// Z
	{
		{MINORECT(0,2,1,0),
		MINOBIT(
		0b1100,
		0b0110,
		0b0000,
		0b0000),
		MINOCOORD(0,0 ,1,0 ,1,1 ,2,1)
		},
		{MINORECT(0,2,2,1),
		MINOBIT(
		0b0010,
		0b0110,
		0b0100,
		0b0000),
		MINOCOORD(2,0 ,1,1 ,2,1 ,1,2)
		},
		{MINORECT(1,2,2,0),
		MINOBIT(
		0b0000,
		0b1100,
		0b0110,
		0b0000),
		MINOCOORD(0,1 ,1,1 ,1,2 ,2,2)
		},
		{MINORECT(0,1,2,0),
		MINOBIT(
		0b0100,
		0b1100,
		0b1000,
		0b0000),
		MINOCOORD(1,0 ,0,1 ,1,1 ,0,2)
		}
	}
};

constexpr Srs srsA[7][4] = {
{
	{-2,+0,  +1,+0,  +1,-2,  -2,+1}, // 3->0
	{-2,+0,  +1,+0,  -2,-1,  +1,+2}, // 0->1
	{-1,+0,  +2,+0,  -1,+2,  +2,-1}, // 1->2
	{+2,+0,  -1,+0,  +2,+1,  -1,-2}, // 2->3
},
{
	{-1,+0,  -1,-1,  +0,+2,  -1,+2}, // 3->0
	{-1,+0,  -1,+1,  +0,-2,  -1,-2}, // 0->1
	{+1,+0,  +1,-1,  +0,+2,  +1,+2}, // 1->2
	{+1,+0,  +1,+1,  +0,-2,  +1,-2}, // 2->3
},
{
	{-1,+0,  -1,-1,  +0,+2,  -1,+2}, // 3->0
	{-1,+0,  -1,+1,  +0,-2,  -1,-2}, // 0->1
	{+1,+0,  +1,-1,  +0,+2,  +1,+2}, // 1->2
	{+1,+0,  +1,+1,  +0,-2,  +1,-2}, // 2->3
},
{
	{-1,+0,  -1,-1,  +0,+2,  -1,+2}, // 3->0
	{-1,+0,  -1,+1,  +0,-2,  -1,-2}, // 0->1
	{+1,+0,  +1,-1,  +0,+2,  +1,+2}, // 1->2
	{+1,+0,  +1,+1,  +0,-2,  +1,-2}, // 2->3
},
{
	{-1,+0,  -1,-1,  +0,+2,  -1,+2}, // 3->0
	{-1,+0,  -1,+1,  +0,-2,  -1,-2}, // 0->1
	{+1,+0,  +1,-1,  +0,+2,  +1,+2}, // 1->2
	{+1,+0,  +1,+1,  +0,-2,  +1,-2}, // 2->3
},
{
	{-1,+0,  -1,-1,  +0,+2,  -1,+2}, // 3->0
	{-1,+0,  -1,+1,  +0,-2,  -1,-2}, // 0->1
	{+1,+0,  +1,-1,  +0,+2,  +1,+2}, // 1->2
	{+1,+0,  +1,+1,  +0,-2,  +1,-2}, // 2->3
},
{
	{-1,+0,  -1,-1,  +0,+2,  -1,+2}, // 3->0
	{-1,+0,  -1,+1,  +0,-2,  -1,-2}, // 0->1
	{+1,+0,  +1,-1,  +0,+2,  +1,+2}, // 1->2
	{+1,+0,  +1,+1,  +0,-2,  +1,-2}, // 2->3
}
};

constexpr Srs srsB[7][4] = {
{
	{+2,+0,  -1,+0,  +2,+1,  -1,-2}, // 1->0
	{+1,+0,  -2,+0,  +1,-2,  -2,+1}, // 2->1
	{+1,+0,  -2,+0,  -2,-1,  +1,+2}, // 3->2
	{-1,+0,  +2,+0,  -1,+2,  +2,-1}, // 0->3
},
{
	{+1,+0,  +1,-1,  +0,+2,  +1,+2}, // 1->0
	{-1,+0,  -1,+1,  +0,-2,  -1,-2}, // 2->1
	{-1,+0,  -1,-1,  +0,+2,  -1,+2}, // 3->2
	{+1,+0,  +1,+1,  +0,-2,  +1,-2}, // 0->3
},
{
	{+1,+0,  +1,-1,  +0,+2,  +1,+2}, // 1->0
	{-1,+0,  -1,+1,  +0,-2,  -1,-2}, // 2->1
	{-1,+0,  -1,-1,  +0,+2,  -1,+2}, // 3->2
	{+1,+0,  +1,+1,  +0,-2,  +1,-2}, // 0->3
},
{
	{+1,+0,  +1,-1,  +0,+2,  +1,+2}, // 1->0
	{-1,+0,  -1,+1,  +0,-2,  -1,-2}, // 2->1
	{-1,+0,  -1,-1,  +0,+2,  -1,+2}, // 3->2
	{+1,+0,  +1,+1,  +0,-2,  +1,-2}, // 0->3
},
{
	{+1,+0,  +1,-1,  +0,+2,  +1,+2}, // 1->0
	{-1,+0,  -1,+1,  +0,-2,  -1,-2}, // 2->1
	{-1,+0,  -1,-1,  +0,+2,  -1,+2}, // 3->2
	{+1,+0,  +1,+1,  +0,-2,  +1,-2}, // 0->3
},
{
	{+1,+0,  +1,-1,  +0,+2,  +1,+2}, // 1->0
	{-1,+0,  -1,+1,  +0,-2,  -1,-2}, // 2->1
	{-1,+0,  -1,-1,  +0,+2,  -1,+2}, // 3->2
	{+1,+0,  +1,+1,  +0,-2,  +1,-2}, // 0->3
},
{
	{+1,+0,  +1,-1,  +0,+2,  +1,+2}, // 1->0
	{-1,+0,  -1,+1,  +0,-2,  -1,-2}, // 2->1
	{-1,+0,  -1,-1,  +0,+2,  -1,+2}, // 3->2
	{+1,+0,  +1,+1,  +0,-2,  +1,-2}, // 0->3
}
};

const MinoShape *MinoConst::GetShape(int m, int r) {
	return &minoShape[m][r];
}
const Srs *MinoConst::GetSrsA(int m, int r) {
	return &srsA[m][r];
}
const Srs *MinoConst::GetSrsB(int m, int r) {
	return &srsB[m][r];
}
