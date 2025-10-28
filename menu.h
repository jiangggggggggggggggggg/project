#pragma once
#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/treectrl.h>
#include <stack>
#include "canvas.h"

class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);

private:
    MyDrawPanel* m_drawPanel;
    wxTreeCtrl* m_treeCtrl;
    wxSplitterWindow* m_splitter;
    wxString m_currentFile;

    // ��������ջ
    std::stack<MyDrawPanel::DrawPanelState> undoStack;
    std::stack<MyDrawPanel::DrawPanelState> redoStack;

    // �˵��¼�����
    void OnNew(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);
    void OnCopy(wxCommandEvent& event);
    void OnPaste(wxCommandEvent& event);
    void OnZoomin(wxCommandEvent& event);
    void OnZoomout(wxCommandEvent& event);
    void OnToggleStatusBar(wxCommandEvent& event);
    void OnToggleGrid(wxCommandEvent& event);
    void OnDeleteSelected(wxCommandEvent& event);
    void OnDeleteWire(wxCommandEvent& event);
    void OnEditProperties(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    // �Զ����¼�����
    void OnCustomEvent(wxCommandEvent& event);

    // ��������
    void SaveStateForUndo();
    void DoSave(const wxString& filename);
    void UpdateTitle();

    wxDECLARE_EVENT_TABLE();
};