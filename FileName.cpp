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

// ---------- ��������ͼ�� ----------
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

// ===== ��ͼ���� =====
class MyDrawPanel : public wxPanel {
public:
    MyDrawPanel(wxWindow* parent)
        : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE),
        m_scale(1.0), m_isDrawingWire(false),
        m_isDraggingGate(false), m_draggedIndex(-1)
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        Bind(wxEVT_PAINT, &MyDrawPanel::OnPaint, this);
        Bind(wxEVT_LEFT_DOWN, &MyDrawPanel::OnMouseDown, this);
        Bind(wxEVT_LEFT_UP, &MyDrawPanel::OnMouseUp, this);
        Bind(wxEVT_MOTION, &MyDrawPanel::OnMouseMove, this);

        // ��ʼ�� shapeLibrary
        if (shapeLibrary.empty()) {
            shapeLibrary["AND"] = {
                { ShapeType::Line, { {0,0},{40,0} } },
                { ShapeType::Line, { {0,60},{40,60} } },
                { ShapeType::Line, { {0,0},{0,60} } },
                { ShapeType::Arc, {}, {40,30}, 30, 270, 90 },
                { ShapeType::Line, { {-20,15},{0,15} } },
                { ShapeType::Line, { {-20,45},{0,45} } },
                { ShapeType::Line, { {70,30},{90,30} } }
            };
            shapeLibrary["OR"] = {
                { ShapeType::Arc, {}, {20,30}, 60, 270, 90 },
                { ShapeType::Arc, {}, {0,30}, 60, 270, 90 },
                { ShapeType::Line, { {-30,15},{0,15} } },
                { ShapeType::Line, { {-30,45},{0,45} } },
                { ShapeType::Line, { {70,30},{90,30} } }
            };
            shapeLibrary["NOT"] = {
                { ShapeType::Polygon, { {0,0},{0,60},{60,30} } },
                { ShapeType::Circle, {}, {70,30}, 6 },
                { ShapeType::Line, { {-20,30},{0,30} } },
                { ShapeType::Line, { {76,30},{96,30} } }
            };
            shapeLibrary["BUFFER"] = {
                { ShapeType::Polygon, { {0,0},{0,60},{60,30} } },
                { ShapeType::Line, { {-20,30},{0,30} } },
                { ShapeType::Line, { {60,30},{80,30} } }
            };
            shapeLibrary["XOR"] = {
                { ShapeType::Arc, {}, {20,30}, 60, 270, 90 },
                { ShapeType::Arc, {}, {0,30}, 60, 270, 90 },
                { ShapeType::Arc, {}, {-10,30}, 60, 270, 90 },
                { ShapeType::Line, { {-35,15},{0,15} } },
                { ShapeType::Line, { {-35,45},{0,45} } },
                { ShapeType::Line, { {70,30},{90,30} } }
            };
            shapeLibrary["LED"] = {
                { ShapeType::Circle, {}, {40,40}, 30 },
                { ShapeType::Text, {}, {30,80}, 0,0,0,"LED" }
            };

            // ��� NAND / NOR / XNOR
            shapeLibrary["NAND"] = shapeLibrary["AND"];
            shapeLibrary["NAND"].push_back({ ShapeType::Circle, {}, {95,30}, 6 });

            shapeLibrary["NOR"] = shapeLibrary["OR"];
            shapeLibrary["NOR"].push_back({ ShapeType::Circle, {}, {95,30}, 6 });

            shapeLibrary["XNOR"] = shapeLibrary["XOR"];
            shapeLibrary["XNOR"].push_back({ ShapeType::Circle, {}, {95,30}, 6 });
        }
    }

    void AddShape(const wxString& shape) {
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

    void ZoomIn() { m_scale *= 1.2; Refresh(); }
    void ZoomOut() { m_scale /= 1.2; if (m_scale < 0.2) m_scale = 0.2; Refresh(); }

private:
    std::vector<Gate> m_gates;
    std::vector<Wire> m_wires;
    double m_scale;

    // �������
    bool m_isDrawingWire;
    wxPoint m_wireStart;
    wxPoint m_currentMouse;

    // ��ק���
    bool m_isDraggingGate;
    int m_draggedIndex;
    wxPoint m_dragOffset;

    // ---------- ͨ�û��� ----------
    void DrawGate(wxDC& dc, const wxString& type, wxPoint pos) {
        auto it = shapeLibrary.find(type);
        if (it == shapeLibrary.end()) {
            dc.DrawText("δ֪���", pos);
            return;
        }
        for (auto& s : it->second) {
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

    void OnPaint(wxPaintEvent&) {
        wxAutoBufferedPaintDC dc(this);
        dc.Clear();
        dc.SetUserScale(m_scale, m_scale);

        wxPen redPen(wxColour(200, 0, 0), 2, wxPENSTYLE_SOLID);
        wxPen dashPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SHORT_DASH);

        dc.SetPen(*wxBLACK_PEN);

        for (size_t i = 0; i < m_gates.size(); ++i) {
            auto& g = m_gates[i];
            DrawGate(dc, g.type, g.pos);

            if (m_isDraggingGate && (int)i == m_draggedIndex) {
                wxRect r = GetGateBBox(g);
                dc.SetPen(dashPen);
                dc.SetBrush(*wxTRANSPARENT_BRUSH);
                dc.DrawRectangle(r);
                dc.SetPen(*wxBLACK_PEN);
                dc.SetBrush(*wxWHITE_BRUSH);
            }
        }

        for (auto& w : m_wires) {
            dc.DrawLine(w.start, w.end);
        }

        if (m_isDrawingWire) {
            dc.SetPen(redPen);
            dc.DrawLine(m_wireStart, m_currentMouse);
            dc.SetPen(*wxBLACK_PEN);
        }
    }

    void OnMouseDown(wxMouseEvent& evt) {
        wxPoint pos = ToLogical(evt.GetPosition());
        int hitIndex = -1;
        for (int i = (int)m_gates.size() - 1; i >= 0; --i) {
            if (GetGateBBox(m_gates[i]).Contains(pos)) {
                hitIndex = i; break;
            }
        }

        if (hitIndex != -1) {
            m_isDraggingGate = true;
            m_draggedIndex = hitIndex;
            m_dragOffset = wxPoint(pos.x - m_gates[hitIndex].pos.x, pos.y - m_gates[hitIndex].pos.y);
            if (!HasCapture()) CaptureMouse();
            SetCursor(wxCursor(wxCURSOR_SIZING));
        }
        else {
            m_isDrawingWire = true;
            m_wireStart = pos;
            m_currentMouse = pos;
            if (!HasCapture()) CaptureMouse();
            SetCursor(wxCursor(wxCURSOR_CROSS));
        }
        Refresh();
    }

    void OnMouseMove(wxMouseEvent& evt) {
        wxPoint pos = ToLogical(evt.GetPosition());
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
        wxPoint pos = m_currentMouse;
        if (m_isDraggingGate) {
            m_isDraggingGate = false;
            m_draggedIndex = -1;
            if (HasCapture()) ReleaseMouse();
            SetCursor(wxCursor(wxCURSOR_ARROW));
            Refresh();
        }
        else if (m_isDrawingWire) {
            m_wires.push_back({ m_wireStart, pos });
            m_isDrawingWire = false;
            if (HasCapture()) ReleaseMouse();
            SetCursor(wxCursor(wxCURSOR_ARROW));
            Refresh();
        }
    }
};

// ===== ������ =====
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

// ===== �¼��� =====
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

// ===== ������ʵ�� =====
MyFrame::MyFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1000, 650))
{
    // �˵���
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
    SetStatusText("׼������");

    // ������
    wxToolBar* toolBar = CreateToolBar();
    toolBar->AddTool(wxID_NEW, "�½�", wxArtProvider::GetBitmap(wxART_NEW, wxART_TOOLBAR));
    toolBar->AddTool(wxID_OPEN, "��", wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR));
    toolBar->AddTool(wxID_SAVE, "����", wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR));
    toolBar->AddSeparator();
    toolBar->AddTool(wxID_UNDO, "����", wxArtProvider::GetBitmap(wxART_UNDO, wxART_TOOLBAR));
    toolBar->AddTool(wxID_REDO, "����", wxArtProvider::GetBitmap(wxART_REDO, wxART_TOOLBAR));
    toolBar->AddSeparator();
    toolBar->AddTool(wxID_COPY, "����", wxArtProvider::GetBitmap(wxART_COPY, wxART_TOOLBAR));
    toolBar->AddTool(wxID_PASTE, "ճ��", wxArtProvider::GetBitmap(wxART_PASTE, wxART_TOOLBAR));
    toolBar->AddSeparator();
    toolBar->AddTool(wxID_ABOUT, "����", wxArtProvider::GetBitmap(wxART_HELP, wxART_TOOLBAR));
    toolBar->Realize();

    // �ָ��
    m_splitter = new wxSplitterWindow(this, wxID_ANY);
    m_treeCtrl = new wxTreeCtrl(m_splitter, wxID_ANY, wxDefaultPosition, wxSize(200, -1),
        wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_DEFAULT_STYLE);
    m_drawPanel = new MyDrawPanel(m_splitter);
    m_splitter->SplitVertically(m_treeCtrl, m_drawPanel, 200);
    m_splitter->SetSashGravity(0.0);

    // ���ڵ�
    wxTreeItemId root = m_treeCtrl->AddRoot("�����");
    wxTreeItemId inputId = m_treeCtrl->AppendItem(root, "����");
    wxTreeItemId logicId = m_treeCtrl->AppendItem(root, "�߼���");
    wxTreeItemId outputId = m_treeCtrl->AppendItem(root, "���");

    m_treeCtrl->AppendItem(inputId, "����");
    m_treeCtrl->AppendItem(inputId, "��ť");
    m_treeCtrl->AppendItem(logicId, "AND");
    m_treeCtrl->AppendItem(logicId, "OR");
    m_treeCtrl->AppendItem(logicId, "NOT");
    m_treeCtrl->AppendItem(logicId, "XOR");
    m_treeCtrl->AppendItem(logicId, "NAND");
    m_treeCtrl->AppendItem(logicId, "NOR");
    m_treeCtrl->AppendItem(logicId, "XNOR");
    m_treeCtrl->AppendItem(logicId, "BUFFER");
    m_treeCtrl->AppendItem(outputId, "LED");
    m_treeCtrl->AppendItem(outputId, "������");
    m_treeCtrl->ExpandAll();

    // ���ڵ����¼�
    m_treeCtrl->Bind(wxEVT_TREE_SEL_CHANGED, [this](wxTreeEvent& event) {
        wxTreeItemId item = event.GetItem();
        if (!item.IsOk()) return;
        wxString name = m_treeCtrl->GetItemText(item);

        // ���� undo
        undoStack.push(m_drawPanel->GetShapes());
        while (!redoStack.empty()) redoStack.pop();

        m_drawPanel->AddShape(name);
        SetStatusText("ѡ�нڵ�: " + name);
        });

    UpdateTitle();
}

