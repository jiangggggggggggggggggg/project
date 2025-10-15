#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/artprov.h>
#include <wx/toolbar.h>
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <vector>
#include <stack>
#include <wx/clipbrd.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <map>
#include <cmath>
#include <fstream>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>


//      JSON   
#include "json.hpp"
using json = nlohmann::json;

//  Զ    ¼     
wxDECLARE_EVENT(MY_CUSTOM_EVENT, wxCommandEvent);
wxDEFINE_EVENT(MY_CUSTOM_EVENT, wxCommandEvent);

//  Զ    ID
enum {
    ID_SHOW_STATUSBAR = wxID_HIGHEST + 1,
    ID_SHOW_GRID,
    ID_DELETE_SELECTED,
    ID_DELETE_WIRE,
    ID_EDIT_PROPERTIES
};

//    Խṹ  
struct Property {
    wxString name;
    wxString value;
    wxString type; // "string", "int", "double", "bool"
};

struct Gate {
    wxString type;
    wxPoint pos;
    std::vector<Property> properties; //    Ա 
};

struct Wire {
    wxPoint start;
    wxPoint end;
    bool isSelected = false; // ѡ  ״̬
};

enum class ShapeType { Line, Arc, Circle, Polygon, Text };

struct Shape {
    ShapeType type;
    std::vector<wxPoint> pts;
    wxPoint center;
    int radius = 0;
    int startAngle = 0, endAngle = 0;
    wxString text;
};

static std::map<wxString, std::vector<Shape>> shapeLibrary;

// =====    Ա༭ Ի    =====
class PropertyDialog : public wxDialog {
public:
    PropertyDialog(wxWindow* parent, Gate& gate)
        : wxDialog(parent, wxID_ANY, " ༭     - " + gate.type, wxDefaultPosition, wxSize(400, 300)),
        m_gate(gate) {

        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

        //             
        m_propertyGrid = new wxPropertyGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
            wxPG_DEFAULT_STYLE | wxPG_SPLITTER_AUTO_CENTER);
        m_propertyGrid->SetColumnProportion(0, 0.4);
        m_propertyGrid->SetColumnProportion(1, 0.6);

        //    ӻ       
        m_propertyGrid->Append(new wxPropertyCategory("基本属性"));
        m_propertyGrid->Append(new wxIntProperty("X坐标", wxPG_LABEL, m_gate.pos.x));
        m_propertyGrid->Append(new wxIntProperty("Y坐标", wxPG_LABEL, m_gate.pos.y));

        //      Զ       
        if (!m_gate.properties.empty()) {
            m_propertyGrid->Append(new wxPropertyCategory("自定义属性"));

            for (auto& prop : m_gate.properties) {
                if (prop.type == "int") {
                    long value;
                    prop.value.ToLong(&value);
                    m_propertyGrid->Append(new wxIntProperty(prop.name, wxPG_LABEL, value));
                }
                else if (prop.type == "double") {
                    double value;
                    prop.value.ToDouble(&value);
                    m_propertyGrid->Append(new wxFloatProperty(prop.name, wxPG_LABEL, value));
                }
                else if (prop.type == "bool") {
                    bool value = prop.value == "true" || prop.value == "1";
                    m_propertyGrid->Append(new wxBoolProperty(prop.name, wxPG_LABEL, value));
                }
                else {
                    m_propertyGrid->Append(new wxStringProperty(prop.name, wxPG_LABEL, prop.value));
                }
            }
        }

        //          ԰ ť
        wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
        wxButton* addButton = new wxButton(this, wxID_ANY, "添加属性");
        buttonSizer->Add(addButton, 0, wxALL, 5);

        // ȷ  ȡ    ť
        wxStdDialogButtonSizer* stdBtnSizer = new wxStdDialogButtonSizer();
        wxButton* okButton = new wxButton(this, wxID_OK, "确定");
        wxButton* cancelButton = new wxButton(this, wxID_CANCEL, "取消");
        stdBtnSizer->AddButton(okButton);
        stdBtnSizer->AddButton(cancelButton);
        stdBtnSizer->Realize();

        mainSizer->Add(m_propertyGrid, 1, wxEXPAND | wxALL, 5);
        mainSizer->Add(buttonSizer, 0, wxALIGN_LEFT | wxLEFT | wxBOTTOM, 5);
        mainSizer->Add(stdBtnSizer, 0, wxALIGN_CENTER | wxALL, 5);

        SetSizer(mainSizer);

        //    ¼ 
        addButton->Bind(wxEVT_BUTTON, &PropertyDialog::OnAddProperty, this);
        okButton->Bind(wxEVT_BUTTON, &PropertyDialog::OnOK, this);
    }

private:
    Gate& m_gate;
    wxPropertyGrid* m_propertyGrid;

    void OnAddProperty(wxCommandEvent& event) {
        wxTextEntryDialog dialog(this, "请输入属性名称:", "添加属性");
        if (dialog.ShowModal() == wxID_OK) {
            wxString name = dialog.GetValue();
            if (!name.IsEmpty()) {
                m_propertyGrid->Append(new wxStringProperty(name, wxPG_LABEL, ""));
            }
        }
    }

    void OnOK(wxCommandEvent& event) {
        //    »       
        wxVariant xVar = m_propertyGrid->GetPropertyValue("X坐标");
        wxVariant yVar = m_propertyGrid->GetPropertyValue("Y坐标");

        m_gate.pos.x = xVar.GetLong();
        m_gate.pos.y = yVar.GetLong();

        //      Զ       
        m_gate.properties.clear();

        wxPropertyGridIterator it;
        for (it = m_propertyGrid->GetIterator(); !it.AtEnd(); it++) {
            wxPGProperty* prop = *it;
            wxString name = prop->GetName();

            //             
            if (name == "X坐标" || name == "Y坐标") {
                continue;
            }

            //            
            if (prop->IsCategory()) {
                continue;
            }

            Property newProp;
            newProp.name = name;
            newProp.value = prop->GetValueAsString();

            //  ж         
            if (prop->IsKindOf(wxCLASSINFO(wxIntProperty))) {
                newProp.type = "int";
            }
            else if (prop->IsKindOf(wxCLASSINFO(wxFloatProperty))) {
                newProp.type = "double";
            }
            else if (prop->IsKindOf(wxCLASSINFO(wxBoolProperty))) {
                newProp.type = "bool";
            }
            else {
                newProp.type = "string";
            }

            m_gate.properties.push_back(newProp);
        }

        EndModal(wxID_OK);
    }
};

