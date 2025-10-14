#ifndef CANVAS_H
#define CANVAS_H

#include <wx/wx.h>
#include <wx/dcclient.h>
#include <vector>
#include <memory>
#include "logicgates.h"
#include "componentlibrary.h"

// 前向声明
class ComponentLibrary;

// 连线类
class Wire {
public:
    Wire(const wxPoint& start, const wxPoint& end) : startPoint(start), endPoint(end) {}

    void Draw(wxDC& dc) const;
    void SetStartPoint(const wxPoint& point) { startPoint = point; }
    void SetEndPoint(const wxPoint& point) { endPoint = point; }
    wxPoint GetStartPoint() const { return startPoint; }
    wxPoint GetEndPoint() const { return endPoint; }

private:
    wxPoint startPoint;
    wxPoint endPoint;
};

// 选择框类
class SelectionBox {
public:
    SelectionBox() : isSelecting(false) {}

    void StartSelection(const wxPoint& start);
    void UpdateSelection(const wxPoint& current);
    void EndSelection();
    void Draw(wxDC& dc) const;
    bool IsSelecting() const { return isSelecting; }
    wxRect GetSelectionRect() const { return selectionRect; }

private:
    bool isSelecting;
    wxPoint startPoint;
    wxRect selectionRect;
};

class Canvas : public wxPanel {
public:
    Canvas(wxWindow* parent);
    ~Canvas();

    // 元件管理
    void AddGate(LogicGate* gate);
    void RemoveGate(LogicGate* gate);
    void ClearCanvas();

    // 连线管理
    void AddWire(const wxPoint& start, const wxPoint& end);
    void RemoveWire(Wire* wire);
    void ClearWires();

    // 选择操作
    void SetSelectedComponent(ComponentType type);
    void PlaceComponent(const wxPoint& position);  // 添加缺失的方法声明
    void ClearSelection();
    void DeleteSelected();

    // 设置元件库
    void SetComponentLibrary(ComponentLibrary* library) { componentLib = library; }

    // 网格设置
    void SetGridVisible(bool visible) { showGrid = visible; Refresh(); }
    bool IsGridVisible() const { return showGrid; }
    void SetSnapToGrid(bool snap) { snapToGrid = snap; }
    bool IsSnapToGrid() const { return snapToGrid; }

    // 获取元件列表（用于仿真等）
    std::vector<LogicGate*> GetGates() const { return gates; }
    std::vector<Wire*> GetWires() const { return wires; }

private:
    // 事件处理函数
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnMouseLeftDown(wxMouseEvent& event);
    void OnMouseLeftUp(wxMouseEvent& event);
    void OnMouseRightDown(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnKeyDown(wxKeyEvent& event);

    // 内部辅助函数
    void DrawGrid(wxDC& dc);
    void DrawBackground(wxDC& dc);
    wxPoint SnapToGridPoint(const wxPoint& point) const;
    LogicGate* GetGateAtPosition(const wxPoint& position) const;
    Wire* GetWireAtPosition(const wxPoint& position) const;
    void StartWireConnection(const wxPoint& position);
    void UpdateWireConnection(const wxPoint& position);
    void EndWireConnection(const wxPoint& position);
    void UpdateViewport();

    // 成员变量
    std::vector<LogicGate*> gates;
    std::vector<Wire*> wires;
    ComponentLibrary* componentLib;

    // 交互状态
    ComponentType selectedComponentType;
    bool isPlacingComponent;
    bool isDragging;
    bool isConnectingWires;
    LogicGate* draggedGate;
    LogicGate* hoveredGate;
    Wire* hoveredWire;
    wxPoint dragStartPoint;
    wxPoint lastMousePosition;

    // 连线状态
    wxPoint wireStartPoint;
    LogicGate* wireStartGate;
    int wireStartPort;
    bool isDrawingWire;

    // 选择状态
    SelectionBox selectionBox;
    std::vector<LogicGate*> selectedGates;
    std::vector<Wire*> selectedWires;

    // 视图状态
    bool showGrid;
    bool snapToGrid;
    int gridSize;
    wxPoint viewOffset;
    double zoomLevel;

    wxDECLARE_EVENT_TABLE();
};

#endif // CANVAS_H