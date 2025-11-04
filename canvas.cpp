#include "canvas.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <cmath>
#include <set>

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
    // 确保组件位置在网格上
    wxPoint gridPos = SnapToGrid(wxPoint(50 + (m_gates.size() % 3) * 150, 50 + (m_gates.size() / 3) * 150));
    newGate.pos = gridPos;

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

    // 设置引脚
    auto pinIt = pinDefinitions.find(shape);
    if (pinIt != pinDefinitions.end()) {
        newGate.pins = pinIt->second;
    }

    m_gates.push_back(newGate);
    m_selectedIndex = (int)m_gates.size() - 1;
    m_selectedWireIndex = -1; // 取消线条选中
    Refresh();
}


void MyDrawPanel::RemoveLastShape() {
    if (!m_gates.empty()) {
        // 移除与该门相关的所有连线
        for (auto it = m_wires.begin(); it != m_wires.end(); ) {
            if (it->start.gateIndex == (int)m_gates.size() - 1 ||
                it->end.gateIndex == (int)m_gates.size() - 1) {
                it = m_wires.erase(it);
            }
            else {
                ++it;
            }
        }
        m_gates.pop_back();
        m_selectedIndex = -1;
    }
    Refresh();
}

void MyDrawPanel::DeleteSelected() {
    if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_gates.size()) {
        // 移除与该门相关的所有连线
        for (auto it = m_wires.begin(); it != m_wires.end(); ) {
            if (it->start.gateIndex == m_selectedIndex ||
                it->end.gateIndex == m_selectedIndex) {
                it = m_wires.erase(it);
            }
            else {
                // 更新连线索引
                if (it->start.gateIndex > m_selectedIndex) it->start.gateIndex--;
                if (it->end.gateIndex > m_selectedIndex) it->end.gateIndex--;
                ++it;
            }
        }
        m_gates.erase(m_gates.begin() + m_selectedIndex);
        m_selectedIndex = -1;
        Refresh();
    }
}

