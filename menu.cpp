#include "menu.h"

MenuHandler::MenuHandler(wxFrame* parent) : parentFrame(parent) {}

void MenuHandler::OnNew(wxCommandEvent& event) {
    // �����½��ļ�
}

void MenuHandler::OnOpen(wxCommandEvent& event) {
    wxFileDialog openFileDialog(parentFrame, "Open File", "", "",
        "All Files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    wxString path = openFileDialog.GetPath();
    wxMessageBox("Selected file:\n" + path, "File Opened", wxOK | wxICON_INFORMATION);
}

void MenuHandler::OnSave(wxCommandEvent& event) {
    // �������ļ�
}

void MenuHandler::OnSaveAs(wxCommandEvent& event) {
    // �������Ϊ�ļ�
}

void MenuHandler::OnUndo(wxCommandEvent& event) {
    // ����������
}

void MenuHandler::OnCut(wxCommandEvent& event) {
    // ������в���
}

void MenuHandler::OnCopy(wxCommandEvent& event) {
    // �����Ʋ���
}

void MenuHandler::OnPaste(wxCommandEvent& event) {
    // ����ճ������
}

void MenuHandler::OnSettings(wxCommandEvent& event) {
    // ��������
}

void MenuHandler::OnMeasureTool(wxCommandEvent& event) {
    // �����������
}

void MenuHandler::OnBatchProcess(wxCommandEvent& event) {
    // ����������
}

void MenuHandler::OnHelp(wxCommandEvent& event) {
    // �������
}

void MenuHandler::OnAbout(wxCommandEvent& event) {
    // ���������Ϣ
}