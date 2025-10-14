//目前已经把画图部分分出去到json文件里面了
//可以实现绘图，移动，画线等功能。然后菜单栏和工具栏可以正常运行
#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/artprov.h>
#include <wx/toolbar.h>
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <vector>
#include <stack>
#include <wx/clipbrd.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <map>
#include <cmath>
#include <fstream>

// 引入 JSON 库
#include "json.hpp"
using json = nlohmann::json;

// 自定义 ID
enum {
    ID_SHOW_STATUSBAR = wxID_HIGHEST + 1,
    ID_SHOW_GRID,
    ID_DELETE_SELECTED,
    ID_ROTATE_SELECTED
};

struct Gate {
    wxString type;
    wxPoint pos;
    double angle = 0; // 旋转角度
};
struct Wire {
    wxPoint start;
    wxPoint end;
};

enum class ShapeType { Line, Arc, Circle, Polygon, Text };

struct Shape {
    ShapeType type;
    std::vector<wxPoint> pts;
    wxPoint center;
    int radius = 0;
    int startAngle = 0, endAngle = 0;
    wxString text;
};

static std::map<wxString, std::vector<Shape>> shapeLibrary;

// ===== JSON 加载函数 (兼容 C++14) =====
void LoadShapesFromJson(const std::string& filename) {
    std::ifstream f(filename.c_str());
    if (!f.is_open()) {
        wxLogError("无法打开图形文件: %s", filename);
        return;
    }
    json j;
    f >> j;

    for (json::iterator it = j.begin(); it != j.end(); ++it) {
        std::string gateName = it.key();
        auto shapes = it.value();

        std::vector<Shape> vec;
        for (size_t i = 0; i < shapes.size(); ++i) {
            auto s = shapes[i];
            Shape shape;
            std::string type = s["type"];
            if (type == "Line") shape.type = ShapeType::Line;
            else if (type == "Arc") shape.type = ShapeType::Arc;
            else if (type == "Circle") shape.type = ShapeType::Circle;
            else if (type == "Polygon") shape.type = ShapeType::Polygon;
            else if (type == "Text") shape.type = ShapeType::Text;

            if (s.find("pts") != s.end()) {
                for (size_t pi = 0; pi < s["pts"].size(); ++pi) {
                    auto p = s["pts"][pi];
                    shape.pts.push_back(wxPoint(p[0], p[1]));
                }
            }
            if (s.find("center") != s.end()) {
                shape.center = wxPoint(s["center"][0], s["center"][1]);
            }
            if (s.find("radius") != s.end()) shape.radius = s["radius"];
            if (s.find("startAngle") != s.end()) shape.startAngle = s["startAngle"];
            if (s.find("endAngle") != s.end()) shape.endAngle = s["endAngle"];
            if (s.find("text") != s.end()) shape.text = s["text"].get<std::string>().c_str();

            vec.push_back(shape);
        }
        shapeLibrary[wxString::FromUTF8(gateName.c_str())] = vec;
    }
}

