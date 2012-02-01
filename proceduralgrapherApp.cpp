/***************************************************************
 * Name:      proceduralgrapherApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Luke Wren (wren6991@gmail.com)
 * Created:   2011-12-14
 * Copyright: Luke Wren ()
 * License:
 **************************************************************/

#include "proceduralgrapherApp.h"

//(*AppHeaders
#include "proceduralgrapherMain.h"
#include <wx/image.h>
//*)
#include <iostream>
#include <wx/msgdlg.h>

IMPLEMENT_APP(proceduralgrapherApp);

bool proceduralgrapherApp::OnInit()
{
    //AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	proceduralgrapherDialog* Dlg = new proceduralgrapherDialog(0);
    	SetTopWindow(Dlg);
    	Dlg->SetIcon(wxIcon("aaaa", wxBITMAP_TYPE_ICO_RESOURCE));
        Dlg->Show();
    	Dlg->init2d();
    	Dlg->endgl();
    }

    return wxsOK;

}

/*int ::(wxEvent& event)
{
    if (event.GetEventType() == wxEVT_KEY_DOWN)
    {
        wxMessageBox("the quick bromn fox jumps over the lazy dog");
    }
}*/



