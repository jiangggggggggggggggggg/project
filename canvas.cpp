#include "canvas.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <cmath>
#include <wx/textdlg.h>
#include <wx/msgdlg.h>
#include <fstream>

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
    m_isDraggingGate(false), m_isDraggingBreakpoint(false),
    m_draggedIndex(-1), m_draggedBreakpointIndex(-1),
    m_selectedIndex(-1), m_selectedWireIndex(-1), m_selectedBreakpointIndex(-1),
    m_showGrid(true), m_showPins(true), m_gridSize(20),
    m_wireMode(WIRE_ORTHOGONAL),
    m_hoveredPin{ -1, -1, wxPoint() },
    m_connectedStartPin{ -1, -1, wxPoint() },
    m_connectedEndPin{ -1, -1, wxPoint() }
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &MyDrawPanel::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &MyDrawPanel::OnMouseDown, this);
    Bind(wxEVT_LEFT_UP, &MyDrawPanel::OnMouseUp, this);
    Bind(wxEVT_RIGHT_DOWN, &MyDrawPanel::OnRightClick, this);
    Bind(wxEVT_MOTION, &MyDrawPanel::OnMouseMove, this);
    Bind(wxEVT_KEY_DOWN, &MyDrawPanel::OnKeyDown, this);

    // 直接使用硬编码定义
    if (shapeLibrary.empty()) {
        InitializeHardcodedShapes();
    }

    if (pinLibrary.empty()) {
        InitializeHardcodedPins();
    }

    SetFocus();
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

    // 加载引脚信息
    auto pinIt = pinLibrary.find(shape);
    if (pinIt != pinLibrary.end()) {
        newGate.pins = pinIt->second;
    }

    m_gates.push_back(newGate);
    m_selectedIndex = (int)m_gates.size() - 1;
    m_selectedWireIndex = -1;
    m_selectedBreakpointIndex = -1;
    Refresh();
}

