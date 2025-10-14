#include <wx/wx.h>
#include "mainframe.h"
#include "menu.h"
#include "canvas.h"
#include "logicgates.h"

class MyApp : public wxApp {
public:
    virtual bool OnInit() override {
        MainFrame* frame = new MainFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);