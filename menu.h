#ifndef MENU_H
#define MENU_H

#include <wx/wx.h>
#include <wx/filedlg.h>

class MenuHandler {
public:
    MenuHandler(wxFrame* parent);

    void OnNew(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnUndo(wxCommandEvent& event);
    void OnCut(wxCommandEvent& event);
    void OnCopy(wxCommandEvent& event);
    void OnPaste(wxCommandEvent& event);
    void OnSettings(wxCommandEvent& event);
    void OnMeasureTool(wxCommandEvent& event);
    void OnBatchProcess(wxCommandEvent& event);
    void OnHelp(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

private:
    wxFrame* parentFrame;
};

#endif