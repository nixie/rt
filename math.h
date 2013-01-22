/*
 * File:    math.h
 * Date:    22.11.2012 23:13
 * Author:  xferra00
 */
#ifndef __MATH_H__
#define __MATH_H__

#include "Eigen/Dense"

Eigen::Matrix3d calcRotationMatrix(double yaw, double pitch, double roll);

#endif
