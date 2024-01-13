#include "draw/lines_shape.h"


namespace draw
{


LinesShapeSettings LinesShapeSettings::Default()
{
    return {{
        true,
        1000.0,
        Look::Default()}};
}


LinesShape::LinesShape(
    const LinesShapeSettings &settings,
    const Lines &lines)
    :
    settings_(settings),
    lines_(lines)
{

}


void LinesShape::Draw(wxpex::GraphicsContext &context)
{
    if (this->lines_.empty())
    {
        return;
    }

    wxpex::MaintainTransform maintainTransform(context);
    ConfigureLook(context, this->settings_.look);
    auto path = context->CreatePath();

    if (this->settings_.infinite)
    {
        auto size = context.GetSize();
        auto translation = context.GetTranslation();
        auto scale = context.GetScale();

        translation /= scale;
        size /= scale;

        auto region =
            tau::Region<double>{{-1.0 * translation, size}};

        for (auto &line: this->lines_)
        {
            auto endPoints = line.Intersect(region);

            if (!endPoints)
            {
                continue;
            }

            auto start = endPoints->first;
            auto end = endPoints->second;
            path.MoveToPoint(start.x, start.y);
            path.AddLineToPoint(end.x, end.y);
            path.CloseSubpath();
        }
    }
    else
    {
        double halfLength = this->settings_.length / 2.0;

        for (auto &line: this->lines_)
        {
            auto start = line.GetEndPoint(halfLength);
            auto end = line.GetEndPoint(-halfLength);
            path.MoveToPoint(start.x, start.y);
            path.AddLineToPoint(end.x, end.y);
            path.CloseSubpath();
        }
    }

    context->DrawPath(path);
}


} // end namespace draw


template struct pex::Group
<
    draw::LinesShapeFields,
    draw::LinesShapeTemplate,
    pex::PlainT<draw::LinesShapeSettings>
>;
