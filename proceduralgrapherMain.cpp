/***************************************************************
 * Name:      proceduralgrapherMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Luke Wren (wren6991@gmail.com)
 * Created:   2011-12-14
 * Copyright: Luke Wren ()
 * License:
 **************************************************************/

#include "proceduralgrapherMain.h"
#include <wx/msgdlg.h>

//(*InternalHeaders(proceduralgrapherDialog)
#include <wx/settings.h>
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "tokenizer.h"
#include "parser.h"
#include "interpreter.h"
#include <iostream>
#include <stdlib.h>
#include <wx/glcanvas.h>

#include "draw.h"

wxTextCtrl *OutputBox;

g_data parserdata;

std::string token_type_names[] = {
    "<eof>",
    "number",
    "id",
    "equals",
    "plus",
    "minus",
    "times",
    "divide",
    "lparen",
    "rparen",
    "exp",
    "<",
    ">",
    "<=",
    ">=",
    "COMMENT!!!!",
    "string",
    "?",
    ":",
    ",",
    "let",
    "if",
    "then",
    "elseif",
    "else",
    "end",
    "while",
    "do",
    "for",
    "to",
    "and",
    "or",
    "not",
    "def",
    "func",
    "return",
    "plot",
    "no. tokens",
    "e_value",
    "e_nostatement",
    "valid ordinate",
    "expression",
    "expression",
    "value",
    "defined procedure",
    "explicit plot",
    "implicit plot"
};

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

//(*IdInit(proceduralgrapherDialog)
const long proceduralgrapherDialog::ID_TXTEXPR = wxNewId();
const long proceduralgrapherDialog::ID_TXTOUTPUT = wxNewId();
const long proceduralgrapherDialog::ID_GLCANVAS1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(proceduralgrapherDialog,wxDialog)
    //(*EventTable(proceduralgrapherDialog)
    //*)
END_EVENT_TABLE()

double dddprint(double);

