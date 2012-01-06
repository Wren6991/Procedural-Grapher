#include "draw.h"

colorf::colorf(){}

colorf::colorf(float r_, float g_, float b_)
{
    r = r_;
    g = g_;
    b = b_;
}

vert3f::vert3f()
{
    x = 0;
    y = 0;
    z = 0;
}

vert3f::vert3f(float x_, float y_, float z_)
{
    x = x_;
    y = y_;
    z = z_;
}

void line2(float ax, float ay, float bx, float by)
{
    glBegin(GL_LINES);
    glVertex3f(ax, ay, -1);
    glVertex3f(bx, by, -1);
    glEnd();
}

void point2(float x, float y)
{
    glBegin(GL_POINTS);
    glVertex3f(x, y, -1);
    glEnd();
}

void triangle2(float ax, float ay, float bx, float by, float cx, float cy)
{
    glBegin(GL_POLYGON);
    glVertex3f(ax, ay, -1);
    glVertex3f(bx, by, -1);
    glVertex3f(cx, cy, -1);
    glEnd();
}

void quad2(float ax, float ay, float bx, float by, float cx, float cy, float dx, float dy)
{
    glBegin(GL_POLYGON);
    glVertex3f(ax, ay, -1);
    glVertex3f(bx, by, -1);
    glVertex3f(cx, cy, -1);
    glVertex3f(dx, dy, -1);
    glEnd();
}

void pentagon2(float ax, float ay, float bx, float by, float cx, float cy, float dx, float dy, float ex, float ey)
{
    glBegin(GL_POLYGON);
    glVertex3f(ax, ay, -1);
    glVertex3f(bx, by, -1);
    glVertex3f(cx, cy, -1);
    glVertex3f(dx, dy, -1);
    glVertex3f(ex, ey, -1);
    glEnd();
}

void poly2(float verts[], int count)
{
    glBegin(GL_POLYGON);
    for(int i = 0; i < count * 2; i+= 2)
    {
        glVertex3f(verts[i], verts[i + 1], -1);
    }
    glEnd();
}

void rect2_4a(float ax, float ay, float bx, float by, colorf basecol, float a1, float a2, float a3, float a4)
{
    glBegin(GL_POLYGON);
    setcolor(basecol, a1);
    glVertex3f(ax, ay, -1);
    setcolor(basecol, a2);
    glVertex3f(bx, ay, -1);
    setcolor(basecol, a4);
    glVertex3f(bx, by, -1);
    setcolor(basecol, a3);
    glVertex3f(ax, by, -1);
    glEnd();
}

void triangle3(vert3f a, vert3f b, vert3f c)
{
    glBegin(GL_POLYGON);
    glVertex3f(a.x, a.y, c.z);
    glVertex3f(b.x, b.y, c.z);
    glVertex3f(c.x, c.y, c.z);
    glEnd();
}

void quad3(vert3f a, vert3f b, vert3f c, vert3f d)
{
    glBegin(GL_POLYGON);
    glVertex3f(a.x, a.y, a.z);
    glVertex3f(b.x, b.y, b.z);
    glVertex3f(c.x, c.y, c.z);
    glVertex3f(d.x, d.y, d.z);
    glEnd();
}

void quad3(float ax, float ay, float az, float bx, float by, float bz, float cx, float cy, float cz, float dx, float dy, float dz)
{
    glBegin(GL_POLYGON);
    glVertex3f(ax, ay, cz);
    glVertex3f(bx, by, cz);
    glVertex3f(cx, cy, cz);
    glVertex3f(dx, dy, dz);
    glEnd();
}

void setcolor(float r, float g, float b)
{
    glColor3f(r, g, b);
}

void setcolor(colorf col)
{
    glColor3f(col.r, col.g, col.b);
}

void setcolor(float r, float g, float b, float a)
{
    glColor4f(r, g, b, a);
}

void setcolor(colorf col, float a)
{
    glColor4f(col.r, col.g, col.b, a);
}
