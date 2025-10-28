#pragma once
#include <wx/wx.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <vector>
#include <map>
#include "json.hpp"

using json = nlohmann::json;

// �Զ����¼����� - ֻ����������
wxDECLARE_EVENT(MY_CUSTOM_EVENT, wxCommandEvent);

// �Զ��� ID
enum {
    ID_SHOW_STATUSBAR = wxID_HIGHEST + 1,
    ID_SHOW_GRID,
    ID_DELETE_SELECTED,
    ID_DELETE_WIRE,
    ID_EDIT_PROPERTIES
};

// ���Խṹ��
struct Property {
    wxString name;
    wxString value;
    wxString type; // "string", "int", "double", "bool"
};

struct Gate {
    wxString type;
    wxPoint pos;
    std::vector<Property> properties; // ���Ա�
};

struct Wire {
    wxPoint start;
    wxPoint end;
    bool isSelected = false; // ѡ��״̬
};

enum class ShapeType { Line, Arc, Circle, Polygon, Text };

struct Shape {
    ShapeType type;
    std::vector<wxPoint> pts;
    wxPoint center;
    int radius = 0;
    int startAngle = 0;
    int endAngle = 0;
    wxString text;
};

// ����ȫ����״��
extern std::map<wxString, std::vector<Shape>> shapeLibrary;

// ���Ա༭�Ի���
class PropertyDialog : public wxDialog {
public:
    PropertyDialog(wxWindow* parent, Gate& gate);

private:
    Gate& m_gate;
    wxPropertyGrid* m_propertyGrid;

    void OnAddProperty(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
};

// JSON ���غ���
void LoadShapesFromJson(const std::string& filename);