// ===== JSON    غ    (     C++14) =====
void LoadShapesFromJson(const std::string& filename) {
    std::ifstream f(filename.c_str());
    if (!f.is_open()) {
        wxLogError("无法打开图形文件: %s", filename);
        return;
    }
    json j;
    f >> j;

    for (json::iterator it = j.begin(); it != j.end(); ++it) {
        std::string gateName = it.key();
        auto shapes = it.value();

        std::vector<Shape> vec;
        for (size_t i = 0; i < shapes.size(); ++i) {
            auto s = shapes[i];
            Shape shape;
            std::string type = s["type"];
            if (type == "Line") shape.type = ShapeType::Line;
            else if (type == "Arc") shape.type = ShapeType::Arc;
            else if (type == "Circle") shape.type = ShapeType::Circle;
            else if (type == "Polygon") shape.type = ShapeType::Polygon;
            else if (type == "Text") shape.type = ShapeType::Text;

            if (s.find("pts") != s.end()) {
                for (size_t pi = 0; pi < s["pts"].size(); ++pi) {
                    auto p = s["pts"][pi];
                    shape.pts.push_back(wxPoint(p[0], p[1]));
                }
            }
            if (s.find("center") != s.end()) {
                shape.center = wxPoint(s["center"][0], s["center"][1]);
            }
            if (s.find("radius") != s.end()) shape.radius = s["radius"];
            if (s.find("startAngle") != s.end()) shape.startAngle = s["startAngle"];
            if (s.find("endAngle") != s.end()) shape.endAngle = s["endAngle"];
            if (s.find("text") != s.end()) shape.text = s["text"].get<std::string>().c_str();

            vec.push_back(shape);
        }
        shapeLibrary[wxString::FromUTF8(gateName.c_str())] = vec;
    }
}

