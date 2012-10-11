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
#include <wx/bitmap.h>
#include <wx/settings.h>
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/image.h>
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
interpreter* interp_ptr;

g_data parserdata;
int setdetail = 100;
std::map <std::string, tagged_value> persistent_vars;

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
    "left brace",
    "right brace",
    "left squarebr.",
    "right squarebr.",
    ".",
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

std::string type_names[] = {
    "nil",
    "number",
    "string",
    "procedure",
    "array"
    };

double lnbodge(double x)            //identical for positive x; differentiates correctly for -x (where it is usually undefined)
{
    if (x > 0)
        return log(x);
    else
        return log(-x) - 1000000;
}
tagged_value log_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"ln\""));
    if (arg->v.type == val_number)
        return tagged_value(lnbodge(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"ln\""));
}

tagged_value sin_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"sin\""));
    if (arg->v.type == val_number)
        return tagged_value(sin(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"sin\""));
}

tagged_value cos_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"cos\""));
    if (arg->v.type == val_number)
        return tagged_value(cos(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"cos\""));
}

tagged_value tan_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"tan\""));
    if (arg->v.type == val_number)
        return tagged_value(tan(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"tan\""));
}

tagged_value asin_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"asin\""));
    if (arg->v.type == val_number)
        return tagged_value(asin(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"asin\""));
}

tagged_value acos_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"acos\""));
    if (arg->v.type == val_number)
        return tagged_value(acos(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"acos\""));
}

tagged_value atan_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"atan\""));
    if (arg->v.type == val_number)
        return tagged_value(atan(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"atan\""));
}

tagged_value abs_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"abs\""));
    if (arg->v.type == val_number)
        return tagged_value(fabs(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"abs\""));
}

tagged_value floor_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"floor\""));
    if (arg->v.type == val_number)
        return tagged_value(floor(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"floor\""));
}

tagged_value ceil_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"ceil\""));
    if (arg->v.type == val_number)
        return tagged_value(ceil(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"ceil\""));
}

tagged_value sqrt_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"sqrt\""));
    if (arg->v.type == val_number)
        return tagged_value(sqrt(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"sqrt\""));
}

tagged_value print_tv(arglist_member* arg)
{
    while (arg != NULL)
    {
        if (arg->v.type == val_number)
            (*OutputBox) << arg->v.val.n;
        else if (arg->v.type == val_string)
            (*OutputBox) << interp_ptr->strings[arg->v.val.str];
        else if (arg->v.type == val_array)
            (*OutputBox) << "(table: " << arg->v.val.arr << ")";
        else if (arg->v.type == val_nil)
            (*OutputBox) << "(nil)";
        else
            throw(error("Error: expected number or string as argument to function \"print\""));
        arg = arg->next;
    }
    (*OutputBox) << "\n";
    return tagged_value();
}

tagged_value size_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"ln\""));
    if (arg->v.type == val_array)
        return tagged_value(interp_ptr->arrays[arg->v.val.arr].size());
    else if (arg->v.type == val_string)
        return tagged_value(interp_ptr->strings[arg->v.val.str].size());
    else
        throw(error("Error: expected array or string as argument to function \"size\""));
}

tagged_value char_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"ln\""));
    if (arg->v.type == val_number)
    {
            tagged_value rv;
            rv.type = val_string;
            char str[] = {0, 0};
            str[0] = static_cast <char> (arg->v.val.n);            //wow i'm tired
            rv.val.str = interp_ptr->addstring(std::string(str));
            return rv;
    }
    else
        throw(error("Error: expected number as argument to function \"char\""));
}

tagged_value rand_tv(arglist_member* arg)
{
    if (arg == NULL)
        return tagged_value(rand() / (float)RAND_MAX);
    else
    {
        if (arg->v.type != val_number)
            throw(error("Error: expected number as argument to function \"char\""));
        return tagged_value(rand() % static_cast <int> (arg->v.val.n));
    }
}

