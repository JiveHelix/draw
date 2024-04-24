#pragma once


#include <jive/scope_flag.h>
#include "draw/views/view_settings.h"


namespace draw
{


enum class LinkType: uint8_t
{
    vertical = 0x01,
    horizontal = 0x02,
    both = vertical & horizontal
};


inline
bool IsHorizontal(LinkType linkType)
{
    return static_cast<uint8_t>(linkType)
        & static_cast<uint8_t>(LinkType::horizontal);
}


inline
bool IsVertical(LinkType linkType)
{
    return static_cast<uint8_t>(linkType)
        & static_cast<uint8_t>(LinkType::vertical);
}


class ViewLink
{
public:
    ViewLink(
        ViewSettingsControl first,
        ViewSettingsControl second,
        LinkType linkType = LinkType::both)
        :
        firstViewControl_(first),
        secondViewControl_(second),
        ignoreSize_(false),
        ignoreScale_(false),
        ignorePosition_(false)
    {
        if (IsHorizontal(linkType))
        {
            this->firstViewSizeWidth_ = IntEndpoint(
                this,
                first.viewSize.width,
                &ViewLink::OnFirstViewWidth_);

            this->secondViewSizeWidth_ = IntEndpoint(
                this,
                second.viewSize.width,
                &ViewLink::OnSecondViewWidth_);

            this->firstScaleHorizontal_ = FloatEndpoint(
                this,
                first.scale.horizontal,
                &ViewLink::OnFirstScaleHorizontal_);

            this->secondScaleHorizontal_ = FloatEndpoint(
                this,
                second.scale.horizontal,
                &ViewLink::OnSecondScaleHorizontal_);

            this->firstPositionX_ = IntEndpoint(
                this,
                first.viewPosition.x,
                &ViewLink::OnFirstPositionX_);

            this->secondPositionX_ = IntEndpoint(
                this,
                second.viewPosition.x,
                &ViewLink::OnSecondPositionX_);
        }

        if (IsVertical(linkType))
        {
            this->firstViewSizeHeight_ = IntEndpoint(
                this,
                first.viewSize.height,
                &ViewLink::OnFirstViewHeight_);

            this->secondViewSizeHeight_ = IntEndpoint(
                this,
                second.viewSize.height,
                &ViewLink::OnSecondViewHeight_);

            this->firstScaleVertical_ = FloatEndpoint(
                this,
                first.scale.vertical,
                &ViewLink::OnFirstScaleVertical_);

            this->secondScaleVertical_ = FloatEndpoint(
                this,
                second.scale.vertical,
                &ViewLink::OnSecondScaleVertical_);

            this->firstPositionY_ = IntEndpoint(
                this,
                first.viewPosition.y,
                &ViewLink::OnFirstPositionY_);

            this->secondPositionY_ = IntEndpoint(
                this,
                second.viewPosition.y,
                &ViewLink::OnSecondPositionY_);
        }
    }

private:
    template<typename T, typename U>
    static void SetChanged_(T &control, U value)
    {
        if (value != control.Get())
        {
            control.Set(value);
        }
    }

    void OnFirstViewHeight_(int height)
    {
        if (this->ignoreSize_)
        {
            return;
        }

        auto ignore = jive::ScopeFlag(this->ignoreSize_);
        auto bypass = ViewSettingsBypass(this->secondViewControl_);
        SetChanged_(this->secondViewSizeHeight_, height);
    }

    void OnSecondViewHeight_(int height)
    {
        if (this->ignoreSize_)
        {
            return;
        }

        auto ignore = jive::ScopeFlag(this->ignoreSize_);
        auto bypass = ViewSettingsBypass(this->firstViewControl_);
        SetChanged_(this->firstViewSizeHeight_, height);
    }

    void OnFirstViewWidth_(int width)
    {
        if (this->ignoreSize_)
        {
            return;
        }

        auto ignore = jive::ScopeFlag(this->ignoreSize_);
        auto bypass = ViewSettingsBypass(this->secondViewControl_);
        SetChanged_(this->secondViewSizeWidth_, width);
    }

    void OnSecondViewWidth_(int width)
    {
        if (this->ignoreSize_)
        {
            return;
        }

        auto ignore = jive::ScopeFlag(this->ignoreSize_);
        auto bypass = ViewSettingsBypass(this->firstViewControl_);
        SetChanged_(this->firstViewSizeWidth_, width);
    }