// =====   ͼ     =====
class MyDrawPanel : public wxPanel {
public:
    MyDrawPanel(wxWindow* parent)
        : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE),
        m_scale(1.0), m_isDrawingWire(false),
        m_isDraggingGate(false), m_draggedIndex(-1),
        m_selectedIndex(-1), m_selectedWireIndex(-1), m_showGrid(true), m_gridSize(20)
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        Bind(wxEVT_PAINT, &MyDrawPanel::OnPaint, this);
        Bind(wxEVT_LEFT_DOWN, &MyDrawPanel::OnMouseDown, this);
        Bind(wxEVT_LEFT_UP, &MyDrawPanel::OnMouseUp, this);
        Bind(wxEVT_RIGHT_DOWN, &MyDrawPanel::OnRightClick, this);
        Bind(wxEVT_MOTION, &MyDrawPanel::OnMouseMove, this);
        Bind(wxEVT_KEY_DOWN, &MyDrawPanel::OnKeyDown, this);

        if (shapeLibrary.empty()) {
            LoadShapesFromJson("D:/学习/综设一/Project1/x64/Debug/shapes.json");
        }

        SetFocus(); // ȷ     Խ  ռ    ¼ 
    }

    void AddShape(const wxString& shape) {
        Gate newGate;
        newGate.type = shape;
        newGate.pos = wxPoint(50 + (m_gates.size() % 3) * 150, 50 + (m_gates.size() / 3) * 150);

        // Ϊ  ͬ           Ĭ      
        if (shape == "LED") {
            Property colorProp;
            colorProp.name = "颜色";
            colorProp.value = "红色";
            colorProp.type = "string";
            newGate.properties.push_back(colorProp);

            Property voltageProp;
            voltageProp.name = "工作电压";
            voltageProp.value = "3.3";
            voltageProp.type = "double";
            newGate.properties.push_back(voltageProp);
        }
        else if (shape == "开关") {
            Property stateProp;
            stateProp.name = "初始状态";
            stateProp.value = "关闭";
            stateProp.type = "string";
            newGate.properties.push_back(stateProp);
        }
        else if (shape == "AND" || shape == "OR" || shape == "NOT" ||
            shape == "XOR" || shape == "NAND" || shape == "NOR" ||
            shape == "XNOR" || shape == "BUFFER") {
            Property delayProp;
            delayProp.name = "传播延迟";
            delayProp.value = "10";
            delayProp.type = "int";
            newGate.properties.push_back(delayProp);
        }

        m_gates.push_back(newGate);
        m_selectedIndex = (int)m_gates.size() - 1;
        m_selectedWireIndex = -1; // ȡ      ѡ  

        // ֪ͨ     ڱ   ״̬   ڳ   
        wxCommandEvent evt(MY_CUSTOM_EVENT);
        evt.SetEventObject(this);
        wxPostEvent(GetParent(), evt);

        Refresh();
    }

    void RemoveLastShape() {
        if (!m_gates.empty()) {
            m_gates.pop_back();
            m_selectedIndex = -1;
        }
        Refresh();
    }

    void DeleteSelected() {
        if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_gates.size()) {
            // ֪ͨ     ڱ   ״̬   ڳ   
            wxCommandEvent evt(MY_CUSTOM_EVENT);
            evt.SetEventObject(this);
            wxPostEvent(GetParent(), evt);

            m_gates.erase(m_gates.begin() + m_selectedIndex);
            m_selectedIndex = -1;
            Refresh();
        }
    }

    void DeleteSelectedWire() {
        if (m_selectedWireIndex >= 0 && m_selectedWireIndex < (int)m_wires.size()) {
            // ֪ͨ     ڱ   ״̬   ڳ   
            wxCommandEvent evt(MY_CUSTOM_EVENT);
            evt.SetEventObject(this);
            wxPostEvent(GetParent(), evt);

            m_wires.erase(m_wires.begin() + m_selectedWireIndex);
            m_selectedWireIndex = -1;
            Refresh();
        }
    }

    void EditSelectedProperties() {
        if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_gates.size()) {
            // ֪ͨ     ڱ   ״̬   ڳ   
            wxCommandEvent evt(MY_CUSTOM_EVENT);
            evt.SetEventObject(this);
            wxPostEvent(GetParent(), evt);

            PropertyDialog dialog(this, m_gates[m_selectedIndex]);
            if (dialog.ShowModal() == wxID_OK) {
                Refresh();
            }
        }
    }

    void ClearShapes() {
        // ֪ͨ     ڱ   ״̬   ڳ   
        wxCommandEvent evt(MY_CUSTOM_EVENT);
        evt.SetEventObject(this);
        wxPostEvent(GetParent(), evt);

        m_gates.clear();
        m_wires.clear();
        m_selectedIndex = -1;
        m_selectedWireIndex = -1;
        Refresh();
    }

    void ToggleGrid() {
        m_showGrid = !m_showGrid;
        Refresh();
    }

    bool IsGridVisible() const { return m_showGrid; }

    //   ȡ  ǰ״̬     ڳ         
    struct DrawPanelState {
        std::vector<Gate> gates;
        std::vector<Wire> wires;
        int selectedIndex;
        int selectedWireIndex;
    };

    DrawPanelState GetState() const {
        DrawPanelState state;
        state.gates = m_gates;
        state.wires = m_wires;
        state.selectedIndex = m_selectedIndex;
        state.selectedWireIndex = m_selectedWireIndex;
        return state;
    }

    void SetState(const DrawPanelState& state) {
        m_gates = state.gates;
        m_wires = state.wires;
        m_selectedIndex = state.selectedIndex;
        m_selectedWireIndex = state.selectedWireIndex;
        Refresh();
    }

    std::vector<wxString> GetShapes() const {
        std::vector<wxString> names;
        for (auto& g : m_gates) names.push_back(g.type);
        return names;
    }

    void SetShapes(const std::vector<wxString>& shapes) {
        m_gates.clear();
        int i = 0;
        for (auto& s : shapes) {
            Gate newGate;
            newGate.type = s;
            newGate.pos = wxPoint(50 + (i % 3) * 150, 50 + (i / 3) * 150);
            m_gates.push_back(newGate);
            i++;
        }
        m_selectedIndex = -1;
        m_selectedWireIndex = -1;
        Refresh();
    }

    //   ȡ     ż      ԣ    ڱ  棩
    std::vector<Gate> GetGates() const {
        return m_gates;
    }

    //          ż      ԣ    ڼ  أ 
    void SetGates(const std::vector<Gate>& gates) {
        m_gates = gates;
        m_selectedIndex = -1;
        m_selectedWireIndex = -1;
        Refresh();
    }

    void ZoomIn() { m_scale *= 1.2; Refresh(); }
    void ZoomOut() { m_scale /= 1.2; if (m_scale < 0.2) m_scale = 0.2; Refresh(); }

