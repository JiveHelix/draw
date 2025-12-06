#include "draw/views/node_settings_view.h"
#include <wxpex/color.h>


#include <wx/settings.h>


namespace draw
{


#ifdef __WXMSW__
inline constexpr auto borderStyle = wxBORDER_NONE;
#else
inline constexpr auto borderStyle = wxBORDER_SIMPLE;
#endif


NodeSettingsView::NodeSettingsView(
    wxWindow *window,
    const NodeSettingsControl *control)
    :
    window_(window),
    control_(),
    highlightEndpoint_()
{
    PEX_NAME("NodeSettingsView");

    if (control)
    {
        this->control_ = *control;

        PEX_MEMBER_ADDRESS(&(*this->control_), "control_");
    }

    this->InitializeHighlight_();
}


void NodeSettingsView::WindowWillBeDestroyed()
{
    this->highlightEndpoint_.Disconnect();
    this->control_.reset();
}


void NodeSettingsView::InitializeHighlight_()
{
    if (this->control_)
    {
        this->window_->Bind(
            wxEVT_LEFT_DOWN,
            &NodeSettingsView::OnLeftDown_,
            this);

        this->window_->Bind(
            wxEVT_LEFT_DCLICK,
            &NodeSettingsView::OnLeftDown_,
            this);

        this->highlightEndpoint_ =
            Endpoint(
                this,
                this->control_->isSelected,
                &NodeSettingsView::OnHighlight_);

        this->OnHighlight_(this->control_->isSelected.Get());
    }
}


void NodeSettingsView::OnLeftDown_(wxMouseEvent &event)
{
    event.Skip();

    if (this->control_)
    {
        this->control_->toggleSelect.Trigger();
    }
}


void NodeSettingsView::OnHighlight_(bool isHighlighted)
{
    assert(this->control_);

    if (isHighlighted)
    {
        this->window_->SetBackgroundColour(
            wxpex::ToWxColour(this->control_->highlightColor.Get()));
    }
    else
    {
        this->window_->SetBackgroundColour(wxColour());
    }

    this->window_->Refresh();
}


CollapsibleNodeSettingsView::CollapsibleNodeSettingsView(
    wxWindow *parent,
    const std::string &nodeName,
    const ShapeExpandControl &expandControl,
    const NodeSettingsControl *control)
    :
    wxpex::Collapsible(parent, nodeName, expandControl, borderStyle),
    nodeSettingsView_(this, control)
{

}


bool CollapsibleNodeSettingsView::Destroy()
{
    this->nodeSettingsView_.WindowWillBeDestroyed();

    return this->wxpex::Collapsible::Destroy();
}


CollapsibleNodeSettingsView::CollapsibleNodeSettingsView(
    wxWindow *parent,
    const std::string &nodeName,
    const NodeSettingsControl *control)
    :
    wxpex::Collapsible(parent, nodeName, borderStyle),
    nodeSettingsView_(this, control)
{

}


BoxedNodeSettingsView::BoxedNodeSettingsView(
    wxWindow *parent,
    const std::string &nodeName,
    const NodeSettingsControl *control)
    :
    wxpex::StaticBox(parent, nodeName),
    nodeSettingsView_(this, control)
{

}


bool BoxedNodeSettingsView::Destroy()
{
    this->nodeSettingsView_.WindowWillBeDestroyed();

    return this->wxpex::StaticBox::Destroy();
}


} // end namespace draw
