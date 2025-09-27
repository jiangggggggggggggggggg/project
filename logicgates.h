#ifndef LOGICGATES_H
#define LOGICGATES_H

#include <wx/wx.h>
#include <vector>

// 基础元件类
class Component {
protected:
    wxPoint m_position;  // 位置
    wxString m_name;     // 名称
    int m_id;            // 唯一ID
    static int m_nextId; // 用于生成唯一ID

public:
    Component(wxPoint position, const wxString& name);
    virtual ~Component() = default;

    // 获取位置
    wxPoint GetPosition() const;
    void SetPosition(wxPoint pos);

    // 获取ID
    int GetId() const;

    // 获取名称
    wxString GetName() const;

    // 绘制元件
    virtual void Draw(wxDC& dc) = 0;

    // 检查点是否在元件内
    virtual bool ContainsPoint(wxPoint point) = 0;

    // 获取输入点
    virtual std::vector<wxPoint> GetInputPoints() = 0;

    // 获取输出点
    virtual wxPoint GetOutputPoint() = 0;
};

// 非门类
class NotGate : public Component {
public:
    NotGate(wxPoint position);

    void Draw(wxDC& dc) override;
    bool ContainsPoint(wxPoint point) override;
    std::vector<wxPoint> GetInputPoints() override;
    wxPoint GetOutputPoint() override;
};

// 与门类
class AndGate : public Component {
public:
    AndGate(wxPoint position);

    void Draw(wxDC& dc) override;
    bool ContainsPoint(wxPoint point) override;
    std::vector<wxPoint> GetInputPoints() override;
    wxPoint GetOutputPoint() override;
};

// 或门类
class OrGate : public Component {
public:
    OrGate(wxPoint position);

    void Draw(wxDC& dc) override;
    bool ContainsPoint(wxPoint point) override;
    std::vector<wxPoint> GetInputPoints() override;
    wxPoint GetOutputPoint() override;
};

// 与非门类
class NandGate : public Component {
public:
    NandGate(wxPoint position);

    void Draw(wxDC& dc) override;
    bool ContainsPoint(wxPoint point) override;
    std::vector<wxPoint> GetInputPoints() override;
    wxPoint GetOutputPoint() override;
};

// 或非门类
class NorGate : public Component {
public:
    NorGate(wxPoint position);

    void Draw(wxDC& dc) override;
    bool ContainsPoint(wxPoint point) override;
    std::vector<wxPoint> GetInputPoints() override;
    wxPoint GetOutputPoint() override;
};

// 异或门类
class XorGate : public Component {
public:
    XorGate(wxPoint position);

    void Draw(wxDC& dc) override;
    bool ContainsPoint(wxPoint point) override;
    std::vector<wxPoint> GetInputPoints() override;
    wxPoint GetOutputPoint() override;
};

#endif // LOGICGATES_H
