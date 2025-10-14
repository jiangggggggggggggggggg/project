#include "canvas.h"
#include <wx/dcgraph.h>
#include <algorithm>  // 添加 algorithm 头文件用于 std::find
#include <cmath>      // 添加 cmath 头文件用于数学函数

// Wire 类的实现
void Wire::Draw(wxDC& dc) const {
    dc.SetPen(wxPen(*wxBLACK, 2));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawLine(startPoint, endPoint);
}

// SelectionBox 类的实现
void SelectionBox::StartSelection(const wxPoint& start) {
    isSelecting = true;
    startPoint = start;
    selectionRect = wxRect(start, wxSize(0, 0));
}

void SelectionBox::UpdateSelection(const wxPoint& current) {
    if (!isSelecting) return;

    int x = wxMin(startPoint.x, current.x);
    int y = wxMin(startPoint.y, current.y);
    int width = abs(current.x - startPoint.x);
    int height = abs(current.y - startPoint.y);

    selectionRect = wxRect(x, y, width, height);
}

void SelectionBox::EndSelection() {
    isSelecting = false;
}

void SelectionBox::Draw(wxDC& dc) const {
    if (!isSelecting || selectionRect.GetWidth() == 0 || selectionRect.GetHeight() == 0) {
        return;
    }

    dc.SetPen(wxPen(*wxBLUE, 1, wxPENSTYLE_DOT));
    dc.SetBrush(wxBrush(wxColour(0, 0, 255, 50))); // 半透明蓝色
    dc.DrawRectangle(selectionRect);
}

// Canvas 类的实现
wxBEGIN_EVENT_TABLE(Canvas, wxPanel)
EVT_PAINT(Canvas::OnPaint)
EVT_SIZE(Canvas::OnSize)
EVT_LEFT_DOWN(Canvas::OnMouseLeftDown)
EVT_LEFT_UP(Canvas::OnMouseLeftUp)
EVT_RIGHT_DOWN(Canvas::OnMouseRightDown)
EVT_MOTION(Canvas::OnMouseMove)
EVT_MOUSEWHEEL(Canvas::OnMouseWheel)
EVT_KEY_DOWN(Canvas::OnKeyDown)
wxEND_EVENT_TABLE()

