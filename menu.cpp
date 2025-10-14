#include "menu.h"

MenuHandler::MenuHandler(wxFrame* parent) : parentFrame(parent) {}

void MenuHandler::OnNew(wxCommandEvent& event) {
    // 处理新建文件
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
    // 处理保存文件
}

void MenuHandler::OnSaveAs(wxCommandEvent& event) {
    // 处理另存为文件
}

void MenuHandler::OnUndo(wxCommandEvent& event) {
    // 处理撤销操作
}

void MenuHandler::OnCut(wxCommandEvent& event) {
    // 处理剪切操作
}

void MenuHandler::OnCopy(wxCommandEvent& event) {
    // 处理复制操作
}

void MenuHandler::OnPaste(wxCommandEvent& event) {
    // 处理粘贴操作
}

void MenuHandler::OnSettings(wxCommandEvent& event) {
    // 处理设置
}

void MenuHandler::OnMeasureTool(wxCommandEvent& event) {
    // 处理测量工具
}

void MenuHandler::OnBatchProcess(wxCommandEvent& event) {
    // 处理批处理
}

void MenuHandler::OnHelp(wxCommandEvent& event) {
    // 处理帮助
}

void MenuHandler::OnAbout(wxCommandEvent& event) {
    // 处理关于信息
}