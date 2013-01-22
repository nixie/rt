#include "math.h"

using namespace Eigen;

Matrix3d calcRotationMatrix(double yaw, double pitch, double roll){
    Matrix3d Rx, Ry, Rz;
    Rx.setZero(); Ry.setZero(); Rz.setZero();

    Rx(0,0) = 1.0;
    Rx(1,1) = Rx(2,2) = cos(yaw);
    Rx(2,1) = sin(yaw);
    Rx(1,2) = -sin(yaw);

    Ry(1,1) = 1.0;
    Ry(0,0) = Ry(2,2) = cos(pitch);
    Ry(0,2) = sin(pitch);
    Ry(2,0) = -sin(pitch);

    Rz(2,2) = 1.0;
    Rz(0,0) = Rz(1,1) = cos(roll);
    Rz(1,0) = sin(roll);
    Rz(0,1) = -sin(roll);

    return Rx*Ry*Rz;
}