private:
    std::vector<Gate> m_gates;
    std::vector<Wire> m_wires;
    double m_scale;

    //        
    bool m_isDrawingWire;
    wxPoint m_wireStart;
    wxPoint m_currentMouse;

    //   ק   
    bool m_isDraggingGate;
    int m_draggedIndex;
    wxPoint m_dragOffset;

    //         
    int m_selectedIndex;
    int m_selectedWireIndex;
    bool m_showGrid;
    int m_gridSize;

    // ---------- ͨ û    ----------
    void DrawGate(wxDC& dc, const Gate& gate) {
        auto it = shapeLibrary.find(gate.type);
        if (it == shapeLibrary.end()) {
            dc.DrawText("未知组件", gate.pos);
            return;
        }

        // ֱ ӻ   
        for (auto& s : it->second) {
            DrawShape(dc, s, gate.pos);
        }

        //          ı       У 
        if (!gate.properties.empty()) {
            wxFont smallFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
            dc.SetFont(smallFont);
            dc.SetTextForeground(wxColour(100, 100, 100));

            int yOffset = 70; //       ·   ʾ    
            for (const auto& prop : gate.properties) {
                wxString propText = wxString::Format("%s: %s", prop.name, prop.value);
                dc.DrawText(propText, gate.pos.x, gate.pos.y + yOffset);
                yOffset += 12;
            }

            //  ָ Ĭ         ɫ
            dc.SetFont(wxNullFont);
            dc.SetTextForeground(*wxBLACK);
        }
    }

    void DrawShape(wxDC& dc, const Shape& s, const wxPoint& pos) {
        if (s.type == ShapeType::Line && s.pts.size() >= 2) {
            dc.DrawLine(pos.x + s.pts[0].x, pos.y + s.pts[0].y,
                pos.x + s.pts[1].x, pos.y + s.pts[1].y);
        }
        else if (s.type == ShapeType::Polygon && s.pts.size() >= 3) {
            std::vector<wxPoint> pts;
            for (auto& p : s.pts) pts.push_back(wxPoint(pos.x + p.x, pos.y + p.y));
            dc.DrawPolygon((int)pts.size(), &pts[0]);
        }
        else if (s.type == ShapeType::Circle) {
            dc.DrawCircle(pos.x + s.center.x, pos.y + s.center.y, s.radius);
        }
        else if (s.type == ShapeType::Arc) {
            dc.DrawEllipticArc(pos.x + s.center.x - s.radius, pos.y + s.center.y - s.radius,
                2 * s.radius, 2 * s.radius, s.startAngle, s.endAngle);
        }
        else if (s.type == ShapeType::Text) {
            dc.DrawText(s.text, pos.x + s.center.x, pos.y + s.center.y);
        }
    }

    void DrawGrid(wxDC& dc) {
        if (!m_showGrid) return;

        wxSize size = GetClientSize();
        wxPen gridPen(wxColour(220, 220, 220), 1, wxPENSTYLE_DOT);
        dc.SetPen(gridPen);

        int logicalWidth = (int)(size.x / m_scale);
        int logicalHeight = (int)(size.y / m_scale);

        for (int x = 0; x < logicalWidth; x += m_gridSize) {
            dc.DrawLine(x, 0, x, logicalHeight);
        }
        for (int y = 0; y < logicalHeight; y += m_gridSize) {
            dc.DrawLine(0, y, logicalWidth, y);
        }

        dc.SetPen(*wxBLACK_PEN);
    }

    wxRect GetGateBBox(const Gate& g) const {
        int w = 80, h = 60;
        if (g.type == "NOT" || g.type == "BUFFER") { w = 70; h = 60; }
        else if (g.type == "LED") { w = 80; h = 90; }
        return wxRect(g.pos.x, g.pos.y, w, h);
    }

    //      Ƿ           
    bool IsPointNearLine(const wxPoint& point, const Wire& wire, int tolerance = 5) {
        //     㵽 ߶εľ   
        wxPoint p = point;
        wxPoint a = wire.start;
        wxPoint b = wire.end;

        //  ߶γ  ȵ ƽ  
        double length2 = (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
        if (length2 == 0) {
            //  ߶  ˻ Ϊ  
            return (p.x - a.x) * (p.x - a.x) + (p.y - a.y) * (p.y - a.y) <= tolerance * tolerance;
        }

        //     ͶӰ    
        double t = std::max(0.0, std::min(1.0,
            ((p.x - a.x) * (b.x - a.x) + (p.y - a.y) * (b.y - a.y)) / length2));

        //     ͶӰ  
        wxPoint projection(
            a.x + t * (b.x - a.x),
            a.y + t * (b.y - a.y)
        );

        //        ƽ  
        double distance2 = (p.x - projection.x) * (p.x - projection.x) +
            (p.y - projection.y) * (p.y - projection.y);

        return distance2 <= tolerance * tolerance;
    }

    wxPoint ToLogical(const wxPoint& devicePt) const {
        double lx = devicePt.x / m_scale;
        double ly = devicePt.y / m_scale;
        return wxPoint(int(lx + 0.5), int(ly + 0.5));
    }

    wxPoint SnapToGrid(const wxPoint& point) {
        if (!m_showGrid) return point;
        int x = (point.x + m_gridSize / 2) / m_gridSize * m_gridSize;
        int y = (point.y + m_gridSize / 2) / m_gridSize * m_gridSize;
        return wxPoint(x, y);
    }

    void OnPaint(wxPaintEvent&) {
        wxAutoBufferedPaintDC dc(this);
        dc.Clear();
        dc.SetUserScale(m_scale, m_scale);

        //         
        DrawGrid(dc);

        wxPen redPen(wxColour(200, 0, 0), 2, wxPENSTYLE_SOLID);
        wxPen dashPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SHORT_DASH);
        wxPen selectionPen(wxColour(0, 100, 200), 2, wxPENSTYLE_SOLID);
        wxPen wireSelectionPen(wxColour(255, 0, 0), 3, wxPENSTYLE_SOLID); // ѡ       ú ɫ    

        dc.SetPen(*wxBLACK_PEN);

        //        
        for (size_t i = 0; i < m_gates.size(); ++i) {
            auto& g = m_gates[i];
            DrawGate(dc, g);

            //     ѡ  ״̬
            if ((int)i == m_selectedIndex) {
                wxRect r = GetGateBBox(g);
                dc.SetPen(selectionPen);
                dc.SetBrush(*wxTRANSPARENT_BRUSH);
                dc.DrawRectangle(r);
                dc.SetPen(*wxBLACK_PEN);
            }

            //       ק״̬
            if (m_isDraggingGate && (int)i == m_draggedIndex) {
                wxRect r = GetGateBBox(g);
                dc.SetPen(dashPen);
                dc.SetBrush(*wxTRANSPARENT_BRUSH);
                dc.DrawRectangle(r);
                dc.SetPen(*wxBLACK_PEN);
            }
        }

        //         
        for (size_t i = 0; i < m_wires.size(); ++i) {
            auto& w = m_wires[i];
            if ((int)i == m_selectedWireIndex) {
                dc.SetPen(wireSelectionPen);
            }
            else {
                dc.SetPen(*wxBLACK_PEN);
            }
            dc.DrawLine(w.start, w.end);
            dc.SetPen(*wxBLACK_PEN);
        }

        //        ڻ      (预览以折线形式显示)
        if (m_isDrawingWire) {
            dc.SetPen(redPen);

            // 计算折线的拐点（优先较长方向先画直线）
            wxPoint midPoint;
            if (std::abs(m_currentMouse.x - m_wireStart.x) > std::abs(m_currentMouse.y - m_wireStart.y)) {
                // 先水平再垂直
                midPoint = wxPoint(m_currentMouse.x, m_wireStart.y);
            }
            else {
                // 先垂直再水平
                midPoint = wxPoint(m_wireStart.x, m_currentMouse.y);
            }
            dc.DrawLine(m_wireStart, midPoint);
            dc.DrawLine(midPoint, m_currentMouse);

            dc.SetPen(*wxBLACK_PEN);
        }
    }

    void OnMouseDown(wxMouseEvent& evt) {
        wxPoint pos = ToLogical(evt.GetPosition());
        if (m_showGrid) {
            pos = SnapToGrid(pos);
        }

        //  ȼ   Ƿ         
        m_selectedWireIndex = -1;
        for (int i = (int)m_wires.size() - 1; i >= 0; --i) {
            if (IsPointNearLine(pos, m_wires[i])) {
                m_selectedWireIndex = i;
                m_selectedIndex = -1; // ȡ     ѡ  
                Refresh();
                return;
            }
        }

        //     Ƿ        
        int hitIndex = -1;
        for (int i = (int)m_gates.size() - 1; i >= 0; --i) {
            if (GetGateBBox(m_gates[i]).Contains(pos)) {
                hitIndex = i;
                break;
            }
        }

        if (hitIndex != -1) {
            if (evt.LeftDown()) {
                m_isDraggingGate = true;
                m_draggedIndex = hitIndex;
                m_selectedIndex = hitIndex;
                m_selectedWireIndex = -1; // ȡ      ѡ  
                m_dragOffset = wxPoint(pos.x - m_gates[hitIndex].pos.x, pos.y - m_gates[hitIndex].pos.y);
                if (!HasCapture()) CaptureMouse();
                SetCursor(wxCursor(wxCURSOR_SIZING));
            }
        }
        else {
            if (evt.LeftDown()) {
                m_isDrawingWire = true;
                m_wireStart = pos; // 已经贴齐网格（如果网格打开）
                m_currentMouse = pos;
                m_selectedIndex = -1;
                m_selectedWireIndex = -1;
                if (!HasCapture()) CaptureMouse();
                SetCursor(wxCursor(wxCURSOR_CROSS));
            }
        }
        Refresh();
    }

    void OnRightClick(wxMouseEvent& evt) {
        wxPoint pos = ToLogical(evt.GetPosition());

        //     Ƿ  Ҽ          
        int hitWireIndex = -1;
        for (int i = (int)m_wires.size() - 1; i >= 0; --i) {
            if (IsPointNearLine(pos, m_wires[i])) {
                hitWireIndex = i;
                break;
            }
        }

        if (hitWireIndex != -1) {
            m_selectedWireIndex = hitWireIndex;
            m_selectedIndex = -1;

            //   ʾ     Ҽ  ˵ 
            wxMenu menu;
            menu.Append(ID_DELETE_WIRE, "删除线条");
            PopupMenu(&menu);
            return;
        }

        //     Ƿ  Ҽ         
        int hitIndex = -1;
        for (int i = (int)m_gates.size() - 1; i >= 0; --i) {
            if (GetGateBBox(m_gates[i]).Contains(pos)) {
                hitIndex = i;
                break;
            }
        }

        m_selectedIndex = hitIndex;
        m_selectedWireIndex = -1;
        Refresh();

        //   ʾ    Ҽ  ˵ 
        if (hitIndex != -1) {
            wxMenu menu;
            menu.Append(ID_EDIT_PROPERTIES, "编辑属性");
            menu.AppendSeparator();
            menu.Append(ID_DELETE_SELECTED, "删除组件");

            PopupMenu(&menu);
        }
    }

    void OnMouseMove(wxMouseEvent& evt) {
        wxPoint pos = ToLogical(evt.GetPosition());
        if (m_showGrid) {
            pos = SnapToGrid(pos);
        }

        if (m_isDraggingGate && evt.Dragging() && evt.LeftIsDown()) {
            if (m_draggedIndex >= 0 && m_draggedIndex < (int)m_gates.size()) {
                m_gates[m_draggedIndex].pos = wxPoint(pos.x - m_dragOffset.x, pos.y - m_dragOffset.y);
                Refresh();
            }
        }
        else if (m_isDrawingWire && evt.Dragging() && evt.LeftIsDown()) {
            m_currentMouse = pos; // 实时更新，已贴齐网格
            Refresh();
        }
    }

    void OnMouseUp(wxMouseEvent&) {
        if (m_isDraggingGate) {
            m_isDraggingGate = false;
            m_draggedIndex = -1;
            if (HasCapture()) ReleaseMouse();
            SetCursor(wxCursor(wxCURSOR_ARROW));

            // ֪ͨ     ڱ   ״̬   ڳ       ק      
            wxCommandEvent evt(MY_CUSTOM_EVENT);
            evt.SetEventObject(this);
            wxPostEvent(GetParent(), evt);

            Refresh();
        }
        else if (m_isDrawingWire) {
            // ֪ͨ     ڱ   ״̬   ڳ        ߿ ʼǰ  
            wxCommandEvent evt(MY_CUSTOM_EVENT);
            evt.SetEventObject(this);
            wxPostEvent(GetParent(), evt);

            // 计算拐点并保存为两段线（保持垂直+水平）
            wxPoint midPoint;
            if (std::abs(m_currentMouse.x - m_wireStart.x) > std::abs(m_currentMouse.y - m_wireStart.y)) {
                midPoint = wxPoint(m_currentMouse.x, m_wireStart.y);
            }
            else {
                midPoint = wxPoint(m_wireStart.x, m_currentMouse.y);
            }

            m_wires.push_back({ m_wireStart, midPoint });
            m_wires.push_back({ midPoint, m_currentMouse });

            m_isDrawingWire = false;
            if (HasCapture()) ReleaseMouse();
            SetCursor(wxCursor(wxCURSOR_ARROW));
            Refresh();
        }
    }

    void OnKeyDown(wxKeyEvent& evt) {
        switch (evt.GetKeyCode()) {
        case WXK_DELETE:
        case WXK_BACK:
            if (m_selectedIndex >= 0) {
                DeleteSelected();
            }
            else if (m_selectedWireIndex >= 0) {
                DeleteSelectedWire();
            }
            break;
        case 'P':
        case 'p':
            if (evt.ControlDown()) {
                EditSelectedProperties();
            }
            break;
        default:
            evt.Skip();
            break;
        }
    }
};

