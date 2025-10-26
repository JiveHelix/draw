#pragma once


#include <optional>
#include <jive/scope_flag.h>
#include "draw/views/view_settings.h"


namespace draw
{


enum class Link: uint8_t
{
    none = 0x00,
    vertical = 0x01,
    horizontal = 0x02,
    both = 0x03
};


inline
bool IsHorizontal(Link link)
{
    return static_cast<uint8_t>(link)
        & static_cast<uint8_t>(Link::horizontal);
}


inline
bool IsVertical(Link link)
{
    return static_cast<uint8_t>(link)
        & static_cast<uint8_t>(Link::vertical);
}


inline
bool IsLinked(Link link)
{
    return link != Link::none;
}


struct LinkOptions
{
    Link scale;
    Link position;
    Link size;
    Link resetZoom;
    Link fitZoom;
    Link recenter;

    LinkOptions()
        :
        scale(Link::both),
        position(Link::both),
        size(Link::both),
        resetZoom(Link::none),
        fitZoom(Link::none),
        recenter(Link::none)
    {

    }

    LinkOptions & SetAll(Link all)
    {
        this->scale = all;
        this->position = all;
        this->size = all;

        return *this;
    }

    LinkOptions & SetScale(Link scale_)
    {
        this->scale = scale_;

        return *this;
    }

    LinkOptions & SetPosition(Link position_)
    {
        this->position = position_;

        return *this;
    }

    LinkOptions & SetSize(Link size_)
    {
        this->size = size_;

        return *this;
    }

    LinkOptions & SetResetZoom(Link resetZoom_)
    {
        this->resetZoom = resetZoom_;

        return *this;
    }

    LinkOptions & SetFitZoom(Link fitZoom_)
    {
        this->fitZoom = fitZoom_;

        return *this;
    }

    LinkOptions & SetRecenter(Link recenter_)
    {
        this->recenter = recenter_;

        return *this;
    }
};


void TriggerResetZoom(
    ViewSettingsControl &viewSettings,
    const LinkOptions &linkOptions);


void TriggerFitZoom(
    ViewSettingsControl &viewSettings,
    const LinkOptions &linkOptions);


void TriggerRecenter(
    ViewSettingsControl &viewSettings,
    const LinkOptions &linkOptions);


void ApplyViewSettings(
    const std::string &name,
    const ViewSettings &source,
    ViewSettingsControl &target,
    const LinkOptions &linkOptions);


class ViewLink
{
public:
    void SetName(const std::string &name);

    ViewLink(
        ViewSettingsControl first,
        ViewSettingsControl second,
        const std::string &name,
        LinkOptions options = LinkOptions{});

    ViewLink(
        ViewSettingsControl first,
        ViewSettingsControl second,
        LinkOptions options = LinkOptions{});

private:
    void OnFirstViewSettings_(const ViewSettings &);

    void OnSecondViewSettings_(const ViewSettings &);

    void OnFirstResetZoom_();

    void OnSecondResetZoom_();

    void OnFirstFitZoom_();

    void OnSecondFitZoom_();

    void OnFirstRecenter_();

    void OnSecondRecenter_();

private:
    ViewSettingsControl firstViewControl_;
    ViewSettingsControl secondViewControl_;
    LinkOptions options_;
    bool ignoreWidth_;
    bool ignoreHeight_;
    bool ignoreHorizontalScale_;
    bool ignoreVerticalScale_;
    bool ignorePositionX_;
    bool ignorePositionY_;
    bool ignoreSignal_;

    using ViewSettingsEndpoint = pex::Endpoint<ViewLink, ViewSettingsControl>;

    using SignalEndpoint =
        pex::Endpoint<ViewLink, pex::control::DefaultSignal>;

    ViewSettingsEndpoint firstViewSettingsEndpoint_;
    ViewSettingsEndpoint secondViewSettingsEndpoint_;

    SignalEndpoint firstResetZoom_;
    SignalEndpoint secondResetZoom_;

    SignalEndpoint firstFitZoom_;
    SignalEndpoint secondFitZoom_;

    SignalEndpoint firstRecenter_;
    SignalEndpoint secondRecenter_;

    std::string name_;
};


} // end namespace draw