// ===== 绘图区类 =====
class MyDrawPanel : public wxPanel {
public:
    MyDrawPanel(wxWindow* parent)
        : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE),
        m_scale(1.0), m_isDrawingWire(false),
        m_isDraggingGate(false), m_draggedIndex(-1),
        m_selectedIndex(-1), m_showGrid(true), m_gridSize(20)
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        Bind(wxEVT_PAINT, &MyDrawPanel::OnPaint, this);
        Bind(wxEVT_LEFT_DOWN, &MyDrawPanel::OnMouseDown, this);
        Bind(wxEVT_LEFT_UP, &MyDrawPanel::OnMouseUp, this);
        Bind(wxEVT_RIGHT_DOWN, &MyDrawPanel::OnRightClick, this);
        Bind(wxEVT_MOTION, &MyDrawPanel::OnMouseMove, this);
        Bind(wxEVT_KEY_DOWN, &MyDrawPanel::OnKeyDown, this);

        if (shapeLibrary.empty()) {
            LoadShapesFromJson("D:/code/Project1/x64/Debug/shapes.json");
        }

        SetFocus(); // 确保可以接收键盘事件
    }

    void AddShape(const wxString& shape) {
        m_gates.push_back({ shape, wxPoint(50 + (m_gates.size() % 3) * 150,
                                           50 + (m_gates.size() / 3) * 150), 0 });
        m_selectedIndex = (int)m_gates.size() - 1;
        Refresh();
    }

    void RemoveLastShape() {
        if (!m_gates.empty()) {
            m_gates.pop_back();
            m_selectedIndex = -1;
        }
        Refresh();
    }

    void DeleteSelected() {
        if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_gates.size()) {
            m_gates.erase(m_gates.begin() + m_selectedIndex);
            m_selectedIndex = -1;
            Refresh();
        }
    }

    void RotateSelected() {
        if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_gates.size()) {
            m_gates[m_selectedIndex].angle += 90;
            if (m_gates[m_selectedIndex].angle >= 360) {
                m_gates[m_selectedIndex].angle = 0;
            }
            Refresh();
        }
    }

    void ClearShapes() {
        m_gates.clear();
        m_wires.clear();
        m_selectedIndex = -1;
        Refresh();
    }

    void ToggleGrid() {
        m_showGrid = !m_showGrid;
        Refresh();
    }

    bool IsGridVisible() const { return m_showGrid; }

    std::vector<wxString> GetShapes() const {
        std::vector<wxString> names;
        for (auto& g : m_gates) names.push_back(g.type);
        return names;
    }

    void SetShapes(const std::vector<wxString>& shapes) {
        m_gates.clear();
        int i = 0;
        for (auto& s : shapes) {
            m_gates.push_back({ s, wxPoint(50 + (i % 3) * 150, 50 + (i / 3) * 150), 0 });
            i++;
        }
        m_selectedIndex = -1;
        Refresh();
    }

    void ZoomIn() { m_scale *= 1.2; Refresh(); }
    void ZoomOut() { m_scale /= 1.2; if (m_scale < 0.2) m_scale = 0.2; Refresh(); }

