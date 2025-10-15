#include "canvas.h"

// ������ʵ��
Wire::Wire(int fromComponent, int fromPin, int toComponent, int toPin)
    : m_fromComponent(fromComponent), m_fromPin(fromPin),
    m_toComponent(toComponent), m_toPin(toPin) {
}

int Wire::GetFromComponent() const {
    return m_fromComponent;
}

int Wire::GetFromPin() const {
    return m_fromPin;
}

int Wire::GetToComponent() const {
    return m_toComponent;
}

int Wire::GetToPin() const {
    return m_toPin;
}

// ������ʵ��
BEGIN_EVENT_TABLE(Canvas, wxPanel)
EVT_PAINT(Canvas::OnPaint)
EVT_LEFT_DOWN(Canvas::OnLeftDown)
EVT_LEFT_UP(Canvas::OnLeftUp)
EVT_MOTION(Canvas::OnMouseMove)
EVT_RIGHT_DOWN(Canvas::OnRightClick)
END_EVENT_TABLE()

Canvas::Canvas(wxWindow* parent)
    : wxPanel(parent, wxID_ANY), m_selectedComponent(-1), m_dragging(false) {
    SetBackgroundColour(*wxWHITE);
}

void Canvas::AddComponent(Component* component) {
    m_components.push_back(component);
    Refresh();  // ˢ�»���
}

void Canvas::RemoveComponent(int id) {
    int index = GetComponentIndex(id);
    if (index != -1) {
        delete m_components[index];
        m_components.erase(m_components.begin() + index);
        Refresh();
    }
}

void Canvas::AddWire(int fromComponent, int fromPin, int toComponent, int toPin) {
    m_wires.emplace_back(fromComponent, fromPin, toComponent, toPin);
    Refresh();
}

void Canvas::Clear() {
    // �ͷ�����Ԫ���ڴ�
    for (Component* comp : m_components) {
        delete comp;
    }
    m_components.clear();
    m_wires.clear();
    m_selectedComponent = -1;
    Refresh();
}

void Canvas::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    DrawWires(dc);   // �Ȼ������ߣ��ڵײ㣩
    DrawComponents(dc);  // �ٻ���Ԫ�������ϲ㣩
}

void Canvas::OnLeftDown(wxMouseEvent& event) {
    wxPoint pos = event.GetPosition();
    m_selectedComponent = FindComponentAtPoint(pos);

    if (m_selectedComponent != -1) {
        m_dragging = true;
        m_dragStart = pos;
    }
    else {
        // �������հ�����ȡ��ѡ��
        m_selectedComponent = -1;
        Refresh();
    }
}

void Canvas::OnLeftUp(wxMouseEvent& event) {
    m_dragging = false;
}

void Canvas::OnMouseMove(wxMouseEvent& event) {
    if (m_dragging && m_selectedComponent != -1) {
        int index = GetComponentIndex(m_selectedComponent);
        if (index != -1) {
            wxPoint currentPos = event.GetPosition();
            wxPoint delta = currentPos - m_dragStart;

            // �ƶ�Ԫ��
            wxPoint newPos = m_components[index]->GetPosition() + delta;
            m_components[index]->SetPosition(newPos);

            m_dragStart = currentPos;
            Refresh();
        }
    }
}

void Canvas::OnRightClick(wxMouseEvent& event) {
    // �Ҽ��˵�����
    wxMenu menu;
    menu.Append(wxID_DELETE, "Delete");

    int componentId = FindComponentAtPoint(event.GetPosition());
    if (componentId != -1) {
        m_selectedComponent = componentId;
        PopupMenu(&menu);
    }
}

void Canvas::DrawComponents(wxDC& dc) {
    for (Component* comp : m_components) {
        comp->Draw(dc);

        // �����ѡ�е�Ԫ��������ѡ�б߿�
        if (comp->GetId() == m_selectedComponent) {
            wxPoint pos = comp->GetPosition();
            dc.SetPen(wxPen(*wxRED, 2, wxPENSTYLE_DOT));
            dc.DrawRectangle(pos.x - 5, pos.y - 5, 50, 50);
            dc.SetPen(wxPen(*wxBLACK, 1));
        }
    }
}

void Canvas::DrawWires(wxDC& dc) {
    dc.SetPen(wxPen(*wxBLUE, 1));

    for (const Wire& wire : m_wires) {
        int fromIndex = GetComponentIndex(wire.GetFromComponent());
        int toIndex = GetComponentIndex(wire.GetToComponent());

        if (fromIndex != -1 && toIndex != -1) {
            Component* fromComp = m_components[fromIndex];
            Component* toComp = m_components[toIndex];

            wxPoint fromPoint;
            if (wire.GetFromPin() == -1) {
                // �������
                fromPoint = fromComp->GetOutputPoint();
            }
            else {
                // ��������
                std::vector<wxPoint> inputs = fromComp->GetInputPoints();
                if (wire.GetFromPin() < inputs.size()) {
                    fromPoint = inputs[wire.GetFromPin()];
                }
            }

            std::vector<wxPoint> toInputs = toComp->GetInputPoints();
            if (wire.GetToPin() < toInputs.size()) {
                wxPoint toPoint = toInputs[wire.GetToPin()];
                dc.DrawLine(fromPoint, toPoint);
            }
        }
    }
}

int Canvas::FindComponentAtPoint(wxPoint point) {
    for (Component* comp : m_components) {
        if (comp->ContainsPoint(point)) {
            return comp->GetId();
        }
    }
    return -1;
}

int Canvas::GetComponentIndex(int id) {
    for (size_t i = 0; i < m_components.size(); ++i) {
        if (m_components[i]->GetId() == id) {
            return i;
        }
    }
    return -1;
}
