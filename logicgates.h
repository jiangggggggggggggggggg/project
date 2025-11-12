// [file name]: logicgates.h
// [file content begin]
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
    ID_EDIT_PROPERTIES,
    ID_EXPORT_NETLIST,
    ID_IMPORT_NETLIST,
    ID_GENERATE_NETLIST,
    ID_SHOW_PINS,
    ID_EXPORT_BOOKSHELF,
    ID_ADD_BREAKPOINT,  // 添加断点
    ID_DELETE_BREAKPOINT // 删除断点
};

// 属性结构体
struct Property {
    wxString name;
    wxString value;
    wxString type; // "string", "int", "double", "bool"
};

// 引脚结构体
struct Pin {
    wxString name; // 引脚名称，如 "in1", "in2", "out"
    wxPoint relativePos; // 相对于组件左上角的坐标
    bool isInput; // 是输入引脚还是输出引脚

    // 获取绝对位置
    wxPoint GetAbsolutePos(const wxPoint& gatePos) const {
        return wxPoint(gatePos.x + relativePos.x, gatePos.y + relativePos.y);
    }
};

// 断点结构体
// 修改 Breakpoint 结构，添加更多信息
struct Breakpoint {
    wxPoint position; // 断点位置
    int wireIndex;    // 所属连线索引
    double t;         // 在线段上的参数位置 (0-1)
    std::vector<int> connectedWires; // 连接的连线索引
    Pin pin;          // 将断点作为特殊引脚
    wxString breakpointId; // 断点唯一标识
    bool isOnWire;    // 是否在连线上（相对于独立断点）
};

// 网表节点连接
struct NetConnection {
    wxString componentId;
    wxString pinName;
};

// 网表节点
struct NetNode {
    wxString netName;
    std::vector<NetConnection> connections;
};

// 网表组件
struct NetComponent {
    wxString componentId;
    wxString componentType;
    wxPoint position;
    std::vector<Property> properties;
    std::vector<Pin> pins;
};

// 网表结构
struct Netlist {
    wxString designName;
    std::vector<NetComponent> components;
    std::vector<NetNode> nodes;
    wxString timestamp;
};

struct Gate {
    wxString type;
    wxPoint pos;
    std::vector<Property> properties; // 属性表
    std::vector<Pin> pins; // 引脚信息
};

struct Wire {
    wxPoint start;
    wxPoint end;
    wxString startCompId; // 起始组件ID或断点ID
    wxString startPinName; // 起始引脚名称
    wxString endCompId;   // 结束组件ID或断点ID
    wxString endPinName;  // 结束引脚名称
    bool isSelected = false; // 选中状态
    bool isConnectedToBreakpoint = false; // 是否连接到断点
};

enum class ShapeType { Line, Arc, Circle, Polygon, Text, Polyline, Rectangle };

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

// 声明全局引脚库
extern std::map<wxString, std::vector<Pin>> pinLibrary;

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
void LoadPinsFromJson(const std::string& filename);

// 硬编码初始化函数
void InitializeHardcodedShapes();
void InitializeHardcodedPins();

// 网表工具函数
Netlist GenerateNetlist(const std::vector<Gate>& gates, const std::vector<Wire>& wires);
bool ExportNetlistToFile(const wxString& filename, const Netlist& netlist);
bool ImportNetlistFromFile(const wxString& filename, Netlist& netlist);

// BookShelf格式导出函数
bool ExportBookShelfNodeFile(const wxString& filename, const Netlist& netlist);
bool ExportBookShelfNetFile(const wxString& filename, const Netlist& netlist);
bool ExportBookShelfFiles(const wxString& baseFilename, const Netlist& netlist);

// BookShelf组件尺寸映射
extern std::map<wxString, std::pair<double, double>> bookShelfComponentSizes;
// [file content end]
