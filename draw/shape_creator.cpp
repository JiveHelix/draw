#include <draw/shape_creator.h>


namespace draw
{


std::vector<SelectedShape> SelectedShapeChoices::GetChoices()
{
    return {
        SelectedShape::ellipse,
        SelectedShape::polygon,
        SelectedShape::regularPolygon,
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

        case (SelectedShape::regularPolygon):
            return "regularPolygon";

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


std::vector<Action> Actions::GetChoices()
{
    return {
        Action::moveToTop,
        Action::moveUp,
        Action::moveDown,
        Action::moveToBottom,
        Action::remove,
        Action::cancel};
}


std::vector<Action> CrossActions::GetChoices()
{
    return {
        Action::remove,
        Action::cancel};
}


std::string ActionConverter::ToString(
    Action selectedShape)
{
    switch (selectedShape)
    {
        case (Action::moveToTop):
            return "Move to top";

        case (Action::moveUp):
            return "Move up";

        case (Action::moveDown):
            return "Move down";

        case (Action::moveToBottom):
            return "Move to bottom";

        case (Action::remove):
            return "Remove";

        case (Action::cancel):
            return "Cancel";

        default:
            throw std::logic_error("Unknown Action");
    }
}


std::ostream & operator<<(std::ostream &output, Action value)
{
    return output << ActionConverter::ToString(value);
}



} // end namespace draw


template class draw::SelectedMenu<draw::SelectedShapeChoices, draw::Actions>;
template class draw::DragCreateSelected<draw::SelectedShapesMenu>;

template class draw::SelectedMenu
<
    draw::SelectedCrossChoices,
    draw::CrossActions
>;

template class draw::DragCreateSelected<draw::SelectedCrossMenu>;