tagged_value prng_tv(arglist_member *arg)
{
    /*srand(123456789);
    while (arg != NULL)
    {
        srand(arg->v.val.n * 10267 * (arg->v.val.n + 3571) + rand() * (arg->v.val.n - 7057));
        rand();
        arg = arg->next;

    }
    return tagged_value(rand() / (float)RAND_MAX);*/
    double x = arg->v.val.n;
    double y = arg->next->v.val.n;
    double seed = 17 + 13 * (pow(5, x) - pow(7, 1 - x) + (pow(11, y) - pow(19, 2 - y)) / 2.0);
    seed = seed - (long)(seed / 5011) * 5011;
    srand(seed);
    rand();
    rand();
    return tagged_value(rand() / static_cast <double> (RAND_MAX));
}

tagged_value getpersistent(arglist_member *arg)
{
    if (arg == NULL || arg->v.type != val_string)
        throw(error("Error: expected string as argument to function \"getpersistent\""));
    return persistent_vars[interp_ptr->strings[arg->v.val.str]];
}

tagged_value setpersistent(arglist_member *arg)
{
    if (arg == NULL || arg->v.type != val_string)
        throw(error("Error: expected string as first argument to function \"setpersistent\""));
    if (arg->next == NULL)
        throw(error("Error: expected more arguments to function \"setpersistent\""));
    persistent_vars[interp_ptr->strings[arg->v.val.str]] = arg->next->v;
    return tagged_value();
}

tagged_value type_tv(arglist_member *arg)
{
    tagged_value rv;
    if (arg == NULL)
    {
        rv.val.str = interp_ptr->addstring("nil");
        rv.type = val_string;
    }
    else
    {
        rv.val.str = interp_ptr->addstring(type_names[arg->v.type]);
        rv.type = val_string;
    }
    return rv;
}

tagged_value mandelbrot_tv(arglist_member *arg)
{
    if (!(arg && arg->next))
        throw(error("Error: mandelbrot expects two arguments."));
    double ca = arg->v.val.n;
    double cb = arg->next->v.val.n;
    double za = ca;
    double zb = cb;
    double zatemp;
    int i;
    for (i = 0; i < 50 && za * za + zb * zb < 4; i++)
    {
        zatemp = za;
        za = za * za - zb * zb + ca;
        zb = 2 * zatemp * zb + cb;
    }
    return tagged_value((double)i / 50.0);
}
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
const long proceduralgrapherDialog::ID_BUTTON1 = wxNewId();
const long proceduralgrapherDialog::ID_BUTTON2 = wxNewId();
const long proceduralgrapherDialog::ID_PANEL2 = wxNewId();
const long proceduralgrapherDialog::ID_CHECKBOX1 = wxNewId();
const long proceduralgrapherDialog::ID_SLIDER1 = wxNewId();
const long proceduralgrapherDialog::ID_STATICTEXT1 = wxNewId();
const long proceduralgrapherDialog::ID_CHECKBOX2 = wxNewId();
const long proceduralgrapherDialog::ID_PANEL1 = wxNewId();
const long proceduralgrapherDialog::ID_NOTEBOOK1 = wxNewId();
const long proceduralgrapherDialog::ID_GLCANVAS1 = wxNewId();
const long proceduralgrapherDialog::ID_TIMER1 = wxNewId();
const long proceduralgrapherDialog::tbrNew = wxNewId();
const long proceduralgrapherDialog::tbrOpen = wxNewId();
const long proceduralgrapherDialog::tbrSave = wxNewId();
const long proceduralgrapherDialog::tbrTime = wxNewId();
const long proceduralgrapherDialog::tbrDebug = wxNewId();
const long proceduralgrapherDialog::ID_TOOLBAR1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(proceduralgrapherDialog,wxFrame)
    //(*EventTable(proceduralgrapherDialog)
    //*)
END_EVENT_TABLE()


