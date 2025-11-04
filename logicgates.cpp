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

// ===== JSON 加载函数 =====
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
            else if (type == "Polyline") shape.type = ShapeType::Polyline;
            // 在LoadShapesFromJson函数中添加
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

void LoadPinsFromJson(const std::string& filename) {
    std::ifstream f(filename.c_str());
    if (!f.is_open()) {
        wxLogError("无法打开引脚文件: %s", filename);
        return;
    }
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