// =====        =====
class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);
private:
    MyDrawPanel* m_drawPanel;
    wxTreeCtrl* m_treeCtrl;
    wxSplitterWindow* m_splitter;
    wxString m_currentFile;

    //         ջ
    std::stack<MyDrawPanel::DrawPanelState> undoStack;
    std::stack<MyDrawPanel::DrawPanelState> redoStack;

    void OnNew(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);

    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);
    void OnCopy(wxCommandEvent& event);
    void OnPaste(wxCommandEvent& event);

    void OnZoomin(wxCommandEvent& event);
    void OnZoomout(wxCommandEvent& event);
    void OnToggleStatusBar(wxCommandEvent& event);
    void OnToggleGrid(wxCommandEvent& event);
    void OnDeleteSelected(wxCommandEvent& event);
    void OnDeleteWire(wxCommandEvent& event);
    void OnEditProperties(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    //    浱ǰ״̬      ջ
    void SaveStateForUndo();

    //  Զ    ¼     
    void OnCustomEvent(wxCommandEvent& event);

    void DoSave(const wxString& filename);
    void UpdateTitle();

    wxDECLARE_EVENT_TABLE();
};

// =====  ¼    =====
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(wxID_NEW, MyFrame::OnNew)
EVT_MENU(wxID_OPEN, MyFrame::OnOpen)
EVT_MENU(wxID_SAVE, MyFrame::OnSave)
EVT_MENU(wxID_SAVEAS, MyFrame::OnSaveAs)
EVT_MENU(wxID_EXIT, MyFrame::OnQuit)

