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
    wxWindow *parent,
    const std::string &nodeName,
    ShapeExpandControl expandControl,
    std::optional<NodeSettingsControl> control)
    :
    wxpex::Collapsible(parent, nodeName, expandControl, borderStyle),
    control_(control),
    highlightEndpoint_(),
    nodeName_(nodeName)
{
    if (this->control_)
    {
        this->Bind(wxEVT_LEFT_DOWN, &NodeSettingsView::OnLeftDown_, this);

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
    this->control_->select.Trigger();
}


void NodeSettingsView::OnHighlight_(bool isHighlighted)
{
    assert(this->control_);

    if (isHighlighted)
    {
        this->SetBackgroundColour(
            wxpex::ToWxColour(this->control_->highlightColor.Get()));
    }
    else
    {
        this->SetBackgroundColour(wxColour());
    }

    this->Refresh();
}


} // end namespace draw
