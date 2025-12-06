#include <draw/views/view_link.h>
#include <functional>


// #define ENABLE_VIEW_LINK_LOG

#define SHORTFILE jive::path::Base(__FILE__)

#ifdef ENABLE_VIEW_LINK_LOG

#include <fmt/core.h>


#define VIEW_LINK_LOG \
    fmt::print("{}:{}:{} {}\n", SHORTFILE, __FUNCTION__, __LINE__, name_);

#else

#define VIEW_LINK_LOG

#endif


#define LINK_SHORT_LOG \
    fmt::print("{}:{}:{} {}\n", SHORTFILE, __FUNCTION__, __LINE__, name_);


namespace draw
{


using TriggerFunction =
    std::function<void(ViewSettingsControl &, const LinkOptions &)>;


void TriggerResetZoom(
    ViewSettingsControl &viewSettings,
    const LinkOptions &linkOptions)
{
    if (linkOptions.resetZoom == Link::both)
    {
        viewSettings.resetZoom.Trigger();
    }
    else if (linkOptions.resetZoom == Link::horizontal)
    {
        viewSettings.resetHorizontalZoom.Trigger();
    }
    else if (linkOptions.resetZoom == Link::vertical)
    {
        viewSettings.resetVerticalZoom.Trigger();
    }
}


void TriggerFitZoom(
    ViewSettingsControl &viewSettings,
    const LinkOptions &linkOptions)
{
    if (linkOptions.fitZoom == Link::both)
    {
        viewSettings.fitZoom.Trigger();
    }
    else if (linkOptions.fitZoom == Link::horizontal)
    {
        viewSettings.fitHorizontalZoom.Trigger();
    }
    else if (linkOptions.fitZoom == Link::vertical)
    {
        viewSettings.fitVerticalZoom.Trigger();
    }
}


void TriggerRecenter(
    ViewSettingsControl &viewSettings,
    const LinkOptions &linkOptions)
{
    if (linkOptions.recenter == Link::both)
    {
        viewSettings.recenter.Trigger();
    }
    else if (linkOptions.recenter == Link::horizontal)
    {
        viewSettings.recenterHorizontal.Trigger();
    }
    else if (linkOptions.recenter == Link::vertical)
    {
        viewSettings.recenterVertical.Trigger();
    }
}


template<typename T, typename U>
bool SetChanged(
    T &control,
    U value,
    [[maybe_unused]] const std::string &name)
{
    if (value != control.Get())
    {
        control.Set(value);

        return true;
    }

    return false;
}


void ApplyViewSettings(
    const std::string &name,
    const ViewSettings &source,
    ViewSettingsControl &target,
    const LinkOptions &linkOptions)
{
    auto defer = pex::MakeDefer(target);

    if (linkOptions.scale == Link::both)
    {
        SetChanged(defer.scale, source.scale, fmt::format("{} scale", name));
    }
    else if (linkOptions.scale == Link::horizontal)
    {
        SetChanged(
            defer.scale.horizontal,
            source.scale.horizontal,
            fmt::format("{} scale.horizontal", name));
    }
    else if (linkOptions.scale == Link::vertical)
    {
        SetChanged(
            defer.scale.vertical,
            source.scale.vertical,
            fmt::format("{} scale.vertical", name));
    }

    if (linkOptions.position == Link::both)
    {
        SetChanged(
            defer.viewPosition,
            source.viewPosition,
            fmt::format("{} viewPosition", name));
    }
    else if (linkOptions.position == Link::horizontal)
    {
        SetChanged(
            defer.viewPosition.x,
            source.viewPosition.x,
            fmt::format("{} viewPosition.x", name));
    }
    else if (linkOptions.position == Link::vertical)
    {
        SetChanged(
            defer.viewPosition.y,
            source.viewPosition.y,
            fmt::format("{} viewPosition.y", name));
    }

    if (linkOptions.size == Link::both)
    {
        SetChanged(
            defer.viewSize,
            source.viewSize,
            fmt::format("{} viewSize", name));
    }
    else if (linkOptions.size == Link::horizontal)
    {
        SetChanged(
            defer.viewSize.width,
            source.viewSize.width,
            fmt::format("{} viewSize.width", name));
    }
    else if (linkOptions.size == Link::vertical)
    {
        SetChanged(
            defer.viewSize.height,
            source.viewSize.height,
            fmt::format("{} viewSize.height", name));
    }
}


void ViewLink::SetName(const std::string &name)
{
    this->name_ = name;
}


ViewLink::ViewLink(
    const ViewSettingsControl &first,
    const ViewSettingsControl &second,
    const std::string &name,
    LinkOptions options)
    :
    ViewLink(
        first,
        second,
        options)
{
    this->name_ = name;
}


ViewLink::ViewLink(
    const ViewSettingsControl &first,
    const ViewSettingsControl &second,
    LinkOptions options)
    :
    firstViewControl_(first),
    secondViewControl_(second),
    options_(options),
    ignoreWidth_(false),
    ignoreHeight_(false),
    ignoreHorizontalScale_(false),
    ignoreVerticalScale_(false),
    ignorePositionX_(false),
    ignorePositionY_(false),
    ignoreSignal_(false),

