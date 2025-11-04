#include "menu.h"
#include "utils.h"
#include <wx/toolbar.h>
#include <wx/artprov.h>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

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
EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
EVT_COMMAND(wxID_ANY, MY_CUSTOM_EVENT, MyFrame::OnCustomEvent)
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

    wxMenu* menuView = new wxMenu;
    menuView->Append(wxID_ZOOM_IN, "Zoom &In\tCtrl-+");
    menuView->Append(wxID_ZOOM_OUT, "Zoom &Out\tCtrl--");
    menuView->AppendCheckItem(ID_SHOW_STATUSBAR, "Show Status Bar")->Check(true);
    menuView->AppendCheckItem(ID_SHOW_GRID, "Show &Grid\tCtrl-G")->Check(true);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT, "&About\tF1");

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuEdit, "&Edit");
    menuBar->Append(menuView, "&View");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("准备就绪 - 新增功能: 属性表(Ctrl+P), 线条删除(右键/Shift-Del), 网格对齐(Ctrl+G), 组件选择(右键)");

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
    SetStatusText("已创建新电路图");
}

void MyFrame::OnOpen(wxCommandEvent& event) {
    SaveStateForUndo();

    wxFileDialog openFileDialog(this, "打开电路图", "", "",
        "Circuit files (*.json)|*.json|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL) return;

    wxString filename = openFileDialog.GetPath();

    if (!LoadCircuitFile(filename)) {
        wxLogError("打开文件失败: %s", filename);
        SetStatusText("打开文件失败");
        return;
    }

    m_currentFile = filename;
    UpdateTitle();
    SetStatusText(wxString::Format("已打开: %s", filename));
}

bool MyFrame::LoadCircuitFile(const wxString& filename) {
    std::ifstream f(filename.ToStdString());
    if (!f.is_open()) {
        return false;
    }

    try {
        json j;
        f >> j;

        // 验证文件格式
        if (j.find("version") == j.end() || j.find("gates") == j.end()) {
            wxLogError("无效的电路图文件格式");
            return false;
        }

        std::vector<Gate> gates;
        std::vector<Wire> wires;

        // 加载门信息
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

            // 设置引脚定义
            auto pinIt = pinDefinitions.find(gate.type);
            if (pinIt != pinDefinitions.end()) {
                gate.pins = pinIt->second;
            }

            gates.push_back(gate);
        }

        // 加载连线信息
        if (j.find("wires") != j.end()) {
            for (auto& wireJson : j["wires"]) {
                Wire wire;

                // 加载起点
                if (wireJson.find("start") != wireJson.end()) {
                    wire.start.gateIndex = wireJson["start"]["gateIndex"];
                    wire.start.pinIndex = wireJson["start"]["pinIndex"];
                    wire.start.position.x = wireJson["start"]["x"];
                    wire.start.position.y = wireJson["start"]["y"];
                }

                // 加载终点
                if (wireJson.find("end") != wireJson.end()) {
                    wire.end.gateIndex = wireJson["end"]["gateIndex"];
                    wire.end.pinIndex = wireJson["end"]["pinIndex"];
                    wire.end.position.x = wireJson["end"]["x"];
                    wire.end.position.y = wireJson["end"]["y"];
                }

                // 加载路径
                if (wireJson.find("path") != wireJson.end()) {
                    for (auto& pointJson : wireJson["path"]) {
                        wxPoint point(pointJson["x"], pointJson["y"]);
                        wire.path.push_back(point);
                    }
                }

                wires.push_back(wire);
            }
        }

        // 设置到画布
        m_drawPanel->SetGates(gates);
        m_drawPanel->SetWires(wires);

        // 加载画布设置
        if (j.find("canvas") != j.end()) {
            auto canvasJson = j["canvas"];
            if (canvasJson.find("scale") != canvasJson.end()) {
                double scale = canvasJson["scale"].get<double>();
                m_drawPanel->SetScale(scale);
            }
            if (canvasJson.find("showGrid") != canvasJson.end()) {
                bool showGrid = canvasJson["showGrid"].get<bool>();
                if (showGrid != m_drawPanel->IsGridVisible()) {
                    m_drawPanel->ToggleGrid();
                }

                // 更新菜单项状态
                wxMenuBar* mb = GetMenuBar();
                if (mb) {
                    wxMenuItem* item = mb->FindItem(ID_SHOW_GRID);
                    if (item) {
                        item->Check(showGrid);
                    }
                }
            }
        }


        // 更新引脚连接状态
        m_drawPanel->UpdatePinConnections();

        return true;
    }
    catch (const std::exception& e) {
        wxLogError("解析文件失败: %s", e.what());
        return false;
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
        "Circuit files (*.json)|*.json|All files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) return;

    wxString filename = saveFileDialog.GetPath();
    if (!filename.Lower().EndsWith(".json")) {
        filename += ".json";
    }

    DoSave(filename);
}

void MyFrame::DoSave(const wxString& filename) {
    json j;
    j["version"] = "1.1";
    j["type"] = "circuit";
    j["created"] = wxDateTime::Now().FormatISOCombined(' ').ToStdString();

    // 保存门信息
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

    // 保存连线信息
    std::vector<json> wiresJson;
    auto wires = m_drawPanel->GetWires();
    for (auto& wire : wires) {
        json wireJson;

        // 保存起点信息
        json startJson;
        startJson["gateIndex"] = wire.start.gateIndex;
        startJson["pinIndex"] = wire.start.pinIndex;
        startJson["x"] = wire.start.position.x;
        startJson["y"] = wire.start.position.y;
        wireJson["start"] = startJson;

        // 保存终点信息
        json endJson;
        endJson["gateIndex"] = wire.end.gateIndex;
        endJson["pinIndex"] = wire.end.pinIndex;
        endJson["x"] = wire.end.position.x;
        endJson["y"] = wire.end.position.y;
        wireJson["end"] = endJson;

        // 保存路径点
        std::vector<json> pathJson;
        for (auto& point : wire.path) {
            json pointJson;
            pointJson["x"] = point.x;
            pointJson["y"] = point.y;
            pathJson.push_back(pointJson);
        }
        wireJson["path"] = pathJson;

        wiresJson.push_back(wireJson);
    }
    j["wires"] = wiresJson;

    // 保存画布设置
    j["canvas"] = {
        {"scale", m_drawPanel->GetScale()},
        {"showGrid", m_drawPanel->IsGridVisible()}
    };

    std::ofstream f(filename.ToStdString());
    if (!f.is_open()) {
        wxLogError("无法保存文件: %s", filename);
        SetStatusText("保存文件失败");
        return;
    }

    f << j.dump(4);
    m_currentFile = filename;
    UpdateTitle();

    SetStatusText(wxString::Format("已保存: %s", filename));
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

void MyFrame::OnEditProperties(wxCommandEvent& event) {
    SaveStateForUndo();
    m_drawPanel->EditSelectedProperties();
    SetStatusText("编辑组件属性");
}

void MyFrame::OnAbout(wxCommandEvent& event) {
    wxMessageBox("电路图编辑器\n支持组件绘制、连线、属性编辑、网格对齐、线条删除等功能\n\n"
        "使用说明:\n"
        "- 双击左侧组件添加到画布\n"
        "- 左键拖拽移动组件\n"
        "- 左键点击并拖拽画线\n"
        "- 右键点击组件显示菜单\n"
        "- 右键点击线条删除线条\n"
        "- Ctrl+Z 撤销\n"
        "- Ctrl+Y 重做\n"
        "- Ctrl+P 编辑属性\n"
        "- Del 删除选中组件\n"
        "- Shift+Del 删除选中线条\n"
        "- Ctrl+G 显示/隐藏网格\n\n"
        "文件格式: JSON (支持保存组件、连线、属性和画布设置)",
        "关于", wxOK | wxICON_INFORMATION, this);
}

void MyFrame::UpdateTitle() {
    wxString title = "电路图编辑器";
    if (!m_currentFile.empty()) {
        title += " - " + m_currentFile;
    }

    // 添加修改标记（如果需要的话）
    // if (isModified) title += " *";

    SetTitle(title);
}