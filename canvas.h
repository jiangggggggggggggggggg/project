#ifndef CANVAS_H
#define CANVAS_H

#include <wx/wx.h>
#include <vector>
#include "logicgates.h"

// 连线类
class Wire {
private:
    int m_fromComponent;  // 起始元件ID
    int m_fromPin;        // 起始引脚
    int m_toComponent;    // 目标元件ID
    int m_toPin;          // 目标引脚

public:
    Wire(int fromComponent, int fromPin, int toComponent, int toPin);

    int GetFromComponent() const;
    int GetFromPin() const;
    int GetToComponent() const;
    int GetToPin() const;
};

// 画布类
class Canvas : public wxPanel {
private:
    std::vector<Component*> m_components;  // 存储所有元件
    std::vector<Wire> m_wires;             // 存储所有连线
    int m_selectedComponent;               // 选中的元件ID，-1表示无选中
    bool m_dragging;                       // 是否正在拖拽
    wxPoint m_dragStart;                   // 拖拽起始点

public:
    Canvas(wxWindow* parent);

    // 添加元件
    void AddComponent(Component* component);

    // 移除元件
    void RemoveComponent(int id);

    // 添加连线
    void AddWire(int fromComponent, int fromPin, int toComponent, int toPin);

    // 清除所有内容
    void Clear();

    // 事件处理函数
    void OnPaint(wxPaintEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnRightClick(wxMouseEvent& event);

private:
    // 绘制所有元件
    void DrawComponents(wxDC& dc);

    // 绘制所有连线
    void DrawWires(wxDC& dc);

    // 查找指定位置的元件
    int FindComponentAtPoint(wxPoint point);

    // 获取元件在列表中的索引
    int GetComponentIndex(int id);

    DECLARE_EVENT_TABLE()
};

#endif // CANVAS_H
