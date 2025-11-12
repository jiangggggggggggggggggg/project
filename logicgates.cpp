#include "logicgates.h"
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>

// 在 .cpp 文件中定义事件
wxDEFINE_EVENT(MY_CUSTOM_EVENT, wxCommandEvent);

// 定义全局形状库
std::map<wxString, std::vector<Shape>> shapeLibrary;

// 定义全局引脚库
std::map<wxString, std::vector<Pin>> pinLibrary;

// BookShelf组件尺寸映射
std::map<wxString, std::pair<double, double>> bookShelfComponentSizes = {
    {"AND", {8.0, 12.0}},
    {"NAND", {8.0, 12.0}},
    {"OR", {8.0, 12.0}},
    {"NOR", {8.0, 12.0}},
    {"NOT", {4.0, 12.0}},
    {"XOR", {8.0, 12.0}},
    {"XNOR", {8.0, 12.0}},
    {"BUFFER", {6.0, 12.0}},
    {"LED", {1.0, 1.0}},
    {"开关", {1.0, 1.0}}
};

// ===== PropertyDialog 实现 =====
PropertyDialog::PropertyDialog(wxWindow* parent, Gate& gate)
    : wxDialog(parent, wxID_ANY, "编辑属性 - " + gate.type, wxDefaultPosition, wxSize(400, 300)),
    m_gate(gate) {

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // 创建属性网格
    m_propertyGrid = new wxPropertyGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxPG_DEFAULT_STYLE | wxPG_SPLITTER_AUTO_CENTER);
    m_propertyGrid->SetColumnProportion(0, 0.4);
    m_propertyGrid->SetColumnProportion(1, 0.6);

    // 添加基本属性
    m_propertyGrid->Append(new wxPropertyCategory("基本属性"));
    m_propertyGrid->Append(new wxIntProperty("X坐标", wxPG_LABEL, m_gate.pos.x));
    m_propertyGrid->Append(new wxIntProperty("Y坐标", wxPG_LABEL, m_gate.pos.y));

    // 添加自定义属性
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

    // 添加引脚信息
    if (!m_gate.pins.empty()) {
        m_propertyGrid->Append(new wxPropertyCategory("引脚信息"));
        for (const auto& pin : m_gate.pins) {
            wxString pinInfo = wxString::Format("%s (%s)", pin.name, pin.isInput ? "输入" : "输出");
            m_propertyGrid->Append(new wxStringProperty(pinInfo, wxPG_LABEL,
                wxString::Format("位置: (%d, %d)", pin.relativePos.x, pin.relativePos.y)));
        }
    }

    // 添加新属性按钮
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* addButton = new wxButton(this, wxID_ANY, "添加属性");
    buttonSizer->Add(addButton, 0, wxALL, 5);

    // 确定取消按钮
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

    // 绑定事件
    addButton->Bind(wxEVT_BUTTON, &PropertyDialog::OnAddProperty, this);
    okButton->Bind(wxEVT_BUTTON, &PropertyDialog::OnOK, this);
}

void PropertyDialog::OnAddProperty(wxCommandEvent& event) {
    wxTextEntryDialog dialog(this, "请输入属性名称:", "添加属性");
    if (dialog.ShowModal() == wxID_OK) {
        wxString name = dialog.GetValue();
        if (!name.IsEmpty()) {
            m_propertyGrid->Append(new wxStringProperty(name, wxPG_LABEL, ""));
        }
    }
}

