#include "canvas.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <cmath>

wxBEGIN_EVENT_TABLE(MyDrawPanel, wxPanel)
EVT_PAINT(MyDrawPanel::OnPaint)
EVT_LEFT_DOWN(MyDrawPanel::OnMouseDown)
EVT_LEFT_UP(MyDrawPanel::OnMouseUp)
EVT_RIGHT_DOWN(MyDrawPanel::OnRightClick)
EVT_MOTION(MyDrawPanel::OnMouseMove)
EVT_KEY_DOWN(MyDrawPanel::OnKeyDown)
wxEND_EVENT_TABLE()

MyDrawPanel::MyDrawPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE),
    m_scale(1.0), m_isDrawingWire(false),
    m_isDraggingGate(false), m_draggedIndex(-1),
    m_selectedIndex(-1), m_selectedWireIndex(-1), m_showGrid(true), m_gridSize(20)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &MyDrawPanel::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &MyDrawPanel::OnMouseDown, this);
    Bind(wxEVT_LEFT_UP, &MyDrawPanel::OnMouseUp, this);
    Bind(wxEVT_RIGHT_DOWN, &MyDrawPanel::OnRightClick, this);
    Bind(wxEVT_MOTION, &MyDrawPanel::OnMouseMove, this);
    Bind(wxEVT_KEY_DOWN, &MyDrawPanel::OnKeyDown, this);

    if (shapeLibrary.empty()) {
        LoadShapesFromJson("shapes.json");
    }

    SetFocus(); // 确保可以接收键盘事件
}

void MyDrawPanel::AddShape(const wxString& shape) {
    Gate newGate;
    newGate.type = shape;
    newGate.pos = wxPoint(50 + (m_gates.size() % 3) * 150, 50 + (m_gates.size() / 3) * 150);

    // 为不同组件类型设置默认属性
    if (shape == "LED") {
        Property colorProp;
        colorProp.name = "颜色";
        colorProp.value = "红色";
        colorProp.type = "string";
        newGate.properties.push_back(colorProp);

        Property voltageProp;
        voltageProp.name = "工作电压";
        voltageProp.value = "3.3";
        voltageProp.type = "double";
        newGate.properties.push_back(voltageProp);
    }
    else if (shape == "开关") {
        Property stateProp;
        stateProp.name = "初始状态";
        stateProp.value = "关闭";
        stateProp.type = "string";
        newGate.properties.push_back(stateProp);
    }
    else if (shape == "AND" || shape == "OR" || shape == "NOT" ||
        shape == "XOR" || shape == "NAND" || shape == "NOR" ||
        shape == "XNOR" || shape == "BUFFER") {
        Property delayProp;
        delayProp.name = "传播延迟";
        delayProp.value = "10";
        delayProp.type = "int";
        newGate.properties.push_back(delayProp);
    }

    m_gates.push_back(newGate);
    m_selectedIndex = (int)m_gates.size() - 1;
    m_selectedWireIndex = -1; // 取消线条选中
    Refresh();
}

void MyDrawPanel::RemoveLastShape() {
    if (!m_gates.empty()) {
        m_gates.pop_back();
        m_selectedIndex = -1;
    }
    Refresh();
}

void MyDrawPanel::DeleteSelected() {
    if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_gates.size()) {
        m_gates.erase(m_gates.begin() + m_selectedIndex);
        m_selectedIndex = -1;
        Refresh();
    }
}

void MyDrawPanel::DeleteSelectedWire() {
    if (m_selectedWireIndex >= 0 && m_selectedWireIndex < (int)m_wires.size()) {
        m_wires.erase(m_wires.begin() + m_selectedWireIndex);
        m_selectedWireIndex = -1;
        Refresh();
    }
}

void MyDrawPanel::EditSelectedProperties() {
    if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_gates.size()) {
        PropertyDialog dialog(this, m_gates[m_selectedIndex]);
        if (dialog.ShowModal() == wxID_OK) {
            Refresh();
        }
    }
}

void MyDrawPanel::ClearShapes() {
    m_gates.clear();
    m_wires.clear();
    m_selectedIndex = -1;
    m_selectedWireIndex = -1;
    Refresh();
}

void MyDrawPanel::ToggleGrid() {
    m_showGrid = !m_showGrid;
    Refresh();
}

// 获取当前状态（用于撤销重做）
MyDrawPanel::DrawPanelState MyDrawPanel::GetState() const {
    DrawPanelState state;
    state.gates = m_gates;
    state.wires = m_wires;
    state.selectedIndex = m_selectedIndex;
    state.selectedWireIndex = m_selectedWireIndex;
    return state;
}

