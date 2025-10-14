#ifndef LOGICGATES_H
#define LOGICGATES_H

#include <wx/wx.h>

class LogicGate {
public:
    LogicGate();
    virtual ~LogicGate() = default;

    virtual wxString GetName() const = 0;
    virtual void Draw(wxDC& dc, const wxPoint& position) = 0;
    virtual bool Evaluate() = 0;

    void SetPosition(const wxPoint& pos);
    wxPoint GetPosition() const;

protected:
    wxPoint position;
    wxSize size;
};

class AndGate : public LogicGate {
public:
    AndGate();
    wxString GetName() const override;
    void Draw(wxDC& dc, const wxPoint& position) override;
    bool Evaluate() override;
};

class OrGate : public LogicGate {
public:
    OrGate();
    wxString GetName() const override;
    void Draw(wxDC& dc, const wxPoint& position) override;
    bool Evaluate() override;
};

class NotGate : public LogicGate {
public:
    NotGate();
    wxString GetName() const override;
    void Draw(wxDC& dc, const wxPoint& position) override;
    bool Evaluate() override;
};

class XorGate : public LogicGate {
public:
    XorGate();
    wxString GetName() const override;
    void Draw(wxDC& dc, const wxPoint& position) override;
    bool Evaluate() override;
};

class NandGate : public LogicGate {
public:
    NandGate();
    wxString GetName() const override;
    void Draw(wxDC& dc, const wxPoint& position) override;
    bool Evaluate() override;
};

class NorGate : public LogicGate {
public:
    NorGate();
    wxString GetName() const override;
    void Draw(wxDC& dc, const wxPoint& position) override;
    bool Evaluate() override;
};

class XnorGate : public LogicGate {
public:
    XnorGate();
    wxString GetName() const override;
    void Draw(wxDC& dc, const wxPoint& position) override;
    bool Evaluate() override;
};

class BufferGate : public LogicGate {
public:
    BufferGate();
    wxString GetName() const override;
    void Draw(wxDC& dc, const wxPoint& position) override;
    bool Evaluate() override;
};

#endif