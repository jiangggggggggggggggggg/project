#include "component.h"

Component::Component(ComponentType type, const wxString& name)
    : type_(type), name_(name), position_(0, 0), size_(50, 30) {
}

Component::~Component() {}

void Component::SetPosition(const wxPoint& pos) {
    position_ = pos;
    UpdatePorts();
}

wxPoint Component::GetPosition() const {
    return position_;
}

void Component::SetSize(const wxSize& size) {
    size_ = size;
    UpdatePorts();
}

wxSize Component::GetSize() const {
    return size_;
}

ComponentType Component::GetType() const {
    return type_;
}

wxString Component::GetName() const {
    return name_;
}

void Component::SetName(const wxString& name) {
    name_ = name;
}

void Component::AddPort(const Port& port) {
    ports_.push_back(port);
}

Port* Component::GetPort(int index) {
    if (index >= 0 && index < ports_.size()) {
        return &ports_[index];
    }
    return nullptr;
}

int Component::GetPortCount() const {
    return ports_.size();
}

std::vector<Port>& Component::GetPorts() {
    return ports_;
}

void Component::UpdatePorts() {
    // 默认实现 - 派生类可以重写来更新端口位置
    // 例如根据元件大小和端口数量重新计算端口位置
}

wxRect Component::GetBoundingBox() const {
    return wxRect(position_, size_);
}

void Component::SetProperty(const wxString& key, const wxString& value) {
    properties_[key] = value;
}

wxString Component::GetProperty(const wxString& key) const {
    auto it = properties_.find(key);
    if (it != properties_.end()) {
        return it->second;
    }
    return wxString();
}

bool Component::HasProperty(const wxString& key) const {
    return properties_.find(key) != properties_.end();
}