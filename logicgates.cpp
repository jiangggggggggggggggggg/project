#include "logicgates.h"

LogicGate::LogicGate() : position(0, 0), size(50, 30) {}

void LogicGate::SetPosition(const wxPoint& pos) {
    position = pos;
}

wxPoint LogicGate::GetPosition() const {
    return position;
}

AndGate::AndGate() {}

wxString AndGate::GetName() const {
    return "AND Gate";
}

void AndGate::Draw(wxDC& dc, const wxPoint& position) {
    // ��������ͼ��
}

bool AndGate::Evaluate() {
    // �����߼�����
    return false;
}

OrGate::OrGate() {}

wxString OrGate::GetName() const {
    return "OR Gate";
}

void OrGate::Draw(wxDC& dc, const wxPoint& position) {
    // ���ƻ���ͼ��
}

bool OrGate::Evaluate() {
    // �����߼�����
    return false;
}

NotGate::NotGate() {}

wxString NotGate::GetName() const {
    return "NOT Gate";
}

void NotGate::Draw(wxDC& dc, const wxPoint& position) {
    // ���Ʒ���ͼ��
}

bool NotGate::Evaluate() {
    // �����߼�����
    return false;
}

XorGate::XorGate() {}

wxString XorGate::GetName() const {
    return "XOR Gate";
}

void XorGate::Draw(wxDC& dc, const wxPoint& position) {
    // ���������ͼ��
}

bool XorGate::Evaluate() {
    // ������߼�����
    return false;
}

NandGate::NandGate() {}

wxString NandGate::GetName() const {
    return "NAND Gate";
}

void NandGate::Draw(wxDC& dc, const wxPoint& position) {
    // ���������ͼ��
}

bool NandGate::Evaluate() {
    // ������߼�����
    return false;
}

NorGate::NorGate() {}

wxString NorGate::GetName() const {
    return "NOR Gate";
}

void NorGate::Draw(wxDC& dc, const wxPoint& position) {
    // ���ƻ����ͼ��
}

bool NorGate::Evaluate() {
    // ������߼�����
    return false;
}

XnorGate::XnorGate() {}

wxString XnorGate::GetName() const {
    return "XNOR Gate";
}

void XnorGate::Draw(wxDC& dc, const wxPoint& position) {
    // ����������ͼ��
}

bool XnorGate::Evaluate() {
    // �������߼�����
    return false;
}

BufferGate::BufferGate() {}

wxString BufferGate::GetName() const {
    return "Buffer";
}

void BufferGate::Draw(wxDC& dc, const wxPoint& position) {
    // ���ƻ�����ͼ��
}

bool BufferGate::Evaluate() {
    // �������߼�����
    return false;
}