    void OnFirstScaleVertical_(double scale)
    {
        if (this->ignoreScale_)
        {
            return;
        }

        auto ignore = jive::ScopeFlag(this->ignoreScale_);
        auto bypass = ViewSettingsBypass(this->secondViewControl_);
        SetChanged_(this->secondScaleVertical_, scale);
    }

    void OnSecondScaleVertical_(double scale)
    {
        if (this->ignoreScale_)
        {
            return;
        }

        auto ignore = jive::ScopeFlag(this->ignoreScale_);
        auto bypass = ViewSettingsBypass(this->firstViewControl_);
        SetChanged_(this->firstScaleVertical_, scale);
    }

    void OnFirstScaleHorizontal_(double scale)
    {
        if (this->ignoreScale_)
        {
            return;
        }

        auto ignore = jive::ScopeFlag(this->ignoreScale_);
        auto bypass = ViewSettingsBypass(this->secondViewControl_);
        SetChanged_(this->secondScaleHorizontal_, scale);
    }

    void OnSecondScaleHorizontal_(double scale)
    {
        if (this->ignoreScale_)
        {
            return;
        }

        auto ignore = jive::ScopeFlag(this->ignoreScale_);
        auto bypass = ViewSettingsBypass(this->firstViewControl_);
        SetChanged_(this->firstScaleHorizontal_, scale);
    }

    void OnFirstPositionY_(int position)
    {
        if (this->ignorePosition_)
        {
            return;
        }

        auto ignore = jive::ScopeFlag(this->ignorePosition_);
        auto bypass = ViewSettingsBypass(this->secondViewControl_);

        auto screenDifferenceY =
            this->secondViewControl_.screenPosition.Get().y
            - this->firstViewControl_.screenPosition.Get().y;

        SetChanged_(this->secondPositionY_, position + screenDifferenceY);
    }

    void OnSecondPositionY_(int position)
    {
        if (this->ignorePosition_)
        {
            return;
        }

        auto ignore = jive::ScopeFlag(this->ignorePosition_);
        auto bypass = ViewSettingsBypass(this->firstViewControl_);

        auto screenDifferenceY =
            this->firstViewControl_.screenPosition.Get().y
            - this->secondViewControl_.screenPosition.Get().y;

        SetChanged_(this->firstPositionY_, position + screenDifferenceY);
    }

    void OnFirstPositionX_(int position)
    {
        if (this->ignorePosition_)
        {
            return;
        }

        auto ignore = jive::ScopeFlag(this->ignorePosition_);
        auto bypass = ViewSettingsBypass(this->secondViewControl_);

        auto screenDifferenceX =
            this->secondViewControl_.screenPosition.Get().x
            - this->firstViewControl_.screenPosition.Get().x;

        SetChanged_(this->secondPositionX_, position + screenDifferenceX);
    }

    void OnSecondPositionX_(int position)
    {
        if (this->ignorePosition_)
        {
            return;
        }

        auto ignore = jive::ScopeFlag(this->ignorePosition_);
        auto bypass = ViewSettingsBypass(this->firstViewControl_);

        auto screenDifferenceX =
            this->firstViewControl_.screenPosition.Get().x
            - this->secondViewControl_.screenPosition.Get().x;

        SetChanged_(this->firstPositionX_, position + screenDifferenceX);
    }

private:
    ViewSettingsControl firstViewControl_;
    ViewSettingsControl secondViewControl_;
    bool ignoreSize_;
    bool ignoreScale_;
    bool ignorePosition_;

    using IntEndpoint = pex::Endpoint<ViewLink, decltype(SizeControl::width)>;

    using FloatEndpoint =
        pex::Endpoint<ViewLink, decltype(ScaleControl::horizontal)>;

    IntEndpoint firstViewSizeWidth_;
    IntEndpoint firstViewSizeHeight_;

    IntEndpoint secondViewSizeWidth_;
    IntEndpoint secondViewSizeHeight_;

    FloatEndpoint firstScaleHorizontal_;
    FloatEndpoint firstScaleVertical_;

    FloatEndpoint secondScaleHorizontal_;
    FloatEndpoint secondScaleVertical_;

    IntEndpoint firstPositionX_;
    IntEndpoint firstPositionY_;

    IntEndpoint secondPositionX_;
    IntEndpoint secondPositionY_;
};


} // end namespace draw
