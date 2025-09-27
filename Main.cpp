#include <wx/wx.h>
#include <wx/splitter.h> // 包含 wxSplitterWindow 类
#include <wx/treectrl.h> // 包含 wxTreeCtrl、wxTreeItemId 类及 wxTR_HAS_BUTTONS 宏
#include "menu.h"
#include "canvas.h"
#include "utils.h"

// 自定义应用类（继承wxApp）
class MyApp : public wxApp {
public:
    virtual bool OnInit() override;
};

// 实现应用程序初始化
bool MyApp::OnInit() {
    // 创建主窗口
    wxFrame* frame = new wxFrame(nullptr, wxID_ANY, "Logisim Clone",
        wxDefaultPosition, wxSize(800, 600));

    // 创建菜单栏
    MenuHandler* menuHandler = new MenuHandler(frame);
    frame->SetMenuBar(menuHandler->CreateMenuBar());

    // 绑定菜单事件
    menuHandler->BindEvents();

    // 创建分割窗口
    wxSplitterWindow* splitter = new wxSplitterWindow(frame, wxID_ANY);
    splitter->SetSashGravity(0.3);
    splitter->SetMinimumPaneSize(100);

    // 创建树形控件
    wxTreeCtrl* tree = new wxTreeCtrl(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS);
    tree->SetForegroundColour(*wxBLACK);
    tree->SetBackgroundColour(*wxWHITE);

    // 添加树形节点
    wxTreeItemId root = tree->AddRoot("Components");
    wxTreeItemId main = tree->AppendItem(root, "main");
    wxTreeItemId wiring = tree->AppendItem(root, "Wiring");
    wxTreeItemId gates = tree->AppendItem(root, "Gates");
    wxTreeItemId plexers = tree->AppendItem(root, "Plexers");
    wxTreeItemId arithmetic = tree->AppendItem(root, "Arithmetic");
    wxTreeItemId memory = tree->AppendItem(root, "Memory");
    wxTreeItemId input_output = tree->AppendItem(root, "Input/Output");
    wxTreeItemId base = tree->AppendItem(root, "Base");

    // 添加 Wiring 的子节点
    tree->AppendItem(wiring, "Splitter");
    tree->AppendItem(wiring, "Pin");
    tree->AppendItem(wiring, "Probe");
    tree->AppendItem(wiring, "Tunnel");
    tree->AppendItem(wiring, "Pull Resistor");
    tree->AppendItem(wiring, "Clock");
    tree->AppendItem(wiring, "Constant");

    // 添加 Gates 的子节点
    tree->AppendItem(gates, "NOT Gate 非门");
    tree->AppendItem(gates, "Buffer 缓冲器");
    tree->AppendItem(gates, "AND Gate 与门");
    tree->AppendItem(gates, "OR Gate 或门");
    tree->AppendItem(gates, "NAND Gate 与非门");
    tree->AppendItem(gates, "NOR Gate 或非门");
    tree->AppendItem(gates, "XOR Gate 异或门");
    tree->AppendItem(gates, "XNOR Gate 异或非门");

    // 添加Arithmetic节点的子节点
    tree->AppendItem(arithmetic, "Adder");
    tree->AppendItem(arithmetic, "Subtractor");
    tree->AppendItem(arithmetic, "Multiplier");
    tree->AppendItem(arithmetic, "Divider");

    // 添加Memory节点的子节点
    tree->AppendItem(memory, "D Flip-Flop D触发器");
    tree->AppendItem(memory, "SR Flip-Flop SR触发器");
    tree->AppendItem(memory, "JK Flip-Flop JK触发器");
    tree->AppendItem(memory, "T Flip-Flop T触发器");
    tree->AppendItem(memory, "Register 寄存器");

    // 添加Input/Output节点的子节点
    tree->AppendItem(input_output, "LED");
    tree->AppendItem(input_output, "Button");

    // 展开所有节点
    tree->Expand(root);

    // 创建画布
    Canvas* canvas = new Canvas(splitter);

    // 分割窗口
    splitter->SplitVertically(tree, canvas);

    // 使用sizer布局
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(splitter, 1, wxEXPAND | wxALL, 0);
    frame->SetSizer(sizer);

    // 显示窗口
    frame->Show(true);
    return true;
}

// 声明应用入口
wxIMPLEMENT_APP(MyApp);
