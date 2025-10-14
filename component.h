#ifndef COMPONENT_H
#define COMPONENT_H

#include <wx/wx.h>
#include <string>
#include <vector>
#include <unordered_map>

// Ԫ������ö��
enum class ComponentType {
    // ������
    WIRE_SPLITTER,
    WIRE_PIN,
    WIRE_PROBE,
    WIRE_TUNNEL,
    WIRE_PULL_RESISTOR,
    WIRE_CLOCK,
    WIRE_CONSTANT,

    // �߼���
    GATE_NOT,
    GATE_BUFFER,
    GATE_AND,
    GATE_OR,
    GATE_NAND,
    GATE_NOR,
    GATE_XOR,
    GATE_XNOR,

    // ����Ԫ��
    ARITH_ADDER,
    ARITH_SUBTRACTOR,
    ARITH_MULTIPLIER,
    ARITH_DIVIDER,

    // �洢��
    MEMORY_D_FLIPFLOP,
    MEMORY_SR_FLIPFLOP,
    MEMORY_JK_FLIPFLOP,
    MEMORY_T_FLIPFLOP,
    MEMORY_REGISTER,

    // �������
    IO_LED,
    IO_BUTTON,
    IO_SEVEN_SEGMENT,

    // ��·ѡ����
    PLEXER_MUX,
    PLEXER_DEMUX,

    // ����
    OTHER_TEXT,
    UNKNOWN
};

// �˿�����
enum class PortType {
    INPUT,
    OUTPUT,
    BIDIRECTIONAL
};

// �˿ڽṹ
struct Port {
    wxString name;
    PortType type;
    wxPoint position;  // �����Ԫ����λ��
    bool value;        // ��ǰֵ
    Port* connection;  // ���ӵĶ˿�
};

// Ԫ������
class Component {
public:
    Component(ComponentType type, const wxString& name);
    virtual ~Component();

    // ���麯�� - ������������ʵ��
    virtual void Draw(wxDC& dc) = 0;
    virtual Component* Clone() const = 0;
    virtual std::vector<bool> Evaluate(const std::vector<bool>& inputs) = 0;

    // �麯�� - ������Ĭ��ʵ��
    virtual void UpdatePorts();
    virtual wxRect GetBoundingBox() const;

    // ��ͨ��Ա����
    void SetPosition(const wxPoint& pos);
    wxPoint GetPosition() const;
    void SetSize(const wxSize& size);
    wxSize GetSize() const;
    ComponentType GetType() const;
    wxString GetName() const;
    void SetName(const wxString& name);

    // �˿ڹ���
    void AddPort(const Port& port);
    Port* GetPort(int index);
    int GetPortCount() const;
    std::vector<Port>& GetPorts();

    // ���Թ���
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