proceduralgrapherDialog::proceduralgrapherDialog(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(proceduralgrapherDialog)
    wxBoxSizer* BoxSizer2;
    wxBoxSizer* BoxSizer3;

    Create(parent, id, _("Procedural Grapher"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("id"));
    SetClientSize(wxSize(-1,-1));
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    BoxSizer2 = new wxBoxSizer(wxVERTICAL);
    txtExpr = new wxTextCtrl(this, ID_TXTEXPR, _("y = x^3 - x"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER|wxTE_PROCESS_TAB|wxTE_MULTILINE|wxTE_CHARWRAP|wxSUNKEN_BORDER|wxVSCROLL, wxDefaultValidator, _T("ID_TXTEXPR"));
    wxFont txtExprFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,_T("Consolas"),wxFONTENCODING_DEFAULT);
    txtExpr->SetFont(txtExprFont);
    BoxSizer2->Add(txtExpr, 30, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    txtOutput = new wxTextCtrl(this, ID_TXTOUTPUT, _("Text"), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxSUNKEN_BORDER|wxVSCROLL, wxDefaultValidator, _T("ID_TXTOUTPUT"));
    txtOutput->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
    BoxSizer2->Add(txtOutput, 10, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1->Add(BoxSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    int GLCanvasAttributes_1[] = {
    	WX_GL_RGBA,
    	WX_GL_DOUBLEBUFFER,
    	WX_GL_DEPTH_SIZE,      16,
    	WX_GL_STENCIL_SIZE,    0,
    	0, 0 };
    GLCanvas1 = new wxGLCanvas(this, ID_GLCANVAS1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GLCANVAS1"), GLCanvasAttributes_1);
    GLCanvas1->SetMinSize(wxSize(300,300));
    BoxSizer3->Add(GLCanvas1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1->Add(BoxSizer3, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    SetSizer(BoxSizer1);
    BoxSizer1->SetSizeHints(this);

    Connect(ID_TXTEXPR,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&proceduralgrapherDialog::Tokenize);
    Connect(ID_TXTOUTPUT,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&proceduralgrapherDialog::OntxtOutputText);
    GLCanvas1->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1LeftDown,0,this);
    GLCanvas1->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1LeftUp,0,this);
    GLCanvas1->Connect(wxEVT_MIDDLE_DOWN,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1MiddleDown,0,this);
    GLCanvas1->Connect(wxEVT_MIDDLE_UP,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1MiddleUp,0,this);
    GLCanvas1->Connect(wxEVT_MOTION,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1MouseMove,0,this);
    GLCanvas1->Connect(wxEVT_MOUSEWHEEL,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1MouseWheel,0,this);
    //*)
    GLContext1 = new wxGLContext(GLCanvas1);
    OutputBox = txtOutput;
    donedrawing = true;
    parserdata.left = -1;
    parserdata.right = 1;
    parserdata.top = 1;
    parserdata.bottom = -1;
    parserdata.detail = 50;
    leftdown = false;
    middledown = false;
    funcs["print"] = dddprint;
    funcs["ln"] = log;
    funcs["sin"] = sin;
    funcs["cos"] = cos;
    funcs["tan"] = tan;
    funcs["asin"] = asin;
    funcs["acos"] = acos;
    funcs["atan"] = atan;
    funcs["abs"] = fabs;
    funcs["floor"] = floor;
    funcs["ceil"] = ceil;
    tokens = tokenize(std::string(txtOutput->GetValue().ToAscii()), funcs);
}

proceduralgrapherDialog::~proceduralgrapherDialog()
{
    //(*Destroy(proceduralgrapherDialog)
    //*)
}

void proceduralgrapherDialog::OnQuit(wxCommandEvent& event)
{
    Close();
}

double dddprint(double x)
{
    (*OutputBox) << x << "\n";
    std::cout << x << "\n";
    return x;
}

void proceduralgrapherDialog::parse()
{
    txtOutput->SetValue("");

    initgl();
    glColor4f(1, 0, 0, 1);

    txtOutput->SetValue("");
    parser p(tokens, funcs);
    block* program;

    try {
        program = p.blk();
        interpreter interp(program, funcs, parserdata);
        interp.evaluate(interp.program);
    }
    catch (token_type_enum t)
    {
        (*txtOutput) << "Error: expected " << token_type_names[t] << " near \"" << p.t.value << "\" (pos. " << p.tindex << ")\n";
    }
    donedrawing = true;
    endgl();
    (*OutputBox) << parserdata.left << ", " << parserdata.right << "\n" << parserdata.top << ", " << parserdata.bottom << "\nComplete.";
}

void proceduralgrapherDialog::Tokenize(wxCommandEvent& event)
{
    tokens = tokenize(std::string(txtExpr->GetValue().ToAscii()), funcs);

    parse();
}

void proceduralgrapherDialog::OncanvasPaint(wxPaintEvent& event)
{

}


void proceduralgrapherDialog::print(std::string str)
{
    (*txtOutput) << str;
}


void proceduralgrapherDialog::initgl()
{
    GLContext1->SetCurrent(*GLCanvas1);

    glClearColor(1, 1, 1, 1);//(0.392, 0.584, 0.929, 1); //(cornflower blue)
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(parserdata.left, parserdata.right, parserdata.bottom, parserdata.top, 1, 1000);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH, GL_NICEST);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(0.5, 0.5, 0.5);
    double step = pow(2, floor(log(parserdata.right - parserdata.left) / log(2))) / 4;
    for(double x = floor(parserdata.left/ step) * step; x < parserdata.right; x+= step)
        line2(x, parserdata.top, x, parserdata.bottom);
    step = pow(2, floor(log(parserdata.top - parserdata.bottom) / log(2))) / 4;
    for(double y = floor(parserdata.bottom / step) * step; y < parserdata.top; y += step)
        line2(parserdata.left, y, parserdata.right, y);

    //int ngridlines = log(parserdata.right - parserdata.left)/log(2);
    glColor3f(0, 0, 0);
    line2(0, parserdata.top, 0, parserdata.bottom);
    line2(parserdata.left, 0, parserdata.right, 0);
}


void proceduralgrapherDialog::endgl()
{
    if (!donedrawing)
        return;
    glFlush();
    GLCanvas1->SwapBuffers();
    donedrawing = false;    //donedrawing is set to true after parsing; once it is true, all drawn primitives can be flushed. It is then reset to false so that the screen is not cleared until the next batch of primitives has been written.
}



void proceduralgrapherDialog::OntxtOutputText(wxCommandEvent& event)
{

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
    glBegin(GL_POINT);
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

void poly2(float verts[], int count)
{
    glBegin(GL_POLYGON);
    for(int i = 0; i < count * 2; i+= 2)
    {
        glVertex3f(verts[i], verts[i + 1], -1);
    }
    glEnd();
}

void color3(float r, float g, float b)
{
    glColor3f(r, g, b);
}

void color4(float r, float g, float b, float a)
{
    glColor4f(r, g, b, a);
}

void proceduralgrapherDialog::OnGLCanvas1LeftDown(wxMouseEvent& event)
{
    leftdown = true;
    mousex = event.GetX();
    mousey = event.GetY();
}

void proceduralgrapherDialog::OnGLCanvas1LeftUp(wxMouseEvent& event)
{
    leftdown = false;
    parserdata.detail = 100;
    parse();
}

void proceduralgrapherDialog::OnGLCanvas1MouseMove(wxMouseEvent& event)
{
    double dx = event.GetX() - mousex;
    double dy = event.GetY() - mousey;
    if (leftdown)
    {
        if (!event.LeftIsDown())
        {
            leftdown = false;
            return;
        }
        double scale = (parserdata.left - parserdata.right) / 300.0;
        parserdata.left += dx * scale;
        parserdata.right += dx * scale;
        parserdata.top -= dy * scale;
        parserdata.bottom -= dy * scale;
        parserdata.detail = 40;
        parse();
   }
   else if (middledown)
   {
        if (!event.MiddleIsDown())
        {
            middledown = false;
            return;
        }
        double centrex = (parserdata.left + parserdata.right) / 2;
        double centrey = (parserdata.top + parserdata.bottom) / 2;
        parserdata.top = (parserdata.top - centrey) * pow(1.01, dy) + centrey;
        parserdata.bottom = (parserdata.bottom - centrey) * pow(1.01, dy) + centrey;
        parserdata.left = (parserdata.left - centrex) * pow(1.01, dy) + centrex;
        parserdata.right = (parserdata.right - centrex) * pow(1.01, dy) + centrex;
        parserdata.detail = 40;
        parse();
   }
   mousex = event.GetX();
   mousey = event.GetY();
}

void proceduralgrapherDialog::OnGLCanvas1MouseWheel(wxMouseEvent& event)
{
    (*OutputBox) << event.GetWheelRotation();
    double centrex, centrey;
    centrex = (parserdata.left + parserdata.right) / 2;
    centrey = (parserdata.top + parserdata.bottom) / 2;
    parserdata.top = (parserdata.top - centrey) * pow(1.1, event.GetWheelRotation()) + centrey;
    parserdata.bottom = (parserdata.bottom - centrey) * pow(1.1, event.GetWheelRotation()) + centrey;
    parserdata.left = (parserdata.left - centrex) * pow(1.1, event.GetWheelRotation()) + centrex;
    parserdata.right = (parserdata.right - centrex) * pow(1.1, event.GetWheelRotation()) + centrex;
}

void proceduralgrapherDialog::OnGLCanvas1MiddleDown(wxMouseEvent& event)
{
    middledown = true;
    mousex = event.GetX();
    mousey = event.GetY();
}

void proceduralgrapherDialog::OnGLCanvas1MiddleUp(wxMouseEvent& event)
{
    middledown = false;
    parserdata.detail = 100;
    parse();
}
