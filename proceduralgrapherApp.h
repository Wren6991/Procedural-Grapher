/***************************************************************
 * Name:      proceduralgrapherApp.h
 * Purpose:   Defines Application Class
 * Author:    Luke Wren (wren6991@gmail.com)
 * Created:   2011-12-14
 * Copyright: Luke Wren ()
 * License:
 **************************************************************/

#ifndef PROCEDURALGRAPHERAPP_H
#define PROCEDURALGRAPHERAPP_H

#include <wx/app.h>
#include <wx/cmdline.h>


#include "proceduralgrapherMain.h"

class proceduralgrapherApp : public wxApp
{
    proceduralgrapherDialog* Dlg;
    public:
        virtual bool OnInit();
};

#endif // PROCEDURALGRAPHERAPP_H
