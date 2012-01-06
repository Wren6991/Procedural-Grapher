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
    "dif",
    "par",
    "from",
    "step",
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
const long proceduralgrapherDialog::ID_CHECKBOX1 = wxNewId();
const long proceduralgrapherDialog::ID_PANEL1 = wxNewId();
const long proceduralgrapherDialog::ID_PANEL2 = wxNewId();
const long proceduralgrapherDialog::ID_NOTEBOOK1 = wxNewId();
const long proceduralgrapherDialog::ID_GLCANVAS1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(proceduralgrapherDialog,wxDialog)
    //(*EventTable(proceduralgrapherDialog)
    //*)
END_EVENT_TABLE()

double dddprint(double);

double lnbodge(double x)
{
    if (x > 0)
        return log(x);
    else
        return log(-x) - 1000000;
}


proceduralgrapherDialog::proceduralgrapherDialog(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(proceduralgrapherDialog)
    wxBoxSizer* BoxSizer2;
    wxBoxSizer* BoxSizer3;

    Create(parent, wxID_ANY, _("Procedural Grapher"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("wxID_ANY"));
    SetClientSize(wxSize(-1,-1));
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    BoxSizer2 = new wxBoxSizer(wxVERTICAL);
    txtExpr = new wxTextCtrl(this, ID_TXTEXPR, _("y = x^3 - x"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER|wxTE_PROCESS_TAB|wxTE_MULTILINE|wxTE_CHARWRAP|wxSUNKEN_BORDER|wxVSCROLL, wxDefaultValidator, _T("ID_TXTEXPR"));
    wxFont txtExprFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,_T("Consolas"),wxFONTENCODING_DEFAULT);
    txtExpr->SetFont(txtExprFont);
    BoxSizer2->Add(txtExpr, 2, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Notebook1 = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, wxTRANSPARENT_WINDOW|wxTAB_TRAVERSAL, _T("ID_NOTEBOOK1"));
    txtOutput = new wxTextCtrl(Notebook1, ID_TXTOUTPUT, _("Text"), wxPoint(-31,26), wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxSUNKEN_BORDER|wxVSCROLL, wxDefaultValidator, _T("ID_TXTOUTPUT"));
    txtOutput->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
    Panel1 = new wxPanel(Notebook1, ID_PANEL1, wxPoint(101,66), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    chk3D = new wxCheckBox(Panel1, ID_CHECKBOX1, _("3D"), wxPoint(8,8), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    chk3D->SetValue(false);
    Panel2 = new wxPanel(Notebook1, ID_PANEL2, wxPoint(155,10), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    Notebook1->AddPage(txtOutput, _("Output"), false);
    Notebook1->AddPage(Panel1, _("View"), false);
    Notebook1->AddPage(Panel2, _("Colours"), false);
    BoxSizer2->Add(Notebook1, 1, wxLEFT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1->Add(BoxSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    int GLCanvasAttributes_1[] = {
    	WX_GL_RGBA,
    	WX_GL_DOUBLEBUFFER,
    	WX_GL_DEPTH_SIZE,      16,
    	WX_GL_STENCIL_SIZE,    0,
    	0, 0 };
    GLCanvas1 = new wxGLCanvas(this, ID_GLCANVAS1, wxDefaultPosition, wxSize(300,300), 0, _T("ID_GLCANVAS1"), GLCanvasAttributes_1);
    BoxSizer3->Add(GLCanvas1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1->Add(BoxSizer3, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    SetSizer(BoxSizer1);
    BoxSizer1->SetSizeHints(this);

    Connect(ID_TXTEXPR,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&proceduralgrapherDialog::Tokenize);
    Connect(ID_TXTOUTPUT,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&proceduralgrapherDialog::OntxtOutputText);
    Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&proceduralgrapherDialog::Onchk3DClick);
    GLCanvas1->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1LeftDown,0,this);
    GLCanvas1->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1LeftUp,0,this);
    GLCanvas1->Connect(wxEVT_MIDDLE_DOWN,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1MiddleDown,0,this);
    GLCanvas1->Connect(wxEVT_MIDDLE_UP,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1MiddleUp,0,this);
    GLCanvas1->Connect(wxEVT_MOTION,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1MouseMove,0,this);
    GLCanvas1->Connect(wxEVT_MOUSEWHEEL,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1MouseWheel,0,this);
    GLCanvas1->Connect(wxEVT_SIZE,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1Resize,0,this);
    //*)
    GLContext1 = new wxGLContext(GLCanvas1);
    OutputBox = txtOutput;
    donedrawing = true;
    parserdata.left = -1;
    parserdata.right = 1.00001;
    parserdata.top = 1.00001;
    parserdata.bottom = -1;
    parserdata.front = 1;
    parserdata.back = -1;
    parserdata.yaw = 0;
    parserdata.pitch = 0;
    parserdata.detail = 100;
    parserdata.colors.push_back(colorf(1, 0, 0));
    parserdata.colors.push_back(colorf(0, 1, 0));
    parserdata.colors.push_back(colorf(0, 0, 1));
    parserdata.colors.push_back(colorf(1, 0.5, 0));
    parserdata.colors.push_back(colorf(0.8, 0, 0.8));
    parserdata.colors.push_back(colorf(0, 0.8, 0.8));
    parserdata.colorindex = 0;
    parserdata.currentcolor = parserdata.colors[0];
    parserdata.is3d = false;
    leftdown = false;
    middledown = false;
    funcs["print"] = dddprint;
    funcs["ln"] = lnbodge;
    funcs["sin"] = sin;
    funcs["cos"] = cos;
    funcs["tan"] = tan;
    funcs["asin"] = asin;
    funcs["acos"] = acos;
    funcs["atan"] = atan;
    funcs["abs"] = fabs;
    funcs["floor"] = floor;
    funcs["ceil"] = ceil;
    funcs["sqrt"] = sqrt;
    tokens = tokenize(std::string("y = x^3 - x"), funcs);
    p = parser(tokens);
    program = p.blk();
    validprogram = true;
    lastcanvaswidth = 300;
    lastcanvasheight = 300;
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
    if (validprogram)
        delete program;
    validprogram = false;
    p = parser(tokens);
    try {
        program = p.blk();
        validprogram = true;
    }
    catch (token_type_enum t)
    {
        (*txtOutput) << "Error: expected " << token_type_names[t] << " near \"" << p.t.value << "\" (pos. " << static_cast <int> (p.tindex) << ")\n";
    }
    parserror = txtOutput->GetValue();
    std::cout << parserror;
}
void proceduralgrapherDialog::interpret()
{
    txtOutput->SetValue(parserror);
    if (parserdata.is3d)
        init3d();
    else
        init2d();

    if (validprogram)
    {
         try {
            interpreter interp(funcs, parserdata);
            interp.evaluate(program);
        }
        catch (token_type_enum t)
        {
            (*txtOutput) << "Error: expected " << token_type_names[t] << " near \"" << p.t.value << "\" (pos. " << static_cast <int> (p.tindex) << ")\n";
        }
        catch (error e)
        {
            (*txtOutput) << e.errstring << "\n";
        }
    }

    donedrawing = true;
    endgl();
    (*OutputBox) << parserdata.left << ", " << parserdata.bottom << ", " << parserdata.back << "\n" << parserdata.right << ", " << parserdata.top << ", " << parserdata.front << "\nComplete.";
}

void proceduralgrapherDialog::Tokenize(wxCommandEvent& event)
{
    tokens = tokenize(std::string(txtExpr->GetValue().ToAscii()), funcs);

    parse();
    interpret();
}

void proceduralgrapherDialog::OncanvasPaint(wxPaintEvent& event)
{

}


void proceduralgrapherDialog::print(std::string str)
{
    (*txtOutput) << str;
}


void proceduralgrapherDialog::init2d()
{
    GLContext1->SetCurrent(*GLCanvas1);

    glViewport(0, 0, lastcanvaswidth, lastcanvasheight);
    glClearColor(1, 1, 1, 1);//(0.392, 0.584, 0.929, 1); //(cornflower blue)
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(parserdata.left, parserdata.right, parserdata.bottom, parserdata.top, 1, 1000);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH, GL_NICEST);
    glEnable(GL_POINT_SMOOTH);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(0.5, 0.5, 0.5);
    double step = pow(2, floor(log((parserdata.right - parserdata.left) * 300.0 / lastcanvaswidth) / log(2))) / 4;
    for(double x = floor(parserdata.left/ step) * step; x < parserdata.right; x+= step)
        line2(x, parserdata.top, x, parserdata.bottom);
    step = pow(2, floor(log((parserdata.top - parserdata.bottom) * 300.0 / lastcanvasheight) / log(2))) / 4;
    for(double y = floor(parserdata.bottom / step) * step; y < parserdata.top; y += step)
        line2(parserdata.left, y, parserdata.right, y);

    //int ngridlines = log(parserdata.right - parserdata.left)/log(2);
    glColor3f(0, 0, 0);
    line2(0, parserdata.top, 0, parserdata.bottom);
    line2(parserdata.left, 0, parserdata.right, 0);
}

void proceduralgrapherDialog::init3d()
{
    GLContext1->SetCurrent(*GLCanvas1);

    glViewport(0, 0, lastcanvaswidth, lastcanvasheight);
    glClearColor(1, 1, 1, 1);//(0.392, 0.584, 0.929, 1); //(cornflower blue)
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1, 1, -1, 1, 1, 1000);//(parserdata.left / 2, parserdata.right / 2, parserdata.bottom / 2, parserdata.top / 2, 1, 1000);

    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };
    GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    glEnable(GL_NORMALIZE);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH, GL_NICEST);
    glEnable(GL_POINT_SMOOTH);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -3);
    glScalef(3/(parserdata.right - parserdata.left), 3/(parserdata.top - parserdata.bottom), 3/(parserdata.front - parserdata.back));
    glTranslatef((parserdata.left + parserdata.right) / -2, (parserdata.top + parserdata.bottom) / -2, (parserdata.front + parserdata.back) / -2);
    glRotatef(parserdata.pitch, 1, 0, 0);
    glRotatef(parserdata.yaw, 0, 1, 0);




    //int ngridlines = log(parserdata.right - parserdata.left)/log(2);
    glColor4f(0, 0, 0, 0.2);
    //quad3(vert3f(parserdata.left, parserdata.top, 0), vert3f(parserdata.right, parserdata.top, 0), vert3f(parserdata.right, parserdata.bottom, 0), vert3f(parserdata.left, parserdata.bottom, 0));
    //quad3(vert3f(0, parserdata.top, parserdata.back), vert3f(0, parserdata.top, parserdata.front), vert3f(0, parserdata.bottom, parserdata.front), vert3f(0, parserdata.bottom, parserdata.back));

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

}


