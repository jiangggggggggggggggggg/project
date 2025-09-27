#include "logicgates.h"

// 初始化静态成员
int Component::m_nextId = 1;

// 基础元件类实现
Component::Component(wxPoint position, const wxString& name)
    : m_position(position), m_name(name), m_id(m_nextId++) {
}

wxPoint Component::GetPosition() const {
    return m_position;
}

void Component::SetPosition(wxPoint pos) {
    m_position = pos;
}

int Component::GetId() const {
    return m_id;
}

wxString Component::GetName() const {
    return m_name;
}

// 非门类实现
NotGate::NotGate(wxPoint position) : Component(position, "NOT Gate") {}

void NotGate::Draw(wxDC& dc) {
    // 绘制非门图形（简化版）
    wxPoint pos = GetPosition();

    // 绘制三角形
    wxPoint points[3] = {
        pos,
        wxPoint(pos.x + 40, pos.y + 20),
        pos + wxPoint(0, 40)
    };
    dc.DrawPolygon(3, points);

    // 绘制圆形（非门标志）
    dc.DrawCircle(pos.x + 40, pos.y + 20, 5);

    // 绘制输入输出线
    dc.DrawLine(pos.x - 10, pos.y + 20, pos.x, pos.y + 20);  // 输入线
    dc.DrawLine(pos.x + 45, pos.y + 20, pos.x + 60, pos.y + 20);  // 输出线
}

bool NotGate::ContainsPoint(wxPoint point) {
    wxPoint pos = GetPosition();
    // 简单的碰撞检测
    return (point.x >= pos.x && point.x <= pos.x + 40 &&
        point.y >= pos.y && point.y <= pos.y + 40);
}

std::vector<wxPoint> NotGate::GetInputPoints() {
    wxPoint pos = GetPosition();
    return { wxPoint(pos.x, pos.y + 20) };
}

wxPoint NotGate::GetOutputPoint() {
    wxPoint pos = GetPosition();
    return wxPoint(pos.x + 40, pos.y + 20);
}

// 与门类实现
AndGate::AndGate(wxPoint position) : Component(position, "AND Gate") {}

void AndGate::Draw(wxDC& dc) {
    // 绘制与门图形（简化版）
    wxPoint pos = GetPosition();

    // 绘制左侧两条输入线
    dc.DrawLine(pos.x - 10, pos.y + 10, pos.x, pos.y + 10);
    dc.DrawLine(pos.x - 10, pos.y + 30, pos.x, pos.y + 30);

    // 绘制与门主体
    wxPoint points[4] = {
        pos,
        wxPoint(pos.x + 30, pos.y),
        wxPoint(pos.x + 40, pos.y + 20),
        wxPoint(pos.x + 30, pos.y + 40)
    };
    dc.DrawPolygon(4, points);

    // 绘制输出线
    dc.DrawLine(pos.x + 40, pos.y + 20, pos.x + 60, pos.y + 20);
}

bool AndGate::ContainsPoint(wxPoint point) {
    wxPoint pos = GetPosition();
    return (point.x >= pos.x && point.x <= pos.x + 40 &&
        point.y >= pos.y && point.y <= pos.y + 40);
}

std::vector<wxPoint> AndGate::GetInputPoints() {
    wxPoint pos = GetPosition();
    return {
        wxPoint(pos.x, pos.y + 10),
        wxPoint(pos.x, pos.y + 30)
    };
}

wxPoint AndGate::GetOutputPoint() {
    wxPoint pos = GetPosition();
    return wxPoint(pos.x + 40, pos.y + 20);
}

// 其他逻辑门的实现类似，这里省略OrGate、NandGate等的具体实现
// 实际项目中需要完成这些类的Draw、ContainsPoint等方法

OrGate::OrGate(wxPoint position) : Component(position, "OR Gate") {}

void OrGate::Draw(wxDC& dc) {
    // 实现或门绘制
    wxPoint pos = GetPosition();

    // 绘制左侧两条输入线
    dc.DrawLine(pos.x - 10, pos.y + 10, pos.x, pos.y + 10);
    dc.DrawLine(pos.x - 10, pos.y + 30, pos.x, pos.y + 30);

    // 绘制或门主体（简化版）
    wxPoint points[4] = {
        pos,
        wxPoint(pos.x + 25, pos.y - 5),
        wxPoint(pos.x + 40, pos.y + 20),
        wxPoint(pos.x + 25, pos.y + 45)
    };
    dc.DrawPolygon(4, points);

    // 绘制输出线
    dc.DrawLine(pos.x + 40, pos.y + 20, pos.x + 60, pos.y + 20);
}

bool OrGate::ContainsPoint(wxPoint point) {
    wxPoint pos = GetPosition();
    return (point.x >= pos.x && point.x <= pos.x + 40 &&
        point.y >= pos.y - 5 && point.y <= pos.y + 45);
}

std::vector<wxPoint> OrGate::GetInputPoints() {
    wxPoint pos = GetPosition();
    return {
        wxPoint(pos.x, pos.y + 10),
        wxPoint(pos.x, pos.y + 30)
    };
}

wxPoint OrGate::GetOutputPoint() {
    wxPoint pos = GetPosition();
    return wxPoint(pos.x + 40, pos.y + 20);
}

// NandGate、NorGate、XorGate的实现略
NandGate::NandGate(wxPoint position) : Component(position, "NAND Gate") {}
void NandGate::Draw(wxDC& dc) {}
bool NandGate::ContainsPoint(wxPoint point) { return false; }
std::vector<wxPoint> NandGate::GetInputPoints() { return {}; }
wxPoint NandGate::GetOutputPoint() { return wxPoint(); }

NorGate::NorGate(wxPoint position) : Component(position, "NOR Gate") {}
void NorGate::Draw(wxDC& dc) {}
bool NorGate::ContainsPoint(wxPoint point) { return false; }
std::vector<wxPoint> NorGate::GetInputPoints() { return {}; }
wxPoint NorGate::GetOutputPoint() { return wxPoint(); }

XorGate::XorGate(wxPoint position) : Component(position, "XOR Gate") {}
void XorGate::Draw(wxDC& dc) {}
bool XorGate::ContainsPoint(wxPoint point) { return false; }
std::vector<wxPoint> XorGate::GetInputPoints() { return {}; }
wxPoint XorGate::GetOutputPoint() { return wxPoint(); }
