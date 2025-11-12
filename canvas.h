// [file name]: canvas.h
// [file content begin]
#pragma once
#include "logicgates.h"
#include <stack>

class MyDrawPanel : public wxPanel {
public:
    // 在类内部定义连线模式枚举
    enum WireMode {
        WIRE_STRAIGHT,   // 直线模式
        WIRE_ORTHOGONAL  // 直角模式
    };

    MyDrawPanel(wxWindow* parent);

    // 绘图操作
    void AddShape(const wxString& shape);
    void RemoveLastShape();
    void DeleteSelected();
    void DeleteSelectedWire();
    void DeleteSelectedBreakpoint();
    void EditSelectedProperties();
    void ClearShapes();
    void ToggleGrid();
    bool IsGridVisible() const { return m_showGrid; }
    void ZoomIn();
    void ZoomOut();
    void TogglePins();

    // 连线模式操作
    void SetWireMode(WireMode mode) { m_wireMode = mode; }
    WireMode GetWireMode() const { return m_wireMode; }
    void ToggleWireMode();

    // 断点操作
    void AddBreakpoint(const wxPoint& position, int wireIndex);
    bool IsPointNearBreakpoint(const wxPoint& point, int tolerance = 6);
    int FindBreakpointAtPoint(const wxPoint& point, int tolerance = 6);
    void UpdateWiresFromBreakpoints();

    // 网表操作
    Netlist GenerateNetlist() const;
    void ImportFromNetlist(const Netlist& netlist);
    void ShowNetlistPreview();

    // 状态管理
    struct DrawPanelState {
        std::vector<Gate> gates;
        std::vector<Wire> wires;
        std::vector<Breakpoint> breakpoints;
        int selectedIndex;
        int selectedWireIndex;
        int selectedBreakpointIndex;
    };

    DrawPanelState GetState() const;
    void SetState(const DrawPanelState& state);

    // 数据访问
    std::vector<wxString> GetShapes() const;
    void SetShapes(const std::vector<wxString>& shapes);
    std::vector<Gate> GetGates() const;
    void SetGates(const std::vector<Gate>& gates);
    std::vector<Wire> GetWires() const { return m_wires; }
    bool ArePinsVisible() const { return m_showPins; }

private:
    std::vector<Gate> m_gates;
    std::vector<Wire> m_wires;
    std::vector<Breakpoint> m_breakpoints; // 断点列表
    double m_scale;

    // 交互状态
    bool m_isDrawingWire;
    wxPoint m_wireStart;
    wxPoint m_currentMouse;
    bool m_isDraggingGate;
    bool m_isDraggingBreakpoint;
    int m_draggedIndex;
    int m_draggedBreakpointIndex;
    wxPoint m_dragOffset;
    int m_selectedIndex;
    int m_selectedWireIndex;
    int m_selectedBreakpointIndex; // 选中的断点索引
    bool m_showGrid;
    bool m_showPins;
    int m_gridSize;
    WireMode m_wireMode;  // 连线模式

    // 引脚吸附
    struct PinConnection {
        int gateIndex;
        int pinIndex;
        wxPoint position;
    };
    PinConnection m_hoveredPin;
    PinConnection m_connectedStartPin;
    PinConnection m_connectedEndPin;

    // 绘制方法
    void DrawGate(wxDC& dc, const Gate& gate);
    void DrawShape(wxDC& dc, const Shape& s, const wxPoint& pos);
    void DrawPin(wxDC& dc, const Pin& pin, const wxPoint& gatePos, bool isHovered = false);
    void DrawBreakpoint(wxDC& dc, const Breakpoint& breakpoint, bool isSelected = false);
    void DrawGrid(wxDC& dc);
    wxRect GetGateBBox(const Gate& g) const;

    // 直角连线绘制方法
    void DrawOrthogonalWire(wxGraphicsContext* gc, const Wire& wire);
    void DrawOrthogonalWire(wxGraphicsContext* gc, const wxPoint& start, const wxPoint& end);

    // 工具方法
    bool IsPointNearStraightLine(const wxPoint& point, const wxPoint& lineStart, const wxPoint& lineEnd, int tolerance);
    bool IsPointNearLine(const wxPoint& point, const Wire& wire, int tolerance = 5);
    PinConnection FindNearestPin(const wxPoint& point, int tolerance = 8);
    PinConnection FindNearestBreakpointPin(const wxPoint& point, int tolerance = 8); // 新增：查找断点引脚
    wxPoint ToLogical(const wxPoint& devicePt) const;
    wxPoint SnapToGrid(const wxPoint& point);
    wxPoint SnapToPin(const wxPoint& point);
    wxPoint SnapToBreakpoint(const wxPoint& point);
    wxPoint CalculatePointOnWire(const Wire& wire, double t);

    // 新增：几何计算工具方法
    wxPoint FindClosestPointOnLine(const wxPoint& lineStart, const wxPoint& lineEnd, const wxPoint& point);
    double CalculateParameterOnLine(const wxPoint& lineStart, const wxPoint& lineEnd, const wxPoint& pointOnLine);
    wxPoint FindClosestPointOnOrthogonalLine(const Wire& wire, const wxPoint& point, double& outT);
    double CalculateDistance(const wxPoint& p1, const wxPoint& p2);

    // 连线跟随移动
    void UpdateConnectedWires(int gateIndex);
    void UpdateWiresFromBreakpoint(int breakpointIndex); // 新增：更新断点连线

    // 事件处理
    void OnPaint(wxPaintEvent&);
    void OnMouseDown(wxMouseEvent& evt);
    void OnRightClick(wxMouseEvent& evt);
    void OnMouseMove(wxMouseEvent& evt);
    void OnMouseUp(wxMouseEvent&);
    void OnKeyDown(wxKeyEvent& evt);

    wxDECLARE_EVENT_TABLE();
};
// [file content end]