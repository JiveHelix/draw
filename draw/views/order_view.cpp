#include "draw/views/order_view.h"
#include <wxpex/button.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/layout_items.h>



namespace draw
{


OrderView::OrderView(wxWindow *parent, const pex::OrderControl &order)
    :
    wxControl(parent, wxID_ANY)
{
    auto down = new wxpex::Button(this, "v", order.moveDown);

    auto up = new wxpex::Button(this, "^", order.moveUp);

    auto upDownSizer = wxpex::LayoutItems(wxpex::horizontalItems, down, up);

    this->SetSizerAndFit(upDownSizer.release());
}



} // end namespace draw
