#pragma once


#include <tau/eigen.h>
#include "draw/waveform_settings.h"


namespace draw
{


using Waveform =
    Eigen::Matrix<uint16_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;


template<typename T>
using Data = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;


template<typename T>
Eigen::Index FloatToIndex(T value)
{
    return static_cast<Eigen::Index>(value);
}


Waveform Resize(
    const Waveform &source,
    const Size &displaySize,
    double verticalScale);


template<typename Matrix>
Waveform DoGenerateWaveform(
    const Eigen::MatrixBase<Matrix> &data,
    size_t maximumValue,
    size_t levelCount,
    size_t columnCount)
{
    auto maximum = tau::Index(levelCount) - 1;

    auto columnDivisor = static_cast<float>(data.cols())
        / static_cast<float>(columnCount - 1);

    auto columnMultiplier = 1.0f / columnDivisor;

    auto valueDivisor = static_cast<float>(maximumValue)
        / static_cast<float>(maximum);

    auto valueMultiplier = 1.0f / valueDivisor;

    Waveform result =
        Waveform::Zero(tau::Index(levelCount), tau::Index(columnCount));

    using Scalar = typename Matrix::Scalar;

    Matrix scaled =
        (data.template cast<float>().array() * valueMultiplier)
            .template cast<Scalar>();

    using Eigen::Index;

    for (Index row = 0; row < data.rows(); ++row)
    {
        for (Index column = 0; column < data.cols(); ++column)
        {
            auto value = tau::Index(scaled(row, column));

            // (0, 0) is the top left
            // Value 0 must go in the last row, and maximum in row zero.
            result(
                maximum - value,
                FloatToIndex(float(column) * columnMultiplier)) += 1;
        }
    }

    return result;
}


} // end namespace draw
