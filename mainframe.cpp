#include "mainframe.h"
#include <wx/aboutdlg.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_TREE_SEL_CHANGED(wxID_ANY, MainFrame::OnTreeItemSelected)
EVT_TREE_ITEM_ACTIVATED(wxID_ANY, MainFrame::OnTreeItemActivated)
EVT_MENU(wxID_OPEN, MainFrame::OnOpenFile)
EVT_MENU(wxID_NEW, MainFrame::OnNewFile)
EVT_MENU(wxID_SAVE, MainFrame::OnSaveFile)
EVT_MENU(wxID_SAVEAS, MainFrame::OnSaveAsFile)
EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
wxEND_EVENT_TABLE()

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "Logisim Clone - Circuit Schematic Editor",
        wxDefaultPosition, wxSize(1000, 700)) {

    // 初始化元件库
    componentLib = new ComponentLibrary();

    // 创建界面
    CreateMenuBar();
    CreateSplitter();
    InitializeComponentLibrary();

    // 设置窗口图标和状态栏
    SetIcon(wxICON(sample)); // 如果有图标资源的话
    CreateStatusBar();
    SetStatusText("Ready - Logisim Clone Circuit Editor");
}

MainFrame::~MainFrame() {
    if (componentLib) {
        delete componentLib;
    }
}

void MainFrame::CreateMenuBar() {
    wxMenuBar* menuBar = new wxMenuBar();

    // File 菜单
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(wxID_NEW, "&New\tCtrl+N", "Create a new circuit");
    fileMenu->Append(wxID_OPEN, "&Open\tCtrl+O", "Open an existing circuit");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_SAVE, "&Save\tCtrl+S", "Save the current circuit");
    fileMenu->Append(wxID_SAVEAS, "Save &As\tCtrl+Shift+S", "Save the current circuit with a new name");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit\tAlt+F4", "Exit the application");
    menuBar->Append(fileMenu, "&File");

    // Edit 菜单
    wxMenu* editMenu = new wxMenu();
    editMenu->Append(wxID_UNDO, "&Undo\tCtrl+Z", "Undo the last action");
    editMenu->Append(wxID_REDO, "&Redo\tCtrl+Y", "Redo the last undone action");
    editMenu->AppendSeparator();
    editMenu->Append(wxID_CUT, "Cu&t\tCtrl+X", "Cut the selection to clipboard");
    editMenu->Append(wxID_COPY, "&Copy\tCtrl+C", "Copy the selection to clipboard");
    editMenu->Append(wxID_PASTE, "&Paste\tCtrl+V", "Paste from clipboard");
    editMenu->AppendSeparator();
    editMenu->Append(wxID_DELETE, "&Delete\tDel", "Delete the selection");
    editMenu->Append(wxID_SELECTALL, "Select &All\tCtrl+A", "Select all components");
    menuBar->Append(editMenu, "&Edit");

    // Project 菜单
    wxMenu* projectMenu = new wxMenu();
    projectMenu->Append(wxID_ANY, "&Add Circuit", "Add a new circuit to the project");
    projectMenu->Append(wxID_ANY, "&Remove Circuit", "Remove the current circuit");
    projectMenu->Append(wxID_ANY, "Circuit &Properties", "Edit circuit properties");
    projectMenu->AppendSeparator();
    projectMenu->Append(wxID_ANY, "&Analyze Circuit", "Analyze the current circuit");
    projectMenu->Append(wxID_ANY, "&Simulate Circuit", "Run circuit simulation");
    menuBar->Append(projectMenu, "&Project");

    // Tools 菜单
    wxMenu* toolsMenu = new wxMenu();
    toolsMenu->Append(wxID_PREFERENCES, "&Settings\tCtrl+.", "Configure application settings");
    toolsMenu->Append(wxID_ANY, "&Measure Tool\tCtrl+M", "Measure distances on the canvas");
    toolsMenu->Append(wxID_ANY, "&Batch Process\tCtrl+B", "Batch process multiple circuits");
    toolsMenu->AppendSeparator();
    toolsMenu->Append(wxID_ANY, "&Library Manager", "Manage component libraries");
    toolsMenu->Append(wxID_ANY, "&Custom Components", "Create and edit custom components");
    menuBar->Append(toolsMenu, "&Tools");

    // Help 菜单
    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(wxID_HELP, "&Help\tF1", "Show help documentation");
    helpMenu->Append(wxID_ABOUT, "&About", "About this application");
    menuBar->Append(helpMenu, "&Help");

    SetMenuBar(menuBar);
}

