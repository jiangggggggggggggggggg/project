#include "menu.h"
#include "utils.h"
#include "canvas.h"

MenuHandler::MenuHandler(wxFrame* parent) : m_parentFrame(parent) {}

wxMenuBar* MenuHandler::CreateMenuBar() {
    wxMenuBar* menuBar = new wxMenuBar();

    // 创建"File"菜单
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(wxID_NEW, "New\tCtrl+N");
    fileMenu->Append(wxID_OPEN, "Open\tCtrl+O");
    fileMenu->Append(wxID_SAVE, "Save\tCtrl+S");
    fileMenu->Append(wxID_SAVEAS, "Save As\tCtrl+Shift+S");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "Exit");
    menuBar->Append(fileMenu, "File");

    // 创建"Edit"菜单
    wxMenu* editMenu = new wxMenu();
    editMenu->Append(wxID_UNDO, "Undo\tCtrl+Z");
    editMenu->Append(wxID_REDO, "Redo\tCtrl+Y");
    editMenu->AppendSeparator();
    editMenu->Append(wxID_CUT, "Cut\tCtrl+X");
    editMenu->Append(wxID_COPY, "Copy\tCtrl+C");
    editMenu->Append(wxID_PASTE, "Paste\tCtrl+V");
    editMenu->Append(wxID_DELETE, "Delete\tDel");
    menuBar->Append(editMenu, "Edit");

    // 创建"Tools"菜单
    wxMenu* toolsMenu = new wxMenu();
    toolsMenu->Append(wxID_ANY, "Settings\tCtrl+S");
    toolsMenu->Append(wxID_ANY, "Measure Tool\tCtrl+M");
    toolsMenu->Append(wxID_ANY, "Batch Process\tCtrl+B");
    menuBar->Append(toolsMenu, "Tools");

    // 创建"Help"菜单
    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(wxID_HELP, "Help\tF1");
    helpMenu->Append(wxID_ABOUT, "About\tCtrl+A");
    menuBar->Append(helpMenu, "Help");

    return menuBar;
}

void MenuHandler::BindEvents() {
    // 绑定File菜单事件
    m_parentFrame->Bind(wxEVT_MENU, &MenuHandler::OnNewFile, this, wxID_NEW);
    m_parentFrame->Bind(wxEVT_MENU, &MenuHandler::OnOpenFile, this, wxID_OPEN);
    m_parentFrame->Bind(wxEVT_MENU, &MenuHandler::OnSaveFile, this, wxID_SAVE);
    m_parentFrame->Bind(wxEVT_MENU, &MenuHandler::OnSaveAsFile, this, wxID_SAVEAS);
    m_parentFrame->Bind(wxEVT_MENU, [this](wxCommandEvent&) { m_parentFrame->Close(); }, wxID_EXIT);

    // 绑定Edit菜单事件
    m_parentFrame->Bind(wxEVT_MENU, &MenuHandler::OnUndo, this, wxID_UNDO);
    m_parentFrame->Bind(wxEVT_MENU, &MenuHandler::OnRedo, this, wxID_REDO);
    m_parentFrame->Bind(wxEVT_MENU, &MenuHandler::OnCut, this, wxID_CUT);
    m_parentFrame->Bind(wxEVT_MENU, &MenuHandler::OnCopy, this, wxID_COPY);
    m_parentFrame->Bind(wxEVT_MENU, &MenuHandler::OnPaste, this, wxID_PASTE);

    // 绑定Tools菜单事件（使用自定义ID）
    const int ID_SETTINGS = 1001;
    const int ID_MEASURE = 1002;
    const int ID_BATCH = 1003;
    m_parentFrame->Bind(wxEVT_MENU, &MenuHandler::OnSettings, this, ID_SETTINGS);
    m_parentFrame->Bind(wxEVT_MENU, &MenuHandler::OnMeasureTool, this, ID_MEASURE);
    m_parentFrame->Bind(wxEVT_MENU, &MenuHandler::OnBatchProcess, this, ID_BATCH);

    // 绑定Help菜单事件
    m_parentFrame->Bind(wxEVT_MENU, &MenuHandler::OnHelp, this, wxID_HELP);
    m_parentFrame->Bind(wxEVT_MENU, &MenuHandler::OnAbout, this, wxID_ABOUT);
}

void MenuHandler::OnNewFile(wxCommandEvent& event) {
    // 清空画布
    // 实际实现需要获取Canvas实例并调用Clear()
    wxMessageBox("New file functionality will be implemented here", "Not Implemented", wxOK);
}

void MenuHandler::OnOpenFile(wxCommandEvent& event) {
    wxFileDialog openFileDialog(m_parentFrame, "Open File", "", "",
        "Logisim Files (*.circ)|*.circ|All Files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    wxString path = openFileDialog.GetPath();
    // 调用工具类加载文件
    // bool success = FileUtils::LoadCircuit(path);
    wxMessageBox("Opening file: " + path, "File Opened", wxOK);
}

void MenuHandler::OnSaveFile(wxCommandEvent& event) {
    wxMessageBox("Save file functionality will be implemented here", "Not Implemented", wxOK);
}

void MenuHandler::OnSaveAsFile(wxCommandEvent& event) {
    wxMessageBox("Save As functionality will be implemented here", "Not Implemented", wxOK);
}

void MenuHandler::OnUndo(wxCommandEvent& event) {
    wxMessageBox("Undo functionality will be implemented here", "Not Implemented", wxOK);
}

void MenuHandler::OnRedo(wxCommandEvent& event) {
    wxMessageBox("Redo functionality will be implemented here", "Not Implemented", wxOK);
}

void MenuHandler::OnCut(wxCommandEvent& event) {
    wxMessageBox("Cut functionality will be implemented here", "Not Implemented", wxOK);
}

void MenuHandler::OnCopy(wxCommandEvent& event) {
    wxMessageBox("Copy functionality will be implemented here", "Not Implemented", wxOK);
}

void MenuHandler::OnPaste(wxCommandEvent& event) {
    wxMessageBox("Paste functionality will be implemented here", "Not Implemented", wxOK);
}

void MenuHandler::OnSettings(wxCommandEvent& event) {
    wxMessageBox("Settings functionality will be implemented here", "Not Implemented", wxOK);
}

void MenuHandler::OnMeasureTool(wxCommandEvent& event) {
    wxMessageBox("Measure tool functionality will be implemented here", "Not Implemented", wxOK);
}

void MenuHandler::OnBatchProcess(wxCommandEvent& event) {
    wxMessageBox("Batch process functionality will be implemented here", "Not Implemented", wxOK);
}

void MenuHandler::OnHelp(wxCommandEvent& event) {
    wxMessageBox("Help information will be displayed here", "Help", wxOK);
}

void MenuHandler::OnAbout(wxCommandEvent& event) {
    wxMessageBox("Logisim Clone\nA simple digital logic simulator", "About", wxOK | wxICON_INFORMATION);
}


