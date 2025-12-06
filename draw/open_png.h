#pragma once


#include <draw/png.h>
#include <pex/string.h>
#include <draw/views/view_settings.h>


namespace draw
{


template<typename PngPixel, typename T>
concept HasLoadGrayPng = requires(T t)
{
    { t.LoadGrayPng(std::declval<GrayPng<PngPixel>>()) };
};


template<typename PngPixel, typename T>
concept HasLoadPng = requires(T t)
{
    { t.LoadPng(std::declval<Png<PngPixel>>()) };
};


template<typename PngPixel, typename Derived>
void OpenPng(
    const pex::StringControl &fileNameControl,
    const ViewSettingsControl &viewSettingsControl,
    Derived &derived)
requires(HasLoadGrayPng<PngPixel, Derived> || HasLoadPng<PngPixel, Derived>)
{
    auto deferFileName = pex::MakeDefer(fileNameControl);

    auto fileName = deferFileName.Get();

    if (!jive::path::IsFile(fileName))
    {
        auto [directory, file] = jive::path::Split(fileName);

        wxpex::FileDialogOptions options{};
        options.message = "Choose a PNG file";
        options.wildcard = "*.png";

        wxFileDialog openFile(
            nullptr,
            wxString(options.message),
            wxString(directory),
            wxString(file),
            wxString(options.wildcard),
            options.style);

        if (openFile.ShowModal() == wxID_CANCEL)
        {
            return;
        }

        deferFileName.Set(openFile.GetPath().utf8_string());
    }

    // Open PNG file, and read data into Eigen matrix.
    // Display pixel view.

    Size pngSize;

    if constexpr (HasLoadGrayPng<PngPixel, Derived>)
    {
        GrayPng<PngPixel> png(deferFileName.Get());
        pngSize = png.GetSize();
        derived.LoadGrayPng(png);
    }
    else
    {
        // HasLoadPng
        Png<PngPixel> png(deferFileName.Get());
        pngSize = png.GetSize();
        derived.LoadPng(png);
    }

    viewSettingsControl.imageSize.Set(pngSize);
    viewSettingsControl.fitZoom.Trigger();

    derived.Display();
}


template<typename PngPixel, typename Derived>
void OpenPng(
    const pex::StringControl &,
    const ViewSettingsControl &,
    Derived &)
requires(!(HasLoadGrayPng<PngPixel, Derived> || HasLoadPng<PngPixel, Derived>))
{
    // There is nothing to do.
}


} // end namespace draw
