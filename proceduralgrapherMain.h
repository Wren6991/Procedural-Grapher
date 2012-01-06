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
#include <wx/panel.h>
#include <wx/dialog.h>
//*)
#include "parser.h"
#include <vector>
#include <map>

class proceduralgrapherDialog: public wxDialog
{
    public:

        proceduralgrapherDialog(wxWindow* parent,wxWindowID id = -1);
        virtual ~proceduralgrapherDialog();
        void init2d();
        void init3d();
        void endgl();
        void print(std::string str);

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
        //*)
        void parse();
        void interpret();
        bool validprogram;
        std::string parserror;

        //(*Identifiers(proceduralgrapherDialog)
        static const long ID_TXTEXPR;
        static const long ID_TXTOUTPUT;
        static const long ID_CHECKBOX1;
        static const long ID_PANEL1;
        static const long ID_PANEL2;
        static const long ID_NOTEBOOK1;
        static const long ID_GLCANVAS1;
        //*)

        //(*Declarations(proceduralgrapherDialog)
        wxTextCtrl* txtOutput;
        wxNotebook* Notebook1;
        wxGLCanvas* GLCanvas1;
        wxPanel* Panel1;
        wxTextCtrl* txtExpr;
        wxBoxSizer* BoxSizer1;
        wxPanel* Panel2;
        wxCheckBox* chk3D;
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
        DECLARE_EVENT_TABLE()
};

#endif // PROCEDURALGRAPHERMAIN_H
