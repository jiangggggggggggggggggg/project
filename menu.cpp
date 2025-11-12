#include "menu.h"
#include "utils.h"
#include <wx/toolbar.h>
#include <wx/artprov.h>
#include <fstream>

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(wxID_NEW, MyFrame::OnNew)
EVT_MENU(wxID_OPEN, MyFrame::OnOpen)
EVT_MENU(wxID_SAVE, MyFrame::OnSave)
EVT_MENU(wxID_SAVEAS, MyFrame::OnSaveAs)
EVT_MENU(wxID_EXIT, MyFrame::OnQuit)
EVT_MENU(wxID_UNDO, MyFrame::OnUndo)
EVT_MENU(wxID_REDO, MyFrame::OnRedo)
EVT_MENU(wxID_COPY, MyFrame::OnCopy)
EVT_MENU(wxID_PASTE, MyFrame::OnPaste)
EVT_MENU(wxID_ZOOM_IN, MyFrame::OnZoomin)
EVT_MENU(wxID_ZOOM_OUT, MyFrame::OnZoomout)
EVT_MENU(ID_SHOW_STATUSBAR, MyFrame::OnToggleStatusBar)
EVT_MENU(ID_SHOW_GRID, MyFrame::OnToggleGrid)
EVT_MENU(ID_DELETE_SELECTED, MyFrame::OnDeleteSelected)
EVT_MENU(ID_DELETE_WIRE, MyFrame::OnDeleteWire)
EVT_MENU(ID_EDIT_PROPERTIES, MyFrame::OnEditProperties)
EVT_MENU(ID_EXPORT_NETLIST, MyFrame::OnExportNetlist)
EVT_MENU(ID_IMPORT_NETLIST, MyFrame::OnImportNetlist)
EVT_MENU(ID_GENERATE_NETLIST, MyFrame::OnGenerateNetlist)
EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
EVT_MENU(ID_SHOW_PINS, MyFrame::OnTogglePins)
EVT_COMMAND(wxID_ANY, MY_CUSTOM_EVENT, MyFrame::OnCustomEvent)
EVT_MENU(ID_EXPORT_BOOKSHELF, MyFrame::OnExportBookShelf)
EVT_MENU(ID_ADD_BREAKPOINT, MyFrame::OnAddBreakpoint)
EVT_MENU(ID_DELETE_BREAKPOINT, MyFrame::OnDeleteBreakpoint)
wxEND_EVENT_TABLE()

