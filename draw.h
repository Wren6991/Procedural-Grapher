#ifndef _DRAW_H_
#define _DRAW_H_

#include <wx/glcanvas.h>
class colorf
{
    public:
    float r;
    float g;
    float b;
    colorf();
    colorf(float, float, float);
};

void line2(float, float, float, float);
void triangle2(float, float, float, float, float, float);
void quad2(float, float, float, float, float, float, float, float);
void pentagon2(float, float, float, float, float, float, float, float, float, float);    //yes, it looks retarded, but 5 sided shapes are common in the inequality filling and it's not really worth the overhead of building an array each time
void poly2(float[], int);
void point2(float, float);
void rect2_4a(float, float, float, float, colorf, float, float, float, float);  //2d rectangle with an alpha component for each vertex.
void setcolor(float, float, float);
void setcolor(colorf);
void setcolor(float, float, float, float);
void setcolor(colorf, float);

#endif //_DRAW_H_
