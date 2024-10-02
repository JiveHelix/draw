#pragma once


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


struct LinkOptions
{
    Link scale;
    Link position;
    Link size;

    LinkOptions()
        :
        scale(Link::both),
        position(Link::both),
        size(Link::both)
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
};


class ViewLink
{
public:
    ViewLink(
        ViewSettingsControl first,
        ViewSettingsControl second,
        LinkOptions options = LinkOptions{})
        :
        firstViewControl_(first),
        secondViewControl_(second),
        options_(options),
        ignoreSize_(false),
        ignoreScale_(false),
        ignorePosition_(false)
    {
        if (IsHorizontal(options.size))
        {
            this->firstViewSizeWidth_ = IntEndpoint(
                this,
                first.viewSize.width,
                &ViewLink::OnFirstViewWidth_);

            this->secondViewSizeWidth_ = IntEndpoint(
                this,
                second.viewSize.width,
                &ViewLink::OnSecondViewWidth_);
        }

        if (IsHorizontal(options.scale))
        {
            this->firstScaleHorizontal_ = FloatEndpoint(
                this,
                first.scale.horizontal,
                &ViewLink::OnFirstScaleHorizontal_);

            this->secondScaleHorizontal_ = FloatEndpoint(
                this,
                second.scale.horizontal,
                &ViewLink::OnSecondScaleHorizontal_);
        }

        if (IsHorizontal(options.position))
        {
            this->firstPositionX_ = IntEndpoint(
                this,
                first.viewPosition.x,
                &ViewLink::OnFirstPositionX_);

            this->secondPositionX_ = IntEndpoint(
                this,
                second.viewPosition.x,
                &ViewLink::OnSecondPositionX_);
        }

        if (IsVertical(options.size))
        {
            this->firstViewSizeHeight_ = IntEndpoint(
                this,
                first.viewSize.height,
                &ViewLink::OnFirstViewHeight_);

            this->secondViewSizeHeight_ = IntEndpoint(
                this,
                second.viewSize.height,
                &ViewLink::OnSecondViewHeight_);
        }

        if (IsVertical(options.scale))
        {
            this->firstScaleVertical_ = FloatEndpoint(
                this,
                first.scale.vertical,
                &ViewLink::OnFirstScaleVertical_);

            this->secondScaleVertical_ = FloatEndpoint(
                this,
                second.scale.vertical,
                &ViewLink::OnSecondScaleVertical_);
        }

        if (IsVertical(options.position))
        {
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
    static bool SetChanged_(T &control, U value)
    {
        if (value != control.Get())
        {
            control.Set(value);

            return true;
        }

        return false;
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

        if (SetChanged_(this->secondScaleVertical_, scale))
        {
            if (!IsVertical(this->options_.position))
            {
                // The vertical component of position on the linked view will
                // not be adjusted.
                this->secondViewControl_.recenterVertical.Trigger();
            }
        }
    }

    void OnSecondScaleVertical_(double scale)
    {
        if (this->ignoreScale_)
        {
            return;
        }

        auto ignore = jive::ScopeFlag(this->ignoreScale_);
        auto bypass = ViewSettingsBypass(this->firstViewControl_);

        if (SetChanged_(this->firstScaleVertical_, scale))
        {
            if (!IsVertical(this->options_.position))
            {
                // The vertical component of position on the linked view will
                // not be adjusted.
                this->firstViewControl_.recenterVertical.Trigger();
            }
        }
    }

    void OnFirstScaleHorizontal_(double scale)
    {
        if (this->ignoreScale_)
        {
            return;
        }

        auto ignore = jive::ScopeFlag(this->ignoreScale_);
        auto bypass = ViewSettingsBypass(this->secondViewControl_);

        if (SetChanged_(this->secondScaleHorizontal_, scale))
        {
            if (!IsHorizontal(this->options_.position))
            {
                // The horizontal component of position on the linked view will
                // not be adjusted.
                this->secondViewControl_.recenterHorizontal.Trigger();
            }
        }
    }

    void OnSecondScaleHorizontal_(double scale)
    {
        if (this->ignoreScale_)
        {
            return;
        }

        auto ignore = jive::ScopeFlag(this->ignoreScale_);
        auto bypass = ViewSettingsBypass(this->firstViewControl_);

        if (SetChanged_(this->firstScaleHorizontal_, scale))
        {
            if (!IsHorizontal(this->options_.position))
            {
                // The horizontal component of position on the linked view will
                // not be adjusted.
                this->firstViewControl_.recenterHorizontal.Trigger();
            }
        }
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
    LinkOptions options_;
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
