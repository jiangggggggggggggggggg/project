#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include "canvas.h"
#include "menu.h"
#include "componentlibrary.h"

class MainFrame : public wxFrame {
public:
    MainFrame();
    ~MainFrame();

private:
    void CreateMenuBar();
    void CreateTreeCtrl();
    void CreateSplitter();
    void InitializeComponentLibrary();
    void PopulateTreeFromLibrary();

    // 事件处理函数
    void OnTreeItemSelected(wxTreeEvent& event);
    void OnTreeItemActivated(wxTreeEvent& event);
    void OnOpenFile(wxCommandEvent& event);
    void OnNewFile(wxCommandEvent& event);
    void OnSaveFile(wxCommandEvent& event);
    void OnSaveAsFile(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    // 成员变量
    wxTreeCtrl* treeCtrl;
    Canvas* canvas;
    MenuHandler* menuHandler;
    wxSplitterWindow* splitter;
    ComponentLibrary* componentLib;

    wxDECLARE_EVENT_TABLE();
};

#endif // MAINFRAME_H