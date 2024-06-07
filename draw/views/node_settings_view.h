#pragma once

#include <optional>
#include <wxpex/collapsible.h>
#include <pex/endpoint.h>

#include "draw/node_settings.h"
#include "draw/views/shape_display.h"



namespace draw
{

/**
 ** This frame can change its background color to a highlight color.  When the
 ** highlight color is turned off, it clears the background color with
 ** `SetBackgroundColour(wxColour())`, which reverts back to using the
 ** background color of the parent(s).
 **
 ** If the background color is set exlicitly elsewhere in your code, it will
 ** not be restored when the highlight color is turned off. This situation may
 ** be remedied when wxWidgets corrects the inconsistent behavior of
 ** `wxWindow::InheritsBackgroundColour()`.
 **/

class NodeSettingsView: public wxpex::Collapsible
{
public:
    NodeSettingsView(
        wxWindow *parent,
        const std::string &nodeName,
        ShapeExpandControl expandControl,
        std::optional<NodeSettingsControl> control);

private:
    void OnHighlight_(bool isHighlighted);

    void OnLeftDown_(wxMouseEvent &event);

    using Endpoint =
        pex::Endpoint
        <
            NodeSettingsView,
            decltype(NodeSettingsControl::isSelected)
        >;

    std::optional<NodeSettingsControl> control_;
    Endpoint highlightEndpoint_;
};


} // end namespace draw
