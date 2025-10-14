#include "canvas.h"
#include <wx/dcgraph.h>
#include <algorithm>  // ��� algorithm ͷ�ļ����� std::find
#include <cmath>      // ��� cmath ͷ�ļ�������ѧ����

// Wire ���ʵ��
void Wire::Draw(wxDC& dc) const {
    dc.SetPen(wxPen(*wxBLACK, 2));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawLine(startPoint, endPoint);
}

// SelectionBox ���ʵ��
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
    dc.SetBrush(wxBrush(wxColour(0, 0, 255, 50))); // ��͸����ɫ
    dc.DrawRectangle(selectionRect);
}

// Canvas ���ʵ��
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
    isPlacingComponent(false),        // ��ʼ��ȱʧ�ĳ�Ա����
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
    SetBackgroundStyle(wxBG_STYLE_PAINT); // ˫����
    SetFocus(); // ���ռ����¼�
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

// ���ȱʧ�� PlaceComponent ����ʵ��
void Canvas::PlaceComponent(const wxPoint& position) {
    if (!isPlacingComponent || !componentLib) return;

    LogicGate* newGate = componentLib->CreateComponent(selectedComponentType);
    if (newGate) {
        newGate->SetPosition(position);
        AddGate(newGate);

        // ���ַ���ģʽ�������������ö����ͬԪ��
        // �����ֻ����һ����ȡ��ע���������У�
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
    // ɾ��ѡ�е�����
    for (auto wire : selectedWires) {
        RemoveWire(wire);
    }
    selectedWires.clear();

    // ɾ��ѡ�е�Ԫ��
    for (auto gate : selectedGates) {
        RemoveGate(gate);
    }
    selectedGates.clear();
}

void Canvas::OnPaint(wxPaintEvent& event) {
    // ʹ�� wxPaintDC ��� wxAutoBufferedPaintDC
    wxPaintDC dc(this);

    // ���ñ���
    DrawBackground(dc);

    // Ӧ����ͼ�任
    dc.SetUserScale(zoomLevel, zoomLevel);
    dc.SetDeviceOrigin(viewOffset.x, viewOffset.y);

    // ��������
    if (showGrid) {
        DrawGrid(dc);
    }

    // ��������
    for (auto wire : wires) {
        // ��������Ƿ�ѡ��
        if (std::find(selectedWires.begin(), selectedWires.end(), wire) != selectedWires.end()) {
            dc.SetPen(wxPen(*wxBLUE, 3)); // ѡ��״̬����ɫ����
        }
        else {
            dc.SetPen(wxPen(*wxBLACK, 2));
        }
        wire->Draw(dc);
    }

    // �������ڻ��Ƶ����ߣ���ʱ��
    if (isDrawingWire) {
        dc.SetPen(wxPen(*wxRED, 2, wxPENSTYLE_DOT));
        dc.DrawLine(wireStartPoint, lastMousePosition);
    }

    // ����Ԫ��
    for (auto gate : gates) {
        // ���Ԫ���Ƿ�ѡ��
        if (std::find(selectedGates.begin(), selectedGates.end(), gate) != selectedGates.end()) {
            dc.SetPen(wxPen(*wxBLUE, 2)); // ѡ��״̬����ɫ�߿�
        }
        else if (gate == hoveredGate) {
            dc.SetPen(wxPen(*wxGREEN, 2)); // ��ͣ״̬����ɫ�߿�
        }
        else {
            dc.SetPen(wxPen(*wxBLACK, 1));
        }

        gate->Draw(dc, gate->GetPosition());
    }

    // ����ѡ���
    selectionBox.Draw(dc);
}

void Canvas::OnSize(wxSizeEvent& event) {
    Refresh();
    event.Skip();
}

void Canvas::OnMouseLeftDown(wxMouseEvent& event) {
    wxPoint mousePos = event.GetPosition();
    wxPoint canvasPos = SnapToGridPoint(mousePos);

    CaptureMouse(); // �������

    if (isPlacingComponent) {
        // ����Ԫ��ģʽ
        PlaceComponent(canvasPos);
    }
    else {
        // ����ѡ��ģʽ
        LogicGate* clickedGate = GetGateAtPosition(mousePos);
        Wire* clickedWire = GetWireAtPosition(mousePos);

        if (clickedGate) {
            // �����Ԫ��
            if (event.ControlDown()) {
                // Ctrl+�������ѡ
                auto it = std::find(selectedGates.begin(), selectedGates.end(), clickedGate);
                if (it == selectedGates.end()) {
                    selectedGates.push_back(clickedGate);
                }
                else {
                    selectedGates.erase(it);
                }
            }
            else {
                // ��ͨ�����ѡ�񵥸�Ԫ��
                if (std::find(selectedGates.begin(), selectedGates.end(), clickedGate) == selectedGates.end()) {
                    selectedGates.clear();
                    selectedWires.clear();
                    selectedGates.push_back(clickedGate);
                }
                // ��ʼ��ק
                isDragging = true;
                draggedGate = clickedGate;
                dragStartPoint = mousePos;
            }
        }
        else if (clickedWire) {
            // ���������
            if (event.ControlDown()) {
                // Ctrl+�������ѡ
                auto it = std::find(selectedWires.begin(), selectedWires.end(), clickedWire);
                if (it == selectedWires.end()) {
                    selectedWires.push_back(clickedWire);
                }
                else {
                    selectedWires.erase(it);
                }
            }
            else {
                // ��ͨ�����ѡ�񵥸�����
                selectedWires.clear();
                selectedGates.clear();
                selectedWires.push_back(clickedWire);
            }
        }
        else {
            // ����հ״�����ʼ��ѡ
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
        ReleaseMouse(); // �ͷ���겶��
    }

    if (selectionBox.IsSelecting()) {
        // ������ѡ
        selectionBox.EndSelection();

        // ѡ���ѡ��Χ�ڵ�����Ԫ��
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

    // �Ҽ�ȡ����ǰ����
    if (isPlacingComponent) {
        isPlacingComponent = false;
        SetCursor(wxCursor(wxCURSOR_ARROW));
    }
    else if (isDrawingWire) {
        isDrawingWire = false;
        SetCursor(wxCursor(wxCURSOR_ARROW));
    }
    else {
        // ��ʾ�����Ĳ˵�
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

    // ������ͣ״̬
    hoveredGate = GetGateAtPosition(mousePos);
    hoveredWire = GetWireAtPosition(mousePos);

    if (isDragging && draggedGate) {
        // ��קԪ��
        wxPoint delta = mousePos - dragStartPoint;
        wxPoint newPos = draggedGate->GetPosition() + delta;
        draggedGate->SetPosition(SnapToGridPoint(newPos));
        dragStartPoint = mousePos;
        Refresh();
    }
    else if (selectionBox.IsSelecting()) {
        // ����ѡ���
        selectionBox.UpdateSelection(mousePos);
        Refresh();
    }
    else if (isDrawingWire) {
        // �������ڻ��Ƶ�����
        UpdateWireConnection(mousePos);
        Refresh();
    }

    // ���¹��
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
    // ����������
    double zoomFactor = 1.1;
    if (event.GetWheelRotation() > 0) {
        // �Ŵ�
        zoomLevel *= zoomFactor;
    }
    else {
        // ��С
        zoomLevel /= zoomFactor;
    }

    // �������ŷ�Χ
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
        // ȡ����ǰ����
        isPlacingComponent = false;
        isDrawingWire = false;
        SetCursor(wxCursor(wxCURSOR_ARROW));
        Refresh();
        break;
    case 'A':
        if (event.ControlDown()) {
            // Ctrl+A: ȫѡ
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

    dc.SetPen(wxPen(wxColour(220, 220, 220), 1)); // ǳ��ɫ����

    // ����������㣨������ͼƫ�ƣ�
    int startX = (int)(-viewOffset.x / zoomLevel) / gridSize * gridSize;
    int startY = (int)(-viewOffset.y / zoomLevel) / gridSize * gridSize;

    // ���ƴ�ֱ��
    for (int x = startX; x < clientSize.x / zoomLevel; x += gridSize) {
        dc.DrawLine(x, 0, x, clientSize.y / zoomLevel);
    }

    // ����ˮƽ��
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
    // �򵥵ĵ����⣺���λ���Ƿ���Ԫ������������
    for (auto gate : gates) {
        wxPoint gatePos = gate->GetPosition();
        wxSize gateSize(50, 30); // ����Ԫ����С

        wxRect gateRect(gatePos, gateSize);
        if (gateRect.Contains(position)) {
            return gate;
        }
    }
    return nullptr;
}

Wire* Canvas::GetWireAtPosition(const wxPoint& position) const {
    // �򵥵����ߵ�����
    const int clickTolerance = 5;

    for (auto wire : wires) {
        wxPoint start = wire->GetStartPoint();
        wxPoint end = wire->GetEndPoint();

        // ����㵽�߶εľ���
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
    // ��ʼ���ߣ���ʵ�֣�
    isDrawingWire = true;
    wireStartPoint = position;
    SetCursor(wxCursor(wxCURSOR_CROSS));
}

void Canvas::UpdateWireConnection(const wxPoint& position) {
    if (isDrawingWire) {
        // ������ʱ������ʾ
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
    // �����ӿڣ����ڹ����ȣ�
    Refresh();
}