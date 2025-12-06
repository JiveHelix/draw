#pragma once

#include <optional>
#include <pex/endpoint.h>
#include <wxpex/collapsible.h>
#include <wxpex/static_box.h>

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

class NodeSettingsView
{
public:
    static constexpr auto observerName = "draw::views::NodeSettingsView";

    NodeSettingsView(
        wxWindow *window,
        const NodeSettingsControl *control);

    void WindowWillBeDestroyed();

private:
    void InitializeHighlight_();

    void OnHighlight_(bool isHighlighted);

    void OnLeftDown_(wxMouseEvent &event);

    using Endpoint =
        pex::Endpoint
        <
            NodeSettingsView,
            decltype(NodeSettingsControl::isSelected)
        >;

    wxWindow *window_;
    std::optional<NodeSettingsControl> control_;
    Endpoint highlightEndpoint_;
};


class CollapsibleNodeSettingsView: public wxpex::Collapsible
{
public:
    CollapsibleNodeSettingsView(
        wxWindow *parent,
        const std::string &nodeName,
        const ShapeExpandControl &expandControl,
        const NodeSettingsControl *control = nullptr);

    CollapsibleNodeSettingsView(
        wxWindow *parent,
        const std::string &nodeName,
        const NodeSettingsControl *control = nullptr);

    bool Destroy() override;

private:
    NodeSettingsView nodeSettingsView_;
};


class BoxedNodeSettingsView: public wxpex::StaticBox
{
public:
    BoxedNodeSettingsView(
        wxWindow *parent,
        const std::string &nodeName,
        const NodeSettingsControl *control = nullptr);

    bool Destroy() override;

private:
    NodeSettingsView nodeSettingsView_;
};


} // end namespace draw
