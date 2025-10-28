#include "logicgates.h"
#include <fstream>

// 在 .cpp 文件中定义事件
wxDEFINE_EVENT(MY_CUSTOM_EVENT, wxCommandEvent);

// 定义全局形状库
std::map<wxString, std::vector<Shape>> shapeLibrary;

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

        // 跳过基本属性
        if (name == "X坐标" || name == "Y坐标") {
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