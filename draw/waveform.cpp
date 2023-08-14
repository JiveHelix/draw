#include "draw/waveform.h"


namespace draw
{


Waveform Resize(
    const Waveform &source,
    const Size &displaySize,
    double verticalScale)
{
    Waveform result = Waveform::Zero(displaySize.height, displaySize.width);

    double widthFactor = static_cast<double>(result.cols())
        / static_cast<double>(source.cols());

    double heightFactor = verticalScale * static_cast<double>(result.rows())
        / static_cast<double>(source.rows());

    Eigen::Index sourceRowCount = source.rows();
    Eigen::Index resultRowCount = result.rows();

    for (Eigen::Index row = 0; row < sourceRowCount; ++row)
    {
        Eigen::Index logicalRow = sourceRowCount - row - 1;

        auto resultRow = tau::Index(
            std::round(static_cast<double>(logicalRow) * heightFactor));

        auto nextRow = tau::Index(
            std::round(static_cast<double>(logicalRow + 1) * heightFactor));

        nextRow = std::min(resultRowCount, nextRow);

        if (resultRow >= nextRow)
        {
            // The vertical scale has pushed us off of the display.
            continue;
        }

        auto blockHeight = nextRow - resultRow;

        Eigen::Index targetIndex = resultRowCount - nextRow;

        for (Eigen::Index column = 0; column < source.cols(); ++column)
        {
            auto resultColumn =
                tau::Index(
                    std::round(static_cast<double>(column) * widthFactor));

            auto nextColumn = 
                tau::Index(
                    std::round(static_cast<double>(column + 1) * widthFactor));

            nextColumn = std::min(nextColumn, result.cols());
            auto blockWidth = nextColumn - resultColumn;

            result.block(targetIndex, resultColumn, blockHeight, blockWidth)
                .array() = source(row, column);
        }
    }

    return result;
}


} // end namespace draw
