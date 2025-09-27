#ifndef LOGICGATES_H
#define LOGICGATES_H

#include <wx/wx.h>
#include <vector>

// ����Ԫ����
class Component {
protected:
    wxPoint m_position;  // λ��
    wxString m_name;     // ����
    int m_id;            // ΨһID
    static int m_nextId; // ��������ΨһID

public:
    Component(wxPoint position, const wxString& name);
    virtual ~Component() = default;

    // ��ȡλ��
    wxPoint GetPosition() const;
    void SetPosition(wxPoint pos);

    // ��ȡID
    int GetId() const;

    // ��ȡ����
    wxString GetName() const;

    // ����Ԫ��
    virtual void Draw(wxDC& dc) = 0;

    // �����Ƿ���Ԫ����
    virtual bool ContainsPoint(wxPoint point) = 0;

    // ��ȡ�����
    virtual std::vector<wxPoint> GetInputPoints() = 0;

    // ��ȡ�����
    virtual wxPoint GetOutputPoint() = 0;
};

// ������
class NotGate : public Component {
public:
    NotGate(wxPoint position);

    void Draw(wxDC& dc) override;
    bool ContainsPoint(wxPoint point) override;
    std::vector<wxPoint> GetInputPoints() override;
    wxPoint GetOutputPoint() override;
};

// ������
class AndGate : public Component {
public:
    AndGate(wxPoint position);

    void Draw(wxDC& dc) override;
    bool ContainsPoint(wxPoint point) override;
    std::vector<wxPoint> GetInputPoints() override;
    wxPoint GetOutputPoint() override;
};

// ������
class OrGate : public Component {
public:
    OrGate(wxPoint position);

    void Draw(wxDC& dc) override;
    bool ContainsPoint(wxPoint point) override;
    std::vector<wxPoint> GetInputPoints() override;
    wxPoint GetOutputPoint() override;
};

// �������
class NandGate : public Component {
public:
    NandGate(wxPoint position);

    void Draw(wxDC& dc) override;
    bool ContainsPoint(wxPoint point) override;
    std::vector<wxPoint> GetInputPoints() override;
    wxPoint GetOutputPoint() override;
};

// �������
class NorGate : public Component {
public:
    NorGate(wxPoint position);

    void Draw(wxDC& dc) override;
    bool ContainsPoint(wxPoint point) override;
    std::vector<wxPoint> GetInputPoints() override;
    wxPoint GetOutputPoint() override;
};

// �������
class XorGate : public Component {
public:
    XorGate(wxPoint position);

    void Draw(wxDC& dc) override;
    bool ContainsPoint(wxPoint point) override;
    std::vector<wxPoint> GetInputPoints() override;
    wxPoint GetOutputPoint() override;
};

#endif // LOGICGATES_H
