#pragma once
#include <wx/wx.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <vector>
#include <map>
#include "json.hpp"

using json = nlohmann::json;

// 自定义事件类型 - 只声明不定义
wxDECLARE_EVENT(MY_CUSTOM_EVENT, wxCommandEvent);

// 自定义 ID
enum {
    ID_SHOW_STATUSBAR = wxID_HIGHEST + 1,
    ID_SHOW_GRID,
    ID_DELETE_SELECTED,
    ID_DELETE_WIRE,
    ID_EDIT_PROPERTIES
};

// 引脚类型
enum class PinType { Input, Output };

// 引脚结构体
struct Pin {
    wxPoint position;    // 引脚位置（相对于组件原点）
    PinType type;        // 输入/输出
    wxString name;       // 引脚名称
    bool connected;      // 是否已连接
};

// 属性结构体
struct Property {
    wxString name;
    wxString value;
    wxString type; // "string", "int", "double", "bool"
};

struct Gate {
    wxString type;
    wxPoint pos;
    std::vector<Property> properties; // 属性表
    std::vector<Pin> pins;           // 引脚列表
};

// 连线端点
struct WireEndpoint {
    int gateIndex;      // 连接的组件索引
    int pinIndex;       // 连接的引脚索引
    wxPoint position;   // 端点位置（绝对坐标）
};

struct Wire {
    WireEndpoint start;
    WireEndpoint end;
    std::vector<wxPoint> path;  // 连线路径（支持折线）
    bool isSelected = false;
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

// 声明全局形状库
extern std::map<wxString, std::vector<Shape>> shapeLibrary;
// 声明全局引脚定义
extern std::map<wxString, std::vector<Pin>> pinDefinitions;

// 属性编辑对话框
class PropertyDialog : public wxDialog {
public:
    PropertyDialog(wxWindow* parent, Gate& gate);

private:
    Gate& m_gate;
    wxPropertyGrid* m_propertyGrid;

    void OnAddProperty(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
};

// JSON 加载函数
void LoadShapesFromJson(const std::string& filename);
void LoadPinDefinitions();
wxPoint GetPinPosition(const Gate& gate, int pinIndex);