#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/artprov.h>
#include <wx/toolbar.h>
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/dcbuffer.h>
#include <vector>
#include <stack>
#include<wx/clipbrd.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h> 

enum {
    ID_SHOW_STATUSBAR = wxID_HIGHEST + 1
};
struct Gate {
    wxString type;
    wxPoint pos;
};
struct Wire {
    wxPoint start;
    wxPoint end;
};
// ===== 绘图区类 =====
class MyDrawPanel : public wxPanel {
public:
    MyDrawPanel(wxWindow* parent)
        : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE),
        m_scale(1.0), m_isDrawingWire(false)
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        Bind(wxEVT_PAINT, &MyDrawPanel::OnPaint, this);
        Bind(wxEVT_LEFT_DOWN, &MyDrawPanel::OnMouseDown, this);
        Bind(wxEVT_LEFT_UP, &MyDrawPanel::OnMouseUp, this);
        Bind(wxEVT_MOTION, &MyDrawPanel::OnMouseMove, this);
    }

    void AddShape(const wxString& shape) {
        // 默认位置简单布局
        m_gates.push_back({ shape, wxPoint(50 + (m_gates.size() % 3) * 150,
                                           50 + (m_gates.size() / 3) * 150) });
        Refresh();
    }

    void RemoveLastShape() {
        if (!m_gates.empty()) m_gates.pop_back();
        Refresh();
    }

    void ClearShapes() {
        m_gates.clear();
        m_wires.clear();
        Refresh();
    }

    std::vector<wxString> GetShapes() const {
        std::vector<wxString> names;
        for (auto& g : m_gates) names.push_back(g.type);
        return names;
    }

    void SetShapes(const std::vector<wxString>& shapes) {
        m_gates.clear();
        int i = 0;
        for (auto& s : shapes) {
            m_gates.push_back({ s, wxPoint(50 + (i % 3) * 150, 50 + (i / 3) * 150) });
            i++;
        }
        Refresh();
    }

    void ZoomIn() {
        m_scale *= 1.2;
        Refresh();
    }

    void ZoomOut() {
        m_scale /= 1.2;
        if (m_scale < 0.2) m_scale = 0.2;
        Refresh();
    }

