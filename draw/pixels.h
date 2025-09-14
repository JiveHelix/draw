#pragma once


#include <memory>
#include <pex/endpoint.h>
#include <tau/color.h>
#include <wxpex/async.h>


namespace draw
{


using Pixels = tau::RgbPixels<uint8_t>;
using PixelMatrix = typename Pixels::Data;


using AsyncPixels = wxpex::MakeAsync<std::shared_ptr<Pixels>>;

using PixelsControl =
    typename AsyncPixels::Control<typename AsyncPixels::Model>;

template<typename Observer>
using PixelsEndpoint = pex::Endpoint<Observer, PixelsControl>;


class PixelsContext
{
public:
    PixelsContext(Pixels &pixels)
        :
        pixels_(pixels),
        image_(
            static_cast<int>(pixels.size.width),
            static_cast<int>(pixels.size.height),
            pixels.data.data(),
            true),
        bitmap_(this->image_),
        context_(this->bitmap_)
    {

    }

    PixelsContext(Pixels &&) = delete;
    PixelsContext(const PixelsContext &) = delete;

    ~PixelsContext()
    {
        this->context_.SelectObject(wxNullBitmap);
        auto updated = this->bitmap_.ConvertToImage();

        this->pixels_.data = Eigen::Map<typename Pixels::Data>(
            updated.GetData(),
            this->pixels_.size.GetArea(),
            3).eval();
    }

    operator wxDC & ()
    {
        return this->context_;
    }

    operator wxMemoryDC & ()
    {
        return this->context_;
    }

    wxMemoryDC & Get()
    {
        return this->context_;
    }

private:
    Pixels & pixels_;
    wxImage image_;
    wxBitmap bitmap_;
    wxMemoryDC context_;
};


} // end namespace draw
