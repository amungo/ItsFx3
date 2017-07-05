#include <cmath>

#include "point3d_t.h"

point3d_t::point3d_t(float x, float y, float z) :
    x(x),
    y(y),
    z(z)
{
}

void point3d_t::rotateOX(float angle_rad)
{
    float sina = sinf( angle_rad );
    float cosa = cosf( angle_rad );
    float x1 = x;
    float y1 = y * cosa - z * sina;
    float z1 = y * sina + z * cosa;
    x = x1;
    y = y1;
    z = z1;
}

void point3d_t::rotateOY(float angle_rad)
{
    float sina = sinf( angle_rad );
    float cosa = cosf( angle_rad );
    float x1 =  x * cosa + z * sina;
    float y1 =  y;
    float z1 = -x * sina + z * cosa;
    x = x1;
    y = y1;
    z = z1;
}

void point3d_t::rotateOZ(float angle_rad)
{
    float sina = sinf( angle_rad );
    float cosa = cosf( angle_rad );
    float x1 =  x * cosa - y * sina;
    float y1 =  x * sina + y * cosa;
    float z1 = z;
    x = x1;
    y = y1;
    z = z1;
}
