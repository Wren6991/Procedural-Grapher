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

IMPLEMENT_APP(proceduralgrapherApp);

bool proceduralgrapherApp::OnInit()
{
    //AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	proceduralgrapherDialog Dlg(0);
    	SetTopWindow(&Dlg);
    	Dlg.SetIcon(wxIcon("aaaa", wxBITMAP_TYPE_ICO_RESOURCE));
        Dlg.ShowModal();
    	Dlg.initgl();
    	Dlg.endgl();
    	wxsOK = false;
    }

    return wxsOK;

}
