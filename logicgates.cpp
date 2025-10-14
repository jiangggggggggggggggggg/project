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
    // 绘制与门图形
}

bool AndGate::Evaluate() {
    // 与门逻辑计算
    return false;
}

OrGate::OrGate() {}

wxString OrGate::GetName() const {
    return "OR Gate";
}

void OrGate::Draw(wxDC& dc, const wxPoint& position) {
    // 绘制或门图形
}

bool OrGate::Evaluate() {
    // 或门逻辑计算
    return false;
}

NotGate::NotGate() {}

wxString NotGate::GetName() const {
    return "NOT Gate";
}

void NotGate::Draw(wxDC& dc, const wxPoint& position) {
    // 绘制非门图形
}

bool NotGate::Evaluate() {
    // 非门逻辑计算
    return false;
}

XorGate::XorGate() {}

wxString XorGate::GetName() const {
    return "XOR Gate";
}

void XorGate::Draw(wxDC& dc, const wxPoint& position) {
    // 绘制异或门图形
}

bool XorGate::Evaluate() {
    // 异或门逻辑计算
    return false;
}

NandGate::NandGate() {}

wxString NandGate::GetName() const {
    return "NAND Gate";
}

void NandGate::Draw(wxDC& dc, const wxPoint& position) {
    // 绘制与非门图形
}

bool NandGate::Evaluate() {
    // 与非门逻辑计算
    return false;
}

NorGate::NorGate() {}

wxString NorGate::GetName() const {
    return "NOR Gate";
}

void NorGate::Draw(wxDC& dc, const wxPoint& position) {
    // 绘制或非门图形
}

bool NorGate::Evaluate() {
    // 或非门逻辑计算
    return false;
}

XnorGate::XnorGate() {}

wxString XnorGate::GetName() const {
    return "XNOR Gate";
}

void XnorGate::Draw(wxDC& dc, const wxPoint& position) {
    // 绘制异或非门图形
}

bool XnorGate::Evaluate() {
    // 异或非门逻辑计算
    return false;
}

BufferGate::BufferGate() {}

wxString BufferGate::GetName() const {
    return "Buffer";
}

void BufferGate::Draw(wxDC& dc, const wxPoint& position) {
    // 绘制缓冲器图形
}

bool BufferGate::Evaluate() {
    // 缓冲器逻辑计算
    return false;
}