MyFrame::MyFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1000, 650))
{
    // 菜单栏
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(wxID_NEW, "&New\tCtrl-N");
    menuFile->Append(wxID_OPEN, "&Open...\tCtrl-O");
    menuFile->Append(wxID_SAVE, "&Save\tCtrl-S");
    menuFile->Append(wxID_SAVEAS, "Save &As...");
    menuFile->AppendSeparator();
    menuFile->Append(ID_EXPORT_NETLIST, "导出网表...");
    menuFile->Append(ID_IMPORT_NETLIST, "导入网表...");
    menuFile->Append(ID_EXPORT_BOOKSHELF, "导出BookShelf格式...");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT, "E&xit\tAlt-F4");

    wxMenu* menuEdit = new wxMenu;
    menuEdit->Append(wxID_UNDO, "&Undo\tCtrl-Z");
    menuEdit->Append(wxID_REDO, "&Redo\tCtrl-Y");
    menuEdit->AppendSeparator();
    menuEdit->Append(wxID_COPY, "&Copy\tCtrl-C");
    menuEdit->Append(wxID_PASTE, "&Paste\tCtrl-V");
    menuEdit->AppendSeparator();
    menuEdit->Append(ID_EDIT_PROPERTIES, "编辑属性\tCtrl-P");
    menuEdit->Append(ID_DELETE_SELECTED, "删除组件\tDel");
    menuEdit->Append(ID_DELETE_WIRE, "删除线条\tShift-Del");
    menuEdit->Append(ID_DELETE_BREAKPOINT, "删除断点\tCtrl-D");
    menuEdit->AppendSeparator();
    menuEdit->Append(ID_ADD_BREAKPOINT, "添加断点\tCtrl-B");

    wxMenu* menuTools = new wxMenu;
    menuTools->Append(ID_GENERATE_NETLIST, "生成网表\tCtrl-N");
    menuTools->Append(ID_EXPORT_NETLIST, "导出网表...");
    menuTools->Append(ID_IMPORT_NETLIST, "导入网表...");

    wxMenu* menuView = new wxMenu;
    menuView->Append(wxID_ZOOM_IN, "Zoom &In\tCtrl-+");
    menuView->Append(wxID_ZOOM_OUT, "Zoom &Out\tCtrl--");
    menuView->AppendCheckItem(ID_SHOW_STATUSBAR, "Show Status Bar")->Check(true);
    menuView->AppendCheckItem(ID_SHOW_GRID, "Show &Grid\tCtrl-G")->Check(true);
    menuView->AppendCheckItem(ID_SHOW_PINS, "显示引脚\tCtrl-I")->Check(true);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT, "&About\tF1");

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuEdit, "&Edit");
    menuBar->Append(menuTools, "&Tools");
    menuBar->Append(menuView, "&View");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("准备就绪 - 新增功能: 网表生成/导入导出, 属性表(Ctrl+P), 线条删除(右键/Shift-Del), 网格对齐(Ctrl+G), 组件选择(右键), 断点功能(Ctrl+B/Ctrl-D)");

    // 工具栏
    wxToolBar* toolBar = CreateToolBar();
    toolBar->AddTool(wxID_NEW, "新建", wxArtProvider::GetBitmap(wxART_NEW, wxART_TOOLBAR));
    toolBar->AddTool(wxID_OPEN, "打开", wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR));
    toolBar->AddTool(wxID_SAVE, "保存", wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR));
    toolBar->AddSeparator();
    toolBar->AddTool(wxID_UNDO, "撤销", wxArtProvider::GetBitmap(wxART_UNDO, wxART_TOOLBAR));
    toolBar->AddTool(wxID_REDO, "重做", wxArtProvider::GetBitmap(wxART_REDO, wxART_TOOLBAR));
    toolBar->AddSeparator();
    toolBar->AddTool(wxID_ZOOM_IN, "放大", wxArtProvider::GetBitmap(wxART_PLUS, wxART_TOOLBAR));
    toolBar->AddTool(wxID_ZOOM_OUT, "缩小", wxArtProvider::GetBitmap(wxART_MINUS, wxART_TOOLBAR));
    toolBar->AddSeparator();
    toolBar->AddTool(ID_DELETE_SELECTED, "删除组件", wxArtProvider::GetBitmap(wxART_DELETE, wxART_TOOLBAR));
    toolBar->AddTool(ID_DELETE_WIRE, "删除线条", wxArtProvider::GetBitmap(wxART_CUT, wxART_TOOLBAR));
    toolBar->AddTool(ID_ADD_BREAKPOINT, "添加断点", wxArtProvider::GetBitmap(wxART_ADD_BOOKMARK, wxART_TOOLBAR));
    toolBar->AddSeparator();
    toolBar->AddTool(ID_GENERATE_NETLIST, "生成网表", wxArtProvider::GetBitmap(wxART_LIST_VIEW, wxART_TOOLBAR));
    toolBar->Realize();

    // 分割窗口
    m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_splitter->SetMinimumPaneSize(100);

    // 左侧树形控件
    m_treeCtrl = new wxTreeCtrl(m_splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE);
    wxTreeItemId root = m_treeCtrl->AddRoot("组件库");
    m_treeCtrl->AppendItem(root, "AND");
    m_treeCtrl->AppendItem(root, "OR");
    m_treeCtrl->AppendItem(root, "NOT");
    m_treeCtrl->AppendItem(root, "XOR");
    m_treeCtrl->AppendItem(root, "NAND");
    m_treeCtrl->AppendItem(root, "NOR");
    m_treeCtrl->AppendItem(root, "XNOR");
    m_treeCtrl->AppendItem(root, "BUFFER");
    m_treeCtrl->AppendItem(root, "LED");
    m_treeCtrl->AppendItem(root, "开关");
    m_treeCtrl->Expand(root);

    // 右侧绘图面板
    m_drawPanel = new MyDrawPanel(m_splitter);

    m_splitter->SplitVertically(m_treeCtrl, m_drawPanel, 200);

    // 绑定树形控件事件
    m_treeCtrl->Bind(wxEVT_TREE_ITEM_ACTIVATED, [this](wxTreeEvent& evt) {
        wxString itemText = m_treeCtrl->GetItemText(evt.GetItem());
        if (itemText != "组件库") {
            // 保存状态用于撤销
            SaveStateForUndo();
            m_drawPanel->AddShape(itemText);
        }
        });

    UpdateTitle();
}

