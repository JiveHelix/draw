#include <catch2/catch.hpp>

#include <jive/range.h>
#include <tau/region.h>
#include <tau/random.h>
#include <draw/view.h>


template<typename T, typename U>
T CheckAdd(T left, U right)
{
    if (left + right >= std::numeric_limits<T>::max())
    {
        return std::numeric_limits<T>::max();
    }

    return static_cast<T>(left + right);
}


template<template <typename> typename Pair, typename T>
Pair<T> MakeInterior(
    tau::UniformRandom<T> &uniformRandom,
    const tau::Point2d<T> &topLeft,
    const tau::Size<T> &size)
{
    assert(size.width > 0);
    assert(size.height > 0);

    tau::RestoreDistribution restore(uniformRandom);

    auto low = topLeft.x;
    auto high = CheckAdd(topLeft.x, size.width - 1);

    uniformRandom.SetRange(low, high);

    auto interiorX = uniformRandom();

    REQUIRE(interiorX <= uniformRandom.GetHigh());

    low = topLeft.y;
    high = CheckAdd(topLeft.y, size.height - 1);

    uniformRandom.SetRange(low, high);

    auto interiorY = uniformRandom();

    REQUIRE(interiorY <= uniformRandom.GetHigh());

    return {interiorX, interiorY};
}



TEMPLATE_TEST_CASE(
    "Overlapping views",
    "[view]",
    int16_t,
    int32_t,
    int64_t,
    float,
    double)
{
    auto seed = GENERATE(
        take(16, random(tau::SeedLimits::min(), tau::SeedLimits::max())));

    tau::UniformRandom<TestType> uniformRandom{seed};

    auto sourceSize = MakeInterior<tau::Size, TestType>(
        uniformRandom,
        tau::Point2d<TestType>{10, 10},
        tau::Size<TestType>{2000, 2000});

    auto viewPosition = MakeInterior<tau::Point2d, TestType>(
        uniformRandom,
        tau::Point2d<TestType>{0, 0},
        sourceSize);

    tau::Scale scale(1.0, 1.0);

    auto view = draw::View(
        tau::Region<TestType>{{viewPosition, sourceSize}},
        sourceSize,
        scale);

    if (viewPosition.x < 0)
    {
        REQUIRE(view.target.topLeft.x == -viewPosition.x);
        REQUIRE(view.source.topLeft.x == 0);
    }
    else
    {
        REQUIRE(view.target.topLeft.x == 0);

        if (view.source.HasArea())
        {
            REQUIRE(view.source.topLeft.x == viewPosition.x);
        }
    }

    if (viewPosition.y < 0)
    {
        REQUIRE(view.target.topLeft.y == -viewPosition.y);
        REQUIRE(view.source.topLeft.y == 0);
    }
    else
    {
        REQUIRE(view.target.topLeft.y == 0);

        if (view.source.HasArea())
        {
            REQUIRE(view.source.topLeft.y == viewPosition.y);
        }
    }

    REQUIRE(view.source.GetBottomRight().x <= Approx(sourceSize.width));
    REQUIRE(view.source.GetBottomRight().y <= Approx(sourceSize.height));
    REQUIRE(view.source.size.GetArea() == Approx(view.target.size.GetArea()));
}


TEMPLATE_TEST_CASE(
    "Overlapping view with negative position",
    "[region]",
    int16_t,
    int32_t,
    int64_t,
    float,
    double)
{
    auto seed = GENERATE(
        take(16, random(tau::SeedLimits::min(), tau::SeedLimits::max())));

    tau::UniformRandom<TestType> uniformRandom{seed};

    auto sourceSize = MakeInterior<tau::Size, TestType>(
        uniformRandom,
        tau::Point2d<TestType>{10, 10},
        tau::Size<TestType>{2000, 2000});

    auto viewPosition = MakeInterior<tau::Point2d, TestType>(
        uniformRandom,
        (sourceSize / -2).ToPoint2d(),
        sourceSize / 2);

    tau::Scale scale(1.0, 1.0);

    auto view = draw::View(
        tau::Region<TestType>{{viewPosition, sourceSize}},
        sourceSize,
        scale);

    if (viewPosition.x < 0)
    {
        REQUIRE(view.target.topLeft.x == -viewPosition.x);
        REQUIRE(view.source.topLeft.x == 0);
    }
    else
    {
        REQUIRE(view.target.topLeft.x == 0);

        if (view.source.HasArea())
        {
            REQUIRE(view.source.topLeft.x == viewPosition.x);
        }
    }

    if (viewPosition.y < 0)
    {
        REQUIRE(view.target.topLeft.y == -viewPosition.y);
        REQUIRE(view.source.topLeft.y == 0);
    }
    else
    {
        REQUIRE(view.target.topLeft.y == 0);

        if (view.source.HasArea())
        {
            REQUIRE(view.source.topLeft.y == viewPosition.y);
        }
    }

    REQUIRE(view.source.GetBottomRight().x <= Approx(sourceSize.width));
    REQUIRE(view.source.GetBottomRight().y <= Approx(sourceSize.height));
    REQUIRE(view.source.size.GetArea() == Approx(view.target.size.GetArea()));
}