void MainFrame::CreateSplitter() {
    splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D);
    splitter->SetSashGravity(0.2); // 左侧占20%宽度
    splitter->SetMinimumPaneSize(150); // 最小窗格大小

    // 创建左侧树形控件 - 使用 wxTR_HIDE_ROOT 隐藏根节点
    treeCtrl = new wxTreeCtrl(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTR_HAS_BUTTONS | wxTR_SINGLE | wxTR_HIDE_ROOT);
    treeCtrl->SetForegroundColour(*wxBLACK);
    treeCtrl->SetBackgroundColour(wxColour(240, 240, 240)); // 浅灰色背景

    // 创建右侧画布
    canvas = new Canvas(splitter);
    canvas->SetComponentLibrary(componentLib);

    // 分割窗口
    splitter->SplitVertically(treeCtrl, canvas);

    // 设置主sizer
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(splitter, 1, wxEXPAND | wxALL, 0);
    SetSizer(mainSizer);

    // 设置分割条位置
    splitter->SetSashPosition(200);
}

void MainFrame::InitializeComponentLibrary() {
    if (componentLib) {
        componentLib->Initialize();
        PopulateTreeFromLibrary();
    }
}

void MainFrame::PopulateTreeFromLibrary() {
    if (!componentLib || !treeCtrl) return;

    treeCtrl->DeleteAllItems();
    // 创建根节点（会被隐藏）
    wxTreeItemId root = treeCtrl->AddRoot("Component Library");

    // 获取所有类别
    std::vector<wxString> categories = componentLib->GetCategories();

    for (const auto& category : categories) {
        wxTreeItemId categoryItem = treeCtrl->AppendItem(root, category);

        // 获取该类别下的所有元件
        std::vector<ComponentInfo*> components = componentLib->GetComponentsByCategory(category);

        for (ComponentInfo* comp : components) {
            treeCtrl->AppendItem(categoryItem, comp->name);
            // 工具提示功能暂时移除，信息将在状态栏显示
        }

        // 展开类别节点（这些是根节点的直接子节点，不会被隐藏）
        treeCtrl->Expand(categoryItem);
    }

    // 不要尝试展开隐藏的根节点
}

void MainFrame::OnTreeItemSelected(wxTreeEvent& event) {
    wxTreeItemId itemId = event.GetItem();
    if (!itemId.IsOk()) return;

    wxString itemText = treeCtrl->GetItemText(itemId);

    // 由于使用了 wxTR_HIDE_ROOT，所有可见节点都是类别或元件
    // 检查是否是类别节点（根节点的直接子节点）
    if (treeCtrl->GetItemParent(itemId) == treeCtrl->GetRootItem()) {
        // 这是类别节点
        SetStatusText(wxString::Format("Category: %s", itemText));
    }
    else {
        // 这是元件节点
        ComponentInfo* compInfo = componentLib->GetComponentInfo(itemText);
        if (compInfo) {
            // 在状态栏显示详细元件信息
            wxString statusText = wxString::Format("Selected: %s - %s (Inputs: %d, Outputs: %d)",
                compInfo->name, compInfo->description, compInfo->inputCount, compInfo->outputCount);
            SetStatusText(statusText);

            // 通知画布选择了一个元件
            if (canvas) {
                canvas->SetSelectedComponent(compInfo->type);
            }
        }
    }

    event.Skip();
}

