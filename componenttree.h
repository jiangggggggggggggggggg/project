#ifndef COMPONENTTREE_H
#define COMPONENTTREE_H

#include <wx/treectrl.h>
#include <wx/dnd.h>

// �Զ������οؼ���֧����קԪ��
class ComponentTreeCtrl : public wxTreeCtrl {
public:
    ComponentTreeCtrl(wxWindow* parent, wxWindowID id = wxID_ANY);

protected:
    void OnBeginDrag(wxTreeEvent& event);

private:
    DECLARE_EVENT_TABLE()
};

// ��קԴ�ࣺ�̳�wxDropSource���ṩ�ı�����
class ComponentDragSource : public wxDropSource {
public:
    ComponentDragSource(wxWindow* win, const wxString& data);

private:
    wxTextDataObject m_dataObject; // �洢��ק���ı�����
};

#endif // COMPONENTTREE_H