private:
    std::vector<Gate> m_gates;
    std::vector<Wire> m_wires;
    double m_scale;

    bool m_isDrawingWire;
    wxPoint m_wireStart;
    wxPoint m_currentMouse;

    // ===== 绘制各种逻辑门 =====
    void DrawAndGate(wxDC& dc, wxPoint pos) {
        int x = pos.x, y = pos.y;
        int w = 80, h = 60;
        dc.DrawLine(x, y, x + w / 2, y);
        dc.DrawLine(x, y + h, x + w / 2, y + h);
        dc.DrawLine(x, y, x, y + h);
        dc.DrawArc(x + w / 2, y + h, x + w / 2, y, x + w, y + h / 2);
        dc.DrawLine(x - 20, y + h / 4, x, y + h / 4);
        dc.DrawLine(x - 20, y + 3 * h / 4, x, y + 3 * h / 4);
        dc.DrawLine(x + w, y + h / 2, x + w + 20, y + h / 2);
    }

    void DrawOrGate(wxDC& dc, wxPoint pos) {
        int x = pos.x, y = pos.y;
        int w = 80, h = 60;
        dc.DrawArc(x - 10, y + h, x - 10, y, x + 20, y + h / 2);
        dc.DrawArc(x + 20, y + h, x + 20, y, x + w, y + h / 2);
        dc.DrawLine(x - 30, y + h / 4, x + 5, y + h / 4);
        dc.DrawLine(x - 30, y + 3 * h / 4, x + 5, y + 3 * h / 4);
        dc.DrawLine(x + w, y + h / 2, x + w + 20, y + h / 2);
    }

    void DrawNotGate(wxDC& dc, wxPoint pos) {
        int x = pos.x, y = pos.y;
        int w = 70, h = 60;
        wxPoint tri[3] = { wxPoint(x, y), wxPoint(x, y + h), wxPoint(x + w, y + h / 2) };
        dc.DrawPolygon(3, tri);
        dc.DrawCircle(x + w + 8, y + h / 2, 8);
        dc.DrawLine(x - 20, y + h / 2, x, y + h / 2);
        dc.DrawLine(x + w + 16, y + h / 2, x + w + 36, y + h / 2);
    }

    void DrawXorGate(wxDC& dc, wxPoint pos) {
        int x = pos.x, y = pos.y;
        int w = 80, h = 60;
        dc.DrawArc(x - 15, y + h, x - 15, y, x + 15, y + h / 2);
        dc.DrawArc(x, y + h, x, y, x + w, y + h / 2);
        dc.DrawLine(x - 35, y + h / 4, x, y + h / 4);
        dc.DrawLine(x - 35, y + 3 * h / 4, x, y + 3 * h / 4);
        dc.DrawLine(x + w, y + h / 2, x + w + 20, y + h / 2);
    }

    void DrawLed(wxDC& dc, wxPoint pos) {
        dc.SetBrush(*wxRED_BRUSH);
        dc.DrawCircle(pos.x + 40, pos.y + 40, 30);
        dc.DrawText("LED", pos.x + 30, pos.y + 80);
    }

    void OnPaint(wxPaintEvent&) {
        wxAutoBufferedPaintDC dc(this);
        dc.Clear();
        dc.SetUserScale(m_scale, m_scale);
        dc.SetPen(*wxBLACK_PEN);

        for (auto& g : m_gates) {
            if (g.type == "AND") DrawAndGate(dc, g.pos);
            else if (g.type == "OR") DrawOrGate(dc, g.pos);
            else if (g.type == "NOT") DrawNotGate(dc, g.pos);
            else if (g.type == "XOR") DrawXorGate(dc, g.pos);
            else if (g.type == "LED") DrawLed(dc, g.pos);
            else dc.DrawText("未知组件", g.pos);
        }

        for (auto& w : m_wires) {
            dc.DrawLine(w.start, w.end);
        }

        if (m_isDrawingWire) {
            dc.SetPen(*wxRED_PEN);
            dc.DrawLine(m_wireStart, m_currentMouse);
            dc.SetPen(*wxBLACK_PEN);
        }
    }

    void OnMouseDown(wxMouseEvent& evt) {
        if (!m_isDrawingWire) {
            m_isDrawingWire = true;
            m_wireStart = evt.GetPosition();
            m_currentMouse = m_wireStart;
        }
        else {
            m_wires.push_back({ m_wireStart, evt.GetPosition() });
            m_isDrawingWire = false;
        }
        Refresh();
    }

    void OnMouseUp(wxMouseEvent& evt) {
        if (m_isDrawingWire) {
            m_currentMouse = evt.GetPosition();
            Refresh();
        }
    }

    void OnMouseMove(wxMouseEvent& evt) {
        if (m_isDrawingWire && evt.Dragging() && evt.LeftIsDown()) {
            m_currentMouse = evt.GetPosition();
            Refresh();
        }
    }
};

// ===== 主窗口 =====
class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);

private:
    MyDrawPanel* m_drawPanel;
    wxTreeCtrl* m_treeCtrl;
    wxSplitterWindow* m_splitter;
    wxString m_currentFile;

    std::stack<std::vector<wxString>> undoStack;
    std::stack<std::vector<wxString>> redoStack;

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
    void OnAbout(wxCommandEvent& event);

    void DoSave(const wxString& filename);
    void UpdateTitle();

    wxDECLARE_EVENT_TABLE();
};

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

EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
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

    wxMenu* menuView = new wxMenu;
    menuView->Append(wxID_ZOOM_IN, "Zoom &In\tCtrl-+");
    menuView->Append(wxID_ZOOM_OUT, "Zoom &Out\tCtrl--");
    menuView->AppendCheckItem(ID_SHOW_STATUSBAR, "Show Status Bar")->Check(true);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT, "&About\tF1");

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuEdit, "&Edit");
    menuBar->Append(menuView, "&View");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("准备就绪");

    // 工具栏
    wxToolBar* toolBar = CreateToolBar();
    toolBar->AddTool(wxID_NEW, "新建", wxArtProvider::GetBitmap(wxART_NEW, wxART_TOOLBAR));
    toolBar->AddTool(wxID_OPEN, "打开", wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR));
    toolBar->AddTool(wxID_SAVE, "保存", wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR));
    toolBar->AddSeparator();
    toolBar->AddTool(wxID_UNDO, "撤销", wxArtProvider::GetBitmap(wxART_UNDO, wxART_TOOLBAR));
    toolBar->AddTool(wxID_REDO, "重做", wxArtProvider::GetBitmap(wxART_REDO, wxART_TOOLBAR));
    toolBar->AddSeparator();
    toolBar->AddTool(wxID_COPY, "复制", wxArtProvider::GetBitmap(wxART_COPY, wxART_TOOLBAR));
    toolBar->AddTool(wxID_PASTE, "粘贴", wxArtProvider::GetBitmap(wxART_PASTE, wxART_TOOLBAR));
    toolBar->AddSeparator();
    toolBar->AddTool(wxID_ABOUT, "关于", wxArtProvider::GetBitmap(wxART_HELP, wxART_TOOLBAR));
    toolBar->Realize();

    // 分割布局
    m_splitter = new wxSplitterWindow(this, wxID_ANY);
    m_treeCtrl = new wxTreeCtrl(m_splitter, wxID_ANY, wxDefaultPosition, wxSize(200, -1),
        wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_DEFAULT_STYLE);
    m_drawPanel = new MyDrawPanel(m_splitter);
    m_splitter->SplitVertically(m_treeCtrl, m_drawPanel, 200);
    m_splitter->SetSashGravity(0.0);

    // 树节点
    wxTreeItemId root = m_treeCtrl->AddRoot("组件库");
    wxTreeItemId inputId = m_treeCtrl->AppendItem(root, "输入");
    wxTreeItemId logicId = m_treeCtrl->AppendItem(root, "逻辑门");
    wxTreeItemId outputId = m_treeCtrl->AppendItem(root, "输出");

    m_treeCtrl->AppendItem(inputId, "开关");
    m_treeCtrl->AppendItem(inputId, "按钮");
    m_treeCtrl->AppendItem(logicId, "AND");
    m_treeCtrl->AppendItem(logicId, "OR");
    m_treeCtrl->AppendItem(logicId, "NOT");
    m_treeCtrl->AppendItem(outputId, "LED");
    m_treeCtrl->AppendItem(outputId, "蜂鸣器");
    m_treeCtrl->ExpandAll();

    // 树节点点击事件
    m_treeCtrl->Bind(wxEVT_TREE_SEL_CHANGED, [this](wxTreeEvent& event) {
        wxTreeItemId item = event.GetItem();
        if (!item.IsOk()) return;
        wxString name = m_treeCtrl->GetItemText(item);

        // 保存 undo
        undoStack.push(m_drawPanel->GetShapes());
        while (!redoStack.empty()) redoStack.pop();

        m_drawPanel->AddShape(name);
        SetStatusText("选中节点: " + name);
        });

    UpdateTitle();
}

// ===== 功能实现 =====
void MyFrame::OnNew(wxCommandEvent& event) {
    undoStack.push(m_drawPanel->GetShapes());
    redoStack = std::stack<std::vector<wxString>>();
    m_drawPanel->ClearShapes();
    m_currentFile.Clear();
    UpdateTitle();
    SetStatusText("新建文件");
}