void MainFrame::OnTreeItemActivated(wxTreeEvent& event) {
    wxTreeItemId itemId = event.GetItem();
    if (!itemId.IsOk()) return;

    wxString itemText = treeCtrl->GetItemText(itemId);

    // 检查是否是类别节点
    if (treeCtrl->GetItemParent(itemId) == treeCtrl->GetRootItem()) {
        // 类别项被双击：切换展开/折叠
        if (treeCtrl->IsExpanded(itemId)) {
            treeCtrl->Collapse(itemId);
        }
        else {
            treeCtrl->Expand(itemId);
        }
    }
    else {
        // 元件项被双击：准备放置
        ComponentInfo* compInfo = componentLib->GetComponentInfo(itemText);
        if (compInfo && canvas) {
            canvas->SetSelectedComponent(compInfo->type);
            SetStatusText(wxString::Format("Ready to place: %s - Click on canvas to place", compInfo->name));
        }
    }

    event.Skip();
}

void MainFrame::OnNewFile(wxCommandEvent& event) {
    if (canvas) {
        if (wxMessageBox("Create new circuit? Unsaved changes will be lost.",
            "New Circuit", wxYES_NO | wxICON_QUESTION) == wxYES) {
            canvas->ClearCanvas();
            SetStatusText("New circuit created");
            SetTitle("Logisim Clone - Untitled");
        }
    }
    event.Skip();
}

void MainFrame::OnOpenFile(wxCommandEvent& event) {
    wxFileDialog openFileDialog(this, "Open Circuit File", "", "",
        "Circuit Files (*.circ)|*.circ|All Files (*.*)|*.*",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    wxString filePath = openFileDialog.GetPath();
    // 这里应该实现文件加载逻辑
    wxMessageBox(wxString::Format("Would load circuit from: %s", filePath),
        "Open File", wxOK | wxICON_INFORMATION);
    SetStatusText(wxString::Format("Loaded: %s", filePath));
    SetTitle(wxString::Format("Logisim Clone - %s", openFileDialog.GetFilename()));

    event.Skip();
}

void MainFrame::OnSaveFile(wxCommandEvent& event) {
    // 这里应该实现文件保存逻辑
    wxMessageBox("Save functionality to be implemented",
        "Save File", wxOK | wxICON_INFORMATION);
    SetStatusText("Circuit saved");
    event.Skip();
}

void MainFrame::OnSaveAsFile(wxCommandEvent& event) {
    wxFileDialog saveFileDialog(this, "Save Circuit As", "", "",
        "Circuit Files (*.circ)|*.circ|All Files (*.*)|*.*",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveFileDialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    wxString filePath = saveFileDialog.GetPath();
    // 这里应该实现文件另存为逻辑
    wxMessageBox(wxString::Format("Would save circuit to: %s", filePath),
        "Save As", wxOK | wxICON_INFORMATION);
    SetStatusText(wxString::Format("Saved as: %s", filePath));
    SetTitle(wxString::Format("Logisim Clone - %s", saveFileDialog.GetFilename()));

    event.Skip();
}

void MainFrame::OnExit(wxCommandEvent& event) {
    if (wxMessageBox("Are you sure you want to exit?", "Exit Application",
        wxYES_NO | wxICON_QUESTION) == wxYES) {
        Close(true);
    }
    else {
        event.Skip();
    }
}

void MainFrame::OnAbout(wxCommandEvent& event) {
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName("Logisim Clone");
    aboutInfo.SetVersion("1.0.0");
    aboutInfo.SetDescription("A digital circuit schematic editor and simulator\n"
        "Inspired by Logisim");
    aboutInfo.SetCopyright("(C) 2024");
    aboutInfo.SetWebSite("https://github.com/yourusername/logisim-clone");
    aboutInfo.AddDeveloper("Your Name");
    aboutInfo.SetLicense("GPL v3 or later");

    wxAboutBox(aboutInfo);
    event.Skip();
}