// ===== ����ʵ�� =====
void MyFrame::OnNew(wxCommandEvent& event) {
    undoStack.push(m_drawPanel->GetShapes());
    redoStack = std::stack<std::vector<wxString>>();
    m_drawPanel->ClearShapes();
    m_currentFile.Clear();
    UpdateTitle();
    SetStatusText("�½��ļ�");
}

void MyFrame::OnOpen(wxCommandEvent& event) {
    wxFileDialog openFileDialog(this, "���ļ�", "", "",
        "Logic Files (*.logic)|*.logic|All Files (*.*)|*.*",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    wxFileInputStream input(openFileDialog.GetPath());
    if (!input.IsOk()) {
        wxLogError("�޷����ļ� '%s'.", openFileDialog.GetPath());
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
    SetStatusText("�Ѵ�: " + m_currentFile);
}

void MyFrame::DoSave(const wxString& filename) {
    wxFileOutputStream output(filename);
    if (!output.IsOk()) {
        wxLogError("�޷������ļ� '%s'.", filename);
        return;
    }

    wxTextOutputStream text(output);
    for (auto& shape : m_drawPanel->GetShapes()) {
        text << shape << "\n";
    }

    m_currentFile = filename;
    UpdateTitle();
    SetStatusText("�ѱ���: " + filename);
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
    wxFileDialog saveFileDialog(this, "�����ļ�", "", "",
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
        SetStatusText("�����ɹ�");
    }
    else {
        SetStatusText("û�пɳ����Ĳ���");
    }
}

void MyFrame::OnRedo(wxCommandEvent& event) {
    if (!redoStack.empty()) {
        undoStack.push(m_drawPanel->GetShapes());
        m_drawPanel->SetShapes(redoStack.top());
        redoStack.pop();
        SetStatusText("�����ɹ�");
    }
    else {
        SetStatusText("û�п������Ĳ���");
    }
}

void MyFrame::OnCopy(wxCommandEvent& event) {
    const auto& shapes = m_drawPanel->GetShapes();
    if (!shapes.empty()) {
        wxTheClipboard->Open();
        wxTheClipboard->SetData(new wxTextDataObject(shapes.back()));
        wxTheClipboard->Close();
        SetStatusText("�Ѹ���: " + shapes.back());
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
            SetStatusText("��ճ��: " + data.GetText());
        }
        wxTheClipboard->Close();
    }
}

void MyFrame::OnZoomin(wxCommandEvent& event) {
    m_drawPanel->ZoomIn();
    SetStatusText("�Ŵ���ͼ");
}

void MyFrame::OnZoomout(wxCommandEvent& event) {
    m_drawPanel->ZoomOut();
    SetStatusText("��С��ͼ");
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
    wxMessageBox("�߼���·��ͼ Demo\nʹ�� wxWidgets ʵ��\n\n֧�ֻ������ļ�����������������������ơ�",
        "����", wxOK | wxICON_INFORMATION, this);
}

void MyFrame::UpdateTitle() {
    wxString title = "wxWidgets ��ͼDemo";
    if (!m_currentFile.IsEmpty()) {
        title += " - " + m_currentFile;
    }
    SetTitle(title);
}

// ===== Ӧ����� =====
class MyApp : public wxApp {
public:
    virtual bool OnInit() override {
        MyFrame* frame = new MyFrame("wxWidgets ��ͼDemo");
        frame->Show(true);
        return true;
    }
};
wxIMPLEMENT_APP(MyApp);