void MyFrame::OnOpen(wxCommandEvent& event) {
    wxFileDialog openFileDialog(this, "打开文件", "", "",
        "Logic Files (*.logic)|*.logic|All Files (*.*)|*.*",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    wxFileInputStream input(openFileDialog.GetPath());
    if (!input.IsOk()) {
        wxLogError("无法打开文件 '%s'.", openFileDialog.GetPath());
        return;
    }

    wxTextInputStream text(input);
    std::vector<wxString> shapes;
    while (!input.Eof()) {
        wxString line = text.ReadLine();
        if (!line.IsEmpty()) shapes.push_back(line);
    }

    m_drawPanel->SetShapes(shapes);
    m_currentFile = openFileDialog.GetPath();
    UpdateTitle();
    SetStatusText("已打开: " + m_currentFile);
}

void MyFrame::DoSave(const wxString& filename) {
    wxFileOutputStream output(filename);
    if (!output.IsOk()) {
        wxLogError("无法保存文件 '%s'.", filename);
        return;
    }

    wxTextOutputStream text(output);
    for (auto& shape : m_drawPanel->GetShapes()) {
        text << shape << "\n";
    }

    m_currentFile = filename;
    UpdateTitle();
    SetStatusText("已保存: " + filename);
}

void MyFrame::OnSave(wxCommandEvent& event) {
    if (m_currentFile.IsEmpty()) {
        OnSaveAs(event);
    }
    else {
        DoSave(m_currentFile);
    }
}

void MyFrame::OnSaveAs(wxCommandEvent& event) {
    wxFileDialog saveFileDialog(this, "保存文件", "", "",
        "Logic Files (*.logic)|*.logic|All Files (*.*)|*.*",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;

    DoSave(saveFileDialog.GetPath());
}

void MyFrame::OnQuit(wxCommandEvent& event) {
    Close(true);
}

void MyFrame::OnUndo(wxCommandEvent& event) {
    if (!undoStack.empty()) {
        redoStack.push(m_drawPanel->GetShapes());
        m_drawPanel->SetShapes(undoStack.top());
        undoStack.pop();
        SetStatusText("撤销成功");
    }
    else {
        SetStatusText("没有可撤销的操作");
    }
}

void MyFrame::OnRedo(wxCommandEvent& event) {
    if (!redoStack.empty()) {
        undoStack.push(m_drawPanel->GetShapes());
        m_drawPanel->SetShapes(redoStack.top());
        redoStack.pop();
        SetStatusText("重做成功");
    }
    else {
        SetStatusText("没有可重做的操作");
    }
}

void MyFrame::OnCopy(wxCommandEvent& event) {
    const auto& shapes = m_drawPanel->GetShapes();
    if (!shapes.empty()) {
        wxTheClipboard->Open();
        wxTheClipboard->SetData(new wxTextDataObject(shapes.back()));
        wxTheClipboard->Close();
        SetStatusText("已复制: " + shapes.back());
    }
}

void MyFrame::OnPaste(wxCommandEvent& event) {
    if (wxTheClipboard->Open()) {
        if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
            wxTextDataObject data;
            wxTheClipboard->GetData(data);
            undoStack.push(m_drawPanel->GetShapes());
            redoStack = std::stack<std::vector<wxString>>();
            m_drawPanel->AddShape(data.GetText());
            SetStatusText("已粘贴: " + data.GetText());
        }
        wxTheClipboard->Close();
    }
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
    if (GetStatusBar()->IsShown()) {
        GetStatusBar()->Hide();
    }
    else {
        GetStatusBar()->Show();
    }
    Layout();
}

void MyFrame::OnAbout(wxCommandEvent& event) {
    wxMessageBox("逻辑电路绘图 Demo\n使用 wxWidgets 实现\n\n支持基本的文件操作、撤销重做和组件绘制。",
        "关于", wxOK | wxICON_INFORMATION, this);
}

void MyFrame::UpdateTitle() {
    wxString title = "wxWidgets 绘图Demo";
    if (!m_currentFile.IsEmpty()) {
        title += " - " + m_currentFile;
    }
    SetTitle(title);
}

// ===== 应用入口 =====
class MyApp : public wxApp {
public:
    virtual bool OnInit() override {
        MyFrame* frame = new MyFrame("wxWidgets 绘图Demo");
        frame->Show(true);
        return true;
    }
};
wxIMPLEMENT_APP(MyApp);