void MyDrawPanel::RemoveLastShape() {
    if (!m_gates.empty()) {
        m_gates.pop_back();
        m_selectedIndex = -1;
        m_selectedBreakpointIndex = -1;
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

void MyDrawPanel::DeleteSelectedBreakpoint() {
    if (m_selectedBreakpointIndex >= 0 && m_selectedBreakpointIndex < (int)m_breakpoints.size()) {
        // 删除连接到这个断点的所有连线
        const Breakpoint& bp = m_breakpoints[m_selectedBreakpointIndex];
        for (int wireIndex : bp.connectedWires) {
            if (wireIndex >= 0 && wireIndex < (int)m_wires.size()) {
                m_wires[wireIndex] = Wire(); // 标记为删除
            }
        }

        // 移除标记为删除的连线
        m_wires.erase(std::remove_if(m_wires.begin(), m_wires.end(),
            [](const Wire& w) { return w.start == wxPoint() && w.end == wxPoint(); }),
            m_wires.end());

        m_breakpoints.erase(m_breakpoints.begin() + m_selectedBreakpointIndex);
        m_selectedBreakpointIndex = -1;
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
    m_breakpoints.clear();
    m_selectedIndex = -1;
    m_selectedWireIndex = -1;
    m_selectedBreakpointIndex = -1;
    Refresh();
}

void MyDrawPanel::ToggleGrid() {
    m_showGrid = !m_showGrid;
    Refresh();
}

void MyDrawPanel::TogglePins() {
    m_showPins = !m_showPins;
    Refresh();
}

void MyDrawPanel::ToggleWireMode() {
    m_wireMode = (m_wireMode == WIRE_STRAIGHT) ? WIRE_ORTHOGONAL : WIRE_STRAIGHT;
    Refresh();
}

// 断点操作 - 优化版本
void MyDrawPanel::AddBreakpoint(const wxPoint& position, int wireIndex) {
    if (wireIndex < 0 || wireIndex >= (int)m_wires.size()) {
        return;
    }

    const Wire& wire = m_wires[wireIndex];

    // 计算在线段上的准确位置
    wxPoint accuratePoint;
    double t = 0.0;

    if (m_wireMode == WIRE_STRAIGHT) {
        accuratePoint = FindClosestPointOnLine(wire.start, wire.end, position);
        t = CalculateParameterOnLine(wire.start, wire.end, accuratePoint);
    }
    else {
        accuratePoint = FindClosestPointOnOrthogonalLine(wire, position, t);
    }

    // 创建断点引脚
    Pin breakpointPin;
    breakpointPin.name = "breakpoint";
    breakpointPin.relativePos = wxPoint(0, 0);
    breakpointPin.isInput = true;

    Breakpoint bp;
    bp.position = accuratePoint;
    bp.wireIndex = wireIndex;
    bp.t = std::max(0.0, std::min(1.0, t));
    bp.pin = breakpointPin;
    bp.breakpointId = wxString::Format("BP%d", (int)m_breakpoints.size() + 1);

    m_breakpoints.push_back(bp);
    Refresh();
}

// 检查点是否靠近断点
bool MyDrawPanel::IsPointNearBreakpoint(const wxPoint& point, int tolerance) {
    return FindBreakpointAtPoint(point, tolerance) != -1;
}

// 查找点附近的断点
int MyDrawPanel::FindBreakpointAtPoint(const wxPoint& point, int tolerance) {
    for (size_t i = 0; i < m_breakpoints.size(); ++i) {
        const Breakpoint& bp = m_breakpoints[i];
        double distance = sqrt(pow(point.x - bp.position.x, 2) + pow(point.y - bp.position.y, 2));
        if (distance <= tolerance + 2) {
            return (int)i;
        }
    }
    return -1;
}

// 计算线段上的点
wxPoint MyDrawPanel::CalculatePointOnWire(const Wire& wire, double t) {
    t = std::max(0.0, std::min(1.0, t));
    return wxPoint(
        wire.start.x + t * (wire.end.x - wire.start.x),
        wire.start.y + t * (wire.end.y - wire.start.y)
    );
}

// 吸附到断点
wxPoint MyDrawPanel::SnapToBreakpoint(const wxPoint& point) {
    int bpIndex = FindBreakpointAtPoint(point, 10);
    if (bpIndex != -1) {
        return m_breakpoints[bpIndex].position;
    }
    return point;
}

// 更新连线从断点
void MyDrawPanel::UpdateWiresFromBreakpoints() {
    // 这里可以实现从断点重新生成连线的逻辑
}

MyDrawPanel::DrawPanelState MyDrawPanel::GetState() const {
    DrawPanelState state;
    state.gates = m_gates;
    state.wires = m_wires;
    state.breakpoints = m_breakpoints;
    state.selectedIndex = m_selectedIndex;
    state.selectedWireIndex = m_selectedWireIndex;
    state.selectedBreakpointIndex = m_selectedBreakpointIndex;
    return state;
}

void MyDrawPanel::SetState(const DrawPanelState& state) {
    m_gates = state.gates;
    m_wires = state.wires;
    m_breakpoints = state.breakpoints;
    m_selectedIndex = state.selectedIndex;
    m_selectedWireIndex = state.selectedWireIndex;
    m_selectedBreakpointIndex = state.selectedBreakpointIndex;
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

        auto pinIt = pinLibrary.find(s);
        if (pinIt != pinLibrary.end()) {
            newGate.pins = pinIt->second;
        }

        m_gates.push_back(newGate);
        i++;
    }
    m_selectedIndex = -1;
    m_selectedWireIndex = -1;
    m_selectedBreakpointIndex = -1;
    Refresh();
}

std::vector<Gate> MyDrawPanel::GetGates() const {
    return m_gates;
}

void MyDrawPanel::SetGates(const std::vector<Gate>& gates) {
    m_gates = gates;
    m_selectedIndex = -1;
    m_selectedWireIndex = -1;
    m_selectedBreakpointIndex = -1;
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

Netlist MyDrawPanel::GenerateNetlist() const {
    return ::GenerateNetlist(m_gates, m_wires);
}

void MyDrawPanel::ImportFromNetlist(const Netlist& netlist) {
    m_gates.clear();
    m_wires.clear();
    m_breakpoints.clear();

    for (const auto& comp : netlist.components) {
        Gate gate;
        gate.type = comp.componentType;
        gate.pos = comp.position;
        gate.properties = comp.properties;
        gate.pins = comp.pins;
        m_gates.push_back(gate);
    }

    Refresh();
}

void MyDrawPanel::ShowNetlistPreview() {
    Netlist netlist = GenerateNetlist();

    wxString previewText;
    previewText << "网表预览 - " << netlist.designName << "\n";
    previewText << "生成时间: " << netlist.timestamp << "\n\n";

    previewText << "=== 组件列表 ===\n";
    for (const auto& comp : netlist.components) {
        previewText << comp.componentId << " : " << comp.componentType;
        previewText << " @ (" << comp.position.x << ", " << comp.position.y << ")\n";

        for (const auto& pin : comp.pins) {
            previewText << "  - " << pin.name << " (" << (pin.isInput ? "输入" : "输出") << ")\n";
        }
    }

    previewText << "\n=== 网络连接 ===\n";
    for (const auto& node : netlist.nodes) {
        previewText << node.netName << " : ";
        for (size_t i = 0; i < node.connections.size(); ++i) {
            previewText << node.connections[i].componentId << ":" << node.connections[i].pinName;
            if (i < node.connections.size() - 1) {
                previewText << " <-> ";
            }
        }
        previewText << "\n";
    }

    wxMessageBox(previewText, "网表预览", wxOK | wxICON_INFORMATION, this);
}

// ---------- 绘制方法 ----------
void MyDrawPanel::DrawGate(wxDC& dc, const Gate& gate) {
    auto it = shapeLibrary.find(gate.type);
    if (it == shapeLibrary.end()) {
        // 绘制默认矩形作为后备
        dc.SetPen(*wxBLACK_PEN);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRectangle(gate.pos.x, gate.pos.y, 80, 60);
        dc.DrawText(gate.type, gate.pos.x + 10, gate.pos.y + 20);
        return;
    }

    // 绘制组件形状
    for (auto& s : it->second) {
        DrawShape(dc, s, gate.pos);
    }

    // 绘制引脚
    if (m_showPins) {
        for (size_t j = 0; j < gate.pins.size(); ++j) {
            const auto& pin = gate.pins[j];
            bool isHovered = (m_hoveredPin.gateIndex == (int)(&gate - &m_gates[0]) &&
                m_hoveredPin.pinIndex == (int)j);
            DrawPin(dc, pin, gate.pos, isHovered);
        }
    }

    // 绘制属性文本
    if (!gate.properties.empty()) {
        wxFont smallFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        dc.SetFont(smallFont);
        dc.SetTextForeground(wxColour(100, 100, 100));

        int yOffset = 70;
        for (const auto& prop : gate.properties) {
            wxString propText = wxString::Format("%s: %s", prop.name, prop.value);
            dc.DrawText(propText, gate.pos.x, gate.pos.y + yOffset);
            yOffset += 12;
        }

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
    else if (s.type == ShapeType::Polyline && s.pts.size() >= 2) {
        std::vector<wxPoint> pts;
        for (auto& p : s.pts) pts.push_back(wxPoint(pos.x + p.x, pos.y + p.y));
        dc.DrawLines((int)pts.size(), &pts[0]);
    }
    else if (s.type == ShapeType::Rectangle && s.pts.size() >= 2) {
        dc.DrawRectangle(pos.x + s.pts[0].x, pos.y + s.pts[0].y,
            s.pts[1].x - s.pts[0].x, s.pts[1].y - s.pts[0].y);
    }
}

void MyDrawPanel::DrawPin(wxDC& dc, const Pin& pin, const wxPoint& gatePos, bool isHovered) {
    wxPoint pinPos = pin.GetAbsolutePos(gatePos);

    wxColour pinColor;
    if (isHovered) {
        pinColor = wxColour(0, 255, 0);
    }
    else {
        pinColor = pin.isInput ? wxColour(0, 128, 0) : wxColour(128, 0, 0);
    }

    wxPen pinPen(pinColor, 2);
    dc.SetPen(pinPen);

    if (isHovered) {
        dc.SetBrush(wxBrush(pinColor, wxBRUSHSTYLE_SOLID));
    }
    else {
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
    }

    int circleSize = isHovered ? 6 : 4;
    dc.DrawCircle(pinPos, circleSize);

    wxFont smallFont(7, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(smallFont);
    dc.SetTextForeground(pinColor);

    wxString pinText = pin.name;
    wxSize textSize = dc.GetTextExtent(pinText);

    wxPoint textPos;
    if (pin.isInput) {
        textPos = wxPoint(pinPos.x - textSize.GetWidth() - 2, pinPos.y - textSize.GetHeight() / 2);
    }
    else {
        textPos = wxPoint(pinPos.x + 6, pinPos.y - textSize.GetHeight() / 2);
    }

    dc.DrawText(pinText, textPos);

    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetTextForeground(*wxBLACK);
    dc.SetFont(wxNullFont);
}

// 断点绘制方法 - 优化版本
void MyDrawPanel::DrawBreakpoint(wxDC& dc, const Breakpoint& breakpoint, bool isSelected) {
    wxPoint pos = breakpoint.position;

    if (isSelected) {
        dc.SetPen(wxPen(wxColour(255, 0, 0), 2));
        dc.SetBrush(wxBrush(wxColour(255, 200, 200)));
    }
    else {
        dc.SetPen(wxPen(wxColour(0, 0, 255), 2));
        dc.SetBrush(wxBrush(wxColour(200, 200, 255)));
    }

    // 绘制断点圆圈
    dc.DrawCircle(pos, 6);

    // 绘制内部小圆点
    if (isSelected) {
        dc.SetPen(wxPen(wxColour(255, 255, 255), 1));
        dc.SetBrush(wxBrush(wxColour(255, 255, 255)));
    }
    else {
        dc.SetPen(wxPen(wxColour(0, 0, 0), 1));
        dc.SetBrush(wxBrush(wxColour(0, 0, 0)));
    }
    dc.DrawCircle(pos, 2);

    // 绘制断点ID（小型文本）
    wxFont smallFont(6, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(smallFont);
    dc.SetTextForeground(isSelected ? wxColour(255, 255, 255) : wxColour(0, 0, 0));
    wxSize textSize = dc.GetTextExtent(breakpoint.breakpointId);
    dc.DrawText(breakpoint.breakpointId, pos.x - textSize.GetWidth() / 2, pos.y - 15);

    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetTextForeground(*wxBLACK);
}

// 直角连线绘制方法
void MyDrawPanel::DrawOrthogonalWire(wxGraphicsContext* gc, const Wire& wire) {
    DrawOrthogonalWire(gc, wire.start, wire.end);
}

void MyDrawPanel::DrawOrthogonalWire(wxGraphicsContext* gc, const wxPoint& start, const wxPoint& end) {
    // 计算中间拐点
    wxPoint mid1, mid2;

    // 简单的直角连线算法：先水平后垂直
    int midX = start.x + (end.x - start.x) / 2;
    mid1 = wxPoint(midX, start.y);
    mid2 = wxPoint(midX, end.y);

    // 绘制三段线：起点->中间点1->中间点2->终点
    gc->StrokeLine(start.x, start.y, mid1.x, mid1.y);
    gc->StrokeLine(mid1.x, mid1.y, mid2.x, mid2.y);
    gc->StrokeLine(mid2.x, mid2.y, end.x, end.y);
}

void MyDrawPanel::DrawGrid(wxDC& dc) {
    if (!m_showGrid) return;

    wxSize size = GetClientSize();

    wxPen gridPen(wxColour(240, 240, 240), 1);
    dc.SetPen(gridPen);

    int startX = 0;
    int startY = 0;
    int endX = (int)(size.x / m_scale);
    int endY = (int)(size.y / m_scale);

    for (int x = startX; x < endX; x += m_gridSize) {
        dc.DrawLine(x, startY, x, endY);
    }
    for (int y = startY; y < endY; y += m_gridSize) {
        dc.DrawLine(startX, y, endX, y);
    }

    wxPen boldGridPen(wxColour(220, 220, 220), 1);
    dc.SetPen(boldGridPen);

    for (int x = startX; x < endX; x += m_gridSize * 5) {
        dc.DrawLine(x, startY, x, endY);
    }
    for (int y = startY; y < endY; y += m_gridSize * 5) {
        dc.DrawLine(startX, y, endX, y);
    }

    dc.SetPen(*wxBLACK_PEN);
}

wxRect MyDrawPanel::GetGateBBox(const Gate& g) const {
    if (g.type == "AND") {
        return wxRect(g.pos.x - 15, g.pos.y, 100, 60);
    }
    else if (g.type == "NAND") {
        return wxRect(g.pos.x - 15, g.pos.y, 110, 60);
    }
    else if (g.type == "OR" || g.type == "NOR") {
        return wxRect(g.pos.x - 20, g.pos.y, 85, 60);
    }
    else if (g.type == "XOR" || g.type == "XNOR") {
        return wxRect(g.pos.x - 25, g.pos.y, 90, 60);
    }
    else if (g.type == "NOT" || g.type == "BUFFER") {
        return wxRect(g.pos.x - 15, g.pos.y, 90, 60);
    }
    else if (g.type == "LED") {
        return wxRect(g.pos.x, g.pos.y, 60, 60);
    }
    else if (g.type == "开关") {
        return wxRect(g.pos.x - 5, g.pos.y - 30, 35, 35);
    }
    return wxRect(g.pos.x, g.pos.y, 80, 60);
}

// 辅助函数：计算点到直线段的距离（不递归）
bool MyDrawPanel::IsPointNearStraightLine(const wxPoint& point, const wxPoint& lineStart,
    const wxPoint& lineEnd, int tolerance) {
    wxPoint p = point;
    wxPoint a = lineStart;
    wxPoint b = lineEnd;

    double length2 = (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
    if (length2 == 0) {
        return (p.x - a.x) * (p.x - a.x) + (p.y - a.y) * (p.y - a.y) <= tolerance * tolerance;
    }

    double t = std::max(0.0, std::min(1.0,
        ((p.x - a.x) * (b.x - a.x) + (p.y - a.y) * (b.y - a.y)) / length2));

    wxPoint projection(
        a.x + t * (b.x - a.x),
        a.y + t * (b.y - a.y)
    );

    double distance2 = (p.x - projection.x) * (p.x - projection.x) +
        (p.y - projection.y) * (p.y - projection.y);

    return distance2 <= tolerance * tolerance;
}

bool MyDrawPanel::IsPointNearLine(const wxPoint& point, const Wire& wire, int tolerance) {
    if (m_wireMode == WIRE_STRAIGHT) {
        // 直线模式：直接检测
        return IsPointNearStraightLine(point, wire.start, wire.end, tolerance);
    }
    else {
        // 直角模式：检测三段直线
        int midX = wire.start.x + (wire.end.x - wire.start.x) / 2;
        wxPoint mid1 = wxPoint(midX, wire.start.y);
        wxPoint mid2 = wxPoint(midX, wire.end.y);

        // 使用辅助函数检测每段，避免递归导致栈溢出
        return IsPointNearStraightLine(point, wire.start, mid1, tolerance) ||
            IsPointNearStraightLine(point, mid1, mid2, tolerance) ||
            IsPointNearStraightLine(point, mid2, wire.end, tolerance);
    }
}

// 从断点查找引脚的方法
MyDrawPanel::PinConnection MyDrawPanel::FindNearestBreakpointPin(const wxPoint& point, int tolerance) {
    PinConnection nearestPin = { -1, -1, wxPoint() };
    double minDistance = tolerance * tolerance;

    for (size_t i = 0; i < m_breakpoints.size(); ++i) {
        const Breakpoint& bp = m_breakpoints[i];
        double distance2 = (point.x - bp.position.x) * (point.x - bp.position.x) +
            (point.y - bp.position.y) * (point.y - bp.position.y);

        if (distance2 < minDistance) {
            minDistance = distance2;
            nearestPin.gateIndex = -1; // 使用-1表示断点
            nearestPin.pinIndex = (int)i;
            nearestPin.position = bp.position;
        }
    }

    return nearestPin;
}

// 修改现有的 FindNearestPin 方法，包含断点引脚
MyDrawPanel::PinConnection MyDrawPanel::FindNearestPin(const wxPoint& point, int tolerance) {
    // 首先查找普通组件引脚
    PinConnection nearestPin = { -1, -1, wxPoint() };
    double minDistance = tolerance * tolerance;

    // 查找组件引脚
    for (size_t i = 0; i < m_gates.size(); ++i) {
        const auto& gate = m_gates[i];
        for (size_t j = 0; j < gate.pins.size(); ++j) {
            const auto& pin = gate.pins[j];
            wxPoint pinPos = pin.GetAbsolutePos(gate.pos);

            double distance2 = (point.x - pinPos.x) * (point.x - pinPos.x) +
                (point.y - pinPos.y) * (point.y - pinPos.y);

            if (distance2 < minDistance) {
                minDistance = distance2;
                nearestPin.gateIndex = (int)i;
                nearestPin.pinIndex = (int)j;
                nearestPin.position = pinPos;
            }
        }
    }

    // 查找断点引脚
    PinConnection breakpointPin = FindNearestBreakpointPin(point, tolerance);
    if (breakpointPin.pinIndex != -1) {
        double breakpointDistance = (point.x - breakpointPin.position.x) * (point.x - breakpointPin.position.x) +
            (point.y - breakpointPin.position.y) * (point.y - breakpointPin.position.y);
        if (breakpointDistance < minDistance) {
            return breakpointPin;
        }
    }

    return nearestPin;
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

wxPoint MyDrawPanel::SnapToPin(const wxPoint& point) {
    PinConnection nearestPin = FindNearestPin(point, 10);
    if (nearestPin.gateIndex != -1 || nearestPin.pinIndex != -1) {
        return nearestPin.position;
    }
    return point;
}

// 新增：计算点到直线段的最近点
wxPoint MyDrawPanel::FindClosestPointOnLine(const wxPoint& lineStart, const wxPoint& lineEnd, const wxPoint& point) {
    wxPoint lineVec(lineEnd.x - lineStart.x, lineEnd.y - lineStart.y);
    double lineLength2 = lineVec.x * lineVec.x + lineVec.y * lineVec.y;

    if (lineLength2 == 0) {
        return lineStart; // 线段长度为0
    }

    // 计算投影参数
    double t = ((point.x - lineStart.x) * lineVec.x + (point.y - lineStart.y) * lineVec.y) / lineLength2;
    t = std::max(0.0, std::min(1.0, t)); // 限制在线段范围内

    return wxPoint(
        lineStart.x + t * lineVec.x,
        lineStart.y + t * lineVec.y
    );
}

// 新增：计算点在直线段上的参数位置
double MyDrawPanel::CalculateParameterOnLine(const wxPoint& lineStart, const wxPoint& lineEnd, const wxPoint& pointOnLine) {
    if (lineStart.x == lineEnd.x) {
        // 垂直线段
        if (lineEnd.y == lineStart.y) return 0.0;
        return double(pointOnLine.y - lineStart.y) / (lineEnd.y - lineStart.y);
    }
    else {
        // 水平或其他线段
        if (lineEnd.x == lineStart.x) return 0.0;
        return double(pointOnLine.x - lineStart.x) / (lineEnd.x - lineStart.x);
    }
}

// 新增：计算点到直角连线的最近点
wxPoint MyDrawPanel::FindClosestPointOnOrthogonalLine(const Wire& wire, const wxPoint& point, double& outT) {
    // 计算直角连线的三个线段
    int midX = wire.start.x + (wire.end.x - wire.start.x) / 2;
    wxPoint mid1(midX, wire.start.y);
    wxPoint mid2(midX, wire.end.y);

    // 分别计算到三个线段的最近点
    wxPoint closestPoint1 = FindClosestPointOnLine(wire.start, mid1, point);
    wxPoint closestPoint2 = FindClosestPointOnLine(mid1, mid2, point);
    wxPoint closestPoint3 = FindClosestPointOnLine(mid2, wire.end, point);

    // 计算距离
    double dist1 = CalculateDistance(point, closestPoint1);
    double dist2 = CalculateDistance(point, closestPoint2);
    double dist3 = CalculateDistance(point, closestPoint3);

    // 找到最近的点
    if (dist1 <= dist2 && dist1 <= dist3) {
        // 在第一段线段上
        outT = CalculateParameterOnLine(wire.start, mid1, closestPoint1) * 0.333;
        return closestPoint1;
    }
    else if (dist2 <= dist1 && dist2 <= dist3) {
        // 在第二段线段上
        outT = 0.333 + CalculateParameterOnLine(mid1, mid2, closestPoint2) * 0.333;
        return closestPoint2;
    }
    else {
        // 在第三段线段上
        outT = 0.666 + CalculateParameterOnLine(mid2, wire.end, closestPoint3) * 0.334;
        return closestPoint3;
    }
}

// 新增：计算两点间距离
double MyDrawPanel::CalculateDistance(const wxPoint& p1, const wxPoint& p2) {
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

// 新增：更新连接到断点的所有连线
void MyDrawPanel::UpdateWiresFromBreakpoint(int breakpointIndex) {
    if (breakpointIndex < 0 || breakpointIndex >= (int)m_breakpoints.size()) {
        return;
    }

    const Breakpoint& bp = m_breakpoints[breakpointIndex];

    for (int wireIndex : bp.connectedWires) {
        if (wireIndex >= 0 && wireIndex < (int)m_wires.size()) {
            Wire& wire = m_wires[wireIndex];

            // 更新起点或终点为当前断点位置
            if (wire.startCompId == bp.breakpointId) {
                wire.start = bp.position;
            }
            if (wire.endCompId == bp.breakpointId) {
                wire.end = bp.position;
            }
        }
    }
    Refresh();
}

// 连线跟随移动方法
void MyDrawPanel::UpdateConnectedWires(int gateIndex) {
    // 添加递归保护
    static bool isUpdating = false;
    if (isUpdating) return;
    isUpdating = true;

    if (gateIndex < 0 || gateIndex >= (int)m_gates.size()) {
        isUpdating = false;
        return;
    }

    wxString compId = wxString::Format("U%d", gateIndex + 1);
    const Gate& movedGate = m_gates[gateIndex];

    // 更新所有连接到这个元件的连线
    for (auto& wire : m_wires) {
        if (wire.startCompId == compId) {
            // 找到对应的引脚并更新起点坐标
            for (const auto& pin : movedGate.pins) {
                if (pin.name == wire.startPinName) {
                    wire.start = pin.GetAbsolutePos(movedGate.pos);
                    break;
                }
            }
        }

        if (wire.endCompId == compId) {
            // 找到对应的引脚并更新终点坐标
            for (const auto& pin : movedGate.pins) {
                if (pin.name == wire.endPinName) {
                    wire.end = pin.GetAbsolutePos(movedGate.pos);
                    break;
                }
            }
        }
    }

    isUpdating = false;
}

// ---------- 事件处理 ----------
void MyDrawPanel::OnPaint(wxPaintEvent&) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    dc.SetUserScale(m_scale, m_scale);

    // 绘制网格
    DrawGrid(dc);

    // 创建支持抗锯齿的图形上下文
    wxGraphicsContext* gc = wxGraphicsContext::Create(dc);

    if (gc) {
        gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);

        // 绘制组件（使用普通DC，因为图形比较复杂）
        for (size_t i = 0; i < m_gates.size(); ++i) {
            auto& g = m_gates[i];
            DrawGate(dc, g);

            // 绘制选中状态
            if ((int)i == m_selectedIndex) {
                wxRect r = GetGateBBox(g);
                dc.SetPen(wxPen(wxColour(0, 100, 200), 2));
                dc.SetBrush(*wxTRANSPARENT_BRUSH);
                dc.DrawRectangle(r);
                dc.SetPen(*wxBLACK_PEN);
            }
        }

        // 使用图形上下文绘制连线（抗锯齿）
        for (size_t i = 0; i < m_wires.size(); ++i) {
            auto& w = m_wires[i];

            wxColour wireColor = *wxBLACK;
            double width = 1.5;

            if (!w.startCompId.empty() && !w.endCompId.empty()) {
                wireColor = wxColour(0, 100, 0);
                width = 2.0;
            }
            else if ((int)i == m_selectedWireIndex) {
                wireColor = wxColour(255, 0, 0);
                width = 3.0;
            }

            wxGraphicsPen pen = gc->CreatePen(wxGraphicsPenInfo(wireColor).Width(width));
            gc->SetPen(pen);

            // 根据连线模式绘制
            if (m_wireMode == WIRE_STRAIGHT) {
                // 直线模式
                gc->StrokeLine(w.start.x, w.start.y, w.end.x, w.end.y);
            }
            else {
                // 直角模式
                DrawOrthogonalWire(gc, w);
            }
        }

        // 绘制断点
        for (size_t i = 0; i < m_breakpoints.size(); ++i) {
            DrawBreakpoint(dc, m_breakpoints[i], (int)i == m_selectedBreakpointIndex);
        }

        // 绘制正在画的线
        if (m_isDrawingWire) {
            double width = (m_connectedEndPin.gateIndex != -1 || m_connectedEndPin.pinIndex != -1) ? 3.0 : 2.0;
            wxGraphicsPen pen = gc->CreatePen(wxGraphicsPenInfo(wxColour(0, 0, 255)).Width(width));
            gc->SetPen(pen);

            if (m_wireMode == WIRE_STRAIGHT) {
                // 直线模式
                gc->StrokeLine(m_wireStart.x, m_wireStart.y, m_currentMouse.x, m_currentMouse.y);
            }
            else {
                // 直角模式
                DrawOrthogonalWire(gc, m_wireStart, m_currentMouse);
            }
        }

        delete gc;
    }
    else {
        // 回退到普通DC绘制
        wxPen redPen(wxColour(200, 0, 0), 2);
        wxPen selectionPen(wxColour(0, 100, 200), 2);
        wxPen wireSelectionPen(wxColour(255, 0, 0), 3);
        wxPen connectedWirePen(wxColour(0, 100, 0), 2);
        wxPen drawingWirePen(wxColour(0, 0, 255), 2);

        // 绘制组件
        for (size_t i = 0; i < m_gates.size(); ++i) {
            auto& g = m_gates[i];
            DrawGate(dc, g);

            if ((int)i == m_selectedIndex) {
                wxRect r = GetGateBBox(g);
                dc.SetPen(selectionPen);
                dc.SetBrush(*wxTRANSPARENT_BRUSH);
                dc.DrawRectangle(r);
                dc.SetPen(*wxBLACK_PEN);
            }
        }

        // 绘制连线 - 回退模式下只支持直线模式
        for (size_t i = 0; i < m_wires.size(); ++i) {
            auto& w = m_wires[i];

            if (!w.startCompId.empty() && !w.endCompId.empty()) {
                dc.SetPen(connectedWirePen);
            }
            else if ((int)i == m_selectedWireIndex) {
                dc.SetPen(wireSelectionPen);
            }
            else {
                dc.SetPen(*wxBLACK_PEN);
            }

            dc.DrawLine(w.start, w.end);
        }

        // 绘制断点
        for (size_t i = 0; i < m_breakpoints.size(); ++i) {
            DrawBreakpoint(dc, m_breakpoints[i], (int)i == m_selectedBreakpointIndex);
        }

        // 绘制正在画的线
        if (m_isDrawingWire) {
            dc.SetPen(drawingWirePen);
            dc.DrawLine(m_wireStart, m_currentMouse);
        }
    }

    dc.SetPen(*wxBLACK_PEN);
}

void MyDrawPanel::OnRightClick(wxMouseEvent& evt) {
    wxPoint pos = ToLogical(evt.GetPosition());

    // 首先检查是否点击了断点
    int hitBreakpointIndex = FindBreakpointAtPoint(pos);
    if (hitBreakpointIndex != -1) {
        m_selectedBreakpointIndex = hitBreakpointIndex;
        m_selectedWireIndex = -1;
        m_selectedIndex = -1;

        wxMenu menu;
        menu.Append(ID_DELETE_BREAKPOINT, "删除断点");

        // 绑定删除断点事件
        menu.Bind(wxEVT_MENU, [this](wxCommandEvent&) {
            DeleteSelectedBreakpoint();
            }, ID_DELETE_BREAKPOINT);

        PopupMenu(&menu);
        return;
    }

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
        m_selectedBreakpointIndex = -1;

        wxMenu menu;
        menu.Append(ID_ADD_BREAKPOINT, "添加断点");
        menu.AppendSeparator();
        menu.Append(ID_DELETE_WIRE, "删除线条");

        // 绑定添加断点事件 - 使用正确的捕获方式
        menu.Bind(wxEVT_MENU, [this, pos, hitWireIndex](wxCommandEvent&) {
            AddBreakpoint(pos, hitWireIndex);
            }, ID_ADD_BREAKPOINT);

        // 绑定删除连线事件
        menu.Bind(wxEVT_MENU, [this](wxCommandEvent&) {
            DeleteSelectedWire();
            }, ID_DELETE_WIRE);

        PopupMenu(&menu);
        return;
    }

    int hitIndex = -1;
    for (int i = (int)m_gates.size() - 1; i >= 0; --i) {
        if (GetGateBBox(m_gates[i]).Contains(pos)) {
            hitIndex = i;
            break;
        }
    }

    m_selectedIndex = hitIndex;
    m_selectedWireIndex = -1;
    m_selectedBreakpointIndex = -1;
    Refresh();

    if (hitIndex != -1) {
        wxMenu menu;
        menu.Append(ID_EDIT_PROPERTIES, "编辑属性");
        menu.AppendSeparator();
        menu.Append(ID_DELETE_SELECTED, "删除组件");
        menu.AppendCheckItem(ID_SHOW_PINS, "显示引脚")->Check(m_showPins);

        // 绑定组件相关事件
        menu.Bind(wxEVT_MENU, [this](wxCommandEvent&) {
            EditSelectedProperties();
            }, ID_EDIT_PROPERTIES);

        menu.Bind(wxEVT_MENU, [this](wxCommandEvent&) {
            DeleteSelected();
            }, ID_DELETE_SELECTED);

        menu.Bind(wxEVT_MENU, [this](wxCommandEvent&) {
            TogglePins();
            }, ID_SHOW_PINS);

        PopupMenu(&menu);
    }
}
void MyDrawPanel::OnMouseMove(wxMouseEvent& evt) {
    wxPoint pos = ToLogical(evt.GetPosition());

    // 断点拖动逻辑
    if (m_isDraggingBreakpoint && evt.Dragging() && evt.LeftIsDown()) {
        if (m_draggedBreakpointIndex >= 0 && m_draggedBreakpointIndex < (int)m_breakpoints.size()) {
            wxPoint newPos = pos;

            if (m_showGrid) {
                newPos = SnapToGrid(newPos);
            }

            // 更新断点位置
            m_breakpoints[m_draggedBreakpointIndex].position = newPos;

            // 立即更新连接到这个断点的所有连线
            UpdateWiresFromBreakpoint(m_draggedBreakpointIndex);

            Refresh();
        }
    }
    else if (m_isDrawingWire || (!m_isDraggingGate && !m_isDrawingWire && !m_isDraggingBreakpoint)) {
        PinConnection nearestPin = FindNearestPin(pos, 10);
        if (nearestPin.gateIndex != -1 || nearestPin.pinIndex != -1) {
            m_hoveredPin = nearestPin;

            if (m_isDrawingWire) {
                if (nearestPin.gateIndex != m_connectedStartPin.gateIndex ||
                    nearestPin.pinIndex != m_connectedStartPin.pinIndex) {
                    SetCursor(wxCursor(wxCURSOR_HAND));
                }
                else {
                    SetCursor(wxCursor(wxCURSOR_NO_ENTRY));
                }
            }
            else {
                SetCursor(wxCursor(wxCURSOR_HAND));
            }
        }
        else {
            m_hoveredPin = { -1, -1, wxPoint() };
            if (!m_isDrawingWire && !m_isDraggingGate && !m_isDraggingBreakpoint) {
                SetCursor(wxCursor(wxCURSOR_ARROW));
            }
            else if (m_isDrawingWire) {
                SetCursor(wxCursor(wxCURSOR_CROSS));
            }
        }
    }

    if (m_isDrawingWire) {
        PinConnection nearestPin = FindNearestPin(pos, 10);
        if ((nearestPin.gateIndex != -1 || nearestPin.pinIndex != -1) &&
            (nearestPin.gateIndex != m_connectedStartPin.gateIndex ||
                nearestPin.pinIndex != m_connectedStartPin.pinIndex)) {
            m_currentMouse = nearestPin.position;
            m_connectedEndPin = nearestPin;
        }
        else {
            // 尝试吸附到断点
            wxPoint snappedPos = SnapToBreakpoint(pos);
            if (snappedPos != pos) {
                m_currentMouse = snappedPos;
                m_connectedEndPin = { -1, -1, wxPoint() };
            }
            else {
                if (m_showGrid) {
                    m_currentMouse = SnapToGrid(pos);
                }
                else {
                    m_currentMouse = pos;
                }
                m_connectedEndPin = { -1, -1, wxPoint() };
            }
        }
        Refresh();
    }
    else if (m_isDraggingGate && evt.Dragging() && evt.LeftIsDown()) {
        if (m_draggedIndex >= 0 && m_draggedIndex < (int)m_gates.size()) {
            wxPoint newPos = wxPoint(pos.x - m_dragOffset.x, pos.y - m_dragOffset.y);

            if (m_showGrid) {
                newPos = SnapToGrid(newPos);
            }

            // 更新元件位置
            m_gates[m_draggedIndex].pos = newPos;

            // 更新连接到这个元件的所有连线
            UpdateConnectedWires(m_draggedIndex);

            Refresh();
        }
    }
}

void MyDrawPanel::OnMouseUp(wxMouseEvent&) {
    if (m_isDraggingBreakpoint) {
        m_isDraggingBreakpoint = false;
        m_draggedBreakpointIndex = -1;
        if (HasCapture()) ReleaseMouse();
        SetCursor(wxCursor(wxCURSOR_ARROW));

        // 更新连接到这个断点的所有连线
        if (m_draggedBreakpointIndex >= 0) {
            UpdateWiresFromBreakpoint(m_draggedBreakpointIndex);
        }
    }
    else if (m_isDraggingGate) {
        // 确保连线位置最终更新
        if (m_draggedIndex >= 0 && m_draggedIndex < (int)m_gates.size()) {
            UpdateConnectedWires(m_draggedIndex);
        }

        m_isDraggingGate = false;
        m_draggedIndex = -1;
        if (HasCapture()) ReleaseMouse();
        SetCursor(wxCursor(wxCURSOR_ARROW));
    }
    else if (m_isDrawingWire) {
        if (m_connectedEndPin.gateIndex != -1 || m_connectedEndPin.pinIndex != -1) {
            Wire newWire;
            newWire.start = m_wireStart;
            newWire.end = m_currentMouse;

            // 处理起始连接
            if (m_connectedStartPin.gateIndex != -1) {
                // 连接到组件引脚
                const auto& startGate = m_gates[m_connectedStartPin.gateIndex];
                const auto& startPin = startGate.pins[m_connectedStartPin.pinIndex];
                newWire.startCompId = wxString::Format("U%d", m_connectedStartPin.gateIndex + 1);
                newWire.startPinName = startPin.name;
            }
            else if (m_connectedStartPin.gateIndex == -1 && m_connectedStartPin.pinIndex != -1) {
                // 连接到断点
                const auto& startBreakpoint = m_breakpoints[m_connectedStartPin.pinIndex];
                newWire.startCompId = startBreakpoint.breakpointId;
                newWire.startPinName = "breakpoint";
                newWire.isConnectedToBreakpoint = true;
            }

            // 处理结束连接
            if (m_connectedEndPin.gateIndex != -1) {
                // 连接到组件引脚
                const auto& endGate = m_gates[m_connectedEndPin.gateIndex];
                const auto& endPin = endGate.pins[m_connectedEndPin.pinIndex];
                newWire.endCompId = wxString::Format("U%d", m_connectedEndPin.gateIndex + 1);
                newWire.endPinName = endPin.name;
            }
            else if (m_connectedEndPin.gateIndex == -1 && m_connectedEndPin.pinIndex != -1) {
                // 连接到断点
                const auto& endBreakpoint = m_breakpoints[m_connectedEndPin.pinIndex];
                newWire.endCompId = endBreakpoint.breakpointId;
                newWire.endPinName = "breakpoint";
                newWire.isConnectedToBreakpoint = true;
            }

            m_wires.push_back(newWire);

            // 记录连线连接到断点
            if (m_connectedStartPin.gateIndex == -1 && m_connectedStartPin.pinIndex != -1) {
                m_breakpoints[m_connectedStartPin.pinIndex].connectedWires.push_back((int)m_wires.size() - 1);
            }
            if (m_connectedEndPin.gateIndex == -1 && m_connectedEndPin.pinIndex != -1) {
                m_breakpoints[m_connectedEndPin.pinIndex].connectedWires.push_back((int)m_wires.size() - 1);
            }
        }
        else {
            // 检查是否连接到断点
            int breakpointIndex = FindBreakpointAtPoint(m_currentMouse, 8);
            if (breakpointIndex != -1) {
                Wire newWire;
                newWire.start = m_wireStart;
                newWire.end = m_breakpoints[breakpointIndex].position;
                newWire.endCompId = m_breakpoints[breakpointIndex].breakpointId;
                newWire.endPinName = "breakpoint";
                newWire.isConnectedToBreakpoint = true;
                m_wires.push_back(newWire);

                // 记录连线连接到断点
                m_breakpoints[breakpointIndex].connectedWires.push_back((int)m_wires.size() - 1);
            }
        }

        m_isDrawingWire = false;
        m_connectedStartPin = { -1, -1, wxPoint() };
        m_connectedEndPin = { -1, -1, wxPoint() };
        if (HasCapture()) ReleaseMouse();
        SetCursor(wxCursor(wxCURSOR_ARROW));
    }
}

void MyDrawPanel::OnMouseDown(wxMouseEvent& evt) {
    wxPoint pos = ToLogical(evt.GetPosition());

    // 首先检查是否点击了断点
    int breakpointIndex = FindBreakpointAtPoint(pos);
    if (breakpointIndex != -1) {
        if (evt.LeftDown()) {
            m_isDraggingBreakpoint = true;
            m_draggedBreakpointIndex = breakpointIndex;
            m_selectedBreakpointIndex = breakpointIndex;
            m_selectedIndex = -1;
            m_selectedWireIndex = -1;
            if (!HasCapture()) CaptureMouse();
            SetCursor(wxCursor(wxCURSOR_SIZING));
        }
        Refresh();
        return;
    }

    PinConnection nearestPin = FindNearestPin(pos, 8);
    if (nearestPin.gateIndex != -1 || nearestPin.pinIndex != -1) {
        pos = nearestPin.position;
    }
    else if (m_showGrid) {
        pos = SnapToGrid(pos);
    }
    else {
        // 尝试吸附到断点
        pos = SnapToBreakpoint(pos);
    }

    m_selectedWireIndex = -1;
    for (int i = (int)m_wires.size() - 1; i >= 0; --i) {
        if (IsPointNearLine(pos, m_wires[i])) {
            m_selectedWireIndex = i;
            m_selectedIndex = -1;
            m_selectedBreakpointIndex = -1;
            Refresh();
            return;
        }
    }

    if (nearestPin.gateIndex != -1 || nearestPin.pinIndex != -1) {
        if (evt.LeftDown()) {
            m_isDrawingWire = true;
            m_wireStart = nearestPin.position;
            m_currentMouse = nearestPin.position;
            m_connectedStartPin = nearestPin;
            m_selectedIndex = -1;
            m_selectedWireIndex = -1;
            m_selectedBreakpointIndex = -1;
            if (!HasCapture()) CaptureMouse();
            SetCursor(wxCursor(wxCURSOR_CROSS));
        }
        Refresh();
        return;
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
            m_selectedWireIndex = -1;
            m_selectedBreakpointIndex = -1;
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
            m_selectedBreakpointIndex = -1;
            if (!HasCapture()) CaptureMouse();
            SetCursor(wxCursor(wxCURSOR_CROSS));
        }
    }
    Refresh();
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
        else if (m_selectedBreakpointIndex >= 0) {
            DeleteSelectedBreakpoint();
        }
        break;
    case 'P':
    case 'p':
        if (evt.ControlDown()) {
            EditSelectedProperties();
        }
        break;
    case 'G':
    case 'g':
        if (evt.ControlDown()) {
            ToggleGrid();
        }
        break;
    case 'I':
    case 'i':
        if (evt.ControlDown()) {
            TogglePins();
        }
        break;
    case 'W':
    case 'w':
        if (evt.ControlDown()) {
            ToggleWireMode();
        }
        break;
    case 'B':
    case 'b':
        if (evt.ControlDown()) {
            // 在选中的连线上添加断点
            if (m_selectedWireIndex >= 0) {
                wxPoint pos = m_wires[m_selectedWireIndex].start;
                AddBreakpoint(pos, m_selectedWireIndex);
            }
        }
        break;
    case 'D':
    case 'd':
        if (evt.ControlDown()) {
            DeleteSelectedBreakpoint();
        }
        break;
    default:
        evt.Skip();
        break;
    }
}