Canvas::Canvas(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
    componentLib(nullptr),
    isPlacingComponent(false),        // 初始化缺失的成员变量
    isDragging(false),
    isConnectingWires(false),
    draggedGate(nullptr),
    hoveredGate(nullptr),
    hoveredWire(nullptr),
    isDrawingWire(false),
    wireStartGate(nullptr),
    wireStartPort(-1),
    showGrid(true),
    snapToGrid(true),
    gridSize(20),
    viewOffset(0, 0),
    zoomLevel(1.0) {

    SetBackgroundColour(*wxWHITE);
    SetBackgroundStyle(wxBG_STYLE_PAINT); // 双缓冲
    SetFocus(); // 接收键盘事件
}

Canvas::~Canvas() {
    ClearCanvas();
    ClearWires();
}

void Canvas::AddGate(LogicGate* gate) {
    if (gate) {
        gates.push_back(gate);
        Refresh();
    }
}

void Canvas::RemoveGate(LogicGate* gate) {
    auto it = std::find(gates.begin(), gates.end(), gate);
    if (it != gates.end()) {
        delete* it;
        gates.erase(it);
        Refresh();
    }
}

void Canvas::ClearCanvas() {
    for (auto gate : gates) {
        delete gate;
    }
    gates.clear();
    selectedGates.clear();
    Refresh();
}

void Canvas::AddWire(const wxPoint& start, const wxPoint& end) {
    wires.push_back(new Wire(start, end));
    Refresh();
}

void Canvas::RemoveWire(Wire* wire) {
    auto it = std::find(wires.begin(), wires.end(), wire);
    if (it != wires.end()) {
        delete* it;
        wires.erase(it);
        Refresh();
    }
}

void Canvas::ClearWires() {
    for (auto wire : wires) {
        delete wire;
    }
    wires.clear();
    selectedWires.clear();
    Refresh();
}

void Canvas::SetSelectedComponent(ComponentType type) {
    selectedComponentType = type;
    isPlacingComponent = true;
    SetCursor(wxCursor(wxCURSOR_CROSS));
}

// 添加缺失的 PlaceComponent 方法实现
void Canvas::PlaceComponent(const wxPoint& position) {
    if (!isPlacingComponent || !componentLib) return;

    LogicGate* newGate = componentLib->CreateComponent(selectedComponentType);
    if (newGate) {
        newGate->SetPosition(position);
        AddGate(newGate);

        // 保持放置模式，可以连续放置多个相同元件
        // 如果想只放置一个，取消注释下面两行：
        // isPlacingComponent = false;
        // SetCursor(wxCursor(wxCURSOR_ARROW));
    }

    Refresh();
}

void Canvas::ClearSelection() {
    selectedGates.clear();
    selectedWires.clear();
    Refresh();
}

void Canvas::DeleteSelected() {
    // 删除选中的连线
    for (auto wire : selectedWires) {
        RemoveWire(wire);
    }
    selectedWires.clear();

    // 删除选中的元件
    for (auto gate : selectedGates) {
        RemoveGate(gate);
    }
    selectedGates.clear();
}

void Canvas::OnPaint(wxPaintEvent& event) {
    // 使用 wxPaintDC 替代 wxAutoBufferedPaintDC
    wxPaintDC dc(this);

    // 设置背景
    DrawBackground(dc);

    // 应用视图变换
    dc.SetUserScale(zoomLevel, zoomLevel);
    dc.SetDeviceOrigin(viewOffset.x, viewOffset.y);

    // 绘制网格
    if (showGrid) {
        DrawGrid(dc);
    }

    // 绘制连线
    for (auto wire : wires) {
        // 检查连线是否被选中
        if (std::find(selectedWires.begin(), selectedWires.end(), wire) != selectedWires.end()) {
            dc.SetPen(wxPen(*wxBLUE, 3)); // 选中状态用蓝色粗线
        }
        else {
            dc.SetPen(wxPen(*wxBLACK, 2));
        }
        wire->Draw(dc);
    }

    // 绘制正在绘制的连线（临时）
    if (isDrawingWire) {
        dc.SetPen(wxPen(*wxRED, 2, wxPENSTYLE_DOT));
        dc.DrawLine(wireStartPoint, lastMousePosition);
    }

    // 绘制元件
    for (auto gate : gates) {
        // 检查元件是否被选中
        if (std::find(selectedGates.begin(), selectedGates.end(), gate) != selectedGates.end()) {
            dc.SetPen(wxPen(*wxBLUE, 2)); // 选中状态用蓝色边框
        }
        else if (gate == hoveredGate) {
            dc.SetPen(wxPen(*wxGREEN, 2)); // 悬停状态用绿色边框
        }
        else {
            dc.SetPen(wxPen(*wxBLACK, 1));
        }

        gate->Draw(dc, gate->GetPosition());
    }

    // 绘制选择框
    selectionBox.Draw(dc);
}

void Canvas::OnSize(wxSizeEvent& event) {
    Refresh();
    event.Skip();
}

void Canvas::OnMouseLeftDown(wxMouseEvent& event) {
    wxPoint mousePos = event.GetPosition();
    wxPoint canvasPos = SnapToGridPoint(mousePos);

    CaptureMouse(); // 捕获鼠标

    if (isPlacingComponent) {
        // 放置元件模式
        PlaceComponent(canvasPos);
    }
    else {
        // 正常选择模式
        LogicGate* clickedGate = GetGateAtPosition(mousePos);
        Wire* clickedWire = GetWireAtPosition(mousePos);

        if (clickedGate) {
            // 点击了元件
            if (event.ControlDown()) {
                // Ctrl+点击：多选
                auto it = std::find(selectedGates.begin(), selectedGates.end(), clickedGate);
                if (it == selectedGates.end()) {
                    selectedGates.push_back(clickedGate);
                }
                else {
                    selectedGates.erase(it);
                }
            }
            else {
                // 普通点击：选择单个元件
                if (std::find(selectedGates.begin(), selectedGates.end(), clickedGate) == selectedGates.end()) {
                    selectedGates.clear();
                    selectedWires.clear();
                    selectedGates.push_back(clickedGate);
                }
                // 开始拖拽
                isDragging = true;
                draggedGate = clickedGate;
                dragStartPoint = mousePos;
            }
        }
        else if (clickedWire) {
            // 点击了连线
            if (event.ControlDown()) {
                // Ctrl+点击：多选
                auto it = std::find(selectedWires.begin(), selectedWires.end(), clickedWire);
                if (it == selectedWires.end()) {
                    selectedWires.push_back(clickedWire);
                }
                else {
                    selectedWires.erase(it);
                }
            }
            else {
                // 普通点击：选择单根连线
                selectedWires.clear();
                selectedGates.clear();
                selectedWires.push_back(clickedWire);
            }
        }
        else {
            // 点击空白处：开始框选
            selectedGates.clear();
            selectedWires.clear();
            selectionBox.StartSelection(mousePos);
        }
    }

    Refresh();
    event.Skip();
}

void Canvas::OnMouseLeftUp(wxMouseEvent& event) {
    if (HasCapture()) {
        ReleaseMouse(); // 释放鼠标捕获
    }

    if (selectionBox.IsSelecting()) {
        // 结束框选
        selectionBox.EndSelection();

        // 选择框选范围内的所有元件
        wxRect selectionRect = selectionBox.GetSelectionRect();
        for (auto gate : gates) {
            wxPoint gatePos = gate->GetPosition();
            if (selectionRect.Contains(gatePos)) {
                selectedGates.push_back(gate);
            }
        }
    }

    isDragging = false;
    draggedGate = nullptr;

    if (isDrawingWire) {
        EndWireConnection(event.GetPosition());
    }

    Refresh();
    event.Skip();
}

void Canvas::OnMouseRightDown(wxMouseEvent& event) {
    wxPoint mousePos = event.GetPosition();

    // 右键取消当前操作
    if (isPlacingComponent) {
        isPlacingComponent = false;
        SetCursor(wxCursor(wxCURSOR_ARROW));
    }
    else if (isDrawingWire) {
        isDrawingWire = false;
        SetCursor(wxCursor(wxCURSOR_ARROW));
    }
    else {
        // 显示上下文菜单
        wxMenu contextMenu;
        contextMenu.Append(wxID_CUT, "Cut");
        contextMenu.Append(wxID_COPY, "Copy");
        contextMenu.Append(wxID_PASTE, "Paste");
        contextMenu.AppendSeparator();
        contextMenu.Append(wxID_DELETE, "Delete");
        contextMenu.AppendSeparator();
        contextMenu.Append(wxID_ANY, "Properties");

        PopupMenu(&contextMenu, mousePos);
    }

    Refresh();
    event.Skip();
}

void Canvas::OnMouseMove(wxMouseEvent& event) {
    wxPoint mousePos = event.GetPosition();
    lastMousePosition = mousePos;

    // 更新悬停状态
    hoveredGate = GetGateAtPosition(mousePos);
    hoveredWire = GetWireAtPosition(mousePos);

    if (isDragging && draggedGate) {
        // 拖拽元件
        wxPoint delta = mousePos - dragStartPoint;
        wxPoint newPos = draggedGate->GetPosition() + delta;
        draggedGate->SetPosition(SnapToGridPoint(newPos));
        dragStartPoint = mousePos;
        Refresh();
    }
    else if (selectionBox.IsSelecting()) {
        // 更新选择框
        selectionBox.UpdateSelection(mousePos);
        Refresh();
    }
    else if (isDrawingWire) {
        // 更新正在绘制的连线
        UpdateWireConnection(mousePos);
        Refresh();
    }

    // 更新光标
    if (hoveredGate || hoveredWire) {
        SetCursor(wxCursor(wxCURSOR_HAND));
    }
    else if (isPlacingComponent) {
        SetCursor(wxCursor(wxCURSOR_CROSS));
    }
    else {
        SetCursor(wxCursor(wxCURSOR_ARROW));
    }

    event.Skip();
}

void Canvas::OnMouseWheel(wxMouseEvent& event) {
    // 鼠标滚轮缩放
    double zoomFactor = 1.1;
    if (event.GetWheelRotation() > 0) {
        // 放大
        zoomLevel *= zoomFactor;
    }
    else {
        // 缩小
        zoomLevel /= zoomFactor;
    }

    // 限制缩放范围
    zoomLevel = wxMax(0.1, wxMin(5.0, zoomLevel));

    Refresh();
    event.Skip();
}

void Canvas::OnKeyDown(wxKeyEvent& event) {
    switch (event.GetKeyCode()) {
    case WXK_DELETE:
    case WXK_BACK:
        DeleteSelected();
        break;
    case WXK_ESCAPE:
        // 取消当前操作
        isPlacingComponent = false;
        isDrawingWire = false;
        SetCursor(wxCursor(wxCURSOR_ARROW));
        Refresh();
        break;
    case 'A':
        if (event.ControlDown()) {
            // Ctrl+A: 全选
            selectedGates = gates;
            selectedWires = wires;
            Refresh();
        }
        break;
    }
    event.Skip();
}

void Canvas::DrawGrid(wxDC& dc) {
    wxSize clientSize = GetClientSize();

    dc.SetPen(wxPen(wxColour(220, 220, 220), 1)); // 浅灰色网格

    // 计算网格起点（考虑视图偏移）
    int startX = (int)(-viewOffset.x / zoomLevel) / gridSize * gridSize;
    int startY = (int)(-viewOffset.y / zoomLevel) / gridSize * gridSize;

    // 绘制垂直线
    for (int x = startX; x < clientSize.x / zoomLevel; x += gridSize) {
        dc.DrawLine(x, 0, x, clientSize.y / zoomLevel);
    }

    // 绘制水平线
    for (int y = startY; y < clientSize.y / zoomLevel; y += gridSize) {
        dc.DrawLine(0, y, clientSize.x / zoomLevel, y);
    }
}

void Canvas::DrawBackground(wxDC& dc) {
    wxSize clientSize = GetClientSize();
    dc.SetBrush(wxBrush(*wxWHITE));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(0, 0, clientSize.x, clientSize.y);
}

wxPoint Canvas::SnapToGridPoint(const wxPoint& point) const {
    if (!snapToGrid) return point;

    int x = (point.x / gridSize) * gridSize;
    int y = (point.y / gridSize) * gridSize;
    return wxPoint(x, y);
}

LogicGate* Canvas::GetGateAtPosition(const wxPoint& position) const {
    // 简单的点击检测：检查位置是否在元件矩形区域内
    for (auto gate : gates) {
        wxPoint gatePos = gate->GetPosition();
        wxSize gateSize(50, 30); // 假设元件大小

        wxRect gateRect(gatePos, gateSize);
        if (gateRect.Contains(position)) {
            return gate;
        }
    }
    return nullptr;
}

Wire* Canvas::GetWireAtPosition(const wxPoint& position) const {
    // 简单的连线点击检测
    const int clickTolerance = 5;

    for (auto wire : wires) {
        wxPoint start = wire->GetStartPoint();
        wxPoint end = wire->GetEndPoint();

        // 计算点到线段的距离
        double lineLength = sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2));
        if (lineLength == 0) continue;

        double t = wxMax(0, wxMin(1, ((position.x - start.x) * (end.x - start.x) +
            (position.y - start.y) * (end.y - start.y)) /
            (lineLength * lineLength)));

        wxPoint projection(start.x + t * (end.x - start.x),
            start.y + t * (end.y - start.y));

        double distance = sqrt(pow(position.x - projection.x, 2) +
            pow(position.y - projection.y, 2));

        if (distance <= clickTolerance) {
            return wire;
        }
    }
    return nullptr;
}

void Canvas::StartWireConnection(const wxPoint& position) {
    // 开始连线（简化实现）
    isDrawingWire = true;
    wireStartPoint = position;
    SetCursor(wxCursor(wxCURSOR_CROSS));
}

void Canvas::UpdateWireConnection(const wxPoint& position) {
    if (isDrawingWire) {
        // 更新临时连线显示
        lastMousePosition = position;
    }
}

void Canvas::EndWireConnection(const wxPoint& position) {
    if (isDrawingWire) {
        AddWire(wireStartPoint, position);
        isDrawingWire = false;
        SetCursor(wxCursor(wxCURSOR_ARROW));
    }
}

void Canvas::UpdateViewport() {
    // 更新视口（用于滚动等）
    Refresh();
}