void MyDrawPanel::SetState(const DrawPanelState& state) {
    m_gates = state.gates;
    m_wires = state.wires;
    m_selectedIndex = state.selectedIndex;
    m_selectedWireIndex = state.selectedWireIndex;
    Refresh();
}

std::vector<wxString> MyDrawPanel::GetShapes() const {
    std::vector<wxString> names;
    for (auto& g : m_gates) names.push_back(g.type);
    return names;
}

void MyDrawPanel::SetShapes(const std::vector<wxString>& shapes) {
    m_gates.clear();
    int i = 0;
    for (auto& s : shapes) {
        Gate newGate;
        newGate.type = s;
        newGate.pos = wxPoint(50 + (i % 3) * 150, 50 + (i / 3) * 150);
        m_gates.push_back(newGate);
        i++;
    }
    m_selectedIndex = -1;
    m_selectedWireIndex = -1;
    Refresh();
}

// 获取所有门及其属性（用于保存）
std::vector<Gate> MyDrawPanel::GetGates() const {
    return m_gates;
}

// 设置所有门及其属性（用于加载）
void MyDrawPanel::SetGates(const std::vector<Gate>& gates) {
    m_gates = gates;
    m_selectedIndex = -1;
    m_selectedWireIndex = -1;
    Refresh();
}

void MyDrawPanel::ZoomIn() {
    m_scale *= 1.2;
    Refresh();
}

void MyDrawPanel::ZoomOut() {
    m_scale /= 1.2;
    if (m_scale < 0.2) m_scale = 0.2;
    Refresh();
}

// ---------- 绘制方法 ----------
void MyDrawPanel::DrawGate(wxDC& dc, const Gate& gate) {
    auto it = shapeLibrary.find(gate.type);
    if (it == shapeLibrary.end()) {
        dc.DrawText("未知组件", gate.pos);
        return;
    }

    // 直接绘制
    for (auto& s : it->second) {
        DrawShape(dc, s, gate.pos);
    }

    // 绘制属性文本（如果有）
    if (!gate.properties.empty()) {
        wxFont smallFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        dc.SetFont(smallFont);
        dc.SetTextForeground(wxColour(100, 100, 100));

        int yOffset = 70; // 在组件下方显示属性
        for (const auto& prop : gate.properties) {
            wxString propText = wxString::Format("%s: %s", prop.name, prop.value);
            dc.DrawText(propText, gate.pos.x, gate.pos.y + yOffset);
            yOffset += 12;
        }

        // 恢复默认字体和颜色
        dc.SetFont(wxNullFont);
        dc.SetTextForeground(*wxBLACK);
    }
}

