#include "draw/quad_brain.h"


namespace draw
{


QuadBounds::QuadBounds(QuadControl control)
    :
    shear(Bounds2d::Make(control.shear)),
    perspective(Bounds2d::Make(control.perspective)),
    rotation(control.rotation.GetBounds())
{

}


} // end namespace draw
