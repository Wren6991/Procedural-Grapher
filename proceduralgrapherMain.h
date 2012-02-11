/***************************************************************
 * Name:      proceduralgrapherMain.h
 * Purpose:   Defines Application Frame
 * Author:    Luke Wren (wren6991@gmail.com)
 * Created:   2011-12-14
 * Copyright: Luke Wren ()
 * License:
 **************************************************************/

#ifndef PROCEDURALGRAPHERMAIN_H
#define PROCEDURALGRAPHERMAIN_H

//(*Headers(proceduralgrapherDialog)
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/glcanvas.h>
#include <wx/toolbar.h>
#include <wx/stopwatch.h>
#include <wx/panel.h>
#include <wx/filedlg.h>
#include <wx/button.h>
#include <wx/frame.h>
#include <wx/timer.h>
//*)
#include "parser.h"
#include <vector>
#include <map>

class proceduralgrapherDialog: public wxFrame
{
    public:

        proceduralgrapherDialog(wxWindow* parent,wxWindowID id = -1);
        virtual ~proceduralgrapherDialog();
        void init2d();
        void init3d();
        void endgl();
        void print(std::string str);
        void loadfile(std::string);

    private:

        //(*Handlers(proceduralgrapherDialog)
        void OnQuit(wxCommandEvent& event);
        void Tokenize(wxCommandEvent& event);
        void OncanvasPaint(wxPaintEvent& event);
        void OntxtOutputText(wxCommandEvent& event);
        void OnGLCanvas1LeftDown(wxMouseEvent& event);
        void OnGLCanvas1LeftUp(wxMouseEvent& event);
        void OnGLCanvas1MouseMove(wxMouseEvent& event);
        void OnGLCanvas1MouseWheel(wxMouseEvent& event);
        void OnGLCanvas1MiddleDown(wxMouseEvent& event);
        void OnGLCanvas1MiddleUp(wxMouseEvent& event);
        void OnGLCanvas1Paint(wxPaintEvent& event);
        void OnGLCanvas1Resize(wxSizeEvent& event);
        void OnNotebook1PageChanged(wxNotebookEvent& event);
        void Onchk3DClick(wxCommandEvent& event);
        void OnTimer1Trigger(wxTimerEvent& event);
        void OnbtnStartStopTimeClick(wxCommandEvent& event);
        void OnbtnResetTimeClick(wxCommandEvent& event);
        void OnPanel1Paint(wxPaintEvent& event);
        void OnGLCanvas1RightDown(wxMouseEvent& event);
        void OnchkGridClick(wxCommandEvent& event);
        void OnPollTimerTrigger(wxTimerEvent& event);
        void OnFileOpen(wxCommandEvent& event);
        void OnSave(wxCommandEvent& event);
        void OnFileSave(wxCommandEvent& event);
        void OnFileSaveAs(wxCommandEvent& event);
        void OnFileNew(wxCommandEvent& event);
        void OnClose(wxCloseEvent& event);
        //*)
        void parse();
        void interpret();
        bool validprogram;
        std::string parserror;

        //(*Identifiers(proceduralgrapherDialog)
        static const long ID_TXTEXPR;
        static const long ID_TXTOUTPUT;
        static const long ID_CHECKBOX1;
        static const long ID_CHECKBOX2;
        static const long ID_PANEL1;
        static const long ID_BUTTON1;
        static const long ID_BUTTON2;
        static const long ID_PANEL2;
        static const long ID_NOTEBOOK1;
        static const long ID_GLCANVAS1;
        static const long ID_TIMER1;
        static const long tbrNew;
        static const long tbrOpen;
        static const long tbrSave;
        static const long tbrTime;
        static const long ID_TOOLBAR1;
        //*)

        //(*Declarations(proceduralgrapherDialog)
        wxToolBarToolBase* ToolBarItem4;
        wxToolBar* ToolBar1;
        wxButton* btnStartStopTime;
        wxTextCtrl* txtOutput;
        wxNotebook* Notebook1;
        wxToolBarToolBase* ToolBarItem3;
        wxFileDialog* FileDialogOpen;
        wxGLCanvas* GLCanvas1;
        wxButton* btnResetTime;
        wxPanel* Panel1;
        wxTextCtrl* txtExpr;
        wxToolBarToolBase* ToolBarItem1;
        wxStopWatch StopWatch1;
        wxBoxSizer* BoxSizer1;
        wxPanel* Panel2;
        wxCheckBox* chkGrid;
        wxToolBarToolBase* ToolBarItem2;
        wxTimer Timer1;
        wxCheckBox* chk3D;
        wxFileDialog* FileDialogSaveAs;
        //*)
        std::vector <token> tokens;
        parser p;
        std::map <std::string, dfuncd> funcs;
        block *program;
        wxGLContext* GLContext1;
        bool donedrawing;
        double mousex;
        double mousey;
        bool leftdown;
        bool middledown;
        int lastcanvaswidth;
        int lastcanvasheight;
        bool filehaschanged;
        std::string filename;
        std::string filepath;
        DECLARE_EVENT_TABLE()
};

#endif // PROCEDURALGRAPHERMAIN_H
