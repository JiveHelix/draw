#include "brain.h"

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

#include <fmt/core.h>
#include <pex/list.h>
#include <wxpex/app.h>
#include <wxpex/check_box.h>
#include <wxpex/border_sizer.h>
#include <wxpex/scrolled.h>

#include <draw/pixels.h>
#include <draw/polygon_shape.h>
#include <draw/quad_shape.h>
#include <draw/ellipse_shape.h>
#include <draw/views/pixel_view_settings.h>
#include <draw/views/pixel_view.h>
#include <draw/polygon_brain.h>
#include <draw/shapes.h>
#include <draw/shape_list.h>
#include <draw/views/shape_list_view.h>

#include "observer.h"


template<typename Derived>
class ShapeDemoBrain: public Brain<Derived>
{
public:
    ShapeDemoBrain()
        :
        Brain<Derived>(),
        shapesId_(),
        observer_(this, UserControl(this->user_)),
        demoModel_(),
        demoControl_(this->demoModel_),

        shapesEndpoint_(
            this,
            this->demoControl_.shapes,
            &ShapeDemoBrain::OnShapes_)
    {
        REGISTER_PEX_NAME(this, "ShapeDemoBrain");

        REGISTER_PEX_PARENT(demoModel_);
        REGISTER_PEX_PARENT(demoControl_);
    }

    wxWindow * CreateControls(wxWindow *parent)
    {
        this->userControl_.pixelView.canvas.viewSettings.imageSize.Set(
            draw::Size(1920, 1080));

        auto scrolled = new wxpex::Scrolled(parent);

        auto shapeListView =
            new draw::ShapeListView(scrolled, this->demoControl_);

        scrolled->ConfigureSizer(
            wxpex::verticalScrolled,
            wxpex::BorderSizer(shapeListView, 1));

        return scrolled;
    }

    void Display()
    {
        auto shapes = draw::Shapes(this->shapesId_.Get());

        auto shapeControl = std::rbegin(this->demoControl_.shapes);
        auto rend = std::rend(this->demoControl_.shapes);

        while (shapeControl != rend)
        {
            const auto &shapeValue = shapeControl->Get();
            shapes.Append(shapeValue.GetValueBase()->Copy());
            ++shapeControl;
        }

        this->userControl_.pixelView.asyncShapes.Set(shapes);
    }

protected:
    void OnShapes_(const typename draw::ShapesControl::Type &)
    {
        this->Display();
    }

protected:
    draw::ShapesId shapesId_;
    Observer<ShapeDemoBrain> observer_;
    draw::ShapeListModel demoModel_;
    draw::ShapeListControl demoControl_;

    using ShapesEndpoint = draw::ShapesEndpoint<ShapeDemoBrain>;
    ShapesEndpoint shapesEndpoint_;
};
