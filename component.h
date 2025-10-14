#ifndef COMPONENT_H
#define COMPONENT_H

#include <wx/wx.h>
#include <string>
#include <vector>
#include <unordered_map>

// 元件类型枚举
enum class ComponentType {
    // 连线类
    WIRE_SPLITTER,
    WIRE_PIN,
    WIRE_PROBE,
    WIRE_TUNNEL,
    WIRE_PULL_RESISTOR,
    WIRE_CLOCK,
    WIRE_CONSTANT,

    // 逻辑门
    GATE_NOT,
    GATE_BUFFER,
    GATE_AND,
    GATE_OR,
    GATE_NAND,
    GATE_NOR,
    GATE_XOR,
    GATE_XNOR,

    // 算术元件
    ARITH_ADDER,
    ARITH_SUBTRACTOR,
    ARITH_MULTIPLIER,
    ARITH_DIVIDER,

    // 存储器
    MEMORY_D_FLIPFLOP,
    MEMORY_SR_FLIPFLOP,
    MEMORY_JK_FLIPFLOP,
    MEMORY_T_FLIPFLOP,
    MEMORY_REGISTER,

    // 输入输出
    IO_LED,
    IO_BUTTON,
    IO_SEVEN_SEGMENT,

    // 多路选择器
    PLEXER_MUX,
    PLEXER_DEMUX,

    // 其他
    OTHER_TEXT,
    UNKNOWN
};

// 端口类型
enum class PortType {
    INPUT,
    OUTPUT,
    BIDIRECTIONAL
};

// 端口结构
struct Port {
    wxString name;
    PortType type;
    wxPoint position;  // 相对于元件的位置
    bool value;        // 当前值
    Port* connection;  // 连接的端口
};

// 元件基类
class Component {
public:
    Component(ComponentType type, const wxString& name);
    virtual ~Component();

    // 纯虚函数 - 必须在派生类实现
    virtual void Draw(wxDC& dc) = 0;
    virtual Component* Clone() const = 0;
    virtual std::vector<bool> Evaluate(const std::vector<bool>& inputs) = 0;

    // 虚函数 - 可以有默认实现
    virtual void UpdatePorts();
    virtual wxRect GetBoundingBox() const;

    // 普通成员函数
    void SetPosition(const wxPoint& pos);
    wxPoint GetPosition() const;
    void SetSize(const wxSize& size);
    wxSize GetSize() const;
    ComponentType GetType() const;
    wxString GetName() const;
    void SetName(const wxString& name);

    // 端口管理
    void AddPort(const Port& port);
    Port* GetPort(int index);
    int GetPortCount() const;
    std::vector<Port>& GetPorts();

    // 属性管理
    void SetProperty(const wxString& key, const wxString& value);
    wxString GetProperty(const wxString& key) const;
    bool HasProperty(const wxString& key) const;

protected:
    ComponentType type_;
    wxString name_;
    wxPoint position_;
    wxSize size_;
    std::vector<Port> ports_;
    std::unordered_map<wxString, wxString> properties_;
};

#endif // COMPONENT_H