void MyDrawPanel::DeleteSelectedWire() {
    if (m_selectedWireIndex >= 0 && m_selectedWireIndex < (int)m_wires.size()) {
        // 更新引脚的连接状态
        Wire& wire = m_wires[m_selectedWireIndex];
        if (wire.start.gateIndex >= 0 && wire.start.gateIndex < (int)m_gates.size() &&
            wire.start.pinIndex >= 0 && wire.start.pinIndex < (int)m_gates[wire.start.gateIndex].pins.size()) {
            m_gates[wire.start.gateIndex].pins[wire.start.pinIndex].connected = false;
        }
        if (wire.end.gateIndex >= 0 && wire.end.gateIndex < (int)m_gates.size() &&
            wire.end.pinIndex >= 0 && wire.end.pinIndex < (int)m_gates[wire.end.gateIndex].pins.size()) {
            m_gates[wire.end.gateIndex].pins[wire.end.pinIndex].connected = false;
        }

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
    m_wires.clear();
    int i = 0;
    for (auto& s : shapes) {
        Gate newGate;
        newGate.type = s;
        wxPoint gridPos = SnapToGrid(wxPoint(50 + (i % 3) * 150, 50 + (i / 3) * 150));
        newGate.pos = gridPos;

        // 设置引脚
        auto pinIt = pinDefinitions.find(s);
        if (pinIt != pinDefinitions.end()) {
            newGate.pins = pinIt->second;
        }

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
    m_wires.clear(); // 清空连线，因为加载时只加载门信息
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

void MyDrawPanel::DrawPin(wxDC& dc, const Gate& gate, int pinIndex) {
    if (pinIndex < 0 || pinIndex >= (int)gate.pins.size()) return;

    wxPoint pinPos = GetPinPosition(gate, pinIndex);

    // 绘制引脚点
    dc.SetPen(*wxBLACK_PEN);
    if (gate.pins[pinIndex].connected) {
        dc.SetBrush(*wxGREEN_BRUSH); // 已连接的引脚显示为绿色
    }
    else {
        dc.SetBrush(*wxWHITE_BRUSH); // 未连接的引脚显示为白色
    }
    dc.DrawCircle(pinPos, 4); // 增大引脚点大小

    // 绘制引脚标签（在较大缩放级别时显示）
    if (m_scale > 0.8) {
        wxFont smallFont(7, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        dc.SetFont(smallFont);
        dc.SetTextForeground(wxColour(100, 100, 100));

        wxString label = gate.pins[pinIndex].name;
        wxSize textSize = dc.GetTextExtent(label);

        // 根据引脚类型调整标签位置
        if (gate.pins[pinIndex].type == PinType::Input) {
            dc.DrawText(label, pinPos.x - textSize.GetWidth() - 5, pinPos.y - textSize.GetHeight() / 2);
        }
        else {
            dc.DrawText(label, pinPos.x + 6, pinPos.y - textSize.GetHeight() / 2);
        }

        dc.SetFont(wxNullFont);
        dc.SetTextForeground(*wxBLACK);
    }
}

void MyDrawPanel::DrawWire(wxDC& dc, const Wire& wire) {
    if (wire.path.empty()) return;

    // 检查是否有相交的线段
    bool hasIntersection = CheckWireIntersections(wire);

    // 设置连线样式
    wxPen wirePen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID);
    if (wire.isSelected) {
        wirePen.SetColour(wxColour(255, 0, 0));
        wirePen.SetWidth(3);
    }
    else if (hasIntersection) {
        // 相交的线段用虚线显示
        wirePen.SetStyle(wxPENSTYLE_SHORT_DASH);
        wirePen.SetColour(wxColour(255, 100, 0)); // 橙色虚线表示相交
    }
    dc.SetPen(wirePen);

    // 绘制连线路径
    for (size_t i = 1; i < wire.path.size(); ++i) {
        dc.DrawLine(wire.path[i - 1], wire.path[i]);
    }

    // 绘制端点
    dc.SetBrush(*wxBLACK_BRUSH);
    if (!wire.path.empty()) {
        dc.DrawCircle(wire.path.front(), 3);
        dc.DrawCircle(wire.path.back(), 3);
    }

    // 在相交点绘制红色圆圈提示
    if (hasIntersection) {
        auto intersections = FindWireIntersectionPoints(wire);
        dc.SetPen(*wxRED_PEN);
        dc.SetBrush(*wxRED_BRUSH);
        for (const auto& point : intersections) {
            dc.DrawCircle(point, 4);
        }
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
    else if (g.type == "开关") { w = 60; h = 60; }
    return wxRect(g.pos.x - 10, g.pos.y - 10, w + 20, h + 20); // 扩大边界框便于选择
}

// ---------- 引脚和连线工具方法 ----------
bool MyDrawPanel::FindPinAtPosition(const wxPoint& pos, WireEndpoint& endpoint) {
    for (size_t i = 0; i < m_gates.size(); ++i) {
        for (size_t j = 0; j < m_gates[i].pins.size(); ++j) {
            if (IsPointNearPin(pos, m_gates[i], j)) {
                endpoint.gateIndex = (int)i;
                endpoint.pinIndex = (int)j;
                endpoint.position = GetPinPosition(m_gates[i], j);
                return true;
            }
        }
    }
    return false;
}

bool MyDrawPanel::IsPointNearPin(const wxPoint& point, const Gate& gate, int pinIndex) {
    if (pinIndex < 0 || pinIndex >= (int)gate.pins.size()) return false;

    wxPoint pinPos = GetPinPosition(gate, pinIndex);
    int dx = point.x - pinPos.x;
    int dy = point.y - pinPos.y;
    // 扩大吸附范围到12像素半径
    return (dx * dx + dy * dy) <= 144; // 12像素半径的平方
}

bool MyDrawPanel::IsPointNearWire(const wxPoint& point, const Wire& wire, int tolerance) {
    if (wire.path.size() < 2) return false;

    for (size_t i = 1; i < wire.path.size(); ++i) {
        wxPoint a = wire.path[i - 1];
        wxPoint b = wire.path[i];

        // 线段长度的平方
        double length2 = (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
        if (length2 == 0) {
            // 线段退化为点
            if ((point.x - a.x) * (point.x - a.x) + (point.y - a.y) * (point.y - a.y) <= tolerance * tolerance) {
                return true;
            }
            continue;
        }

        // 计算投影比例
        double t = std::max(0.0, std::min(1.0,
            ((point.x - a.x) * (b.x - a.x) + (point.y - a.y) * (b.y - a.y)) / length2));

        // 计算投影点
        wxPoint projection(
            a.x + t * (b.x - a.x),
            a.y + t * (b.y - a.y)
        );

        // 计算距离平方
        double distance2 = (point.x - projection.x) * (point.x - projection.x) +
            (point.y - projection.y) * (point.y - projection.y);

        if (distance2 <= tolerance * tolerance) {
            return true;
        }
    }
    return false;
}

// 检查两条线段是否相交
bool MyDrawPanel::DoLinesIntersect(const wxPoint& a1, const wxPoint& a2, const wxPoint& b1, const wxPoint& b2) {
    // 计算方向
    int dir1 = ((b1.x - a1.x) * (a2.y - a1.y) - (b1.y - a1.y) * (a2.x - a1.x));
    int dir2 = ((b2.x - a1.x) * (a2.y - a1.y) - (b2.y - a1.y) * (a2.x - a1.x));

    int dir3 = ((a1.x - b1.x) * (b2.y - b1.y) - (a1.y - b1.y) * (b2.x - b1.x));
    int dir4 = ((a2.x - b1.x) * (b2.y - b1.y) - (a2.y - b1.y) * (b2.x - b1.x));

    // 检查是否相交
    if (((dir1 > 0 && dir2 < 0) || (dir1 < 0 && dir2 > 0)) &&
        ((dir3 > 0 && dir4 < 0) || (dir3 < 0 && dir4 > 0))) {
        return true;
    }

    return false;
}

// 检查连线是否与其他连线相交
bool MyDrawPanel::CheckWireIntersections(const Wire& wire) {
    for (const auto& otherWire : m_wires) {
        if (&wire == &otherWire) continue; // 跳过自身

        for (size_t i = 1; i < wire.path.size(); ++i) {
            for (size_t j = 1; j < otherWire.path.size(); ++j) {
                if (DoLinesIntersect(wire.path[i - 1], wire.path[i],
                    otherWire.path[j - 1], otherWire.path[j])) {
                    return true;
                }
            }
        }
    }
    return false;
}

// 找到连线的所有相交点
std::vector<wxPoint> MyDrawPanel::FindWireIntersectionPoints(const Wire& wire) {
    std::vector<wxPoint> intersections;

    for (const auto& otherWire : m_wires) {
        if (&wire == &otherWire) continue; // 跳过自身

        for (size_t i = 1; i < wire.path.size(); ++i) {
            for (size_t j = 1; j < otherWire.path.size(); ++j) {
                wxPoint a1 = wire.path[i - 1];
                wxPoint a2 = wire.path[i];
                wxPoint b1 = otherWire.path[j - 1];
                wxPoint b2 = otherWire.path[j];

                if (DoLinesIntersect(a1, a2, b1, b2)) {
                    // 计算交点（简化版，实际应用中可能需要更精确的计算）
                    if (a1.x == a2.x && b1.y == b2.y) { // 垂直线段和水平线段
                        intersections.push_back(wxPoint(a1.x, b1.y));
                    }
                    else if (a1.y == a2.y && b1.x == b2.x) { // 水平线段和垂直线段
                        intersections.push_back(wxPoint(b1.x, a1.y));
                    }
                    else {
                        // 取中点作为近似交点
                        intersections.push_back(wxPoint((a1.x + a2.x + b1.x + b2.x) / 4,
                            (a1.y + a2.y + b1.y + b2.y) / 4));
                    }
                }
            }
        }
    }

    return intersections;
}

// 更新连线路径（当组件移动时调用）
void MyDrawPanel::UpdateWirePathsForGate(int gateIndex) {
    for (auto& wire : m_wires) {
        if (wire.start.gateIndex == gateIndex) {
            // 更新起点位置
            wire.start.position = GetPinPosition(m_gates[gateIndex], wire.start.pinIndex);
            // 重新计算路径
            wire.path = CalculateWirePath(wire.start, wire.end);
        }
        if (wire.end.gateIndex == gateIndex) {
            // 更新终点位置
            wire.end.position = GetPinPosition(m_gates[gateIndex], wire.end.pinIndex);
            // 重新计算路径
            wire.path = CalculateWirePath(wire.start, wire.end);
        }
    }
}

std::vector<wxPoint> MyDrawPanel::CalculateWirePath(const WireEndpoint& start, const WireEndpoint& end) {
    std::vector<wxPoint> path;
    path.push_back(start.position);

    // 改进的曼哈顿布线算法 - 确保所有路径点都在网格上
    int dx = end.position.x - start.position.x;
    int dy = end.position.y - start.position.y;

    // 强制所有坐标都在网格上
    wxPoint snappedStart = SnapToGrid(start.position);
    wxPoint snappedEnd = SnapToGrid(end.position);

    // 计算中间点，确保在网格上
    if (abs(dx) > abs(dy)) {
        // 水平方向变化更大，先水平后垂直
        wxPoint mid1 = SnapToGrid(wxPoint(snappedStart.x + dx / 2, snappedStart.y));
        wxPoint mid2 = SnapToGrid(wxPoint(snappedStart.x + dx / 2, snappedEnd.y));

        path.push_back(mid1);
        path.push_back(mid2);
    }
    else {
        // 垂直方向变化更大，先垂直后水平
        wxPoint mid1 = SnapToGrid(wxPoint(snappedStart.x, snappedStart.y + dy / 2));
        wxPoint mid2 = SnapToGrid(wxPoint(snappedEnd.x, snappedStart.y + dy / 2));

        path.push_back(mid1);
        path.push_back(mid2);
    }

    path.push_back(snappedEnd);

    // 移除重复的点
    std::vector<wxPoint> cleanedPath;
    for (size_t i = 0; i < path.size(); ++i) {
        if (i == 0 || path[i] != path[i - 1]) {
            cleanedPath.push_back(path[i]);
        }
    }

    return cleanedPath;
}

std::vector<wxPoint> MyDrawPanel::CalculateWirePath(const WireEndpoint& start, const wxPoint& end) {
    std::vector<wxPoint> path;
    path.push_back(start.position);

    int dx = end.x - start.position.x;
    int dy = end.y - start.position.y;

    // 强制所有坐标都在网格上
    wxPoint snappedStart = SnapToGrid(start.position);
    wxPoint snappedEnd = SnapToGrid(end);

    // 计算中间点，确保在网格上
    if (abs(dx) > abs(dy)) {
        // 水平方向变化更大，先水平后垂直
        wxPoint mid1 = SnapToGrid(wxPoint(snappedStart.x + dx / 2, snappedStart.y));
        wxPoint mid2 = SnapToGrid(wxPoint(snappedStart.x + dx / 2, snappedEnd.y));

        path.push_back(mid1);
        path.push_back(mid2);
    }
    else {
        // 垂直方向变化更大，先垂直后水平
        wxPoint mid1 = SnapToGrid(wxPoint(snappedStart.x, snappedStart.y + dy / 2));
        wxPoint mid2 = SnapToGrid(wxPoint(snappedEnd.x, snappedStart.y + dy / 2));

        path.push_back(mid1);
        path.push_back(mid2);
    }

    path.push_back(snappedEnd);

    // 移除重复的点
    std::vector<wxPoint> cleanedPath;
    for (size_t i = 0; i < path.size(); ++i) {
        if (i == 0 || path[i] != path[i - 1]) {
            cleanedPath.push_back(path[i]);
        }
    }

    return cleanedPath;
}

void MyDrawPanel::UpdatePinConnections() {
    // 重置所有引脚的连接状态
    for (auto& gate : m_gates) {
        for (auto& pin : gate.pins) {
            pin.connected = false;
        }
    }

    // 根据连线更新引脚连接状态
    for (auto& wire : m_wires) {
        if (wire.start.gateIndex >= 0 && wire.start.gateIndex < (int)m_gates.size() &&
            wire.start.pinIndex >= 0 && wire.start.pinIndex < (int)m_gates[wire.start.gateIndex].pins.size()) {
            m_gates[wire.start.gateIndex].pins[wire.start.pinIndex].connected = true;
        }
        if (wire.end.gateIndex >= 0 && wire.end.gateIndex < (int)m_gates.size() &&
            wire.end.pinIndex >= 0 && wire.end.pinIndex < (int)m_gates[wire.end.gateIndex].pins.size()) {
            m_gates[wire.end.gateIndex].pins[wire.end.pinIndex].connected = true;
        }
    }
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

    wxPen selectionPen(wxColour(0, 100, 200), 2, wxPENSTYLE_SOLID);
    wxPen dashPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SHORT_DASH);

    // 绘制连线
    for (size_t i = 0; i < m_wires.size(); ++i) {
        Wire wire = m_wires[i];
        wire.isSelected = ((int)i == m_selectedWireIndex);
        DrawWire(dc, wire);
    }

    // 绘制组件
    for (size_t i = 0; i < m_gates.size(); ++i) {
        auto& g = m_gates[i];
        DrawGate(dc, g);

        // 绘制引脚
        for (size_t j = 0; j < g.pins.size(); ++j) {
            DrawPin(dc, g, j);
        }

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

    // 绘制正在画的线
    if (m_isDrawingWire) {
        Wire tempWire;
        tempWire.start = m_wireStart;
        tempWire.path = m_currentPath;
        tempWire.isSelected = false;
        DrawWire(dc, tempWire);

        // 绘制吸附提示
        WireEndpoint endEndpoint;
        if (FindPinAtPosition(m_currentPath.back(), endEndpoint)) {
            dc.SetPen(*wxRED_PEN);
            dc.SetBrush(*wxRED_BRUSH);
            dc.DrawCircle(endEndpoint.position, 6);
        }
    }
}

void MyDrawPanel::OnMouseDown(wxMouseEvent& evt) {
    wxPoint pos = ToLogical(evt.GetPosition());
    if (m_showGrid) {
        pos = SnapToGrid(pos);
    }

    // 先检查是否点击了连线
    m_selectedWireIndex = -1;
    for (int i = (int)m_wires.size() - 1; i >= 0; --i) {
        if (IsPointNearWire(pos, m_wires[i], 8)) { // 扩大连线选择范围
            m_selectedWireIndex = i;
            m_selectedIndex = -1; // 取消组件选中
            Refresh();
            return;
        }
    }

    // 检查是否点击了引脚开始连线
    WireEndpoint endpoint;
    if (FindPinAtPosition(pos, endpoint)) {
        m_isDrawingWire = true;
        m_wireStart = endpoint;
        m_currentPath = CalculateWirePath(m_wireStart, pos);
        m_selectedIndex = -1;
        m_selectedWireIndex = -1;
        if (!HasCapture()) CaptureMouse();
        SetCursor(wxCursor(wxCURSOR_CROSS));
        Refresh();
        return;
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
            // 如果没有点击任何东西，开始自由绘制连线
            m_isDrawingWire = true;
            m_wireStart.gateIndex = -1;
            m_wireStart.pinIndex = -1;
            m_wireStart.position = pos;
            m_currentPath = { pos, pos };
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

    // 检查是否右键点击了连线
    int hitWireIndex = -1;
    for (int i = (int)m_wires.size() - 1; i >= 0; --i) {
        if (IsPointNearWire(pos, m_wires[i], 8)) {
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
            // 更新组件位置（确保在网格上）
            wxPoint newPos = wxPoint(pos.x - m_dragOffset.x, pos.y - m_dragOffset.y);
            if (m_showGrid) {
                newPos = SnapToGrid(newPos);
            }
            m_gates[m_draggedIndex].pos = newPos;

            // 更新与该组件相关的所有连线路径
            UpdateWirePathsForGate(m_draggedIndex);

            Refresh();
        }
    }
    else if (m_isDrawingWire && evt.Dragging() && evt.LeftIsDown()) {
        // 检查是否有引脚可以吸附
        WireEndpoint endEndpoint;
        if (FindPinAtPosition(pos, endEndpoint)) {
            // 吸附到引脚
            m_currentPath = CalculateWirePath(m_wireStart, endEndpoint);
        }
        else {
            // 自由绘制，但确保路径点在网格上
            wxPoint snappedPos = m_showGrid ? SnapToGrid(pos) : pos;
            m_currentPath = CalculateWirePath(m_wireStart, snappedPos);
        }
        Refresh();
    }
}

void MyDrawPanel::OnMouseUp(wxMouseEvent&) {
    if (m_isDraggingGate) {
        m_isDraggingGate = false;
        m_draggedIndex = -1;
        if (HasCapture()) ReleaseMouse();
        SetCursor(wxCursor(wxCURSOR_ARROW));

        // 发送自定义事件通知主窗口保存状态
        wxCommandEvent evt(MY_CUSTOM_EVENT);
        evt.SetEventObject(this);
        wxPostEvent(GetParent(), evt);

        Refresh();
    }
    else if (m_isDrawingWire) {
        // 查找结束引脚
        WireEndpoint endEndpoint;
        if (FindPinAtPosition(m_currentPath.back(), endEndpoint)) {
            // 创建完整的连线
            Wire newWire;
            newWire.start = m_wireStart;
            newWire.end = endEndpoint;
            newWire.path = CalculateWirePath(m_wireStart, endEndpoint);

            // 检查连线是否有效
            bool isValid = false;

            if (m_wireStart.gateIndex >= 0 && m_wireStart.gateIndex < (int)m_gates.size() &&
                endEndpoint.gateIndex >= 0 && endEndpoint.gateIndex < (int)m_gates.size()) {

                Pin& startPin = m_gates[m_wireStart.gateIndex].pins[m_wireStart.pinIndex];
                Pin& endPin = m_gates[endEndpoint.gateIndex].pins[endEndpoint.pinIndex];

                // 允许输出连输入，或者输入连输出
                if ((startPin.type == PinType::Output && endPin.type == PinType::Input) ||
                    (startPin.type == PinType::Input && endPin.type == PinType::Output)) {
                    isValid = true;
                }
            }
            else {
                // 如果有一个端点不是引脚，也允许连接（用于调试）
                isValid = true;
            }

            if (isValid) {
                // 标记引脚为已连接
                if (m_wireStart.gateIndex >= 0 && m_wireStart.gateIndex < (int)m_gates.size() &&
                    m_wireStart.pinIndex >= 0 && m_wireStart.pinIndex < (int)m_gates[m_wireStart.gateIndex].pins.size()) {
                    m_gates[m_wireStart.gateIndex].pins[m_wireStart.pinIndex].connected = true;
                }
                if (endEndpoint.gateIndex >= 0 && endEndpoint.gateIndex < (int)m_gates.size() &&
                    endEndpoint.pinIndex >= 0 && endEndpoint.pinIndex < (int)m_gates[endEndpoint.gateIndex].pins.size()) {
                    m_gates[endEndpoint.gateIndex].pins[endEndpoint.pinIndex].connected = true;
                }
                m_wires.push_back(newWire);

                // 发送自定义事件通知主窗口保存状态
                wxCommandEvent evt(MY_CUSTOM_EVENT);
                evt.SetEventObject(this);
                wxPostEvent(GetParent(), evt);
            }
        }

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
            // 发送自定义事件通知主窗口保存状态
            wxCommandEvent deleteEvt(MY_CUSTOM_EVENT);
            deleteEvt.SetEventObject(this);
            wxPostEvent(GetParent(), deleteEvt);
        }
        else if (m_selectedWireIndex >= 0) {
            DeleteSelectedWire();
            // 发送自定义事件通知主窗口保存状态
            wxCommandEvent deleteEvt(MY_CUSTOM_EVENT);
            deleteEvt.SetEventObject(this);
            wxPostEvent(GetParent(), deleteEvt);
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