EVT_MENU(wxID_UNDO, MyFrame::OnUndo)
EVT_MENU(wxID_REDO, MyFrame::OnRedo)
EVT_MENU(wxID_COPY, MyFrame::OnCopy)
EVT_MENU(wxID_PASTE, MyFrame::OnPaste)

EVT_MENU(wxID_ZOOM_IN, MyFrame::OnZoomin)
EVT_MENU(wxID_ZOOM_OUT, MyFrame::OnZoomout)
EVT_MENU(ID_SHOW_STATUSBAR, MyFrame::OnToggleStatusBar)
EVT_MENU(ID_SHOW_GRID, MyFrame::OnToggleGrid)
EVT_MENU(ID_DELETE_SELECTED, MyFrame::OnDeleteSelected)
EVT_MENU(ID_DELETE_WIRE, MyFrame::OnDeleteWire)
EVT_MENU(ID_EDIT_PROPERTIES, MyFrame::OnEditProperties)

EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)

//  Զ    ¼   
EVT_COMMAND(wxID_ANY, MY_CUSTOM_EVENT, MyFrame::OnCustomEvent)
wxEND_EVENT_TABLE()

// =====       ʵ   =====
MyFrame::MyFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1000, 650))
{
    //  ˵   
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(wxID_NEW, "&New\tCtrl-N");
    menuFile->Append(wxID_OPEN, "&Open...\tCtrl-O");
    menuFile->Append(wxID_SAVE, "&Save\tCtrl-S");
    menuFile->Append(wxID_SAVEAS, "Save &As...");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT, "E&xit\tAlt-F4");

    wxMenu* menuEdit = new wxMenu;
    menuEdit->Append(wxID_UNDO, "&Undo\tCtrl-Z");
    menuEdit->Append(wxID_REDO, "&Redo\tCtrl-Y");
    menuEdit->AppendSeparator();
    menuEdit->Append(wxID_COPY, "&Copy\tCtrl-C");
    menuEdit->Append(wxID_PASTE, "&Paste\tCtrl-V");
    menuEdit->AppendSeparator();
    menuEdit->Append(ID_EDIT_PROPERTIES, "编辑属性\tCtrl-P");
    menuEdit->Append(ID_DELETE_SELECTED, "删除组件\tDel");
    menuEdit->Append(ID_DELETE_WIRE, "删除线条\tShift-Del");

    wxMenu* menuView = new wxMenu;
    menuView->Append(wxID_ZOOM_IN, "Zoom &In\tCtrl-+");
    menuView->Append(wxID_ZOOM_OUT, "Zoom &Out\tCtrl--");
    menuView->AppendCheckItem(ID_SHOW_STATUSBAR, "Show Status Bar")->Check(true);
    menuView->AppendCheckItem(ID_SHOW_GRID, "Show &Grid\tCtrl-G")->Check(true);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT, "&About\tF1");

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuEdit, "&Edit");
    menuBar->Append(menuView, "&View");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("准备就绪 - 新增功能: 属性表(Ctrl+P), 线条删除(右键/Shift-Del), 网格对齐(Ctrl+G), 组件选择(右键)");

    //       
    wxToolBar* toolBar = CreateToolBar();
    toolBar->AddTool(wxID_NEW, "新建", wxArtProvider::GetBitmap(wxART_NEW, wxART_TOOLBAR));
    toolBar->AddTool(wxID_OPEN, "打开", wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR));
    toolBar->AddTool(wxID_SAVE, "保存", wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR));
    toolBar->AddSeparator();
    toolBar->AddTool(wxID_UNDO, "撤销", wxArtProvider::GetBitmap(wxART_UNDO, wxART_TOOLBAR));
    toolBar->AddTool(wxID_REDO, "重做", wxArtProvider::GetBitmap(wxART_REDO, wxART_TOOLBAR));
    toolBar->AddSeparator();
    toolBar->AddTool(wxID_ZOOM_IN, "放大", wxArtProvider::GetBitmap(wxART_PLUS, wxART_TOOLBAR));
    toolBar->AddTool(wxID_ZOOM_OUT, "缩小", wxArtProvider::GetBitmap(wxART_MINUS, wxART_TOOLBAR));
    toolBar->AddSeparator();
    toolBar->AddTool(ID_DELETE_SELECTED, "删除组件", wxArtProvider::GetBitmap(wxART_DELETE, wxART_TOOLBAR));
    toolBar->AddTool(ID_DELETE_WIRE, "删除线条", wxArtProvider::GetBitmap(wxART_CUT, wxART_TOOLBAR));
    toolBar->Realize();

    //  ָ  
    m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_splitter->SetMinimumPaneSize(100);

    //       οؼ 
    m_treeCtrl = new wxTreeCtrl(m_splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE);
    wxTreeItemId root = m_treeCtrl->AddRoot("组件库");
    m_treeCtrl->AppendItem(root, "AND");
    m_treeCtrl->AppendItem(root, "OR");
    m_treeCtrl->AppendItem(root, "NOT");
    m_treeCtrl->AppendItem(root, "XOR");
    m_treeCtrl->AppendItem(root, "NAND");
    m_treeCtrl->AppendItem(root, "NOR");
    m_treeCtrl->AppendItem(root, "XNOR");
    m_treeCtrl->AppendItem(root, "BUFFER");
    m_treeCtrl->AppendItem(root, "LED");
    m_treeCtrl->AppendItem(root, "开关");
    m_treeCtrl->Expand(root);

    //  Ҳ  ͼ   
    m_drawPanel = new MyDrawPanel(m_splitter);

    m_splitter->SplitVertically(m_treeCtrl, m_drawPanel, 200);

    //      οؼ  ¼ 
    m_treeCtrl->Bind(wxEVT_TREE_ITEM_ACTIVATED, [this](wxTreeEvent& evt) {
        wxString itemText = m_treeCtrl->GetItemText(evt.GetItem());
        if (itemText != "组件库") {
            //     ״̬   ڳ   
            SaveStateForUndo();
            m_drawPanel->AddShape(itemText);
        }
        });

    UpdateTitle();
}

