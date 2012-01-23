#include "interpreter.h"
#include <wx/glcanvas.h>
#include <algorithm>

std::string val_names[] = {
    "nil",
    "number",
    "string",
    "procedure",
    "array"
    };

int edge_table[256] =
{
0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0
};

short tri_table[256][16] =
{{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
{3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},
{3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
{3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},
{9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
{2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},
{8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
{4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},
{3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},
{1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},
{4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},
{4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
{5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},
{2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},
{9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
{0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
{2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},
{10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},
{5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},
{5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},
{9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},
{1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},
{10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},
{8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},
{2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},
{7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},
{2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},
{11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},
{5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
{11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
{11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},
{9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},
{2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},
{6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},
{3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},
{6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
{10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},
{6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},
{8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},
{7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
{3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},
{0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},
{9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
{8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},
{5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
{0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
{6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},
{10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},
{10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},
{8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},
{1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},
{0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},
{10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},
{3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},
{6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
{9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},
{8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
{3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},
{6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},
{0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},
{10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},
{10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},
{2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
{7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},
{7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},
{2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
{1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
{11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},
{8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
{0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},
{7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
{10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
{2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
{6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},
{7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},
{2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},
{10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},
{10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},
{0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},
{7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},
{6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},
{8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},
{9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},
{6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},
{4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},
{10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
{8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},
{0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},
{1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},
{8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},
{10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},
{4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
{10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
{11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},
{9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
{6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},
{7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},
{3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
{7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},
{3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},
{6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
{9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},
{1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
{4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
{7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},
{6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
{3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},
{0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},
{6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},
{0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
{11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
{6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},
{5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},
{9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},
{1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
{1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
{10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},
{0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},
{5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},
{10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},
{11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},
{9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},
{7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
{2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},
{8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},
{9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},
{9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
{1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},
{9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},
{5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},
{0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},
{10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
{2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},
{0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
{0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
{9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},
{5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},
{3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
{5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},
{8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},
{0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},
{9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},
{1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},
{3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
{4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},
{9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
{11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},
{11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},
{2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},
{9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
{3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
{1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},
{4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
{3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},
{0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},
{1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};

double vertlist[12][3];
vert3f normallist[12];


interpreter::interpreter(std::map<std::string, dfuncd> funcs_, g_data data_)
{
    funcs = funcs_;
    data = data_;
    vars["mousex"] = data.mousex;
    vars["mousey"] = data.mousey;
    vars["time"] = data.time / 1000.0;
    vars["pi"] = 3.14159265358979323846264338327950288419716939937510;
    vars["e"] = 2.71828183;
}

interpreter::~interpreter()
{
    std::map<std::string, tagged_value>::iterator iter;
    for (iter = vars.begin(); iter != vars.end(); iter++)
        switch (iter->second.type)
        {
            case val_number:
                break;
            case val_string:
                //delete iter->second.val.str;
                break;
            case val_procedure:
                break;
            case val_array:
                break;
            default:
                break;
        }

    /*for (int i = 0; i < arrays.size(); i++)
    {
        std::cout << "Enumerating array " << i << ":\n";
        std::map<int, tagged_value>::iterator iter = arrays[i].begin();
        for(; iter != arrays[i].end(); iter++)
            std::cout << iter->first << ": (" << val_names[iter->second.type] << ") " << iter->second.val.str << "\n";

    }*/

}

void interpreter::getnextcolor()
{
    data.colorindex++;
    if (data.colorindex >= data.colors.size())
        data.colorindex = 0;
    data.currentcolor = data.colors[data.colorindex];
}

void interpreter::evaluate(block* blk)
{
    for(unsigned int i = 0; i < blk->statements.size(); i++)
    {
        evaluate(blk->statements[i]);
    }
}

void interpreter::evaluate(statement* stat)
{
   switch(stat->type)
    {
        case t_while:
            temp = evaluate(stat->stat.whilestat->cond);
            while (temp.type == val_number && temp.val.n > 0)
            {
                evaluate(stat->stat.whilestat->whileblock);
                temp = evaluate(stat->stat.whilestat->cond);
            }
            break;
        case t_for:
            int a, b;
            temp = evaluate(stat->stat.forstat->a);
            if (temp.type != val_number)
                throw(error("Error: attempt to use non-numeric value as loop bound"));
            a = temp.val.n;
            temp = evaluate(stat->stat.forstat->b);
            if (temp.type != val_number)
                throw(error("Error: attempt to use non-numeric value as loop bound"));
            b = temp.val.n;
            for (int i = a; i <= b; i++)
            {
                vars[stat->stat.forstat->id] = tagged_value((double)i);
                evaluate(stat->stat.forstat->forblock);
            }
            break;
        case t_if:
            temp = evaluate(stat->stat.ifstat->cond);
            if (temp.type == val_number && temp.val.n > 0)  //if first condition is true
            {
                evaluate(stat->stat.ifstat->ifblock);
            }
            else
            {
                bool blocked = false;
                for (unsigned int i = 0; i < stat->stat.ifstat->elseifs.size(); i++)     //for each elseif statement
                {
                temp = evaluate(stat->stat.ifstat->elseifs[i]->cond);
                    if (temp.type == val_number && temp.val.n > 0)  //if elseif condition is true
                    {
                        evaluate(stat->stat.ifstat->elseifs[i]->ifblock);        //evaluate block
                        blocked = true;
                        break;
                    }
                }
                if(!blocked && stat->stat.ifstat->haselse)                      //evaluate else block:
                    evaluate(stat->stat.ifstat->elseblock);
            }
            break;
        case t_let:
            if (stat->stat.assignstat->ismultiple)
            {
                int n_ass = stat->stat.assignstat->extra_lvalues.size();
                tagged_value singleval = evaluate(stat->stat.assignstat->rvalue);
                tagged_value *vals = new tagged_value[n_ass];
                for (int i = 0; i < n_ass; i++)
                {
                    vals[i] = evaluate(stat->stat.assignstat->extra_rvalues[i]);
                }
                vars[stat->stat.assignstat->lvalue.str] = singleval;
                for (int i = 0; i < n_ass; i++)
                {
                    if (!stat->stat.assignstat->extra_lvalues[i].isarray)
                        vars[stat->stat.assignstat->extra_lvalues[i].str] = vals[i];
                    else
                    {
                        temp = evaluate(stat->stat.assignstat->extra_lvalues[i].ai->array->arritem->array);
                        if (temp.type != val_array)
                             throw(error("Error: attempt to index non-array (" + val_names[temp.type] + ")"));
                        std::cout << "Assigning to array " << temp.val.arr << ", index " << evaluate(stat->stat.assignstat->extra_lvalues[i].ai->array->arritem->array).val.n << "\n";
                        arrays[temp.val.arr][evaluate(stat->stat.assignstat->extra_lvalues[i].ai->array->arritem->index).val.n] = vals[i];
                    }

                    vars[stat->stat.assignstat->extra_lvalues[i].str] = vals[i];
                }
                delete vals;
            }
            else
                if (!stat->stat.assignstat->lvalue.isarray)
                    vars[stat->stat.assignstat->lvalue.str] = evaluate(stat->stat.assignstat->rvalue);
                else
                {
                    temp = evaluate(stat->stat.assignstat->lvalue.ai->array->arritem->array);
                    if (temp.type != val_array)
                         throw(error("Error: attempt to index non-array (" + val_names[temp.type] + ")"));
                    arrays[temp.val.arr][evaluate(stat->stat.assignstat->lvalue.ai->array->arritem->index).val.n] = evaluate(stat->stat.assignstat->rvalue);
                }
            break;
        case t_func:
        {
            arglist_member* arglist_top;
            arglist_member* arglist_current;
            arglist_member* arglist_next;
            arglist_top = NULL;
            if (stat->stat.funcstat->args.size() > 0)
            {
                arglist_top = new arglist_member;
                arglist_top->v = evaluate(stat->stat.funcstat->args[0]);
                arglist_current = arglist_top;
            }
            for (unsigned int i = 1; i < stat->stat.funcstat->args.size(); i++)
            {
                arglist_next = new arglist_member;
                arglist_next->v = evaluate(stat->stat.funcstat->args[i]);
                arglist_current->next = arglist_next;
                arglist_current = arglist_next;
            }
            arglist_current->next = NULL;
            funcs[stat->stat.funcstat->name](arglist_top);
            if (arglist_top != NULL)
                delete arglist_top;
            break;
        }
        case s_plot_exp:
            evaluate(stat->stat.expplot);
            break;
        case s_plot_imp:
            evaluate(stat->stat.impplot);
            break;
        case t_def:
            std::cout << "Defining procedure " << stat->stat.defstat->name << "\n";
            proc =  new procedure(stat->stat.defstat->args, stat->stat.defstat->entrypoint);
            procedures.push_back(proc);
            vars[stat->stat.defstat->name] = tagged_value(proc);
            break;
        case n_procedure:
            if (vars[stat->stat.procstat->name].type != val_procedure) //not defined
                throw(n_procedure);
            proc = vars[stat->stat.procstat->name].val.proc;

            if(stat->stat.procstat->args.size() == proc->args.size())
            {
                for (unsigned int i = 0; i < proc->args.size(); i++)
                {
                    vars[proc->args[i]] = evaluate(stat->stat.procstat->args[i]);
                }
                try
                {
                    evaluate(proc->entrypoint);
                }
                catch (token_type_enum t)
                {
                    if (t != t_return)
                        throw(t);
                }
            }
            else
            {
                if (stat->stat.procstat->args.size() > proc->args.size())
                    throw(error(std::string("Error: too many arguments to procedure ") + stat->stat.procstat->name));
                else
                    throw(error(std::string("Error: too few arguments to procedure ") + stat->stat.procstat->name));
            }
            break;
        case t_return:
            returnvalue = evaluate(stat->stat.returnstat->expr);
            throw(t_return);
            break;
        case t_par:
            evaluate(stat->stat.parplot);
            break;
        default:
            throw(stat->type);
    }
}

double min(double a, double b)
{
    return a < b ? a : b;
}

double max(double a, double b)
{
    return a > b ? a : b;
}

tagged_value interpreter::evaluate(expression *expr)
{
    tagged_value rv = evaluate(expr->comparisons[0]);
    if (rv.type == val_number)
    {
        for(unsigned int i = 0; i < expr->operators.size(); i++)
        {
            temp = evaluate(expr->comparisons[i + 1]);
            if (temp.type != val_number)
                throw(error("Error: attempt to compare number with non-number."));
            if (expr->operators[i] == t_and)

                rv.val.n = min(rv.val.n, temp.val.n);
            else
                rv.val.n = max(rv.val.n, temp.val.n);
        }
    }
    return rv;
}

tagged_value interpreter::evaluate(comparison *comp)
{
    tagged_value rv = evaluate(comp->a);    // rv => return value;
    if (comp->oper == t_eof)
        return rv;
    temp = evaluate(comp->b);
    if (temp.type != val_number)
        throw(error("Error: attempt to compare number with non-number."));
    switch (comp->oper)
    {
        case t_lessthan:
            rv.val.n = temp.val.n - rv.val.n;
            break;
        case t_greaterthan:
            rv.val.n = rv.val.n - temp.val.n;
            break;
        case t_lteq:
            rv.val.n = temp.val.n - rv.val.n;
            if (rv.val.n == 0)
                rv.val.n = 0.00001;
            break;
        case t_gteq:
            rv.val.n = rv.val.n - temp.val.n;
            if (rv.val.n == 0)
                rv.val.n = 0.00001;
            break;
        case t_equals:
            rv.val.n = temp.val.n - rv.val.n - 100000;
            if(rv.val.n == -100000)
                rv.val.n = 0.00001;
            break;
        default:
            break;
    }
    return rv;
}

tagged_value interpreter::evaluate(sum* s)
{
    tagged_value rv = evaluate(s->terms[0]);
    if (rv.type != val_number)
        return rv;
    for(unsigned int i = 0; i < s->operators.size(); i++)
    {
        temp = evaluate(s->terms[i + 1]);
        if (temp.type != val_number)
            throw(error("Error: attempted to perform arithmetic on non-number."));
        if (s->operators[i] == t_plus)
            rv.val.n += temp.val.n;
        else
            rv.val.n -= temp.val.n;
    }
    return rv;
}

tagged_value interpreter::evaluate(term* t)
{
    tagged_value rv = evaluate(t->values[0]);
    if (rv.type != val_number)
        return rv;
    for(unsigned int i = 0; i < t->operators.size(); i++)
    {
        temp = evaluate(t->values[i + 1]);
        if (temp.type != val_number)
            throw(error("Error: attempted to perform arithmetic on non-number."));
        if (t->operators[i] == t_times)
            rv.val.n *= temp.val.n;
        else
            rv.val.n /= temp.val.n;
    }
    return rv;
}

tagged_value interpreter::evaluate(value *v)
{
    tagged_value rv;
    switch (v->type)
    {
        case t_number:
            rv.type = val_number;
            rv.val.n = v->n;
            break;
        case t_id:
            rv = vars[v->var];
            break;
        case t_func:
        {
            arglist_member* arglist_top;
            arglist_member* arglist_current;
            arglist_member* arglist_next;
            arglist_top = NULL;
            if (v->funccall->args.size() > 0)
            {
                arglist_top = new arglist_member;
                arglist_top->v = evaluate(v->funccall->args[0]);
                arglist_current = arglist_top;
            }
            for (unsigned int i = 1; i < v->funccall->args.size(); i++)
            {
                arglist_next = new arglist_member;
                arglist_next->v = evaluate(v->funccall->args[i]);
                arglist_current->next = arglist_next;
                arglist_current = arglist_next;
                arglist_current->next = NULL;
            }

            rv = funcs[v->funccall->name](arglist_top);
            if (arglist_top != NULL)
                delete arglist_top;
            break;
        }
        case n_expression:
            rv = evaluate(v->expr);
            break;
        case n_procedure:
        {
            std::map <std::string, tagged_value> temps;
            if (vars[v->proccall->name].type != val_procedure) //not defined
                throw(n_procedure);
            proc = vars[v->proccall->name].val.proc;

            if(v->proccall->args.size() == proc->args.size())
            {
                for (unsigned int i = 0; i < proc->args.size(); i++)
                {
                    temps[proc->args[i]] = vars[proc->args[i]];
                    vars[proc->args[i]] = evaluate(v->proccall->args[i]);
                }
                try
                {
                    evaluate(proc->entrypoint);
                }
                catch (token_type_enum t)
                {
                    if (t != t_return)
                        throw(t);
                    rv = returnvalue;
                }
                for (std::map <std::string, tagged_value>::iterator iter = temps.begin(); iter != temps.end(); iter++)
                {
                    vars[iter->first] = iter->second;
                }
            }
            else
            {
                if (v->proccall->args.size() > proc->args.size())
                    throw(error(std::string("Error: too many arguments to procedure ") + v->proccall->name));
                else
                    throw(error(std::string("Error: too few arguments to procedure ") + v->proccall->name));
            }
            break;
        }
        case t_lsquareb:
            rv = evaluate(v->arritem->array);
            if (rv.type == val_string)
            {
                rv = tagged_value(static_cast <double> (strings[rv.val.str][evaluate(v->arritem->index).val.n]));
            }
            else if (rv.type != val_array)
                throw(error("Error: attempt to index non-array"));
            else
                rv = arrays[rv.val.arr][floor(evaluate(v->arritem->index).val.n)];
            break;
        case t_lbrace:
            rv.type = val_array;
            rv.val.arr = arrays.size();
            arrays.push_back(std::map<int, tagged_value>());
            //std::cout << "Creating array " << rv.val.arr << ":\n";
            for (unsigned int i = 0; i < v->arrinit->explist.size(); i++)
            {
                temp = evaluate(v->arrinit->explist[i]);
                arrays[rv.val.arr][i].type = temp.type;
                arrays[rv.val.arr][i].val = temp.val;
                //std::cout << "Pushed " << val_names[arrays[rv.val.arr][i].type] << " to array " << rv.val.arr << "\n";
            }
            //std::cout << "Created array " << rv.val.arr << ", size " << v->arrinit->explist.size() << "\n";
            break;
        case t_dif:
            rv = evaluate(v->b);
            if (rv.type != val_number)
                throw(error("Error: attempt to differentiate non-numeric expression"));
            temp = vars[v->var];
            if (temp.type != val_number)
                throw(error("Error: attempt to differentiate with respect to non-numeric variable"));
            vars[v->var].val.n += 0.00001;
            rv.val.n = (evaluate(v->b).val.n - rv.val.n) / 0.00001;
            vars[v->var].val.n = temp.val.n;
            break;
        case t_string:
            rv.type = val_string;
            rv.val.str = addstring(v->var);
            break;
        default:
            //n = 0;
            break;
    }
    if (v->expd)
    {
        temp = evaluate(v->b);
        if (rv.type != val_number || temp.type != val_number)
             throw(error("Error: attempted to perform arithmetic on non-number."));
        rv.val.n = pow(rv.val.n, temp.val.n);
    }

    if (v->negative)
    {
        if (rv.type != val_number)
             throw(error("Error: attempted to perform arithmetic on non-number."));
        rv.val.n = -rv.val.n;
    }
    return rv;
}

void interpreter::evaluate(explicitplot* relation)
{
    setcolor(data.currentcolor);
    if (!data.is3d)
    {
        if (relation->rangevar == "y")
        {
            double x, lastx, lasty, step;
            tagged_value y;
            x = data.left;
            step = (data.right - data.left)/data.detail;
            vars["x"].type = val_number;
            vars["x"].val.n = x;
            y = evaluate(relation->expr);
            if (y.type != val_number)
                throw(error("Error: attempt to plot non-numeric expression"));
            while(x < data.right + step)
            {
                lastx = x;
                lasty = y.val.n;
                x += step;
                vars["x"].val.n = x;
                y = evaluate(relation->expr);
                line2(lastx, lasty, x, y.val.n);
            }
        }
        else if (relation->rangevar == "x")
        {
            double y, lastx, lasty, step;
            tagged_value x;
            y = data.bottom;
            step = (data.top - data.bottom)/data.detail;
            vars["y"].type = val_number;
            vars["y"].val.n = y;
            x = evaluate(relation->expr);
            if (x.type != val_number)
                throw(error("Error: attempt to plot non-numeric expression"));
            while(y < data.top + step)
            {
                lastx = x.val.n;
                lasty = y;
                y += step;
                vars["y"] = y;
                x = evaluate(relation->expr);
                line2(lastx, lasty, x.val.n, y);
            }
        }
        else if (relation->rangevar == "r")
        {
            double  theta, lastr, lasttheta, step;
            tagged_value r;
            theta = -10;
            step = 10.0/data.detail;
            vars["theta"].type = val_number;
            vars["theta"].val.n = theta;
            r = evaluate(relation->expr);
            if (r.type != val_number)
                throw(error("Error: attempt to plot non-numeric expression"));
            while(theta < 10 + step)
            {
                lastr = r.val.n;
                lasttheta = theta;
                theta += step;
                vars["theta"].val.n = theta;
                r = evaluate(relation->expr);
                line2(lastr * cos(lasttheta), lastr * sin(lasttheta), r.val.n * cos(theta), r.val.n * sin(theta));
            }
        }
        else
        {
            throw (e_ordinate);
        }
    }
    else
    {
        if (relation->rangevar == "y")
        {
            int ncells = data.detail / 2 + 1;
            double** grid = new double*[ncells + 3];
            for (int i = 0; i < ncells + 3; i++)
                grid[i] = new double[ncells + 3];             //    "fencing" vertex (+1) and 1 step padding on each side for normal calculation (+2).
            double stepx = (data.right - data.left) / ncells;
            double stepz = (data.front - data.back) / ncells;
            double x, z;
            vars["x"].type = val_number;
            vars["z"].type = val_number;
            if(evaluate(relation->expr).type != val_number)
                throw(error("Error: attempt to plot non-numeric expression"));
            x = data.left - stepx;
            for (int i = 0; i < ncells + 3; i++)
            {
                vars["x"].val.n = x;
                z = data.back - stepz;
                for(int j = 0; j < ncells + 3; j++)
                {
                    vars["z"].val.n = z;
                    grid[i][j] = evaluate(relation->expr).val.n;
                    z += stepz;
                }
                x += stepx;
            }
            vert3f** normals = new vert3f*[ncells + 1];
            for(int i = 0; i < ncells + 1; i++)
                normals[i] = new vert3f[ncells + 1];

            for(int i = 0; i <= ncells; i++)
                for(int j = 0; j <= ncells; j++)
                {
                    normals[i][j] = vert3f(
                                           (grid[i+2][j+1] - grid[i][j+1]) * stepz * 2,
                                           -(stepx * stepz * 4),
                                           (grid[i+1][j+2] - grid[i+1][j]) * stepx * 2
                                           );                                           //cross product of the two tangent vectors
                }
            double lastx = data.left;

            int lasti = 0;
            double lastz;
            int lastj;
            for (int i = 1; i <= ncells; i++)
            {
                x = lastx + stepx;
                lastj = 0;
                lastz = data.back;
                for(int j = 1; j <= ncells; j++)
                {
                    z = lastz + stepz;
                    glBegin(GL_POLYGON);
                    glNormal3f(normals[lasti][lastj].x, normals[lasti][lastj].y, normals[lasti][lastj].z);
                    glVertex3d(lastx, grid[lasti + 1][lastj + 1], lastz);   // +1 because of padding on left and bottom of grid (for normal calcs)
                    glNormal3f(normals[i][lastj].x, normals[i][lastj].y, normals[i][lastj].z);
                    glVertex3d(x, grid[i + 1][lastj + 1], lastz);
                    glNormal3f(normals[i][j].x, normals[i][j].y, normals[i][j].z);
                    glVertex3d(x, grid[i + 1][j + 1], z);
                    glNormal3f(normals[lasti][j].x, normals[lasti][j].y, normals[lasti][j].z);
                    glVertex3d(lastx, grid[lasti + 1][j + 1], z);
                    glEnd();
                    lastz = z;
                    lastj = j;
                }
                lastx = x;
                lasti = i;
            }
            for (int i = 0; i < ncells + 3; i++)
                delete[] grid[i];
            delete[] grid;
            for (int i = 0; i < ncells + 1; i++)
                delete[] normals[i];
            delete[] normals;
        }
        else if (relation->rangevar == "x")
        {
            int ncells = data.detail / 2 + 1;
            double** grid = new double*[ncells + 3];
            for (int i = 0; i < ncells + 3; i++)
                grid[i] = new double[ncells + 3];             //    "fencing" vertex (+1) and 1 step padding on each side for normal calculation (+2).
            double stepy = (data.top - data.bottom) / ncells;
            double stepz = (data.front - data.back) / ncells;
            double y, z;
            vars["y"].type = val_number;
            vars["z"].type = val_number;
            if(evaluate(relation->expr).type != val_number)
                throw(error("Error: attempt to plot non-numeric expression"));
            y = data.bottom - stepy;
            for (int i = 0; i < ncells + 3; i++)
            {
                vars["y"].val.n = y;
                z = data.back - stepz;
                for(int j = 0; j < ncells + 3; j++)
                {
                    vars["z"].val.n = z;
                    grid[i][j] = evaluate(relation->expr).val.n;
                    z += stepz;
                }
                y += stepy;
            }
            vert3f** normals = new vert3f*[ncells + 1];
            for(int i = 0; i < ncells + 1; i++)
                normals[i] = new vert3f[ncells + 1];

            for(int i = 0; i <= ncells; i++)
                for(int j = 0; j <= ncells; j++)
                {
                    normals[i][j] = vert3f(
                                           -(stepy * stepz * 4),
                                           (grid[i+2][j+1] - grid[i][j+1]) * stepz * 2,
                                           (grid[i+1][j+2] - grid[i+1][j]) * stepy * 2
                                           );                                           //cross product of the two tangent vectors
                }
            double lasty = data.bottom;

            int lasti = 0;
            double lastz;
            int lastj;
            for (int i = 1; i <= ncells; i++)
            {
                y = lasty + stepy;
                lastj = 0;
                lastz = data.back;
                for(int j = 1; j <= ncells; j++)
                {
                    z = lastz + stepz;
                    glBegin(GL_POLYGON);
                    glNormal3f(normals[lasti][lastj].x, normals[lasti][lastj].y, normals[lasti][lastj].z);
                    glVertex3d(grid[lasti + 1][lastj + 1], lasty, lastz);   // +1 because of padding on left and bottom of grid (for normal calcs)
                    glNormal3f(normals[i][lastj].x, normals[i][lastj].y, normals[i][lastj].z);
                    glVertex3d(grid[i + 1][lastj + 1], y, lastz);
                    glNormal3f(normals[i][j].x, normals[i][j].y, normals[i][j].z);
                    glVertex3d(grid[i + 1][j + 1], y, z);
                    glNormal3f(normals[lasti][j].x, normals[lasti][j].y, normals[lasti][j].z);
                    glVertex3d(grid[lasti + 1][j + 1], lasty, z);
                    glEnd();
                    lastz = z;
                    lastj = j;
                }
                lasty = y;
                lasti = i;
            }
            for (int i = 0; i < ncells + 3; i++)
                delete grid[i];
            delete grid;
            for (int i = 0; i < ncells + 1; i++)
                delete normals[i];
            delete normals;
        }
        else if (relation->rangevar == "z")
        {
            int ncells = data.detail / 2 + 1;
            double** grid = new double*[ncells + 3];
            for (int i = 0; i < ncells + 3; i++)
                grid[i] = new double[ncells + 3];             //    "fencing" vertex (+1) and 1 step padding on each side for normal calculation (+2).
            double stepx = (data.right - data.left) / ncells;
            double stepy = (data.top - data.bottom) / ncells;
            double x, y;
            vars["x"].type = val_number;
            vars["y"].type = val_number;
            if(evaluate(relation->expr).type != val_number)
                throw(error("Error: attempt to plot non-numeric expression"));
            x = data.left - stepx;
            for (int i = 0; i < ncells + 3; i++)
            {
                vars["x"].val.n = x;
                y = data.bottom - stepy;
                for(int j = 0; j < ncells + 3; j++)
                {
                    vars["y"].val.n = y;
                    grid[i][j] = evaluate(relation->expr).val.n;
                    y += stepy;
                }
                x += stepx;
            }
            vert3f** normals = new vert3f*[ncells + 1];
            for(int i = 0; i < ncells + 1; i++)
                normals[i] = new vert3f[ncells + 1];

            for(int i = 0; i <= ncells; i++)
                for(int j = 0; j <= ncells; j++)
                {
                    normals[i][j] = vert3f(
                                           (grid[i+2][j+1] - grid[i][j+1]) * stepy * 2,
                                           (grid[i+1][j+2] - grid[i+1][j]) * stepx * 2,
                                            -(stepx * stepy * 4)
                                          );                                           //cross product of the two tangent vectors
                }
            double lastx = data.left;

            int lasti = 0;
            double lasty;
            int lastj;
            for (int i = 1; i <= ncells; i++)
            {
                x = lastx + stepx;
                lastj = 0;
                lasty = data.bottom;
                for(int j = 1; j <= ncells; j++)
                {
                    y = lasty + stepy;
                    glBegin(GL_POLYGON);
                    glNormal3f(normals[lasti][lastj].x, normals[lasti][lastj].y, normals[lasti][lastj].z);
                    glVertex3d(lastx, lasty, grid[lasti + 1][lastj + 1]);   // +1 because of padding on left and bottom of grid (for normal calcs)
                    glNormal3f(normals[i][lastj].x, normals[i][lastj].y, normals[i][lastj].z);
                    glVertex3d(x, lasty, grid[i + 1][lastj + 1]);
                    glNormal3f(normals[i][j].x, normals[i][j].y, normals[i][j].z);
                    glVertex3d(x, y, grid[i + 1][j + 1]);
                    glNormal3f(normals[lasti][j].x, normals[lasti][j].y, normals[lasti][j].z);
                    glVertex3d(lastx, y, grid[lasti + 1][j + 1]);
                    glEnd();
                    lasty = y;
                    lastj = j;
                }
                lastx = x;
                lasti = i;
            }
            for (int i = 0; i < ncells + 3; i++)
                delete grid[i];
            delete grid;
            for (int i = 0; i < ncells + 1; i++)
                delete normals[i];
            delete normals;
        }
        else
        {
            throw (e_ordinate);
        }

    }
    getnextcolor();
}

void interpreter::evaluate(implicitplot* relation)
{
    if (!data.is3d)
    {
        bool equalsonly = relation->haseq && !relation->hasineq;
        int ncells = data.detail / 2 + 1;
        double** grid = new double*[ncells + 1];
        for (int i = 0; i <= ncells; i++)
            grid[i] = new double[ncells + 1];
        double stepx = (data.right - data.left) / (ncells - 1);
        double stepy = (data.top - data.bottom) / (ncells - 1);
        double x, y;
        if(evaluate(relation->expr).type != val_number)
            throw(error("Error: attempt to plot non-numeric expression"));
        vars["x"].type = val_number;
        vars["y"].type = val_number;
        x = floor(data.left/stepx) * stepx;
        for (int i = 0; i <= ncells; i++)
        {
            vars["x"].val.n = x;
            y = floor(data.bottom/stepy) * stepy;
            for(int j = 0; j <= ncells; j++)
            {
                vars["y"].val.n = y;
                grid[i][j] = evaluate(relation->expr).val.n;
                if (equalsonly)
                    grid[i][j] = (grid[i][j] == 0.00001 ? -100000 : grid[i][j]) + 100000;   //undo logical value munging.
                y += stepy;
            }
            x += stepx;
        }
        double lastx = floor(data.left/stepx) * stepx;
        x = lastx + stepx;
        int lasti = 0;
        double lasty;
        int lastj, mscase;

        if (!(relation->haseq||relation->hasineq))
        {
            for (int i = 1; i <= ncells; i++)
            {
                lastj = 0;
                lasty = floor(data.bottom/stepy) * stepy;
                y = lasty + stepy;
                for(int j = 1; j <= ncells; j++)
                {
                    {
                        rect2_4a(lastx, lasty, x, y, data.currentcolor, grid[lasti][lastj], grid[i][lastj], grid[lasti][j], grid[i][j]);
                    }
                    lasty = y;
                    y += stepy;
                    lastj = j;
                }
                lastx = x;
                x += stepx;
                lasti = i;
            }
        }
        else if (relation->haseq && !relation->hasineq)
        {
            setcolor(data.currentcolor);
            for (int i = 1; i <= ncells; i++)
            {
                lastj = 0;
                lasty = floor(data.bottom/stepy) * stepy;
                y = lasty + stepy;
                for(int j = 1; j <= ncells; j++)
                {
                    mscase = 0;
                    if (grid[lasti][lastj] > 0)
                        mscase |= 1;
                    if (grid[i][lastj] > 0)
                        mscase |= 2;
                    if (grid[i][j] > 0)
                        mscase |= 4;
                    if(grid[lasti][j] > 0)
                        mscase |= 8;
                    switch (mscase)
                    {
                        case 0:
                            break;
                        case 1:
                            line2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            break;
                        case 2:
                            line2(x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]), lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            break;
                        case 3:
                            line2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]));
                            break;
                        case 4:
                            line2(lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]));
                            break;
                        case 5:
                            std::cout << lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]) << "\n";
                            line2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            line2(lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]));
                            break;
                        case 6:
                            line2(lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty, lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y);
                            break;
                        case 7:
                            line2(lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]));
                            break;
                        case 15:
                            break;
                        case 14:
                            line2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            break;
                        case 13:
                            line2(x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]), lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            break;
                        case 12:
                            line2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]));
                            break;
                        case 11:
                            line2(lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]));
                            break;
                        case 10:
                            std::cout << lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]) << "\n";
                            line2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            line2(lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]));
                            break;
                        case 9:
                            line2(lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty, lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y);
                            break;
                        case 8:
                            line2(lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]));
                            break;
                        default:
                            break;
                    }
                    lasty = y;
                    y += stepy;
                    lastj = j;
                }
                lastx = x;
                x += stepx;
                lasti = i;
            }
        }
        else
        {
            setcolor(data.currentcolor, 0.5);
            for (int i = 1; i <= ncells; i++)
            {
                lastj = 0;
                lasty = floor(data.bottom/stepy) * stepy;
                y = lasty + stepy;
                for(int j = 1; j <= ncells; j++)
                {
                    mscase = 0;
                    if (grid[lasti][lastj] > 0)
                        mscase |= 1;
                    if (grid[i][lastj] > 0)
                        mscase |= 2;
                    if (grid[i][j] > 0)
                        mscase |= 4;
                    if(grid[lasti][j] > 0)
                        mscase |= 8;
                    switch (mscase)
                    {
                        case 0:
                            break;
                        case 1:
                            triangle2(lastx, lasty, lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            break;
                        case 2:
                            triangle2(x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]), x, lasty, lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            break;
                        case 3:
                            quad2(lastx, lasty, lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]), x, lasty);
                            break;
                        case 4:
                            triangle2(lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]), x, y);
                            break;
                        case 5:
                            triangle2(lastx, lasty, lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            triangle2(lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, x, y, x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]));
                            break;
                        case 6:
                            quad2(x, lasty, lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty, lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, x, y);
                            break;
                        case 7:
                            pentagon2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, x, y, x, lasty, lastx, lasty);
                            break;
                        case 15:
                            quad2(lastx, lasty, lastx, y, x, y, x, lasty);
                            break;
                        case 14:
                            pentagon2(lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty, lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx, y, x, y, x, lasty);
                            break;
                        case 13:
                            pentagon2(x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]), lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty, lastx, lasty, lastx, y, x, y);
                            break;
                        case 12:
                            quad2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx, y, x, y, x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]));
                            break;
                        case 11:
                            pentagon2(x, lasty, lastx, lasty, lastx, y, lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]));
                            break;
                        case 10:
                            std::cout << lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]) << "\n";
                            triangle2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx, y, lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y);
                            triangle2(lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty, x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]), x, lasty);
                            break;
                        case 9:
                            quad2(lastx, lasty, lastx, y, lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            break;
                        case 8:
                            triangle2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx, y, lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y);
                            break;
                        default:
                            break;
                    }
                    lasty = y;
                    y += stepy;
                    lastj = j;
                }
                lastx = x;
                x += stepx;
                lasti = i;
            }
        }
        for (int i = 0; i <= ncells; i++)
            delete grid[i];
        delete grid;
    }
    else
    {
        bool equalsonly = relation->haseq && !relation->hasineq;
        int ncells = data.detail / 6 + 1;
        double*** grid = new double**[ncells + 3];
        for (int i = 0; i < ncells + 3; i++)
        {
            grid[i] = new double*[ncells + 3];
            for(int j = 0; j < ncells + 3; j++)
                grid[i][j] = new double[ncells + 3];
        }

        double stepx = (data.right - data.left) / (ncells - 1);
        double stepy = (data.top - data.bottom) / (ncells - 1);
        double stepz = (data.front - data.back) / (ncells - 1);
        double x, y, z;
        if(evaluate(relation->expr).type != val_number)
            throw(error("Error: attempt to plot non-numeric expression"));
        vars["x"].type = val_number;
        vars["y"].type = val_number;
        vars["z"].type = val_number;
        x = floor(data.left/stepx) * stepx - stepx;
        for (int i = 0; i < ncells + 3; i++)
        {
            vars["x"].val.n = x;
            y = floor(data.bottom/stepy) * stepy - stepy;
            for(int j = 0; j < ncells + 3; j++)
            {
                vars["y"].val.n = y;
                z = floor(data.back/stepz) * stepz - stepz;
                for (int k = 0; k < ncells + 3; k++)
                {
                    vars["z"].val.n = z;
                    grid[i][j][k] = evaluate(relation->expr).val.n;
                    if (equalsonly)
                        grid[i][j][k] = (grid[i][j][k] == 0.00001 ? -100000 : grid[i][j][k]) + 100000;   //undo logical value munging.
                   z += stepz;
                }
                y += stepy;
            }
            x += stepx;
        }

        vert3f*** normals = new vert3f**[ncells + 1];
        for (int i = 0; i <= ncells; i++)
        {
            normals[i] = new vert3f*[ncells + 1];
            for(int j = 0; j <= ncells; j++)
                normals[i][j] = new vert3f[ncells + 1];
        }


        x = floor(data.left/stepx) * stepx;
        setcolor(data.currentcolor);
        if (!(relation->haseq || relation->hasineq))
        {
            for (int i = 2; i <= ncells + 1; i++)
            {
                y = floor(data.bottom/stepy) * stepy;
                for(int j = 2; j <= ncells + 1; j++)
                {
                    z = floor(data.back/stepz) * stepz;
                    for (int k = 2; k <= ncells + 1; k++)
                    {
                        if (grid[i][j][k] >= 0)
                        {
                            glBegin(GL_POINTS);
                            glVertex3f(x, y, z);
                            glEnd();
                        }
                       z += stepz;
                    }
                    y += stepy;
                }
                x += stepx;
            }
        }
        else
        {
            int mcase, edges;
            double lastx, lasty, lastz;
            int lasti, lastj, lastk;
            for (int i = 1; i <= ncells + 1; i++)
            {
                for(int j = 1; j <= ncells + 1; j++)
                {
                    for (int k = 1; k <= ncells + 1; k++)
                    {
                        normals[i - 1][j - 1][k - 1] = vert3f(
                                                              grid[i+1][j  ][k  ] - grid[i-1][j  ][k  ],
                                                              grid[i  ][j+1][k  ] - grid[i  ][j-1][k  ],
                                                              grid[i  ][j  ][k+1] - grid[i  ][j  ][k-1]
                                                              );
                    }
                }
            }
            glBegin(GL_TRIANGLES);
            lasti = 1;
            lastx = floor(data.left/stepx) * stepx;
            for (int i = 2; i < ncells + 1; i++)       // starts at 2 because we use the right vertex (so we can use "last" values), and there is a padding cell to the left used in normal calculations.
            {                                          // first sampled value is at 1; left edge, plus one padding (because lasti is initialized to 1)
                lastj = 1;
                x = lastx + stepx;
                lasty = floor(data.bottom/stepy) * stepy;
                for(int j = 2; j < ncells + 1; j++)
                {
                    lastk = 1;
                    y = lasty + stepy;
                    lastz = floor(data.back/stepz) * stepz;
                    for (int k = 2; k < ncells + 1; k++)
                    {
                        z = lastz + stepz;
                        mcase = 0;
                        if (grid[lasti][lastj][lastk] >= 0)
                            mcase |= 1;
                        if (grid[i][lastj][lastk] >= 0)
                            mcase |= 2;
                        if (grid[i][j][lastk] >= 0)
                            mcase |= 4;
                        if (grid[lasti][j][lastk] >= 0)
                            mcase |= 8;
                        if (grid[lasti][lastj][k] >= 0)
                            mcase |= 16;
                        if (grid[i][lastj][k] >= 0)
                            mcase |= 32;
                        if (grid[i][j][k] >= 0)
                            mcase |= 64;
                        if (grid[lasti][j][k] >= 0)
                            mcase |= 128;

                        edges = edge_table[mcase];
                        double t;

                        if (edges & 1)
                        {
                            t = grid[i][lastj][lastk] / (grid[i][lastj][lastk] - grid[lasti][lastj][lastk]);
                            vertlist[0][0] = x - t * stepx;
                            vertlist[0][1] = lasty;
                            vertlist[0][2] = lastz;
                            normallist[0] = normals[lasti][lastj][lastk] * t + normals[i][lastj][lastk] * (1 - t);
                        }
                        if (edges & 2)
                        {
                            t = grid[i][j][lastk] / (grid[i][j][lastk] - grid[i][lastj][lastk]);
                            vertlist[1][0] = x;
                            vertlist[1][1] = y - t * stepy;
                            vertlist[1][2] = lastz;
                            normallist[1] = normals[i][lastj][lastk] * t + normals[i][j][lastk] * (1 - t);
                       }
                        if (edges & 4)
                        {
                            t = grid[i][j][lastk] / (grid[i][j][lastk] - grid[lasti][j][lastk]);
                            vertlist[2][0] = x - t * stepx;
                            vertlist[2][1] = y;
                            vertlist[2][2] = lastz;
                            normallist[2] = normals[lasti][j][lastk] * t + normals[i][j][lastk] * (1 - t);
                        }
                        if (edges & 8)
                        {
                            t = grid[lasti][j][lastk] / (grid[lasti][j][lastk] - grid[lasti][lastj][lastk]);
                            vertlist[3][0] = lastx;
                            vertlist[3][1] = y - t * stepy;
                            vertlist[3][2] = lastz;
                            normallist[3] = normals[lasti][lastj][lastk] * t + normals[lasti][j][lastk] * (1 - t);
                        }
                        if (edges & 16)
                        {
                            t = grid[i][lastj][k] / (grid[i][lastj][k] - grid[lasti][lastj][k]);
                            vertlist[4][0] = x - t * stepx;
                            vertlist[4][1] = lasty;
                            vertlist[4][2] = z;
                            normallist[4] = normals[lasti][lastj][k] * t + normals[i][lastj][k] * (1 - t);
                        }
                        if (edges & 32)
                        {
                            t = grid[i][j][k] / (grid[i][j][k] - grid[i][lastj][k]);
                            vertlist[5][0] = x;
                            vertlist[5][1] = y - t * stepy;
                            vertlist[5][2] = z;
                            normallist[5] = normals[i][lastj][k] * t + normals[i][j][k] * (1 - t);
                        }
                        if (edges & 64)
                        {
                            t = grid[i][j][k] / (grid[i][j][k] - grid[lasti][j][k]);
                            vertlist[6][0] = x - t * stepx;
                            vertlist[6][1] = y;
                            vertlist[6][2] = z;
                            normallist[6] = normals[lasti][j][k] * t + normals[i][j][k] * (1 - t);
                        }
                        if (edges & 128)
                        {
                            t = grid[lasti][j][k] / (grid[lasti][j][k] - grid[lasti][lastj][k]);
                            vertlist[7][0] = lastx;
                            vertlist[7][1] = y - t * stepy;
                            vertlist[7][2] = z;
                            normallist[7] = normals[lasti][lastj][k] * t + normals[lasti][j][k] * (1 - t);
                        }
                        if (edges & 256)
                        {
                            t = grid[lasti][lastj][k] / (grid[lasti][lastj][k] - grid[lasti][lastj][lastk]);
                            vertlist[8][0] = lastx;
                            vertlist[8][1] = lasty;
                            vertlist[8][2] = z - t * stepz;
                            normallist[8] = normals[lasti][lastj][lastk] * t + normals[lasti][lastj][k] * (1 - t);
                        }
                        if (edges & 512)
                        {
                            t = grid[i][lastj][k] / (grid[i][lastj][k] - grid[i][lastj][lastk]);
                            vertlist[9][0] = x;
                            vertlist[9][1] = lasty;
                            vertlist[9][2] = z - t * stepz;
                            normallist[9] = normals[i][lastj][lastk] * t + normals[i][lastj][k] * (1 - t);
                        }
                        if (edges & 1024)
                        {
                            t = grid[i][j][k] / (grid[i][j][k] - grid[i][j][lastk]);
                            vertlist[10][0] = x;
                            vertlist[10][1] = y;
                            vertlist[10][2] = z - t * stepz;
                            normallist[10] = normals[i][j][lastk] * t + normals[i][j][k] * (1 - t);
                        }
                        if (edges & 2048)
                        {
                            t = grid[lasti][j][k] / (grid[lasti][j][k] - grid[lasti][j][lastk]);
                            vertlist[11][0] = lastx;
                            vertlist[11][1] = y;
                            vertlist[11][2] = z - grid[lasti][j][k] / (grid[lasti][j][k] - grid[lasti][j][lastk]) * stepz;
                            normallist[11] = normals[lasti][j][lastk] * t + normals[lasti][j][k] * (1 - t);
                        }

                        short* trilist = tri_table[mcase];
                        for (int i = 0; trilist[i] != -1; i += 3)
                        {
                            glNormal3f(normallist[trilist[i  ]].x,normallist[trilist[i  ]].y,normallist[trilist[i  ]].z);
                            glVertex3f(  vertlist[trilist[i  ]][0], vertlist[trilist[i  ]][1], vertlist[trilist[i  ]][2]);
                            glNormal3f(normallist[trilist[i+1]].x,normallist[trilist[i+1]].y,normallist[trilist[i+1]].z);
                            glVertex3f(  vertlist[trilist[i+1]][0], vertlist[trilist[i+1]][1], vertlist[trilist[i+1]][2]);
                            glNormal3f(normallist[trilist[i+2]].x,normallist[trilist[i+2]].y,normallist[trilist[i+2]].z);
                            glVertex3f(  vertlist[trilist[i+2]][0], vertlist[trilist[i+2]][1], vertlist[trilist[i+2]][2]);
                        }
                        lastz = z;
                        lastk = k;
                    }
                    lasty = y;
                    lastj = j;
                }
                lastx = x;
                lasti = i;
            }
        }
        glEnd();
        for (int i = 0; i < ncells + 3; i++)
        {
            for(int j = 0; j < ncells + 3; j++)
                delete grid[i][j];
            delete grid[i];
        }
        delete grid;

        for (int i = 0; i < ncells + 1; i++)
        {
            for(int j = 0; j < ncells + 1; j++)
                delete normals[i][j];
            delete normals[i];
        }
        delete normals;

    }
    getnextcolor();

}

void interpreter::evaluate(parametricplot* parp)
{
    setcolor(data.currentcolor);
    double from, to, step;
    if (parp->givenfrom)
    {
        if (evaluate(parp->from).type != val_number)
            throw(error("Error: cannot have non-numeric interval bound"));
        from = evaluate(parp->from).val.n;
    }
    else
        from = -1;

    if (parp->givento)
    {
        if (evaluate(parp->to).type != val_number)
            throw(error("Error: cannot have non-numeric interval bound"));
        to = evaluate(parp->to).val.n;
    }
    else
        to = 1;

    if (parp->givenstep)
    {
        if (evaluate(parp->step).type != val_number)
            throw(error("Error: cannot have non-numeric step size"));
        step = max(evaluate(parp->step).val.n, (to - from) / data.detail / 100);
    }
    else
        step = (to - from) / data.detail;

    int nassignments = parp->assignments.size();
    double t = from;
    vars[parp->parname] = t;
    if (!data.is3d)
    {
        for(int i = 0; i < nassignments; i++)
        {
            vars[parp->assignments[i]->lvalue.str] = evaluate(parp->assignments[i]->rvalue);
            if (vars[parp->assignments[i]->lvalue.str].type != val_number)
                throw(error("Error: attempt to plot non-numeric expression"));
        }
        double lastx = vars["x"].val.n;
        double lasty = vars["y"].val.n;
        double x, y;
        t = t + step;
        for(; t < to + step; t += step)
        {
            vars[parp->parname] = t;
            for(int i = 0; i < nassignments; i++)
                vars[parp->assignments[i]->lvalue.str] = evaluate(parp->assignments[i]->rvalue);
            x = vars["x"].val.n;
            y = vars["y"].val.n;
            line2(lastx, lasty, x, y);
            lastx = x;
            lasty = y;
        }
    }
    else
    {
        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);
        for(int i = 0; i < nassignments; i++)
        {
            vars[parp->assignments[i]->lvalue.str] = evaluate(parp->assignments[i]->rvalue);
            if (vars[parp->assignments[i]->lvalue.str].type != val_number)
                throw(error("Error: attempt to plot non-numeric expression"));
        }
        double lastx = vars["x"].val.n;
        double lasty = vars["y"].val.n;
        double lastz = vars["z"].val.n;
        double x, y, z;
        t = t + step;
        for(; t < to + step; t += step)
        {
            vars[parp->parname] = t;
            for(int i = 0; i < nassignments; i++)
                vars[parp->assignments[i]->lvalue.str] = evaluate(parp->assignments[i]->rvalue);
            x = vars["x"].val.n;
            y = vars["y"].val.n;
            z = vars["z"].val.n;
            glVertex3f(lastx, lasty, lastz);
            glVertex3f(x, y, z);
            lastx = x;
            lasty = y;
            lastz = z;
        }
        glEnd();
        glEnable(GL_LIGHTING);
    }
    getnextcolor();
}

int interpreter::addstring(std::string str)
{
    std::vector<std::string>::iterator iter = std::find(strings.begin(), strings.end(), str);
    if (iter == strings.end())
    {
        //std::cout << "Pushing string \"" << str << "\"\n";
        strings.push_back(str);
        return strings.size() - 1;

    }
    else
    {
        //std::cout << "Assigning existing string at index " << iter  - strings.begin() << "\n";
        return iter - strings.begin();
    }
}

procedure::procedure(){}

procedure::procedure(std::vector <std::string> args_, block* entrypoint_)
{
    args = args_;
    entrypoint = entrypoint_;
}


procedure::~procedure()
{
    delete entrypoint;
}

error::error(std::string errstring_)
{
    errstring = errstring_;
}

arglist_member::arglist_member()
{
    next = NULL;
}

arglist_member::~arglist_member()
{
    if (next != NULL)
        delete next;
}
