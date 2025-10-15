
#ifndef MENU_H
#define MENU_H

#include <wx/wx.h>

class MenuHandler {
private:
    wxFrame* m_parentFrame;  // ������ָ��

public:
    MenuHandler(wxFrame* parent);

    // �����˵���
    wxMenuBar* CreateMenuBar();

    // �󶨲˵��¼�
    void BindEvents();

    // �˵��¼�������
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