    firstViewSettingsEndpoint_(
        this,
        first,
        &ViewLink::OnFirstViewSettings_),

    secondViewSettingsEndpoint_(
        this,
        second,
        &ViewLink::OnSecondViewSettings_)
{
    options.resetZoom = static_cast<Link>(
        static_cast<uint8_t>(options.resetZoom)
            & ~static_cast<uint8_t>(options.scale));

    options.fitZoom = static_cast<Link>(
        static_cast<uint8_t>(options.fitZoom)
            & ~static_cast<uint8_t>(options.scale));

    options.recenter = static_cast<Link>(
        static_cast<uint8_t>(options.recenter)
            & ~static_cast<uint8_t>(options.position));

    this->options_ = options;

    if (IsLinked(options.resetZoom))
    {
        this->firstResetZoom_ = SignalEndpoint(
            this,
            first.resetZoom,
            &ViewLink::OnFirstResetZoom_);

        this->secondResetZoom_ = SignalEndpoint(
            this,
            second.resetZoom,
            &ViewLink::OnSecondResetZoom_);
    }

    if (IsLinked(options.fitZoom))
    {
        this->firstFitZoom_ = SignalEndpoint(
            this,
            first.fitZoom,
            &ViewLink::OnFirstFitZoom_);

        this->secondFitZoom_ = SignalEndpoint(
            this,
            second.fitZoom,
            &ViewLink::OnSecondFitZoom_);
    }

    if (IsLinked(options.recenter))
    {
        this->firstRecenter_ = SignalEndpoint(
            this,
            first.recenter,
            &ViewLink::OnFirstRecenter_);

        this->secondRecenter_ = SignalEndpoint(
            this,
            second.recenter,
            &ViewLink::OnSecondRecenter_);
    }
}


void ViewLink::OnFirstViewSettings_(const ViewSettings &firstViewSettings)
{
    ApplyViewSettings(
        fmt::format("{} firstViewSettings", this->name_),
        firstViewSettings,
        this->secondViewControl_,
        this->options_);
}


void ViewLink::OnSecondViewSettings_(const ViewSettings &secondViewSettings)
{
    ApplyViewSettings(
        fmt::format("{} secondViewSettings", this->name_),
        secondViewSettings,
        this->firstViewControl_,
        this->options_);
}


void ProcessTrigger_(
    [[maybe_unused]] const std::string &message,
    bool &flag,
    TriggerFunction triggerFunction,
    ViewSettingsControl &control,
    LinkOptions options)
{
    if (flag)
    {
        return;
    }

    auto ignore = jive::ScopeFlag(flag);
    triggerFunction(control, options);
}


void ViewLink::OnFirstResetZoom_()
{
    ProcessTrigger_(
        fmt::format("{} firstResetZoom", name_),
        this->ignoreSignal_,
        TriggerResetZoom,
        this->secondViewControl_,
        this->options_);
}

void ViewLink::OnSecondResetZoom_()
{
    ProcessTrigger_(
        fmt::format("{} secondResetZoom", name_),
        this->ignoreSignal_,
        TriggerResetZoom,
        this->firstViewControl_,
        this->options_);
}

void ViewLink::OnFirstFitZoom_()
{
    ProcessTrigger_(
        fmt::format("{} firstFitZoom", name_),
        this->ignoreSignal_,
        TriggerFitZoom,
        this->secondViewControl_,
        this->options_);
}

void ViewLink::OnSecondFitZoom_()
{
    ProcessTrigger_(
        fmt::format("{} secondFitZoom", name_),
        this->ignoreSignal_,
        TriggerFitZoom,
        this->firstViewControl_,
        this->options_);
}

void ViewLink::OnFirstRecenter_()
{
    ProcessTrigger_(
        fmt::format("{} firstRecenter", name_),
        this->ignoreSignal_,
        TriggerRecenter,
        this->secondViewControl_,
        this->options_);
}

void ViewLink::OnSecondRecenter_()
{
    ProcessTrigger_(
        fmt::format("{} secondRecenter", name_),
        this->ignoreSignal_,
        TriggerRecenter,
        this->firstViewControl_,
        this->options_);
}


} // end namespace draw