void MyDrawPanel::DrawShape(wxDC& dc, const Shape& s, const wxPoint& pos) {
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

void MyDrawPanel::DrawGrid(wxDC& dc) {
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

wxRect MyDrawPanel::GetGateBBox(const Gate& g) const {
    int w = 80, h = 60;
    if (g.type == "NOT" || g.type == "BUFFER") { w = 70; h = 60; }
    else if (g.type == "LED") { w = 80; h = 90; }
    return wxRect(g.pos.x, g.pos.y, w, h);
}

// 检查点是否在线条附近
bool MyDrawPanel::IsPointNearLine(const wxPoint& point, const Wire& wire, int tolerance) {
    // 计算点到线段的距离
    wxPoint p = point;
    wxPoint a = wire.start;
    wxPoint b = wire.end;

    // 线段长度的平方
    double length2 = (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
    if (length2 == 0) {
        // 线段退化为点
        return (p.x - a.x) * (p.x - a.x) + (p.y - a.y) * (p.y - a.y) <= tolerance * tolerance;
    }

    // 计算投影比例
    double t = std::max(0.0, std::min(1.0,
        ((p.x - a.x) * (b.x - a.x) + (p.y - a.y) * (b.y - a.y)) / length2));

    // 计算投影点
    wxPoint projection(
        a.x + t * (b.x - a.x),
        a.y + t * (b.y - a.y)
    );

    // 计算距离平方
    double distance2 = (p.x - projection.x) * (p.x - projection.x) +
        (p.y - projection.y) * (p.y - projection.y);

    return distance2 <= tolerance * tolerance;
}

wxPoint MyDrawPanel::ToLogical(const wxPoint& devicePt) const {
    double lx = devicePt.x / m_scale;
    double ly = devicePt.y / m_scale;
    return wxPoint(int(lx + 0.5), int(ly + 0.5));
}

wxPoint MyDrawPanel::SnapToGrid(const wxPoint& point) {
    if (!m_showGrid) return point;
    int x = (point.x + m_gridSize / 2) / m_gridSize * m_gridSize;
    int y = (point.y + m_gridSize / 2) / m_gridSize * m_gridSize;
    return wxPoint(x, y);
}

// ---------- 事件处理 ----------
void MyDrawPanel::OnPaint(wxPaintEvent&) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    dc.SetUserScale(m_scale, m_scale);

    // 绘制网格
    DrawGrid(dc);

    wxPen redPen(wxColour(200, 0, 0), 2, wxPENSTYLE_SOLID);
    wxPen dashPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SHORT_DASH);
    wxPen selectionPen(wxColour(0, 100, 200), 2, wxPENSTYLE_SOLID);
    wxPen wireSelectionPen(wxColour(255, 0, 0), 3, wxPENSTYLE_SOLID); // 选中线条用红色粗线

    dc.SetPen(*wxBLACK_PEN);

    // 绘制组件
    for (size_t i = 0; i < m_gates.size(); ++i) {
        auto& g = m_gates[i];
        DrawGate(dc, g);

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
    for (size_t i = 0; i < m_wires.size(); ++i) {
        auto& w = m_wires[i];
        if ((int)i == m_selectedWireIndex) {
            dc.SetPen(wireSelectionPen);
        }
        else {
            dc.SetPen(*wxBLACK_PEN);
        }
        dc.DrawLine(w.start, w.end);
        dc.SetPen(*wxBLACK_PEN);
    }

    // 绘制正在画的线
    if (m_isDrawingWire) {
        dc.SetPen(redPen);
        dc.DrawLine(m_wireStart, m_currentMouse);
        dc.SetPen(*wxBLACK_PEN);
    }
}

void MyDrawPanel::OnMouseDown(wxMouseEvent& evt) {
    wxPoint pos = ToLogical(evt.GetPosition());
    if (m_showGrid) {
        pos = SnapToGrid(pos);
    }

    // 先检查是否点击了线条
    m_selectedWireIndex = -1;
    for (int i = (int)m_wires.size() - 1; i >= 0; --i) {
        if (IsPointNearLine(pos, m_wires[i])) {
            m_selectedWireIndex = i;
            m_selectedIndex = -1; // 取消组件选中
            Refresh();
            return;
        }
    }

    // 检查是否点击了组件
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
            m_selectedWireIndex = -1; // 取消线条选中
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
            m_selectedWireIndex = -1;
            if (!HasCapture()) CaptureMouse();
            SetCursor(wxCursor(wxCURSOR_CROSS));
        }
    }
    Refresh();
}

void MyDrawPanel::OnRightClick(wxMouseEvent& evt) {
    wxPoint pos = ToLogical(evt.GetPosition());

    // 检查是否右键点击了线条
    int hitWireIndex = -1;
    for (int i = (int)m_wires.size() - 1; i >= 0; --i) {
        if (IsPointNearLine(pos, m_wires[i])) {
            hitWireIndex = i;
            break;
        }
    }

    if (hitWireIndex != -1) {
        m_selectedWireIndex = hitWireIndex;
        m_selectedIndex = -1;

        // 显示线条右键菜单
        wxMenu menu;
        menu.Append(ID_DELETE_WIRE, "删除线条");
        PopupMenu(&menu);
        return;
    }

    // 检查是否右键点击了组件
    int hitIndex = -1;
    for (int i = (int)m_gates.size() - 1; i >= 0; --i) {
        if (GetGateBBox(m_gates[i]).Contains(pos)) {
            hitIndex = i;
            break;
        }
    }

    m_selectedIndex = hitIndex;
    m_selectedWireIndex = -1;
    Refresh();

    // 显示组件右键菜单
    if (hitIndex != -1) {
        wxMenu menu;
        menu.Append(ID_EDIT_PROPERTIES, "编辑属性");
        menu.AppendSeparator();
        menu.Append(ID_DELETE_SELECTED, "删除组件");

        PopupMenu(&menu);
    }
}

void MyDrawPanel::OnMouseMove(wxMouseEvent& evt) {
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

void MyDrawPanel::OnMouseUp(wxMouseEvent&) {
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

void MyDrawPanel::OnKeyDown(wxKeyEvent& evt) {
    switch (evt.GetKeyCode()) {
    case WXK_DELETE:
    case WXK_BACK:
        if (m_selectedIndex >= 0) {
            DeleteSelected();
        }
        else if (m_selectedWireIndex >= 0) {
            DeleteSelectedWire();
        }
        break;
    case 'P':
    case 'p':
        if (evt.ControlDown()) {
            EditSelectedProperties();
        }
        break;
    default:
        evt.Skip();
        break;
    }
}