
#ifndef MENU_H
#define MENU_H

#include <wx/wx.h>

class MenuHandler {
private:
    wxFrame* m_parentFrame;  // 父窗口指针

public:
    MenuHandler(wxFrame* parent);

    // 创建菜单栏
    wxMenuBar* CreateMenuBar();

    // 绑定菜单事件
    void BindEvents();

    // 菜单事件处理函数
    void OnNewFile(wxCommandEvent& event);
    void OnOpenFile(wxCommandEvent& event);
    void OnSaveFile(wxCommandEvent& event);
    void OnSaveAsFile(wxCommandEvent& event);

    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);
    void OnCut(wxCommandEvent& event);
    void OnCopy(wxCommandEvent& event);
    void OnPaste(wxCommandEvent& event);

    void OnSettings(wxCommandEvent& event);
    void OnMeasureTool(wxCommandEvent& event);
    void OnBatchProcess(wxCommandEvent& event);

    void OnHelp(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
};

#endif // MENU_H
