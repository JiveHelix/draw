#include "draw/views/pixel_view_settings.h"


namespace draw
{


PixelViewModel::PixelViewModel()
    :
    PixelViewGroup::Model(),

    viewSettings_(
        this,
        this->viewSettings,
        &PixelViewModel::OnViewSettings_),

    mousePosition_(
        this,
        this->mousePosition,
        &PixelViewModel::OnMousePosition_)
{
    this->OnViewSettings_(this->viewSettings.Get());
}


void PixelViewModel::OnViewSettings_(const ViewSettings &settings)
{
    this->logicalPosition.Set(
        settings.GetLogicalPosition(this->mousePosition.Get()));
}


void PixelViewModel::OnMousePosition_(const Point &point)
{
    this->logicalPosition.Set(
        this->viewSettings.Get().GetLogicalPosition(point));
}



PixelViewControl::PixelViewControl(PixelViewModel &dataViewModel)
    :
    PixelViewGroup::Control(dataViewModel),
    asyncPixels(dataViewModel.pixels.GetWorkerControl()),
    asyncShapes(dataViewModel.shapes.GetWorkerControl())
{

}


} // end namespace draw





template struct pex::Group
    <
        draw::PixelViewFields,
        draw::PixelViewTemplate
    >;


template struct pex::MakeGroup
    <
        draw::PixelViewGroup,
        draw::PixelViewModel,
        draw::PixelViewControl
    >;