void proceduralgrapherDialog::endgl()
{
    if (!donedrawing)
        return;
    glFlush();
    GLCanvas1->SwapBuffers();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    donedrawing = false;    //donedrawing is set to true after parsing; once it is true, all drawn primitives can be flushed. It is then reset to false so that the screen is not cleared until the next batch of primitives has been written.
}



void proceduralgrapherDialog::OntxtOutputText(wxCommandEvent& event)
{

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
    interpret();
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
        if (!parserdata.is3d)
        {
            double scalex = (parserdata.left - parserdata.right) / lastcanvaswidth;
            double scaley = (parserdata.bottom - parserdata.top) / lastcanvasheight;
            parserdata.left += dx * scalex;
            parserdata.right += dx * scalex;
            parserdata.top -= dy * scaley;
            parserdata.bottom -= dy * scaley;
            parserdata.detail = 40;
        }
        else
        {
            parserdata.yaw -= dx;
            parserdata.pitch -= dy;
        }

        interpret();
   }
   else if (middledown)
   {
        if (!(event.MiddleIsDown()||event.RightIsDown()))
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
        parserdata.back  *= pow(1.01, dy);
        parserdata.front *= pow(1.01, dy);
        parserdata.detail = 40;
        interpret();
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
    interpret();
}

void proceduralgrapherDialog::OnGLCanvas1Paint(wxPaintEvent& event)
{
}

void proceduralgrapherDialog::OnGLCanvas1Resize(wxSizeEvent& event)
{
    double centrex = (parserdata.left + parserdata.right) / 2;
    double centrey = (parserdata.bottom + parserdata.top) / 2;
    parserdata.left = (parserdata.left - centrex) * event.GetSize().GetWidth() / lastcanvaswidth + centrex;
    parserdata.right = (parserdata.right - centrex) * event.GetSize().GetWidth() / lastcanvaswidth + centrex;
    parserdata.bottom = (parserdata.bottom - centrey) * event.GetSize().GetHeight() / lastcanvasheight + centrey;
    parserdata.top = (parserdata.top - centrey) * event.GetSize().GetHeight() / lastcanvasheight + centrey;
    lastcanvasheight = event.GetSize().GetHeight();
    lastcanvaswidth = event.GetSize().GetWidth();
    interpret();
}

void proceduralgrapherDialog::Onchk3DClick(wxCommandEvent& event)
{
    parserdata.is3d = chk3D->GetValue();
    interpret();
}
