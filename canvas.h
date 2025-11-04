#pragma once
#include "logicgates.h"
#include <stack>
#include <vector>

class MyDrawPanel : public wxPanel {
public:
    MyDrawPanel(wxWindow* parent);

    // 基本操作
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
    std::vector<Wire> GetWires() const { return m_wires; }
    void SetWires(const std::vector<Wire>& wires) {
        m_wires = wires;
        Refresh();
    }
    double GetScale() const { return m_scale; }
    void SetScale(double scale) {
        m_scale = scale;
        if (m_scale < 0.1) m_scale = 0.1;
        if (m_scale > 5.0) m_scale = 5.0;
        Refresh();
    }

    // 引脚连接状态更新 - 改为 public
    void UpdatePinConnections();

private:
    std::vector<Gate> m_gates;
    std::vector<Wire> m_wires;
    double m_scale;

    // 连线绘制状态
    bool m_isDrawingWire;
    WireEndpoint m_wireStart;
    wxPoint m_currentMouse;
    std::vector<wxPoint> m_currentPath;

    // 拖拽状态
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
    void DrawPin(wxDC& dc, const Gate& gate, int pinIndex);
    void DrawWire(wxDC& dc, const Wire& wire);
    void DrawGrid(wxDC& dc);
    wxRect GetGateBBox(const Gate& g) const;

    // 引脚和连线检测方法
    bool FindPinAtPosition(const wxPoint& pos, WireEndpoint& endpoint);
    bool IsPointNearPin(const wxPoint& point, const Gate& gate, int pinIndex);
    bool IsPointNearWire(const wxPoint& point, const Wire& wire, int tolerance = 5);

    // 几何计算相关方法
    bool DoLinesIntersect(const wxPoint& a1, const wxPoint& a2, const wxPoint& b1, const wxPoint& b2);
    bool CheckWireIntersections(const Wire& wire);
    std::vector<wxPoint> FindWireIntersectionPoints(const Wire& wire);

    void UpdateWirePathsForGate(int gateIndex);
    std::vector<wxPoint> CalculateWirePath(const WireEndpoint& start, const WireEndpoint& end);
    std::vector<wxPoint> CalculateWirePath(const WireEndpoint& start, const wxPoint& end);
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