void MyFrame::SaveStateForUndo() {
    // 保存当前状态到撤销栈
    undoStack.push(m_drawPanel->GetState());
    // 清空重做栈
    while (!redoStack.empty()) redoStack.pop();

    // 更新菜单状态
    wxMenuBar* mb = GetMenuBar();
    if (mb) {
        mb->Enable(wxID_UNDO, !undoStack.empty());
        mb->Enable(wxID_REDO, !redoStack.empty());
    }
}

void MyFrame::OnCustomEvent(wxCommandEvent& event) {
    // 处理自定义事件，保存状态用于撤销
    SaveStateForUndo();
}

void MyFrame::OnNew(wxCommandEvent& event) {
    SaveStateForUndo();
    m_drawPanel->ClearShapes();
    m_currentFile.clear();
    UpdateTitle();
}

void MyFrame::OnOpen(wxCommandEvent& event) {
    SaveStateForUndo();

    wxFileDialog openFileDialog(this, "打开电路图", "", "",
        "Circuit files (*.json)|*.json", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL) return;

    wxString filename = openFileDialog.GetPath();
    std::ifstream f(filename.ToStdString());
    if (!f.is_open()) {
        wxLogError("无法打开文件: %s", filename);
        return;
    }

    try {
        json j;
        f >> j;

        std::vector<Gate> gates;
        for (auto& gateJson : j["gates"]) {
            Gate gate;
            gate.type = gateJson["type"].get<std::string>();
            gate.pos.x = gateJson["x"];
            gate.pos.y = gateJson["y"];

            // 加载属性
            if (gateJson.find("properties") != gateJson.end()) {
                for (auto& propJson : gateJson["properties"]) {
                    Property prop;
                    prop.name = propJson["name"].get<std::string>();
                    prop.value = propJson["value"].get<std::string>();
                    prop.type = propJson["type"].get<std::string>();
                    gate.properties.push_back(prop);
                }
            }

            gates.push_back(gate);
        }

        m_drawPanel->SetGates(gates);
        m_currentFile = filename;
        UpdateTitle();
    }
    catch (const std::exception& e) {
        wxLogError("解析文件失败: %s", e.what());
    }
}

void MyFrame::OnSave(wxCommandEvent& event) {
    if (m_currentFile.empty()) {
        OnSaveAs(event);
    }
    else {
        DoSave(m_currentFile);
    }
}

void MyFrame::OnSaveAs(wxCommandEvent& event) {
    wxFileDialog saveFileDialog(this, "保存电路图", "", "",
        "Circuit files (*.json)|*.json", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) return;

    wxString filename = saveFileDialog.GetPath();
    if (!filename.Lower().EndsWith(".json")) {
        filename += ".json";
    }

    DoSave(filename);
}

void MyFrame::DoSave(const wxString& filename) {
    json j;
    j["version"] = "1.0";
    j["type"] = "circuit";

    std::vector<json> gatesJson;
    for (auto& gate : m_drawPanel->GetGates()) {
        json gateJson;
        gateJson["type"] = gate.type.ToStdString();
        gateJson["x"] = gate.pos.x;
        gateJson["y"] = gate.pos.y;

        // 保存属性
        std::vector<json> propsJson;
        for (auto& prop : gate.properties) {
            json propJson;
            propJson["name"] = prop.name.ToStdString();
            propJson["value"] = prop.value.ToStdString();
            propJson["type"] = prop.type.ToStdString();
            propsJson.push_back(propJson);
        }
        gateJson["properties"] = propsJson;

        gatesJson.push_back(gateJson);
    }
    j["gates"] = gatesJson;

    std::ofstream f(filename.ToStdString());
    if (!f.is_open()) {
        wxLogError("无法保存文件: %s", filename);
        return;
    }

    f << j.dump(4);
    m_currentFile = filename;
    UpdateTitle();
}

void MyFrame::OnQuit(wxCommandEvent& event) {
    Close(true);
}