void MyFrame::SaveStateForUndo() {
    //    浱ǰ״̬      ջ
    undoStack.push(m_drawPanel->GetState());
    //        ջ
    while (!redoStack.empty()) redoStack.pop();

    //    ²˵ ״̬
    wxMenuBar* mb = GetMenuBar();
    if (mb) {
        mb->Enable(wxID_UNDO, !undoStack.empty());
        mb->Enable(wxID_REDO, !redoStack.empty());
    }
}

void MyFrame::OnCustomEvent(wxCommandEvent& event) {
    //      Զ    ¼       ״̬   ڳ   
    SaveStateForUndo();
}

void MyFrame::OnNew(wxCommandEvent& event) {
    SaveStateForUndo();
    m_drawPanel->ClearShapes();
    m_currentFile.clear();
    UpdateTitle();
}

void MyFrame::OnOpen(wxCommandEvent& event) {
    SaveStateForUndo();

    wxFileDialog openFileDialog(this, "打开电路图", "", "",
        "Circuit files (*.json)|*.json", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL) return;

    wxString filename = openFileDialog.GetPath();
    std::ifstream f(filename.ToStdString());
    if (!f.is_open()) {
        wxLogError("无法打开文件: %s", filename);
        return;
    }

    try {
        json j;
        f >> j;

        std::vector<Gate> gates;
        for (auto& gateJson : j["gates"]) {
            Gate gate;
            gate.type = gateJson["type"].get<std::string>();
            gate.pos.x = gateJson["x"];
            gate.pos.y = gateJson["y"];

            //         
            if (gateJson.find("properties") != gateJson.end()) {
                for (auto& propJson : gateJson["properties"]) {
                    Property prop;
                    prop.name = propJson["name"].get<std::string>();
                    prop.value = propJson["value"].get<std::string>();
                    prop.type = propJson["type"].get<std::string>();
                    gate.properties.push_back(prop);
                }
            }

            gates.push_back(gate);
        }

        m_drawPanel->SetGates(gates);
        m_currentFile = filename;
        UpdateTitle();
    }
    catch (const std::exception& e) {
        wxLogError("解析文件失败: %s", e.what());
    }
}

void MyFrame::OnSave(wxCommandEvent& event) {
    if (m_currentFile.empty()) {
        OnSaveAs(event);
    }
    else {
        DoSave(m_currentFile);
    }
}

