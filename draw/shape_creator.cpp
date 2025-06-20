#include <draw/shape_creator.h>


namespace draw
{


std::vector<SelectedShape> SelectedShapeChoices::GetChoices()
{
    return {
        SelectedShape::ellipse,
        SelectedShape::polygon,
        SelectedShape::quad,
        SelectedShape::cross,
        SelectedShape::none};
}


std::string SelectedShapeConverter::ToString(
    SelectedShape selectedShape)
{
    switch (selectedShape)
    {
        case (SelectedShape::ellipse):
            return "ellipse";

        case (SelectedShape::polygon):
            return "polygon";

        case (SelectedShape::quad):
            return "quad";

        case (SelectedShape::cross):
            return "cross";

        case (SelectedShape::none):
            return "none";

        default:
            throw std::logic_error("Unknown SelectedShape");
    }
}


std::ostream & operator<<(std::ostream &output, SelectedShape value)
{
    return output << SelectedShapeConverter::ToString(value);
}


} // end namespace draw


template class draw::SelectedMenu<draw::SelectedShapeChoices>;
template class draw::DragCreateSelected<draw::SelectedShapesMenu>;

template class draw::SelectedMenu<draw::SelectedCrossChoices>;
template class draw::DragCreateSelected<draw::SelectedCrossMenu>;
