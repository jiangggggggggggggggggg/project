#include <wx/wx.h>
#include "canvas.h"
#include "menu.h"

class MyApp : public wxApp {
public:
    bool OnInit() override {
        MyFrame* frame = new MyFrame("电路图编辑器");
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);