private:
    std::vector<Gate> m_gates;
    std::vector<Wire> m_wires;
    double m_scale;

    // 画线相关
    bool m_isDrawingWire;
    wxPoint m_wireStart;
    wxPoint m_currentMouse;

    // 拖拽相关
    bool m_isDraggingGate;
    int m_draggedIndex;
    wxPoint m_dragOffset;

    // 新增功能
    int m_selectedIndex;
    bool m_showGrid;
    int m_gridSize;

    // ---------- 通用绘制 ----------
    void DrawGate(wxDC& dc, const wxString& type, wxPoint pos, double angle) {
        auto it = shapeLibrary.find(type);
        if (it == shapeLibrary.end()) {
            dc.DrawText("未知组件", pos);
            return;
        }

        // 如果不需要旋转，直接绘制
        if (angle == 0) {
            for (auto& s : it->second) {
                DrawShape(dc, s, pos);
            }
        }
        else {
            // 需要旋转的情况
            double rad = angle * M_PI / 180.0;
            double cosA = cos(rad);
            double sinA = sin(rad);

            wxPoint center(pos.x + 40, pos.y + 30); // 假设中心点

            for (auto& s : it->second) {
                if (s.type == ShapeType::Line && s.pts.size() >= 2) {
                    wxPoint p1 = RotatePoint(s.pts[0], center, cosA, sinA);
                    wxPoint p2 = RotatePoint(s.pts[1], center, cosA, sinA);
                    dc.DrawLine(p1, p2);
                }
                else if (s.type == ShapeType::Polygon && s.pts.size() >= 3) {
                    std::vector<wxPoint> pts;
                    for (auto& p : s.pts) {
                        pts.push_back(RotatePoint(wxPoint(pos.x + p.x, pos.y + p.y), center, cosA, sinA));
                    }
                    dc.DrawPolygon((int)pts.size(), &pts[0]);
                }
                else if (s.type == ShapeType::Circle) {
                    wxPoint newCenter = RotatePoint(wxPoint(pos.x + s.center.x, pos.y + s.center.y), center, cosA, sinA);
                    dc.DrawCircle(newCenter, s.radius);
                }
                else if (s.type == ShapeType::Arc) {
                    // 圆弧旋转较复杂，这里简化处理
                    wxPoint newCenter = RotatePoint(wxPoint(pos.x + s.center.x, pos.y + s.center.y), center, cosA, sinA);
                    dc.DrawEllipticArc(newCenter.x - s.radius, newCenter.y - s.radius,
                        2 * s.radius, 2 * s.radius, s.startAngle + angle, s.endAngle + angle);
                }
                else if (s.type == ShapeType::Text) {
                    // 文本不旋转
                    dc.DrawText(s.text, pos.x + s.center.x, pos.y + s.center.y);
                }
            }
        }
    }

    wxPoint RotatePoint(const wxPoint& point, const wxPoint& center, double cosA, double sinA) {
        double x = point.x - center.x;
        double y = point.y - center.y;
        double newX = x * cosA - y * sinA;
        double newY = x * sinA + y * cosA;
        return wxPoint(center.x + (int)newX, center.y + (int)newY);
    }

    void DrawShape(wxDC& dc, const Shape& s, const wxPoint& pos) {
        if (s.type == ShapeType::Line && s.pts.size() >= 2) {
            dc.DrawLine(pos.x + s.pts[0].x, pos.y + s.pts[0].y,
                pos.x + s.pts[1].x, pos.y + s.pts[1].y);
        }
        else if (s.type == ShapeType::Polygon && s.pts.size() >= 3) {
            std::vector<wxPoint> pts;
            for (auto& p : s.pts) pts.push_back(wxPoint(pos.x + p.x, pos.y + p.y));
            dc.DrawPolygon((int)pts.size(), &pts[0]);
        }
        else if (s.type == ShapeType::Circle) {
            dc.DrawCircle(pos.x + s.center.x, pos.y + s.center.y, s.radius);
        }
        else if (s.type == ShapeType::Arc) {
            dc.DrawEllipticArc(pos.x + s.center.x - s.radius, pos.y + s.center.y - s.radius,
                2 * s.radius, 2 * s.radius, s.startAngle, s.endAngle);
        }
        else if (s.type == ShapeType::Text) {
            dc.DrawText(s.text, pos.x + s.center.x, pos.y + s.center.y);
        }
    }

    void DrawGrid(wxDC& dc) {
        if (!m_showGrid) return;

        wxSize size = GetClientSize();
        wxPen gridPen(wxColour(220, 220, 220), 1, wxPENSTYLE_DOT);
        dc.SetPen(gridPen);

        int logicalWidth = (int)(size.x / m_scale);
        int logicalHeight = (int)(size.y / m_scale);

        for (int x = 0; x < logicalWidth; x += m_gridSize) {
            dc.DrawLine(x, 0, x, logicalHeight);
        }
        for (int y = 0; y < logicalHeight; y += m_gridSize) {
            dc.DrawLine(0, y, logicalWidth, y);
        }

        dc.SetPen(*wxBLACK_PEN);
    }

    wxRect GetGateBBox(const Gate& g) const {
        int w = 80, h = 60;
        if (g.type == "NOT" || g.type == "BUFFER") { w = 70; h = 60; }
        else if (g.type == "LED") { w = 80; h = 90; }
        return wxRect(g.pos.x, g.pos.y, w, h);
    }

    wxPoint ToLogical(const wxPoint& devicePt) const {
        double lx = devicePt.x / m_scale;
        double ly = devicePt.y / m_scale;
        return wxPoint(int(lx + 0.5), int(ly + 0.5));
    }

    wxPoint SnapToGrid(const wxPoint& point) {
        if (!m_showGrid) return point;
        int x = (point.x + m_gridSize / 2) / m_gridSize * m_gridSize;
        int y = (point.y + m_gridSize / 2) / m_gridSize * m_gridSize;
        return wxPoint(x, y);
    }

    void OnPaint(wxPaintEvent&) {
        wxAutoBufferedPaintDC dc(this);
        dc.Clear();
        dc.SetUserScale(m_scale, m_scale);

        // 绘制网格
        DrawGrid(dc);

        wxPen redPen(wxColour(200, 0, 0), 2, wxPENSTYLE_SOLID);
        wxPen dashPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SHORT_DASH);
        wxPen selectionPen(wxColour(0, 100, 200), 2, wxPENSTYLE_SOLID);

        dc.SetPen(*wxBLACK_PEN);

        // 绘制组件
        for (size_t i = 0; i < m_gates.size(); ++i) {
            auto& g = m_gates[i];
            DrawGate(dc, g.type, g.pos, g.angle);

            // 绘制选中状态
            if ((int)i == m_selectedIndex) {
                wxRect r = GetGateBBox(g);
                dc.SetPen(selectionPen);
                dc.SetBrush(*wxTRANSPARENT_BRUSH);
                dc.DrawRectangle(r);
                dc.SetPen(*wxBLACK_PEN);
            }

            // 绘制拖拽状态
            if (m_isDraggingGate && (int)i == m_draggedIndex) {
                wxRect r = GetGateBBox(g);
                dc.SetPen(dashPen);
                dc.SetBrush(*wxTRANSPARENT_BRUSH);
                dc.DrawRectangle(r);
                dc.SetPen(*wxBLACK_PEN);
            }
        }

        // 绘制连线
        for (auto& w : m_wires) {
            dc.DrawLine(w.start, w.end);
        }

        // 绘制正在画的线
        if (m_isDrawingWire) {
            dc.SetPen(redPen);
            dc.DrawLine(m_wireStart, m_currentMouse);
            dc.SetPen(*wxBLACK_PEN);
        }
    }

    void OnMouseDown(wxMouseEvent& evt) {
        wxPoint pos = ToLogical(evt.GetPosition());
        if (m_showGrid) {
            pos = SnapToGrid(pos);
        }

        int hitIndex = -1;
        for (int i = (int)m_gates.size() - 1; i >= 0; --i) {
            if (GetGateBBox(m_gates[i]).Contains(pos)) {
                hitIndex = i;
                break;
            }
        }

        if (hitIndex != -1) {
            if (evt.LeftDown()) {
                m_isDraggingGate = true;
                m_draggedIndex = hitIndex;
                m_selectedIndex = hitIndex;
                m_dragOffset = wxPoint(pos.x - m_gates[hitIndex].pos.x, pos.y - m_gates[hitIndex].pos.y);
                if (!HasCapture()) CaptureMouse();
                SetCursor(wxCursor(wxCURSOR_SIZING));
            }
        }
        else {
            if (evt.LeftDown()) {
                m_isDrawingWire = true;
                m_wireStart = pos;
                m_currentMouse = pos;
                m_selectedIndex = -1;
                if (!HasCapture()) CaptureMouse();
                SetCursor(wxCursor(wxCURSOR_CROSS));
            }
        }
        Refresh();
    }

    void OnRightClick(wxMouseEvent& evt) {
        wxPoint pos = ToLogical(evt.GetPosition());
        int hitIndex = -1;
        for (int i = (int)m_gates.size() - 1; i >= 0; --i) {
            if (GetGateBBox(m_gates[i]).Contains(pos)) {
                hitIndex = i;
                break;
            }
        }

        m_selectedIndex = hitIndex;
        Refresh();

        // 显示右键菜单
        if (hitIndex != -1) {
            wxMenu menu;
            menu.Append(ID_ROTATE_SELECTED, "旋转组件");
            menu.AppendSeparator();
            menu.Append(ID_DELETE_SELECTED, "删除组件");

            PopupMenu(&menu);
        }
    }

    void OnMouseMove(wxMouseEvent& evt) {
        wxPoint pos = ToLogical(evt.GetPosition());
        if (m_showGrid) {
            pos = SnapToGrid(pos);
        }

        if (m_isDraggingGate && evt.Dragging() && evt.LeftIsDown()) {
            if (m_draggedIndex >= 0 && m_draggedIndex < (int)m_gates.size()) {
                m_gates[m_draggedIndex].pos = wxPoint(pos.x - m_dragOffset.x, pos.y - m_dragOffset.y);
                Refresh();
            }
        }
        else if (m_isDrawingWire && evt.Dragging() && evt.LeftIsDown()) {
            m_currentMouse = pos;
            Refresh();
        }
    }

    void OnMouseUp(wxMouseEvent&) {
        if (m_isDraggingGate) {
            m_isDraggingGate = false;
            m_draggedIndex = -1;
            if (HasCapture()) ReleaseMouse();
            SetCursor(wxCursor(wxCURSOR_ARROW));
            Refresh();
        }
        else if (m_isDrawingWire) {
            m_wires.push_back({ m_wireStart, m_currentMouse });
            m_isDrawingWire = false;
            if (HasCapture()) ReleaseMouse();
            SetCursor(wxCursor(wxCURSOR_ARROW));
            Refresh();
        }
    }

    void OnKeyDown(wxKeyEvent& evt) {
        switch (evt.GetKeyCode()) {
        case WXK_DELETE:
        case WXK_BACK:
            DeleteSelected();
            break;
        case 'R':
        case 'r':
            if (evt.ControlDown()) {
                RotateSelected();
            }
            break;
        default:
            evt.Skip();
            break;
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
    void OnToggleGrid(wxCommandEvent& event);
    void OnDeleteSelected(wxCommandEvent& event);
    void OnRotateSelected(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    void DoSave(const wxString& filename);
    void UpdateTitle();

    wxDECLARE_EVENT_TABLE();
};

// ===== 事件表 =====
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
EVT_MENU(ID_ROTATE_SELECTED, MyFrame::OnRotateSelected)

EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
wxEND_EVENT_TABLE()

// ===== 主窗口实现 =====
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
    menuEdit->Append(ID_DELETE_SELECTED, "&Delete\tDel");
    menuEdit->Append(ID_ROTATE_SELECTED, "&Rotate\tCtrl-R");

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
    SetStatusText("准备就绪 - 新增功能: 网格对齐(Ctrl+G), 组件选择(右键), 删除(Del), 旋转(Ctrl+R)");

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
    toolBar->AddTool(ID_DELETE_SELECTED, "删除", wxArtProvider::GetBitmap(wxART_DELETE, wxART_TOOLBAR));
    toolBar->AddTool(ID_ROTATE_SELECTED, "旋转", wxArtProvider::GetBitmap(wxART_REFRESH, wxART_TOOLBAR));
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
    m_treeCtrl->AppendItem(logicId, "XOR");
    m_treeCtrl->AppendItem(logicId, "NAND");
    m_treeCtrl->AppendItem(logicId, "NOR");
    m_treeCtrl->AppendItem(logicId, "XNOR");
    m_treeCtrl->AppendItem(logicId, "BUFFER");
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
        SetStatusText("添加组件: " + name);
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

void MyFrame::OnToggleGrid(wxCommandEvent& event) {
    m_drawPanel->ToggleGrid();
    SetStatusText(m_drawPanel->IsGridVisible() ? "显示网格" : "隐藏网格");
}

void MyFrame::OnDeleteSelected(wxCommandEvent& event) {
    m_drawPanel->DeleteSelected();
    SetStatusText("删除选中组件");
}

void MyFrame::OnRotateSelected(wxCommandEvent& event) {
    m_drawPanel->RotateSelected();
    SetStatusText("旋转选中组件");
}

void MyFrame::OnAbout(wxCommandEvent& event) {
    wxMessageBox("逻辑电路绘图 Demo - 增强版\n使用 wxWidgets 实现\n\n新增功能:\n"
        "• 组件选择(右键点击)\n"
        "• 网格对齐(Ctrl+G)\n"
        "• 组件删除(Delete键)\n"
        "• 组件旋转(Ctrl+R)\n"
        "• 键盘快捷键支持",
        "关于", wxOK | wxICON_INFORMATION, this);
}

void MyFrame::UpdateTitle() {
    wxString title = "wxWidgets 绘图Demo - 增强版";
    if (!m_currentFile.IsEmpty()) {
        title += " - " + m_currentFile;
    }
    SetTitle(title);
}

// ===== 应用入口 =====
class MyApp : public wxApp {
public:
    virtual bool OnInit() override {
        MyFrame* frame = new MyFrame("wxWidgets 绘图Demo - 增强版");
        frame->Show(true);
        return true;
    }
};
wxIMPLEMENT_APP(MyApp);