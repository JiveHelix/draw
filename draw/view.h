#pragma once

#include <algorithm>
#include <tau/vector2d.h>
#include <tau/region.h>
#include <tau/scale.h>


namespace draw
{


template<typename T, typename U>
tau::Region<T> ScaleRegion(
    const tau::Region<T> &region,
    const tau::Scale<U> &scale)
{
    if constexpr (!std::is_same_v<T, U>)
    {
        return (region.template Convert<U>() * scale)
            .template Convert<T, tau::Round>();
    }
    else
    {
        return region * scale;
    }
}

template<typename T, typename U>
tau::Region<T> UnscaleRegion(
    const tau::Region<T> &region,
    const tau::Scale<U> &scale)
{
    if constexpr (!std::is_same_v<T, U>)
    {
        return (region.template Convert<U>() / scale)
            .template Convert<T, tau::Round>();
    }
    else
    {
        return region / scale;
    }
}

template<typename T>
void ConstrainRegion(tau::Region<T> &region, tau::Size<T> size)
{
    size.width = std::max<T>(1, size.width);
    size.height = std::max<T>(1, size.height);

    // topLeft must be < size
    region.topLeft.x = std::min<T>(region.topLeft.x, size.width - 1);
    region.topLeft.y = std::min<T>(region.topLeft.y, size.height - 1);

    // topLeft + region.size must fit within the source size.
    auto bottomRight = region.GetBottomRight();

    if (bottomRight.x <= size.width && bottomRight.y <= size.height)
    {
        // The requirement is satisfied.
        return;
    }

    bottomRight.x = std::min(bottomRight.x, size.width);
    bottomRight.y = std::min(bottomRight.y, size.height);
    region.size = bottomRight - region.topLeft;

    if constexpr (std::is_floating_point_v<T>)
    {
        // floating-point rounding error can cause the computed bottomRight
        // corner to extend beyond the size limit.
        // (bottomRight - region.topLeft) + region.topLeft may be greater than
        // bottomRight.
        auto checkBottomRight = region.GetBottomRight();

        while (checkBottomRight.x > size.width)
        {
            region.size.width =
                std::nexttoward(region.size.width, 0);

            checkBottomRight = region.GetBottomRight();
        }

        while (checkBottomRight.y > size.height)
        {
            region.size.height =
                std::nexttoward(region.size.height, 0);

            checkBottomRight = region.GetBottomRight();
        }
    }
}


template<typename T, typename ScaleType = double>
struct View
{
    static_assert(std::is_signed_v<T>, "View must use signed indices");

    tau::Region<T> source;
    tau::Region<T> target;
    tau::Scale<ScaleType> scale;


    static constexpr auto fields = std::make_tuple(
        fields::Field(&View::source, "source"),
        fields::Field(&View::target, "target"),
        fields::Field(&View::scale, "scale"));

    View() = default;

    /*
     * @param view The window on the source image, with position relative to the
     *             origin of the source.
     * @param sourceSize The unscaled size of the source image.
     * @param scale The scale applied to the source image.
     */
    View(
        const tau::Region<T> &viewWindow,
        const tau::Size<T> &sourceSize,
        const tau::Scale<ScaleType> &scale_)
        :
        source{{{0, 0}, sourceSize}},
        target{viewWindow},
        scale{scale_}
    {
        auto scaledSourceRegion = ScaleRegion(this->source, this->scale);

        // The viewWindow is a clipping window on the scaled source image.
        // The source image is at position (0, 0)
        this->source = UnscaleRegion(
            scaledSourceRegion.Intersect(viewWindow),
            this->scale);

        ConstrainRegion(this->source, sourceSize);

        assert(this->source.topLeft.y < sourceSize.height);
        assert(this->source.topLeft.x < sourceSize.width);
        assert(this->source.GetBottomRight().x <= sourceSize.width);
        assert(this->source.GetBottomRight().y <= sourceSize.height);

        // When the viewWindow is positive, start painting the target at zero.
        // When the viewWindow is shifted negative, start painting the target at
        // a positive shift of the same magnitude.
        tau::Point2d<T> targetTopLeft(
            std::min(static_cast<T>(0), viewWindow.topLeft.x),
            std::min(static_cast<T>(0), viewWindow.topLeft.y));

        targetTopLeft *= -1;

        // The size of the target is limited by the source data we have to
        // paint it.
        auto targetIntersection = viewWindow.Intersect(scaledSourceRegion);

        // When the source viewWindow is smaller than the target, the target is
        // clipped by the source.
        this->target = tau::Region<T>{{targetTopLeft, targetIntersection.size}};
    }

    template<typename U, typename Style = tau::Round>
    View<U, ScaleType> Convert() const
    {
        View<U, ScaleType> result;

        result.source = this->source.template Convert<U, Style>();
        result.target = this->target.template Convert<U, Style>();
        result.scale = this->scale;

        return result;
    }

    bool HasArea() const
    {
        return this->source.size.HasArea()
            && this->target.size.HasArea();
    }
};


template<typename T>
std::ostream & operator<<(std::ostream &outputStream, const View<T> &view)
{
    return outputStream << fields::DescribeCompact(view);
}


} // end namespace draw