void MyFrame::OnUndo(wxCommandEvent& event) {
    if (!undoStack.empty()) {
        // 保存当前状态到重做栈
        redoStack.push(m_drawPanel->GetState());
        // 恢复上一个状态
        m_drawPanel->SetState(undoStack.top());
        undoStack.pop();

        // 更新菜单状态
        wxMenuBar* mb = GetMenuBar();
        if (mb) {
            mb->Enable(wxID_UNDO, !undoStack.empty());
            mb->Enable(wxID_REDO, !redoStack.empty());
        }

        SetStatusText("撤销成功");
    }
    else {
        SetStatusText("没有可撤销的操作");
    }
}

void MyFrame::OnRedo(wxCommandEvent& event) {
    if (!redoStack.empty()) {
        // 保存当前状态到撤销栈
        undoStack.push(m_drawPanel->GetState());
        // 恢复重做状态
        m_drawPanel->SetState(redoStack.top());
        redoStack.pop();

        // 更新菜单状态
        wxMenuBar* mb = GetMenuBar();
        if (mb) {
            mb->Enable(wxID_UNDO, !undoStack.empty());
            mb->Enable(wxID_REDO, !redoStack.empty());
        }

        SetStatusText("重做成功");
    }
    else {
        SetStatusText("没有可重做的操作");
    }
}

void MyFrame::OnCopy(wxCommandEvent& event) {
    // 待实现
    SetStatusText("复制功能待实现");
}

void MyFrame::OnPaste(wxCommandEvent& event) {
    // 待实现
    SetStatusText("粘贴功能待实现");
}

void MyFrame::OnZoomin(wxCommandEvent& event) {
    m_drawPanel->ZoomIn();
    SetStatusText("放大视图");
}

void MyFrame::OnZoomout(wxCommandEvent& event) {
    m_drawPanel->ZoomOut();
    SetStatusText("缩小视图");
}

void MyFrame::OnToggleStatusBar(wxCommandEvent& event) {
    wxMenuBar* mb = GetMenuBar();
    wxMenuItem* item = mb->FindItem(ID_SHOW_STATUSBAR);
    bool show = item->IsChecked();
    GetStatusBar()->Show(show);
    Layout();

    if (show) {
        SetStatusText("状态栏已显示");
    }
    else {
        SetStatusText("状态栏已隐藏");
    }
}

void MyFrame::OnToggleGrid(wxCommandEvent& event) {
    wxMenuBar* mb = GetMenuBar();
    wxMenuItem* item = mb->FindItem(ID_SHOW_GRID);
    bool show = item->IsChecked();
    m_drawPanel->ToggleGrid();
    item->Check(m_drawPanel->IsGridVisible());

    if (m_drawPanel->IsGridVisible()) {
        SetStatusText("网格已显示");
    }
    else {
        SetStatusText("网格已隐藏");
    }
}

void MyFrame::OnDeleteSelected(wxCommandEvent& event) {
    SaveStateForUndo();
    m_drawPanel->DeleteSelected();
    SetStatusText("删除选中组件");
}

void MyFrame::OnDeleteWire(wxCommandEvent& event) {
    SaveStateForUndo();
    m_drawPanel->DeleteSelectedWire();
    SetStatusText("删除选中线条");
}

void MyFrame::OnDeleteBreakpoint(wxCommandEvent& event) {
    SaveStateForUndo();
    // 这里需要调用画布面板的删除断点方法
    // 注意：需要确保 m_drawPanel 有 DeleteSelectedBreakpoint 方法
    // 由于我们已经在 canvas.h 中添加了这个方法，可以直接调用
    m_drawPanel->DeleteSelectedBreakpoint();
    SetStatusText("删除选中断点");
}

void MyFrame::OnAddBreakpoint(wxCommandEvent& event) {
    SaveStateForUndo();
    // 这里可以添加从菜单添加断点的逻辑
    // 目前断点主要通过右键菜单添加
    // 我们可以实现一个在鼠标位置添加断点的功能
    SetStatusText("请在连线上右键点击选择'添加断点'，或使用快捷键 Ctrl+B");
}

void MyFrame::OnEditProperties(wxCommandEvent& event) {
    SaveStateForUndo();
    m_drawPanel->EditSelectedProperties();
    SetStatusText("编辑组件属性");
}

