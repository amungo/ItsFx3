#ifndef POINT3D_T_H
#define POINT3D_T_H


class point3d_t
{
public:
    float x;
    float y;
    float z;
public:
    point3d_t( float x = 0.0f, float y = 0.0f, float z = 0.0f );
    void rotateOX( float angle_rad );
    void rotateOY( float angle_rad );
    void rotateOZ( float angle_rad );
};

#endif // POINT3D_T_H
