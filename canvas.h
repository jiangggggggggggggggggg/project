#ifndef CANVAS_H
#define CANVAS_H

#include <wx/wx.h>
#include <vector>
#include "logicgates.h"

// ������
class Wire {
private:
    int m_fromComponent;  // ��ʼԪ��ID
    int m_fromPin;        // ��ʼ����
    int m_toComponent;    // Ŀ��Ԫ��ID
    int m_toPin;          // Ŀ������

public:
    Wire(int fromComponent, int fromPin, int toComponent, int toPin);

    int GetFromComponent() const;
    int GetFromPin() const;
    int GetToComponent() const;
    int GetToPin() const;
};

// ������
class Canvas : public wxPanel {
private:
    std::vector<Component*> m_components;  // �洢����Ԫ��
    std::vector<Wire> m_wires;             // �洢��������
    int m_selectedComponent;               // ѡ�е�Ԫ��ID��-1��ʾ��ѡ��
    bool m_dragging;                       // �Ƿ�������ק
    wxPoint m_dragStart;                   // ��ק��ʼ��

public:
    Canvas(wxWindow* parent);

    // ���Ԫ��
    void AddComponent(Component* component);

    // �Ƴ�Ԫ��
    void RemoveComponent(int id);

    // �������
    void AddWire(int fromComponent, int fromPin, int toComponent, int toPin);

    // �����������
    void Clear();

    // �¼�������
    void OnPaint(wxPaintEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnRightClick(wxMouseEvent& event);

private:
    // ��������Ԫ��
    void DrawComponents(wxDC& dc);

    // ������������
    void DrawWires(wxDC& dc);

    // ����ָ��λ�õ�Ԫ��
    int FindComponentAtPoint(wxPoint point);

    // ��ȡԪ�����б��е�����
    int GetComponentIndex(int id);

    DECLARE_EVENT_TABLE()
};

#endif // CANVAS_H
