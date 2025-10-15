#include "logicgates.h"

// ��ʼ����̬��Ա
int Component::m_nextId = 1;

// ����Ԫ����ʵ��
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

// ������ʵ��
NotGate::NotGate(wxPoint position) : Component(position, "NOT Gate") {}

void NotGate::Draw(wxDC& dc) {
    // ���Ʒ���ͼ�Σ��򻯰棩
    wxPoint pos = GetPosition();

    // ����������
    wxPoint points[3] = {
        pos,
        wxPoint(pos.x + 40, pos.y + 20),
        pos + wxPoint(0, 40)
    };
    dc.DrawPolygon(3, points);

    // ����Բ�Σ����ű�־��
    dc.DrawCircle(pos.x + 40, pos.y + 20, 5);

    // �������������
    dc.DrawLine(pos.x - 10, pos.y + 20, pos.x, pos.y + 20);  // ������
    dc.DrawLine(pos.x + 45, pos.y + 20, pos.x + 60, pos.y + 20);  // �����
}

bool NotGate::ContainsPoint(wxPoint point) {
    wxPoint pos = GetPosition();
    // �򵥵���ײ���
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

// ������ʵ��
AndGate::AndGate(wxPoint position) : Component(position, "AND Gate") {}

void AndGate::Draw(wxDC& dc) {
    // ��������ͼ�Σ��򻯰棩
    wxPoint pos = GetPosition();

    // �����������������
    dc.DrawLine(pos.x - 10, pos.y + 10, pos.x, pos.y + 10);
    dc.DrawLine(pos.x - 10, pos.y + 30, pos.x, pos.y + 30);

    // ������������
    wxPoint points[4] = {
        pos,
        wxPoint(pos.x + 30, pos.y),
        wxPoint(pos.x + 40, pos.y + 20),
        wxPoint(pos.x + 30, pos.y + 40)
    };
    dc.DrawPolygon(4, points);

    // ���������
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

// �����߼��ŵ�ʵ�����ƣ�����ʡ��OrGate��NandGate�ȵľ���ʵ��
// ʵ����Ŀ����Ҫ�����Щ���Draw��ContainsPoint�ȷ���

OrGate::OrGate(wxPoint position) : Component(position, "OR Gate") {}

void OrGate::Draw(wxDC& dc) {
    // ʵ�ֻ��Ż���
    wxPoint pos = GetPosition();

    // �����������������
    dc.DrawLine(pos.x - 10, pos.y + 10, pos.x, pos.y + 10);
    dc.DrawLine(pos.x - 10, pos.y + 30, pos.x, pos.y + 30);

    // ���ƻ������壨�򻯰棩
    wxPoint points[4] = {
        pos,
        wxPoint(pos.x + 25, pos.y - 5),
        wxPoint(pos.x + 40, pos.y + 20),
        wxPoint(pos.x + 25, pos.y + 45)
    };
    dc.DrawPolygon(4, points);

    // ���������
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

// NandGate��NorGate��XorGate��ʵ����
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