void MyFrame::OnSaveAs(wxCommandEvent& event) {
    wxFileDialog saveFileDialog(this, "保存电路图", "", "",
        "Circuit files (*.json)|*.json", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) return;

    wxString filename = saveFileDialog.GetPath();
    if (!filename.Lower().EndsWith(".json")) {
        filename += ".json";
    }

    DoSave(filename);
}

void MyFrame::DoSave(const wxString& filename) {
    json j;
    j["version"] = "1.0";
    j["type"] = "circuit";

    std::vector<json> gatesJson;
    for (auto& gate : m_drawPanel->GetGates()) {
        json gateJson;
        gateJson["type"] = gate.type.ToStdString();
        gateJson["x"] = gate.pos.x;
        gateJson["y"] = gate.pos.y;

        //         
        std::vector<json> propsJson;
        for (auto& prop : gate.properties) {
            json propJson;
            propJson["name"] = prop.name.ToStdString();
            propJson["value"] = prop.value.ToStdString();
            propJson["type"] = prop.type.ToStdString();
            propsJson.push_back(propJson);
        }
        gateJson["properties"] = propsJson;

        gatesJson.push_back(gateJson);
    }
    j["gates"] = gatesJson;

    std::ofstream f(filename.ToStdString());
    if (!f.is_open()) {
        wxLogError("无法保存文件：%s", filename);
        return;
    }

    f << j.dump(4);
    m_currentFile = filename;
    UpdateTitle();
}

void MyFrame::OnQuit(wxCommandEvent& event) { Close(true); }

void MyFrame::OnUndo(wxCommandEvent& event) {
    if (!undoStack.empty()) {
        //    浱ǰ״̬      ջ
        redoStack.push(m_drawPanel->GetState());
        //  ָ   һ  ״̬
        m_drawPanel->SetState(undoStack.top());
        undoStack.pop();

        //    ²˵ ״̬
        wxMenuBar* mb = GetMenuBar();
        if (mb) {
            mb->Enable(wxID_UNDO, !undoStack.empty());
            mb->Enable(wxID_REDO, !redoStack.empty());
        }

        SetStatusText("撤销成功");
    }
    else {
        SetStatusText("没有可撤销的操作");
    }
}



void MyFrame::OnRedo(wxCommandEvent& event) {
    if (!redoStack.empty()) {
        // ���浱ǰ״̬������ջ
        undoStack.push(m_drawPanel->GetState());
        // �ָ�����״̬
        m_drawPanel->SetState(redoStack.top());
        redoStack.pop();

        // ���²˵�״̬
        wxMenuBar* mb = GetMenuBar();
        if (mb) {
            mb->Enable(wxID_UNDO, !undoStack.empty());
            mb->Enable(wxID_REDO, !redoStack.empty());
        }

        SetStatusText("重做成功");
    }
    else {
        SetStatusText("没有可重做的操作");
    }
}

void MyFrame::OnCopy(wxCommandEvent& event) {
    // ��ʵ��
}

void MyFrame::OnPaste(wxCommandEvent& event) {
    // ��ʵ��
}

void MyFrame::OnZoomin(wxCommandEvent& event) { m_drawPanel->ZoomIn(); }
void MyFrame::OnZoomout(wxCommandEvent& event) { m_drawPanel->ZoomOut(); }

void MyFrame::OnToggleStatusBar(wxCommandEvent& event) {
    wxMenuBar* mb = GetMenuBar();
    wxMenuItem* item = mb->FindItem(ID_SHOW_STATUSBAR);
    bool show = item->IsChecked();
    GetStatusBar()->Show(show);
    Layout();
}

void MyFrame::OnToggleGrid(wxCommandEvent& event) {
    wxMenuBar* mb = GetMenuBar();
    wxMenuItem* item = mb->FindItem(ID_SHOW_GRID);
    bool show = item->IsChecked();
    m_drawPanel->ToggleGrid();
    item->Check(m_drawPanel->IsGridVisible());
}

void MyFrame::OnDeleteSelected(wxCommandEvent& event) {
    SaveStateForUndo();
    m_drawPanel->DeleteSelected();
}

void MyFrame::OnDeleteWire(wxCommandEvent& event) {
    SaveStateForUndo();
    m_drawPanel->DeleteSelectedWire();
}

void MyFrame::OnEditProperties(wxCommandEvent& event) {
    SaveStateForUndo();
    m_drawPanel->EditSelectedProperties();
}

void MyFrame::OnAbout(wxCommandEvent& event) {
    wxMessageBox("电路图编辑器\n֧支持组件绘制、连线、属性编辑、网格对齐、线条删除等功能\n\n"
        "使用说明\n"
        "- 双击左侧组件添加到画布\n"
        "- 左键拖拽移动组件\n"
        "- 左键点击并拖拽画线\n"
        "- 右键点击组件显示菜单\n"
        "- 右键点击线条删除线条\n"
        "- Ctrl+Z 撤销\n"
        "- Ctrl+Y 重做\n"
        "- Ctrl+P 编辑属性\n"
        "- Del 删除选中组件\n"
        "- Shift+Del 删除选中线条\n"
        "- Ctrl+G 显示/隐藏网格", "关于", wxOK | wxICON_INFORMATION, this);
}

void MyFrame::UpdateTitle() {
    wxString title = "电路图编辑器";
    if (!m_currentFile.empty()) {
        title += " - " + m_currentFile;
    }
    SetTitle(title);
}

// ===== Ӧ�ó��� =====
class MyApp : public wxApp {
public:
    bool OnInit() override {
        MyFrame* frame = new MyFrame("电路图编辑器");
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);