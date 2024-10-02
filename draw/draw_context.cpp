#include "draw/draw_context.h"


namespace draw
{


void DrawContext::ConfigureLook(const Look &look)
{
    this->look_ = look;

    if (look.antialias)
    {
        this->context_->SetAntialiasMode(wxANTIALIAS_DEFAULT);
    }
    else
    {
        this->context_->SetAntialiasMode(wxANTIALIAS_NONE);
    }

    this->ConfigureColors(look);
}


void DrawContext::ConfigureColors(const Look &look)
{
    if (look.strokeEnable)
    {
        this->context_->SetPen(
            this->context_->CreatePen(
                wxGraphicsPenInfo(
                    wxpex::ToWxColour(look.strokeColor),
                    look.strokeWeight)));
    }
    else
    {
        this->context_->SetPen(this->context_->CreatePen(*wxTRANSPARENT_PEN));
    }

    if (look.fillEnable)
    {
        this->context_->SetBrush(
            this->context_->CreateBrush(
                wxBrush(
                    wxpex::ToWxColour(look.fillColor))));
    }
    else
    {
        this->context_->SetBrush(this->context_->CreateBrush(*wxTRANSPARENT_BRUSH));
    }
}


void DrawContext::ConfigureColors(const Look &look, double value)
{
    if (look.strokeEnable)
    {
        auto strokeColor = look.strokeColor;
        strokeColor.value = value;

        this->context_->SetPen(
            this->context_->CreatePen(
                wxGraphicsPenInfo(
                    wxpex::ToWxColour(strokeColor),
                    look.strokeWeight)));
    }
    else
    {
        this->context_->SetPen(this->context_->CreatePen(*wxTRANSPARENT_PEN));
    }

    if (look.fillEnable)
    {
        auto fillColor = look.fillColor;
        fillColor.value = value;

        this->context_->SetBrush(
            this->context_->CreateBrush(
                wxBrush(
                    wxpex::ToWxColour(fillColor))));
    }
    else
    {
        this->context_->SetBrush(this->context_->CreateBrush(*wxTRANSPARENT_BRUSH));
    }
}


} // end namespace draw
