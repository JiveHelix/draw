#include "draw/draw_context.h"


namespace draw
{


void DrawContext::ConfigureLook(const Look &look)
{
    this->look_ = look;
    this->SetAntialias(look.stroke.antialias);
    this->ConfigureColors(look);
}


void DrawContext::ConfigureColors(const Look &look)
{
    if (look.stroke.enable)
    {
        this->context_->SetPen(
            this->context_->CreatePen(look.stroke.GetPenInfo()));
    }
    else
    {
        this->context_->SetPen(wxNullPen);
    }

    if (look.fill.enable)
    {
        this->context_->SetBrush(
            this->context_->CreateBrush(
                wxBrush(
                    wxpex::ToWxColour(look.fill.color),
                    wxBrushStyle(look.fill.brushStyle))));
    }
    else
    {
        this->context_->SetBrush(wxNullBrush);
    }
}


void DrawContext::ConfigureColors(const Look &look, double value)
{
    if (look.stroke.enable)
    {
        auto stroke = look.stroke;
        stroke.color.value = value;

        this->context_->SetPen(
            this->context_->CreatePen(stroke.GetPenInfo()));
    }
    else
    {
        this->context_->SetPen(this->context_->CreatePen(*wxTRANSPARENT_PEN));
    }

    if (look.fill.enable)
    {
        auto fillColor = look.fill.color;
        fillColor.value = value;

        this->context_->SetBrush(
            this->context_->CreateBrush(
                wxBrush(
                    wxpex::ToWxColour(fillColor),
                    wxBrushStyle(look.fill.brushStyle))));
    }
    else
    {
        this->context_->SetBrush(this->context_->CreateBrush(*wxTRANSPARENT_BRUSH));
    }
}


} // end namespace draw