proceduralgrapherDialog::proceduralgrapherDialog(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(proceduralgrapherDialog)
    wxBoxSizer* BoxSizer4;
    wxBoxSizer* BoxSizer2;
    wxGridSizer* GridSizer1;
    wxBoxSizer* BoxSizer3;

    Create(parent, wxID_ANY, _("Procedural Grapher - Untitled"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    SetClientSize(wxSize(-1,-1));
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    BoxSizer2 = new wxBoxSizer(wxVERTICAL);
    txtExpr = new wxTextCtrl(this, ID_TXTEXPR, _("y = x^3 - x"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER|wxTE_PROCESS_TAB|wxTE_MULTILINE|wxTE_CHARWRAP|wxSUNKEN_BORDER|wxVSCROLL, wxDefaultValidator, _T("ID_TXTEXPR"));
    wxFont txtExprFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,_T("Consolas"),wxFONTENCODING_DEFAULT);
    txtExpr->SetFont(txtExprFont);
    BoxSizer2->Add(txtExpr, 2, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
    Notebook1 = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
    txtOutput = new wxTextCtrl(Notebook1, ID_TXTOUTPUT, _("Text"), wxPoint(-89,-653), wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxSUNKEN_BORDER|wxVSCROLL, wxDefaultValidator, _T("ID_TXTOUTPUT"));
    txtOutput->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
    wxFont txtOutputFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,_T("Consolas"),wxFONTENCODING_DEFAULT);
    txtOutput->SetFont(txtOutputFont);
    Panel2 = new wxPanel(Notebook1, ID_PANEL2, wxPoint(162,14), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    BoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    btnStartStopTime = new wxButton(Panel2, ID_BUTTON1, _("Start Time"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    btnStartStopTime->SetToolTip(_("Start or Stop the timer. The \"time\" variable will increase while the timer is running. [Ctr + T]"));
    BoxSizer4->Add(btnStartStopTime, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    btnResetTime = new wxButton(Panel2, ID_BUTTON2, _("Reset Time"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    btnResetTime->SetToolTip(_("Set time to 0 [Ctrl + R]"));
    BoxSizer4->Add(btnResetTime, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel2->SetSizer(BoxSizer4);
    BoxSizer4->Fit(Panel2);
    BoxSizer4->SetSizeHints(Panel2);
    Panel1 = new wxPanel(Notebook1, ID_PANEL1, wxPoint(159,12), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    GridSizer1 = new wxGridSizer(0, 2, 0, 0);
    chk3D = new wxCheckBox(Panel1, ID_CHECKBOX1, _("3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    chk3D->SetValue(false);
    GridSizer1->Add(chk3D, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 10);
    BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    DetailSlider = new wxSlider(Panel1, ID_SLIDER1, 0, -5, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER1"));
    DetailSlider->SetTickFreq(1);
    DetailSlider->SetToolTip(_("Detail, low to high"));
    BoxSizer3->Add(DetailSlider, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 0);
    StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT1, _("Detail"), wxDefaultPosition, wxSize(31,24), 0, _T("ID_STATICTEXT1"));
    BoxSizer3->Add(StaticText1, 0, wxALIGN_LEFT|wxALIGN_TOP, 0);
    GridSizer1->Add(BoxSizer3, 1, wxTOP|wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 10);
    chkGrid = new wxCheckBox(Panel1, ID_CHECKBOX2, _("Grid"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    chkGrid->SetValue(true);
    GridSizer1->Add(chkGrid, 0, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 10);
    Panel1->SetSizer(GridSizer1);
    GridSizer1->Fit(Panel1);
    GridSizer1->SetSizeHints(Panel1);
    Notebook1->AddPage(txtOutput, _("Output"), false);
    Notebook1->AddPage(Panel2, _("Time"), false);
    Notebook1->AddPage(Panel1, _("View"), false);
    BoxSizer2->Add(Notebook1, 1, wxTOP|wxBOTTOM|wxLEFT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
    BoxSizer1->Add(BoxSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    int GLCanvasAttributes_1[] = {
    	WX_GL_RGBA,
    	WX_GL_DOUBLEBUFFER,
    	WX_GL_DEPTH_SIZE,      16,
    	WX_GL_STENCIL_SIZE,    0,
    	0, 0 };
    GLCanvas1 = new wxGLCanvas(this, ID_GLCANVAS1, wxDefaultPosition, wxSize(300,300), 0, _T("ID_GLCANVAS1"), GLCanvasAttributes_1);
    BoxSizer1->Add(GLCanvas1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(BoxSizer1);
    Timer1.SetOwner(this, ID_TIMER1);
    Timer1.Start(25, false);
    ToolBar1 = new wxToolBar(this, ID_TOOLBAR1, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxSTATIC_BORDER, _T("ID_TOOLBAR1"));
    ToolBarItem1 = ToolBar1->AddTool(tbrNew, _("New"), wxBitmap(wxImage(_T("C:\\Users\\Owner\\Documents\\CodeBlocks\\proceduralgrapher\\page_add.ico"))), wxNullBitmap, wxITEM_NORMAL, _("New File"), wxEmptyString);
    ToolBarItem2 = ToolBar1->AddTool(tbrOpen, _("Open"), wxBitmap(wxImage(_T("C:\\Users\\Owner\\Documents\\CodeBlocks\\proceduralgrapher\\folder_page.ico"))), wxNullBitmap, wxITEM_NORMAL, _("Open File"), wxEmptyString);
    ToolBarItem3 = ToolBar1->AddTool(tbrSave, _("Save\\tCtrl-Shift-S"), wxBitmap(wxImage(_T("C:\\Users\\Owner\\Documents\\CodeBlocks\\proceduralgrapher\\disk.ico"))), wxNullBitmap, wxITEM_NORMAL, _("Save File (Right Click: Save As)"), wxEmptyString);
    ToolBarItem4 = ToolBar1->AddTool(tbrTime, _("Time"), wxBitmap(wxImage(_T("C:\\Users\\Owner\\Documents\\CodeBlocks\\proceduralgrapher\\hourglass.ico"))), wxNullBitmap, wxITEM_CHECK, _("Start/Stop Time"), wxEmptyString);
    ToolBarItem5 = ToolBar1->AddTool(tbrDebug, _("Debug"), wxBitmap(wxImage(_T("C:\\Users\\Owner\\Documents\\CodeBlocks\\proceduralgrapher\\bug_delete.ico"))), wxNullBitmap, wxITEM_NORMAL, _("Debugging Window"), wxEmptyString);
    ToolBar1->Realize();
    SetToolBar(ToolBar1);
    FileDialogOpen = new wxFileDialog(this, _("Open"), wxEmptyString, wxEmptyString, _("Graphscript Files (*.grs)|*.grs|All Files (*.*)|*.*"), wxFD_DEFAULT_STYLE|wxFD_FILE_MUST_EXIST, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
    FileDialogSaveAs = new wxFileDialog(this, _("Save As"), wxEmptyString, wxEmptyString, _("Graphscript Files (*.grs)|*.grs|All Files (*.*)|*.*"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
    BoxSizer1->SetSizeHints(this);

    Connect(ID_TXTEXPR,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&proceduralgrapherDialog::Tokenize);
    Connect(ID_TXTOUTPUT,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&proceduralgrapherDialog::OntxtOutputText);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&proceduralgrapherDialog::OnbtnStartStopTimeClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&proceduralgrapherDialog::OnbtnResetTimeClick);
    Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&proceduralgrapherDialog::Onchk3DClick);
    Connect(ID_SLIDER1,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&proceduralgrapherDialog::OnDetailSliderCmdScrollThumbTrack);
    Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&proceduralgrapherDialog::OnchkGridClick);
    GLCanvas1->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1LeftDown,0,this);
    GLCanvas1->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1LeftUp,0,this);
    GLCanvas1->Connect(wxEVT_MIDDLE_DOWN,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1MiddleDown,0,this);
    GLCanvas1->Connect(wxEVT_MIDDLE_UP,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1MiddleUp,0,this);
    GLCanvas1->Connect(wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1MiddleDown,0,this);
    GLCanvas1->Connect(wxEVT_RIGHT_UP,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1MiddleUp,0,this);
    GLCanvas1->Connect(wxEVT_MOTION,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1MouseMove,0,this);
    GLCanvas1->Connect(wxEVT_MOUSEWHEEL,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1MouseWheel,0,this);
    GLCanvas1->Connect(wxEVT_SIZE,(wxObjectEventFunction)&proceduralgrapherDialog::OnGLCanvas1Resize,0,this);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&proceduralgrapherDialog::OnTimer1Trigger);
    Connect(tbrNew,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&proceduralgrapherDialog::OnFileNew);
    Connect(tbrOpen,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&proceduralgrapherDialog::OnFileOpen);
    Connect(tbrSave,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&proceduralgrapherDialog::OnFileSave);
    Connect(tbrSave,wxEVT_COMMAND_TOOL_RCLICKED,(wxObjectEventFunction)&proceduralgrapherDialog::OnFileSaveAs);
    Connect(tbrTime,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&proceduralgrapherDialog::OnbtnStartStopTimeClick);
    Connect(tbrTime,wxEVT_COMMAND_TOOL_RCLICKED,(wxObjectEventFunction)&proceduralgrapherDialog::OnbtnResetTimeClick);
    Connect(tbrDebug,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&proceduralgrapherDialog::OnDebugClicked);
    Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&proceduralgrapherDialog::OnClose);
    //*)
    wxAcceleratorEntry entries[7];
    entries[0].Set(wxACCEL_CTRL, (int) 'T', ID_BUTTON1);
    entries[1].Set(wxACCEL_CTRL, (int) 'R', ID_BUTTON2);
    entries[2].Set(wxACCEL_CTRL, (int) 'D', ID_CHECKBOX1);
    entries[3].Set(wxACCEL_CTRL, (int) 'G', ID_CHECKBOX2);
    entries[4].Set(wxACCEL_CTRL, (int) 'N', tbrNew);
    entries[5].Set(wxACCEL_CTRL, (int) 'O', tbrOpen);
    entries[6].Set(wxACCEL_CTRL, (int) 'S', tbrSave);
    wxAcceleratorTable accel(7, entries);
    this->SetAcceleratorTable(accel);
    GLContext1 = new wxGLContext(GLCanvas1);
    OutputBox = txtOutput;
    donedrawing = true;
    parserdata.left = -1;
    parserdata.right = 1;
    parserdata.top = 1;
    parserdata.bottom = -1;
    parserdata.front = 1;
    parserdata.back = -1;
    parserdata.yaw = 0;
    parserdata.pitch = 0;
    parserdata.setdetail(100, true);
    parserdata.time = 0;
    Timer1.Stop();
    StopWatch1.Pause();
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
    funcs["print"] = print_tv;
    funcs["ln"] = log_tv;
    funcs["sin"] = sin_tv;
    funcs["cos"] = cos_tv;
    funcs["tan"] = tan_tv;
    funcs["asin"] = asin_tv;
    funcs["acos"] = acos_tv;
    funcs["atan"] = atan_tv;
    funcs["abs"] = abs_tv;
    funcs["floor"] = floor_tv;
    funcs["ceil"] = ceil_tv;
    funcs["sqrt"] = sqrt_tv;
    funcs["size"] = size_tv;
    funcs["char"] = char_tv;
    funcs["rand"] = rand_tv;
    funcs["prng"] = prng_tv;
    funcs["getpersistent"] = getpersistent;
    funcs["setpersistent"] = setpersistent;
    funcs["type"] = type_tv;
    funcs["mandelbrot"] = mandelbrot_tv;
    lastcanvaswidth = 300;
    lastcanvasheight = 300;
    tokens = tokenize(std::string("y = x^3 - x"), funcs);
    p = parser(tokens);
    program = p.blk();
    validprogram = true;
    interpret();
    filename = "Untitled";
    filehaschanged = false;
    debugdialog = new DebugDialog(this);
    debugshown = false;
}

proceduralgrapherDialog::~proceduralgrapherDialog()
{
    std::cout << "Destructor...\n";
    validprogram = false;
    delete program;
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
    p = parser(tokens, funcs);
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
        interpreter interp(funcs, parserdata);
        interp_ptr = &interp;
        try {
            interp.evaluate(program);
        }
        catch (token_type_enum t)
        {
            (*txtOutput) << "Error: expected " << token_type_names[t] << " near \"" << p.t.value << "\" (pos. " << static_cast <int> (p.tindex) << ")\n";
        }
        catch (error e)
        {
            (*txtOutput) << e.errstring << "\n" << "Stack level: " << interp.stacklevel << "\n";
        }
        if (debugshown)
            debugdialog->updatevars(&interp);
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
    if (!filehaschanged)
    {
        filehaschanged = true;
        this->SetTitle("Procedural Grapher - " + filename + "*");
    }
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
    if(chkGrid->GetValue())
    {
        double stepx = pow(2, floor(log((parserdata.right - parserdata.left) * 300.00001 / (double)lastcanvaswidth) / log(2))) / 4.0;
        for(double x = floor(parserdata.left/ stepx) * stepx; x < parserdata.right; x+= stepx)
            line2(x, parserdata.top, x, parserdata.bottom);

        double stepy = pow(2, floor(log((parserdata.top - parserdata.bottom) * 300.00001 / (double)lastcanvasheight) / log(2))) / 4.0;
        for(double y = floor(parserdata.bottom / stepy) * stepy; y < parserdata.top; y += stepy)
            line2(parserdata.left, y, parserdata.right, y);
        glColor3f(1, 0, 0);
    }
    parserdata.mousex = parserdata.left + mousex / (lastcanvaswidth / (parserdata.right - parserdata.left));
    parserdata.mousey = parserdata.top - mousey / (lastcanvasheight / (parserdata.top - parserdata.bottom));

    point2(parserdata.mousex, parserdata.mousey);

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

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);


    glEnable(GL_NORMALIZE);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_LINE_SMOOTH);


    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -3);
    glRotatef(parserdata.pitch, 1, 0, 0);
    glRotatef(parserdata.yaw, 0, 1, 0);
    glScalef(3/(parserdata.right - parserdata.left), 3/(parserdata.top - parserdata.bottom), 3/(parserdata.front - parserdata.back));
    glTranslatef((parserdata.left + parserdata.right) / -2, (parserdata.top + parserdata.bottom) / -2, (parserdata.front + parserdata.back) / -2);

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    glColor3f(0.5, 0.5, 0.5);

    // draw grid:

    double stepx = pow(2, floor(log(parserdata.right - parserdata.left) / log(2))) / 2;
    double stepy = pow(2, floor(log(parserdata.top - parserdata.bottom) / log(2))) / 2;
    double stepz = pow(2, floor(log(parserdata.front - parserdata.back) / log(2))) / 2;
    double leftedge = floor(parserdata.left / stepx) * stepx;
    double rightedge = ceil(parserdata.right / stepx) * stepx;
    double bottomedge = floor(parserdata.bottom / stepy) * stepy;
    double topedge = ceil(parserdata.top / stepy) * stepy;
    double backedge = floor(parserdata.back / stepz) * stepz;
    double frontedge = ceil(parserdata.front / stepz) * stepz;

    if (chkGrid->GetValue())
    {
        for(double x = leftedge; x <= rightedge; x += stepx) //xy plane, vertical
        {
            glBegin(GL_LINES);
            glVertex3f(x, bottomedge, 0);
            glVertex3f(x, topedge, 0);
            glEnd();
        }
        for(double y = bottomedge; y <= topedge; y += stepy) //xy plane, horizontal
        {
            glBegin(GL_LINES);
            glVertex3f(leftedge, y, 0);
            glVertex3f(rightedge, y, 0);
            glEnd();
        }

        for(double z = backedge; z <= frontedge; z += stepz) //xz plane, left/right
        {
            glBegin(GL_LINES);
            glVertex3f(leftedge, 0, z);
            glVertex3f(rightedge, 0, z);
            glEnd();
        }
        for(double x = leftedge; x <= rightedge; x += stepx) //xz plane, front/back
        {
            glBegin(GL_LINES);
            glVertex3f(x, 0, backedge);
            glVertex3f(x, 0, frontedge);
            glEnd();
        }

        for(double y = bottomedge; y <= topedge; y += stepy) //yz plane, front/back
        {
            glBegin(GL_LINES);
            glVertex3f(0, y, backedge);
            glVertex3f(0, y, frontedge);
            glEnd();
        }
        for(double z = backedge; z <= frontedge; z += stepz) //yz plane, vertical
        {
            glBegin(GL_LINES);
            glVertex3f(0, bottomedge, z);
            glVertex3f(0, topedge, z);
            glEnd();
        }
    }


    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);



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
    parserdata.setdetail(setdetail);
    if(!Timer1.IsRunning())
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
            parserdata.setdetail(setdetail * 0.4);
        }
        else
        {
            parserdata.yaw -= dx;
            parserdata.pitch -= dy;
        }

        if(!Timer1.IsRunning())
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
        if (!parserdata.is3d)
            parserdata.setdetail(40);
        if(!Timer1.IsRunning())
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
    parserdata.setdetail(100);
    if(!Timer1.IsRunning())
        interpret();
}

void proceduralgrapherDialog::OnGLCanvas1Paint(wxPaintEvent& event)
{
}

void proceduralgrapherDialog::OnGLCanvas1Resize(wxSizeEvent& event)
{
    double centrex = (parserdata.left + parserdata.right) / 2;
    double centrey = (parserdata.bottom + parserdata.top) / 2;
    double centrez = (parserdata.front + parserdata.back) / 2;
    parserdata.left = (parserdata.left - centrex) * event.GetSize().GetWidth() / lastcanvaswidth + centrex;
    parserdata.right = (parserdata.right - centrex) * event.GetSize().GetWidth() / lastcanvaswidth + centrex;
    parserdata.bottom = (parserdata.bottom - centrey) * event.GetSize().GetHeight() / lastcanvasheight + centrey;
    parserdata.top = (parserdata.top - centrey) * event.GetSize().GetHeight() / lastcanvasheight + centrey;
    parserdata.front = (parserdata.front - centrez) * event.GetSize().GetWidth() / lastcanvaswidth + centrez;
    parserdata.back = (parserdata.back - centrez) * event.GetSize().GetWidth() / lastcanvaswidth + centrez;
    lastcanvasheight = event.GetSize().GetHeight();
    lastcanvaswidth = event.GetSize().GetWidth();
    if(validprogram && !Timer1.IsRunning())
        interpret();
}

void proceduralgrapherDialog::Onchk3DClick(wxCommandEvent& event)
{
    parserdata.is3d = chk3D->GetValue();
    if(!Timer1.IsRunning())
        interpret();
}

void proceduralgrapherDialog::OnTimer1Trigger(wxTimerEvent& event)
{
    parserdata.dt = StopWatch1.Time() - parserdata.time;
    parserdata.time = StopWatch1.Time();
    interpret();
}

void proceduralgrapherDialog::OnbtnStartStopTimeClick(wxCommandEvent& event)
{
    if(Timer1.IsRunning())
    {
        Timer1.Stop();
        StopWatch1.Pause();
        btnStartStopTime->SetLabel("Start Time");
        if (!ToolBarItem4->IsToggled())
            ToolBarItem4->SetToggle(true);
    }
    else
    {
        Timer1.Start();
        StopWatch1.Resume();
        btnStartStopTime->SetLabel("Stop Time");
        if (!ToolBarItem4->IsToggled())
            ToolBarItem4->SetToggle(true);
   }
}

void proceduralgrapherDialog::OnbtnResetTimeClick(wxCommandEvent& event)
{
    bool wasrunning = Timer1.IsRunning();
    Timer1.Stop();
    StopWatch1.Pause();
    Timer1.Start(0);
    StopWatch1.Start(0);
    if(!wasrunning)
    {
        Timer1.Stop();
        StopWatch1.Pause();
    }
    parserdata.time = 0;
    interpret();
}

void proceduralgrapherDialog::OnchkGridClick(wxCommandEvent& event)
{
    interpret();
}

void proceduralgrapherDialog::OnFileOpen(wxCommandEvent& event)
{
    if (filehaschanged)
    {
        int response = wxMessageDialog(this, "Save changes to " + filename + "?", "Open File", wxYES_NO | wxCANCEL | wxICON_EXCLAMATION).ShowModal();
        if (response == wxID_CANCEL)
            return;
        if (response == wxID_YES)
            OnFileSave(event);
    }
    if (FileDialogOpen->ShowModal() == wxID_OK)
    {
        filepath = FileDialogOpen->GetPath();
        txtExpr->LoadFile(filepath);
        filename = FileDialogOpen->GetFilename();
        this->SetTitle("Procedural Grapher - " + filename);
        filehaschanged = false;
    }

}


void proceduralgrapherDialog::OnFileSave(wxCommandEvent& event)
{
    if (filename == "Untitled")
    {
        OnFileSaveAs(event);
        return;
    }
    if (filehaschanged)
    {
        txtExpr->SaveFile(filepath);
        filehaschanged = false;
        this->SetTitle("Procedural Grapher - " + filename);
    }
}

void proceduralgrapherDialog::OnFileSaveAs(wxCommandEvent& event)
{
    if (FileDialogSaveAs->ShowModal() == wxID_OK)
    {
        filehaschanged = false;
        filename = FileDialogSaveAs->GetFilename();
        filepath = FileDialogSaveAs->GetPath();
        txtExpr->SaveFile(filepath);
        this->SetTitle("Procedural Grapher - " + filename);
    }
}

void proceduralgrapherDialog::loadfile(std::string filepath_)
{
    filepath = filepath_;
    txtExpr->LoadFile(filepath);
    int pos = filepath.rfind("\\");
    if (pos == -1)
        pos = filepath.rfind("/");
    filename = filepath.substr(pos + 1, -1);
    this->SetTitle("Procedural Grapher - " + filename);
    filehaschanged = false;
}

void proceduralgrapherDialog::OnFileNew(wxCommandEvent& event)
{
    if (filehaschanged)
    {
        int response = wxMessageDialog(this, "Save changes to " + filename + "?", "New File", wxYES_NO | wxCANCEL | wxICON_EXCLAMATION).ShowModal();
        if (response == wxID_CANCEL)
            return;
        if (response == wxID_YES)
            OnFileSave(event);
    }
    txtExpr->SetValue("");
    filename = "Untitled";
    this->SetTitle("Procedural Grapher - Untitled");
    filehaschanged = false;
}

void proceduralgrapherDialog::OnClose(wxCloseEvent& event)
{
    std::cout << "Close event...\n";
    if (filehaschanged)
    {
        int response = wxMessageDialog(this, "Save changes to " + filename + "?", "Procedural Grapher", wxYES_NO | wxCANCEL | wxICON_EXCLAMATION).ShowModal();
        if (response == wxID_CANCEL)
            event.Veto();
        else if (response == wxID_YES)
        {
            wxCommandEvent cmdevent = wxCommandEvent();
            OnFileSave(cmdevent);
            if (!filehaschanged)
                this->Destroy();
        }
        else
            this->Destroy();
    }
    else
        this->Destroy();
}

void proceduralgrapherDialog::OnDebugClicked(wxCommandEvent& event)
{
    //if (!debugshown)
        debugdialog->Show(true);
        debugshown = true;
}

void proceduralgrapherDialog::OnDetailSliderCmdScrollThumbTrack(wxScrollEvent& event)
{
    setdetail = 100 * pow(1.3, event.GetPosition());
    parserdata.setdetail(setdetail);
    std::cout << "detail: " << parserdata.detail << "\n";
    interpret();
}
