#pragma once


#include <wxpex/graphics.h>
#include <draw/look.h>


namespace draw
{


class DrawContext: public wxpex::GraphicsContext
{
public:
    template<typename T>
    DrawContext(T &&dc)
        :
        wxpex::GraphicsContext(std::forward<T>(dc)),
        look_{}
    {

    }

    DrawContext(const DrawContext &) = delete;
    DrawContext & operator=(const DrawContext &) = delete;

    void ConfigureLook(const Look &look);

    void ConfigureColors(const Look &);

    void ConfigureColors(const Look &, double value);

    const Look & GetLook() const
    {
        return this->look_;
    }

private:
    Look look_;
};


} // end namespace draw