// 网表功能实现
void MyFrame::OnExportNetlist(wxCommandEvent& event) {
    wxFileDialog saveFileDialog(this, "导出网表", "", "",
        "Netlist files (*.json)|*.json", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) return;

    wxString filename = saveFileDialog.GetPath();
    if (!filename.Lower().EndsWith(".json")) {
        filename += ".json";
    }

    Netlist netlist = m_drawPanel->GenerateNetlist();
    if (ExportNetlistToFile(filename, netlist)) {
        SetStatusText("网表导出成功: " + filename);
        wxMessageBox("网表导出成功!", "成功", wxOK | wxICON_INFORMATION, this);
    }
    else {
        SetStatusText("网表导出失败");
        wxMessageBox("网表导出失败!", "错误", wxOK | wxICON_ERROR, this);
    }
}

void MyFrame::OnImportNetlist(wxCommandEvent& event) {
    wxFileDialog openFileDialog(this, "导入网表", "", "",
        "Netlist files (*.json)|*.json", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL) return;

    wxString filename = openFileDialog.GetPath();
    Netlist netlist;
    if (ImportNetlistFromFile(filename, netlist)) {
        SaveStateForUndo();
        m_drawPanel->ImportFromNetlist(netlist);
        SetStatusText("网表导入成功: " + filename);
        wxMessageBox("网表导入成功!", "成功", wxOK | wxICON_INFORMATION, this);
    }
    else {
        SetStatusText("网表导入失败");
        wxMessageBox("网表导入失败!", "错误", wxOK | wxICON_ERROR, this);
    }
}

void MyFrame::OnGenerateNetlist(wxCommandEvent& event) {
    m_drawPanel->ShowNetlistPreview();
    SetStatusText("网表生成完成");
}

void MyFrame::OnTogglePins(wxCommandEvent& event) {
    wxMenuBar* mb = GetMenuBar();
    wxMenuItem* item = mb->FindItem(ID_SHOW_PINS);
    bool show = item->IsChecked();
    m_drawPanel->TogglePins();
    item->Check(m_drawPanel->ArePinsVisible());

    if (m_drawPanel->ArePinsVisible()) {
        SetStatusText("引脚已显示");
    }
    else {
        SetStatusText("引脚已隐藏");
    }
}


void MyFrame::OnAbout(wxCommandEvent& event) {
    wxMessageBox("电路图编辑器\n支持组件绘制、连线、属性编辑、网格对齐、线条删除、网表生成、断点功能\n\n"
        "使用说明:\n"
        "- 双击左侧组件添加到画布\n"
        "- 左键拖拽移动组件\n"
        "- 左键点击并拖拽画线\n"
        "- 右键点击组件显示菜单\n"
        "- 右键点击线条删除线条或添加断点\n"
        "- 右键点击断点删除断点\n"
        "- 左键拖拽断点移动位置\n"
        "- 从断点可以引出新连线\n"
        "- Ctrl+Z 撤销\n"
        "- Ctrl+Y 重做\n"
        "- Ctrl+P 编辑属性\n"
        "- Ctrl+B 添加断点\n"
        "- Ctrl+D 删除断点\n"
        "- Del 删除选中组件\n"
        "- Shift+Del 删除选中线条\n"
        "- Ctrl+G 显示/隐藏网格\n"
        "- 工具菜单: 生成/导入/导出网表", "关于", wxOK | wxICON_INFORMATION, this);
}

void MyFrame::UpdateTitle() {
    wxString title = "电路图编辑器";
    if (!m_currentFile.empty()) {
        title += " - " + m_currentFile;
    }
    SetTitle(title);
}

void MyFrame::OnExportBookShelf(wxCommandEvent& event) {
    SaveStateForUndo();

    wxFileDialog saveFileDialog(this, "导出BookShelf格式", "", "",
        "BookShelf files (*.nodes;*.nets)|*.nodes;*.nets", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) return;

    wxString filename = saveFileDialog.GetPath();
    // 移除可能的扩展名，使用基础文件名
    if (filename.Lower().EndsWith(".nodes") || filename.Lower().EndsWith(".nets")) {
        filename = filename.BeforeLast('.');
    }

    Netlist netlist = m_drawPanel->GenerateNetlist();
    if (ExportBookShelfFiles(filename, netlist)) {
        SetStatusText("BookShelf格式导出成功: " + filename + ".nodes, " + filename + ".nets");
        wxMessageBox("BookShelf格式导出成功!\n生成文件:\n" + filename + ".nodes\n" + filename + ".nets",
            "成功", wxOK | wxICON_INFORMATION, this);
    }
    else {
        SetStatusText("BookShelf格式导出失败");
        wxMessageBox("BookShelf格式导出失败!", "错误", wxOK | wxICON_ERROR, this);
    }
}