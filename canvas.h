#pragma once
#include "logicgates.h"
#include <stack>

class MyDrawPanel : public wxPanel {
public:
    MyDrawPanel(wxWindow* parent);

    // 绘图操作
    void AddShape(const wxString& shape);
    void RemoveLastShape();
    void DeleteSelected();
    void DeleteSelectedWire();
    void EditSelectedProperties();
    void ClearShapes();
    void ToggleGrid();
    bool IsGridVisible() const { return m_showGrid; }
    void ZoomIn();
    void ZoomOut();

    // 状态管理
    struct DrawPanelState {
        std::vector<Gate> gates;
        std::vector<Wire> wires;
        int selectedIndex;
        int selectedWireIndex;
    };

    DrawPanelState GetState() const;
    void SetState(const DrawPanelState& state);

    // 数据访问
    std::vector<wxString> GetShapes() const;
    void SetShapes(const std::vector<wxString>& shapes);
    std::vector<Gate> GetGates() const;
    void SetGates(const std::vector<Gate>& gates);

private:
    std::vector<Gate> m_gates;
    std::vector<Wire> m_wires;
    double m_scale;

    // 交互状态
    bool m_isDrawingWire;
    wxPoint m_wireStart;
    wxPoint m_currentMouse;
    bool m_isDraggingGate;
    int m_draggedIndex;
    wxPoint m_dragOffset;
    int m_selectedIndex;
    int m_selectedWireIndex;
    bool m_showGrid;
    int m_gridSize;

    // 绘制方法
    void DrawGate(wxDC& dc, const Gate& gate);
    void DrawShape(wxDC& dc, const Shape& s, const wxPoint& pos);
    void DrawGrid(wxDC& dc);
    wxRect GetGateBBox(const Gate& g) const;

    // 工具方法
    bool IsPointNearLine(const wxPoint& point, const Wire& wire, int tolerance = 5);
    wxPoint ToLogical(const wxPoint& devicePt) const;
    wxPoint SnapToGrid(const wxPoint& point);

    // 事件处理
    void OnPaint(wxPaintEvent&);
    void OnMouseDown(wxMouseEvent& evt);
    void OnRightClick(wxMouseEvent& evt);
    void OnMouseMove(wxMouseEvent& evt);
    void OnMouseUp(wxMouseEvent&);
    void OnKeyDown(wxKeyEvent& evt);

    wxDECLARE_EVENT_TABLE();
};