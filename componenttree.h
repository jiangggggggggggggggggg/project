#ifndef COMPONENTTREE_H
#define COMPONENTTREE_H

#include <wx/treectrl.h>
#include <wx/dnd.h>

// 自定义树形控件，支持拖拽元件
class ComponentTreeCtrl : public wxTreeCtrl {
public:
    ComponentTreeCtrl(wxWindow* parent, wxWindowID id = wxID_ANY);

protected:
    void OnBeginDrag(wxTreeEvent& event);

private:
    DECLARE_EVENT_TABLE()
};

// 拖拽源类：继承wxDropSource，提供文本数据
class ComponentDragSource : public wxDropSource {
public:
    ComponentDragSource(wxWindow* win, const wxString& data);

private:
    wxTextDataObject m_dataObject; // 存储拖拽的文本数据
};

#endif // COMPONENTTREE_H