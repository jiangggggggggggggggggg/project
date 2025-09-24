#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/filedlg.h>      // 文件选择框
#include <wx/textfile.h>     // 如果你要读取文本文件内容
#include <wx/splitter.h>  // 用于左右分割



// 1.自定义应用类（继承wxApp）
class MyApp : public wxApp {
public:
    // 程序入口（替代main函数）
    virtual bool OnInit() override {
        // 创建主窗口（继承wxFrame，标题“wxWidgets窗口”，大小800x600）
        wxFrame* frame = new wxFrame(nullptr, wxID_ANY, "wxWidgets Test Window",
            wxDefaultPosition, wxSize(800, 600));
        //创建菜单栏
		wxMenuBar* menuBar = new wxMenuBar();


		// 创建“File”菜单
		wxMenu* fileMenu = new wxMenu();
        fileMenu->Append(wxID_NEW, "New\tCtrl+N");//新建，Ctrl-N是快捷键
        fileMenu->Append(wxID_OPEN, "Open\tCtrl+O");
        fileMenu->Append(wxID_SAVE, "Save\tCtrl+S");
		fileMenu->Append(wxID_SAVEAS, "Save As\tCtrl+Shift+S");
        //将菜单添加到菜单栏
        menuBar->Append(fileMenu, "File");

        //创建“Edit”菜单
        wxMenu* editMenu = new wxMenu();
        editMenu->Append(wxID_UNDO, "Undo\tCtrl-Z");
        editMenu->Append(wxID_CUT, "Cut\tCtrl-X");
        editMenu->Append(wxID_COPY, "Copy\tCtrl-C");
        editMenu->Append(wxID_PASTE, "Paste\tCtrl-V");

        menuBar->Append(editMenu, "Edit");

        //创建"Tools"菜单
        wxMenu* toolsMenu = new wxMenu();
        toolsMenu->Append(wxID_ANY, "Settings\tCtrl-S");
        toolsMenu->Append(wxID_ANY, "Measure Tool\tCtrl-M");
        toolsMenu->Append(wxID_ANY, "Batch Process\tCtrl-B");

        menuBar->Append(toolsMenu, "Tools");

        // 创建"help"菜单
        wxMenu* helpMenu = new wxMenu();
        helpMenu->Append(wxID_HELP, "Help\tF1");
        helpMenu->Append(wxID_ABOUT, "About\tCtrl-A");
        menuBar->Append(helpMenu, "Help");

        //将菜单栏设置为窗口的菜单栏
        frame->SetMenuBar(menuBar);

        // 绑定 Open 菜单项的点击事件
        frame->Bind(wxEVT_MENU, &MyApp::OnOpenFile, this, wxID_OPEN);


        // 先创建 splitter，父窗口是 frame
        wxSplitterWindow* splitter = new wxSplitterWindow(frame, wxID_ANY);
        splitter->SetSashGravity(0.3);
        splitter->SetMinimumPaneSize(100);

        // 再用 splitter 作为父窗口创建 tree 和 canvasPanel
        wxTreeCtrl* tree = new wxTreeCtrl(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS);
        
		//设置可见字体颜色和背景颜色
        tree->SetForegroundColour(*wxBLACK);
        tree->SetBackgroundColour(*wxWHITE);


        // 添加根节点
        wxTreeItemId root = tree->AddRoot("Untitled");

        // 添加一级节点
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

		//添加Arithmetic节点的子节点
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



        // 创建绘图区（右侧面板）
        wxPanel* canvasPanel = new wxPanel(splitter, wxID_ANY);//
        canvasPanel->SetBackgroundColour(*wxWHITE); // 模拟画布背景

        // 创建分割窗口（左右分区）
        splitter->SplitVertically(tree, canvasPanel);

        // 将左右控件加入 splitter
        splitter->SplitVertically(tree, canvasPanel);  // 左：树，右：绘图面板

        // 使用 sizer 让 splitter 填满整个窗口
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(splitter, 1, wxEXPAND | wxALL, 0);
        frame->SetSizer(sizer);

        


        // 显示窗口
        frame->Show(true);
        return true; // 必须返回true，否则程序退出

    }

    // 处理打开文件的事件
    void OnOpenFile(wxCommandEvent& event) {
        // 创建一个文件对话框，允许用户选择文件
        wxFileDialog openFileDialog(nullptr, "Open File", "", "",
            "All Files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

        // 如果用户点击取消，则返回
        if (openFileDialog.ShowModal() == wxID_CANCEL)
            return; // 用户取消

        // 获取文件路径
        wxString path = openFileDialog.GetPath();

        // 简单显示打开的文件路径（实际可以用于读取/处理文件）
        wxMessageBox("Selected file:\n" + path, "File Opened", wxOK | wxICON_INFORMATION);
    }
};

// 4. 宏：告诉wxWidgets应用入口类（MyApp）
wxIMPLEMENT_APP(MyApp);
