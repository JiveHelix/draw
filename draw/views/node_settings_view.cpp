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
    std::optional<NodeSettingsControl> control)
    :
    window_(window),
    control_(control),
    highlightEndpoint_()
{
    this->InitializeHighlight_();
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
    }
}


void NodeSettingsView::OnLeftDown_(wxMouseEvent &event)
{
    event.Skip();
    assert(this->control_);

    this->control_->toggleSelect.Trigger();
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
    ShapeExpandControl expandControl,
    std::optional<NodeSettingsControl> control)
    :
    wxpex::Collapsible(parent, nodeName, expandControl, borderStyle),
    nodeSettingsView_(this, control)
{

}


CollapsibleNodeSettingsView::CollapsibleNodeSettingsView(
    wxWindow *parent,
    const std::string &nodeName,
    std::optional<NodeSettingsControl> control)
    :
    wxpex::Collapsible(parent, nodeName, borderStyle),
    nodeSettingsView_(this, control)
{

}


BoxedNodeSettingsView::BoxedNodeSettingsView(
    wxWindow *parent,
    const std::string &nodeName,
    std::optional<NodeSettingsControl> control)
    :
    wxpex::StaticBox(parent, nodeName),
    nodeSettingsView_(this, control)
{

}


} // end namespace draw