void PropertyDialog::OnOK(wxCommandEvent& event) {
    // 更新基本属性
    wxVariant xVar = m_propertyGrid->GetPropertyValue("X坐标");
    wxVariant yVar = m_propertyGrid->GetPropertyValue("Y坐标");

    m_gate.pos.x = xVar.GetLong();
    m_gate.pos.y = yVar.GetLong();

    // 更新自定义属性
    m_gate.properties.clear();

    wxPropertyGridIterator it;
    for (it = m_propertyGrid->GetIterator(); !it.AtEnd(); it++) {
        wxPGProperty* prop = *it;
        wxString name = prop->GetName();

        // 跳过基本属性和引脚信息
        if (name == "X坐标" || name == "Y坐标" || name == "基本属性" || name == "引脚信息" || name == "自定义属性") {
            continue;
        }

        // 跳过分类标题
        if (prop->IsCategory()) {
            continue;
        }

        Property newProp;
        newProp.name = name;
        newProp.value = prop->GetValueAsString();

        // 判断属性类型
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

// ===== 硬编码形状定义 =====
void InitializeHardcodedShapes() {
    wxLogMessage("使用硬编码形状定义");

    // AND 门
    std::vector<Shape> andShapes;
    andShapes.push_back(Shape{ ShapeType::Line, {wxPoint(0, 0), wxPoint(0, 60)}, wxPoint(), 0, 0, 0, "" });
    andShapes.push_back(Shape{ ShapeType::Line, {wxPoint(0, 0), wxPoint(40, 0)}, wxPoint(), 0, 0, 0, "" });
    andShapes.push_back(Shape{ ShapeType::Line, {wxPoint(0, 60), wxPoint(40, 60)}, wxPoint(), 0, 0, 0, "" });
    andShapes.push_back(Shape{ ShapeType::Arc, {}, wxPoint(40, 30), 30, 270, 90, "" });
    andShapes.push_back(Shape{ ShapeType::Line, {wxPoint(-15, 20), wxPoint(0, 20)}, wxPoint(), 0, 0, 0, "" });
    andShapes.push_back(Shape{ ShapeType::Line, {wxPoint(-15, 40), wxPoint(0, 40)}, wxPoint(), 0, 0, 0, "" });
    andShapes.push_back(Shape{ ShapeType::Line, {wxPoint(70, 30), wxPoint(85, 30)}, wxPoint(), 0, 0, 0, "" });
    andShapes.push_back(Shape{ ShapeType::Text, {}, wxPoint(25, 30), 0, 0, 0, "&" });
    shapeLibrary["AND"] = andShapes;

    // NAND 门
    std::vector<Shape> nandShapes;
    nandShapes.push_back(Shape{ ShapeType::Line, {wxPoint(0, 0), wxPoint(0, 60)}, wxPoint(), 0, 0, 0, "" });
    nandShapes.push_back(Shape{ ShapeType::Line, {wxPoint(0, 0), wxPoint(40, 0)}, wxPoint(), 0, 0, 0, "" });
    nandShapes.push_back(Shape{ ShapeType::Line, {wxPoint(0, 60), wxPoint(40, 60)}, wxPoint(), 0, 0, 0, "" });
    nandShapes.push_back(Shape{ ShapeType::Arc, {}, wxPoint(40, 30), 30, 270, 90, "" });
    nandShapes.push_back(Shape{ ShapeType::Circle, {}, wxPoint(75, 30), 5, 0, 0, "" });
    nandShapes.push_back(Shape{ ShapeType::Line, {wxPoint(-15, 20), wxPoint(0, 20)}, wxPoint(), 0, 0, 0, "" });
    nandShapes.push_back(Shape{ ShapeType::Line, {wxPoint(-15, 40), wxPoint(0, 40)}, wxPoint(), 0, 0, 0, "" });
    nandShapes.push_back(Shape{ ShapeType::Line, {wxPoint(80, 30), wxPoint(95, 30)}, wxPoint(), 0, 0, 0, "" });
    nandShapes.push_back(Shape{ ShapeType::Text, {}, wxPoint(25, 30), 0, 0, 0, "&" });
    shapeLibrary["NAND"] = nandShapes;

    // OR 门
    std::vector<Shape> orShapes;
    orShapes.push_back(Shape{ ShapeType::Arc, {}, wxPoint(15, 30), 35, 270, 90, "" });
    orShapes.push_back(Shape{ ShapeType::Arc, {}, wxPoint(-5, 30), 35, 270, 90, "" });
    orShapes.push_back(Shape{ ShapeType::Line, {wxPoint(-20, 20), wxPoint(-5, 20)}, wxPoint(), 0, 0, 0, "" });
    orShapes.push_back(Shape{ ShapeType::Line, {wxPoint(-20, 40), wxPoint(-5, 40)}, wxPoint(), 0, 0, 0, "" });
    orShapes.push_back(Shape{ ShapeType::Line, {wxPoint(50, 30), wxPoint(65, 30)}, wxPoint(), 0, 0, 0, "" });
    orShapes.push_back(Shape{ ShapeType::Text, {}, wxPoint(25, 30), 0, 0, 0, "≥1" });
    shapeLibrary["OR"] = orShapes;

    // NOT 门
    std::vector<Shape> notShapes;
    notShapes.push_back(Shape{ ShapeType::Polygon, {wxPoint(0, 0), wxPoint(0, 60), wxPoint(50, 30)}, wxPoint(), 0, 0, 0, "" });
    notShapes.push_back(Shape{ ShapeType::Circle, {}, wxPoint(55, 30), 5, 0, 0, "" });
    notShapes.push_back(Shape{ ShapeType::Line, {wxPoint(-15, 30), wxPoint(0, 30)}, wxPoint(), 0, 0, 0, "" });
    notShapes.push_back(Shape{ ShapeType::Line, {wxPoint(60, 30), wxPoint(75, 30)}, wxPoint(), 0, 0, 0, "" });
    notShapes.push_back(Shape{ ShapeType::Text, {}, wxPoint(20, 30), 0, 0, 0, "1" });
    shapeLibrary["NOT"] = notShapes;

    // NOR 门
    std::vector<Shape> norShapes;
    norShapes.push_back(Shape{ ShapeType::Arc, {}, wxPoint(15, 30), 35, 270, 90, "" });
    norShapes.push_back(Shape{ ShapeType::Arc, {}, wxPoint(-5, 30), 35, 270, 90, "" });
    norShapes.push_back(Shape{ ShapeType::Circle, {}, wxPoint(55, 30), 5, 0, 0, "" });
    norShapes.push_back(Shape{ ShapeType::Line, {wxPoint(-20, 20), wxPoint(-5, 20)}, wxPoint(), 0, 0, 0, "" });
    norShapes.push_back(Shape{ ShapeType::Line, {wxPoint(-20, 40), wxPoint(-5, 40)}, wxPoint(), 0, 0, 0, "" });
    norShapes.push_back(Shape{ ShapeType::Line, {wxPoint(60, 30), wxPoint(75, 30)}, wxPoint(), 0, 0, 0, "" });
    norShapes.push_back(Shape{ ShapeType::Text, {}, wxPoint(25, 30), 0, 0, 0, "≥1" });
    shapeLibrary["NOR"] = norShapes;

    // XOR 门
    std::vector<Shape> xorShapes;
    xorShapes.push_back(Shape{ ShapeType::Arc, {}, wxPoint(15, 30), 35, 270, 90, "" });
    xorShapes.push_back(Shape{ ShapeType::Arc, {}, wxPoint(-5, 30), 35, 270, 90, "" });
    xorShapes.push_back(Shape{ ShapeType::Arc, {}, wxPoint(-10, 30), 35, 270, 90, "" });
    xorShapes.push_back(Shape{ ShapeType::Line, {wxPoint(-25, 20), wxPoint(-10, 20)}, wxPoint(), 0, 0, 0, "" });
    xorShapes.push_back(Shape{ ShapeType::Line, {wxPoint(-25, 40), wxPoint(-10, 40)}, wxPoint(), 0, 0, 0, "" });
    xorShapes.push_back(Shape{ ShapeType::Line, {wxPoint(50, 30), wxPoint(65, 30)}, wxPoint(), 0, 0, 0, "" });
    xorShapes.push_back(Shape{ ShapeType::Text, {}, wxPoint(25, 30), 0, 0, 0, "=1" });
    shapeLibrary["XOR"] = xorShapes;

    // XNOR 门
    std::vector<Shape> xnorShapes;
    xnorShapes.push_back(Shape{ ShapeType::Arc, {}, wxPoint(15, 30), 35, 270, 90, "" });
    xnorShapes.push_back(Shape{ ShapeType::Arc, {}, wxPoint(-5, 30), 35, 270, 90, "" });
    xnorShapes.push_back(Shape{ ShapeType::Arc, {}, wxPoint(-10, 30), 35, 270, 90, "" });
    xnorShapes.push_back(Shape{ ShapeType::Circle, {}, wxPoint(65, 30), 5, 0, 0, "" });
    xnorShapes.push_back(Shape{ ShapeType::Line, {wxPoint(-25, 20), wxPoint(-10, 20)}, wxPoint(), 0, 0, 0, "" });
    xnorShapes.push_back(Shape{ ShapeType::Line, {wxPoint(-25, 40), wxPoint(-10, 40)}, wxPoint(), 0, 0, 0, "" });
    xnorShapes.push_back(Shape{ ShapeType::Line, {wxPoint(70, 30), wxPoint(85, 30)}, wxPoint(), 0, 0, 0, "" });
    xnorShapes.push_back(Shape{ ShapeType::Text, {}, wxPoint(25, 30), 0, 0, 0, "=1" });
    shapeLibrary["XNOR"] = xnorShapes;

    // BUFFER
    std::vector<Shape> bufferShapes;
    bufferShapes.push_back(Shape{ ShapeType::Polygon, {wxPoint(0, 0), wxPoint(0, 60), wxPoint(50, 30)}, wxPoint(), 0, 0, 0, "" });
    bufferShapes.push_back(Shape{ ShapeType::Line, {wxPoint(-15, 30), wxPoint(0, 30)}, wxPoint(), 0, 0, 0, "" });
    bufferShapes.push_back(Shape{ ShapeType::Line, {wxPoint(50, 30), wxPoint(65, 30)}, wxPoint(), 0, 0, 0, "" });
    bufferShapes.push_back(Shape{ ShapeType::Text, {}, wxPoint(20, 30), 0, 0, 0, "1" });
    shapeLibrary["BUFFER"] = bufferShapes;

    // LED
    std::vector<Shape> ledShapes;
    ledShapes.push_back(Shape{ ShapeType::Circle, {}, wxPoint(30, 30), 20, 0, 0, "" });
    ledShapes.push_back(Shape{ ShapeType::Line, {wxPoint(10, 30), wxPoint(0, 30)}, wxPoint(), 0, 0, 0, "" });
    ledShapes.push_back(Shape{ ShapeType::Line, {wxPoint(50, 30), wxPoint(60, 30)}, wxPoint(), 0, 0, 0, "" });
    ledShapes.push_back(Shape{ ShapeType::Polygon, {wxPoint(25, 15), wxPoint(35, 15), wxPoint(40, 30), wxPoint(35, 45), wxPoint(25, 45), wxPoint(20, 30)}, wxPoint(), 0, 0, 0, "" });
    ledShapes.push_back(Shape{ ShapeType::Text, {}, wxPoint(30, 55), 0, 0, 0, "LED" });
    shapeLibrary["LED"] = ledShapes;

    // 开关
    std::vector<Shape> switchShapes;
    switchShapes.push_back(Shape{ ShapeType::Circle, {}, wxPoint(0, 0), 3, 0, 0, "" });
    switchShapes.push_back(Shape{ ShapeType::Circle, {}, wxPoint(30, 0), 3, 0, 0, "" });
    switchShapes.push_back(Shape{ ShapeType::Line, {wxPoint(0, 0), wxPoint(15, -20)}, wxPoint(), 0, 0, 0, "" });
    switchShapes.push_back(Shape{ ShapeType::Line, {wxPoint(15, -20), wxPoint(30, 0)}, wxPoint(), 0, 0, 0, "" });
    switchShapes.push_back(Shape{ ShapeType::Rectangle, {wxPoint(12, -25), wxPoint(18, -15)}, wxPoint(), 0, 0, 0, "" });
    switchShapes.push_back(Shape{ ShapeType::Text, {}, wxPoint(15, -30), 0, 0, 0, "开关" });
    shapeLibrary["开关"] = switchShapes;

    wxLogMessage("硬编码形状定义完成，组件数量: %zu", shapeLibrary.size());
}

// ===== 硬编码引脚定义 =====
void InitializeHardcodedPins() {
    wxLogMessage("使用硬编码引脚定义");

    // AND 门引脚
    std::vector<Pin> andPins;
    andPins.push_back(Pin{ "in1", wxPoint(-15, 20), true });
    andPins.push_back(Pin{ "in2", wxPoint(-15, 40), true });
    andPins.push_back(Pin{ "out", wxPoint(85, 30), false });
    pinLibrary["AND"] = andPins;

    // NAND 门引脚
    std::vector<Pin> nandPins;
    nandPins.push_back(Pin{ "in1", wxPoint(-15, 20), true });
    nandPins.push_back(Pin{ "in2", wxPoint(-15, 40), true });
    nandPins.push_back(Pin{ "out", wxPoint(95, 30), false });
    pinLibrary["NAND"] = nandPins;

    // OR 门引脚
    std::vector<Pin> orPins;
    orPins.push_back(Pin{ "in1", wxPoint(-20, 20), true });
    orPins.push_back(Pin{ "in2", wxPoint(-20, 40), true });
    orPins.push_back(Pin{ "out", wxPoint(65, 30), false });
    pinLibrary["OR"] = orPins;

    // NOT 门引脚
    std::vector<Pin> notPins;
    notPins.push_back(Pin{ "in", wxPoint(-15, 30), true });
    notPins.push_back(Pin{ "out", wxPoint(75, 30), false });
    pinLibrary["NOT"] = notPins;

    // NOR 门引脚
    std::vector<Pin> norPins;
    norPins.push_back(Pin{ "in1", wxPoint(-20, 20), true });
    norPins.push_back(Pin{ "in2", wxPoint(-20, 40), true });
    norPins.push_back(Pin{ "out", wxPoint(75, 30), false });
    pinLibrary["NOR"] = norPins;

    // XOR 门引脚
    std::vector<Pin> xorPins;
    xorPins.push_back(Pin{ "in1", wxPoint(-25, 20), true });
    xorPins.push_back(Pin{ "in2", wxPoint(-25, 40), true });
    xorPins.push_back(Pin{ "out", wxPoint(65, 30), false });
    pinLibrary["XOR"] = xorPins;

    // XNOR 门引脚
    std::vector<Pin> xnorPins;
    xnorPins.push_back(Pin{ "in1", wxPoint(-25, 20), true });
    xnorPins.push_back(Pin{ "in2", wxPoint(-25, 40), true });
    xnorPins.push_back(Pin{ "out", wxPoint(85, 30), false });
    pinLibrary["XNOR"] = xnorPins;

    // BUFFER 引脚
    std::vector<Pin> bufferPins;
    bufferPins.push_back(Pin{ "in", wxPoint(-15, 30), true });
    bufferPins.push_back(Pin{ "out", wxPoint(65, 30), false });
    pinLibrary["BUFFER"] = bufferPins;

    // LED 引脚
    std::vector<Pin> ledPins;
    ledPins.push_back(Pin{ "in", wxPoint(0, 30), true });
    ledPins.push_back(Pin{ "out", wxPoint(60, 30), false });
    pinLibrary["LED"] = ledPins;

    // 开关引脚
    std::vector<Pin> switchPins;
    switchPins.push_back(Pin{ "in", wxPoint(0, 0), true });
    switchPins.push_back(Pin{ "out", wxPoint(30, 0), false });
    pinLibrary["开关"] = switchPins;

    wxLogMessage("硬编码引脚定义完成，组件数量: %zu", pinLibrary.size());
}

// ===== JSON 加载函数 =====
void LoadShapesFromJson(const std::string& filename) {
    std::ifstream f(filename.c_str());
    if (!f.is_open()) {
        return;
    }

    try {
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
                else if (type == "Polyline") shape.type = ShapeType::Polyline;
                else if (type == "Rectangle") shape.type = ShapeType::Rectangle;

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
    catch (...) {
        shapeLibrary.clear();
    }
}

void LoadPinsFromJson(const std::string& filename) {
    std::ifstream f(filename.c_str());
    if (!f.is_open()) {
        return;
    }

    try {
        json j;
        f >> j;

        for (json::iterator it = j.begin(); it != j.end(); ++it) {
            std::string gateName = it.key();
            auto pins = it.value();

            std::vector<Pin> pinVec;
            for (size_t i = 0; i < pins.size(); ++i) {
                auto p = pins[i];
                Pin pin;
                pin.name = p["name"].get<std::string>();
                pin.relativePos = wxPoint(p["x"], p["y"]);
                pin.isInput = p["isInput"].get<bool>();
                pinVec.push_back(pin);
            }
            pinLibrary[wxString::FromUTF8(gateName.c_str())] = pinVec;
        }
    }
    catch (...) {
        pinLibrary.clear();
    }
}

// ===== 网表功能实现 =====
Netlist GenerateNetlist(const std::vector<Gate>& gates, const std::vector<Wire>& wires) {
    Netlist netlist;

    // 设置设计名称和时间戳
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    netlist.timestamp = ss.str();
    netlist.designName = "CircuitDesign";

    // 添加组件（包括引脚信息）
    for (size_t i = 0; i < gates.size(); ++i) {
        NetComponent comp;
        comp.componentId = wxString::Format("U%d", (int)i + 1);
        comp.componentType = gates[i].type;
        comp.position = gates[i].pos;
        comp.properties = gates[i].properties;
        comp.pins = gates[i].pins;
        netlist.components.push_back(comp);
    }

    // 根据连线生成网络节点
    std::map<wxString, NetNode> netMap;

    for (const auto& wire : wires) {
        if (!wire.startCompId.empty() && !wire.endCompId.empty()) {
            // 使用连线信息创建网络连接
            wxString netName = wxString::Format("NET_%s_%s_to_%s_%s",
                wire.startCompId, wire.startPinName,
                wire.endCompId, wire.endPinName);

            // 添加起始引脚连接
            NetConnection startConn;
            startConn.componentId = wire.startCompId;
            startConn.pinName = wire.startPinName;

            if (netMap.find(netName) == netMap.end()) {
                NetNode node;
                node.netName = netName;
                netMap[netName] = node;
            }
            netMap[netName].connections.push_back(startConn);

            // 添加结束引脚连接
            NetConnection endConn;
            endConn.componentId = wire.endCompId;
            endConn.pinName = wire.endPinName;
            netMap[netName].connections.push_back(endConn);
        }
    }

    // 将网络映射转换为向量
    for (auto& pair : netMap) {
        netlist.nodes.push_back(pair.second);
    }

    return netlist;
}

bool ExportNetlistToFile(const wxString& filename, const Netlist& netlist) {
    json j;

    j["designName"] = netlist.designName.ToStdString();
    j["timestamp"] = netlist.timestamp.ToStdString();

    // 导出组件（包括引脚）
    json componentsJson = json::array();
    for (const auto& comp : netlist.components) {
        json compJson;
        compJson["id"] = comp.componentId.ToStdString();
        compJson["type"] = comp.componentType.ToStdString();
        compJson["x"] = comp.position.x;
        compJson["y"] = comp.position.y;

        // 导出属性
        json propsJson = json::array();
        for (const auto& prop : comp.properties) {
            json propJson;
            propJson["name"] = prop.name.ToStdString();
            propJson["value"] = prop.value.ToStdString();
            propJson["type"] = prop.type.ToStdString();
            propsJson.push_back(propJson);
        }
        compJson["properties"] = propsJson;

        // 导出引脚
        json pinsJson = json::array();
        for (const auto& pin : comp.pins) {
            json pinJson;
            pinJson["name"] = pin.name.ToStdString();
            pinJson["x"] = pin.relativePos.x;
            pinJson["y"] = pin.relativePos.y;
            pinJson["isInput"] = pin.isInput;
            pinsJson.push_back(pinJson);
        }
        compJson["pins"] = pinsJson;

        componentsJson.push_back(compJson);
    }
    j["components"] = componentsJson;

    // 导出网络
    json netsJson = json::array();
    for (const auto& node : netlist.nodes) {
        json netJson;
        netJson["name"] = node.netName.ToStdString();

        json connectionsJson = json::array();
        for (const auto& conn : node.connections) {
            json connJson;
            connJson["component"] = conn.componentId.ToStdString();
            connJson["pin"] = conn.pinName.ToStdString();
            connectionsJson.push_back(connJson);
        }
        netJson["connections"] = connectionsJson;

        netsJson.push_back(netJson);
    }
    j["nets"] = netsJson;

    std::ofstream file(filename.ToStdString());
    if (!file.is_open()) {
        return false;
    }

    file << j.dump(4);
    return true;
}

bool ImportNetlistFromFile(const wxString& filename, Netlist& netlist) {
    std::ifstream file(filename.ToStdString());
    if (!file.is_open()) {
        return false;
    }

    try {
        json j;
        file >> j;

        netlist.designName = j["designName"].get<std::string>();
        netlist.timestamp = j["timestamp"].get<std::string>();

        // 导入组件（包括引脚）
        netlist.components.clear();
        for (const auto& compJson : j["components"]) {
            NetComponent comp;
            comp.componentId = compJson["id"].get<std::string>();
            comp.componentType = compJson["type"].get<std::string>();
            comp.position.x = compJson["x"];
            comp.position.y = compJson["y"];

            // 导入属性
            if (compJson.find("properties") != compJson.end()) {
                for (const auto& propJson : compJson["properties"]) {
                    Property prop;
                    prop.name = propJson["name"].get<std::string>();
                    prop.value = propJson["value"].get<std::string>();
                    prop.type = propJson["type"].get<std::string>();
                    comp.properties.push_back(prop);
                }
            }

            // 导入引脚
            if (compJson.find("pins") != compJson.end()) {
                for (const auto& pinJson : compJson["pins"]) {
                    Pin pin;
                    pin.name = pinJson["name"].get<std::string>();
                    pin.relativePos.x = pinJson["x"];
                    pin.relativePos.y = pinJson["y"];
                    pin.isInput = pinJson["isInput"];
                    comp.pins.push_back(pin);
                }
            }

            netlist.components.push_back(comp);
        }

        // 导入网络
        netlist.nodes.clear();
        for (const auto& netJson : j["nets"]) {
            NetNode node;
            node.netName = netJson["name"].get<std::string>();

            for (const auto& connJson : netJson["connections"]) {
                NetConnection conn;
                conn.componentId = connJson["component"].get<std::string>();
                conn.pinName = connJson["pin"].get<std::string>();
                node.connections.push_back(conn);
            }

            netlist.nodes.push_back(node);
        }

        return true;
    }
    catch (const std::exception& e) {
        wxLogError("解析网表文件失败: %s", e.what());
        return false;
    }
}

// 判断组件是否为终端（输入输出引脚）
bool IsTerminalComponent(const wxString& componentType) {
    return componentType == "LED" || componentType == "开关";
}

// 生成BookShelf .node文件
bool ExportBookShelfNodeFile(const wxString& filename, const Netlist& netlist) {
    std::ofstream file(filename.ToStdString());
    if (!file.is_open()) {
        return false;
    }

    // 计算终端数量
    int terminalCount = 0;
    for (const auto& comp : netlist.components) {
        if (IsTerminalComponent(comp.componentType)) {
            terminalCount++;
        }
    }

    // 写入文件头
    file << "UCLA nodes 1.0\n";
    file << "# Created " << netlist.timestamp.ToStdString() << "\n";
    file << "# Created by: CircuitDesigner\n\n";

    file << "NumNodes " << netlist.components.size() << "\n";
    file << "NumTerminals " << terminalCount << "\n\n";

    // 写入每个组件的信息
    for (const auto& comp : netlist.components) {
        // 获取组件尺寸
        double width = 8.0, height = 12.0; // 默认尺寸
        auto sizeIt = bookShelfComponentSizes.find(comp.componentType);
        if (sizeIt != bookShelfComponentSizes.end()) {
            width = sizeIt->second.first;
            height = sizeIt->second.second;
        }

        file << comp.componentId.ToStdString() << " " << width << " " << height;

        // 如果是终端组件，标记为terminal
        if (IsTerminalComponent(comp.componentType)) {
            file << " terminal";
        }
        file << "\n";
    }

    file.close();
    return true;
}

// 生成BookShelf .net文件
bool ExportBookShelfNetFile(const wxString& filename, const Netlist& netlist) {
    std::ofstream file(filename.ToStdString());
    if (!file.is_open()) {
        return false;
    }

    // 计算总引脚数
    int totalPins = 0;
    for (const auto& node : netlist.nodes) {
        totalPins += node.connections.size();
    }

    // 写入文件头
    file << "UCLA nets 1.0\n";
    file << "# Created " << netlist.timestamp.ToStdString() << "\n";
    file << "# Created by: CircuitDesigner\n\n";

    file << "NumNets " << netlist.nodes.size() << "\n";
    file << "NumPins " << totalPins << "\n\n";

    // 写入每个网络的信息
    for (const auto& node : netlist.nodes) {
        file << "NetDegree " << node.connections.size() << " " << node.netName.ToStdString() << "\n";

        for (const auto& conn : node.connections) {
            // 查找对应的组件和引脚
            const NetComponent* comp = nullptr;
            const Pin* pin = nullptr;

            for (const auto& c : netlist.components) {
                if (c.componentId == conn.componentId) {
                    comp = &c;
                    for (const auto& p : c.pins) {
                        if (p.name == conn.pinName) {
                            pin = &p;
                            break;
                        }
                    }
                    break;
                }
            }

            if (comp && pin) {
                // 确定引脚方向
                char direction = pin->isInput ? 'I' : 'O';

                // 计算引脚偏移（这里需要将相对坐标转换为BookShelf格式）
                // 假设组件中心为原点，引脚偏移相对于中心
                double offsetX = pin->relativePos.x - 40.0; // 假设组件宽度80，中心在40
                double offsetY = pin->relativePos.y - 30.0; // 假设组件高度60，中心在30

                file << comp->componentId.ToStdString() << " " << direction << " : "
                    << offsetX << " " << offsetY << "\n";
            }
        }
        file << "\n";
    }

    file.close();
    return true;
}

// 同时生成.node和.net文件
bool ExportBookShelfFiles(const wxString& baseFilename, const Netlist& netlist) {
    wxString nodeFile = baseFilename + ".nodes";
    wxString netFile = baseFilename + ".nets";

    bool nodeSuccess = ExportBookShelfNodeFile(nodeFile, netlist);
    bool netSuccess = ExportBookShelfNetFile(netFile, netlist);

    return nodeSuccess && netSuccess;
}