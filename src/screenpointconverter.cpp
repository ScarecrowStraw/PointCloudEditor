﻿#include <pcl/apps/point_cloud_editor/screenpointconverter.h>
#include <pcl/apps/point_cloud_editor/cloud.h>
#include <QApplication>
#include <QToolTip>
#include <math.h>
Converter::Converter(CloudPtr CloudPtr,boost::shared_ptr<HightLightPoints> highlight):cloud_ptr_(CloudPtr),
    highlight(highlight)
{
}

bool
Converter::isRightPoint(const Point3D& pt,
                        const GLfloat* project,
                        const GLint *viewport) const
{
    float w = pt.z * project[11];
    float x = (pt.x * project[0] + pt.z * project[8]) / w;
    float y = (pt.y * project[5] + pt.z * project[9]) / w;

    float screen_x = screen_pos_x/(viewport[2]*0.5) - 1.0;
    float screen_y = (viewport[3] -screen_pos_y)/(viewport[3]*0.5) - 1.0;
    // Ignore the points behind the camera

    float displace_x=std::abs(screen_x-x);
    float displace_y=std::abs(screen_y-y);

    if(displace_x<=mistake_dis&&displace_y<=mistake_dis)
        return true;
    else
        return false;
}

Converter::~Converter()
{}

bool
Converter::getDepthValue(int x, int y,Point3D& point)
{
    if (!cloud_ptr_)
        return false;
    screen_pos_x= x;
    screen_pos_y = y;
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
    GLfloat project[16];

    glGetFloatv(GL_PROJECTION_MATRIX, project);
    Point3DVector ptsvec;
    cloud_ptr_->getDisplaySpacePoints(ptsvec);
    for(std::size_t i = 0; i < ptsvec.size(); ++i)
    {
        Point3D pt = ptsvec[i];
        if (isRightPoint(pt, project, viewport))
        {
            point=cloud_ptr_->getInternalCloud()[i];
            return true;
        }
    }
    return false;
}

bool
Converter::getDepthValue(int x, int y,Point3D& point,int& index)
{
    if (!cloud_ptr_)
        return false;
    screen_pos_x= x;
    screen_pos_y = y;
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
    GLfloat project[16];

    glGetFloatv(GL_PROJECTION_MATRIX, project);
    Point3DVector ptsvec;
    cloud_ptr_->getDisplaySpacePoints(ptsvec);
    for(std::size_t i = 0; i < ptsvec.size(); ++i)
    {
        Point3D pt = ptsvec[i];
        if (isRightPoint(pt, project, viewport))
        {
            point=cloud_ptr_->getInternalCloud()[i];
            index=i;
            return true;
        }
    }
    return false;
}

QPoint
Converter::getScreenPosValue(Point3D pt)
{
    QPoint screenpos;
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
    GLfloat project[16];

    glGetFloatv(GL_PROJECTION_MATRIX, project);

    float w = pt.z * project[11];
    float x = (pt.x * project[0] + pt.z* project[8]) / w;
    float y = (pt.y * project[5] + pt.z* project[9]) / w;
    float pos_x=(x+1.0)*(viewport[2]*0.5);
    float pos_y=viewport[3]-(y+1.0)*(viewport[3]*0.5);
    screenpos=QPoint(qreal(pos_x),qreal(pos_y));
    return screenpos;
}

bool
Converter::isRenderer(Point3D &pt)
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
    GLfloat project[16];

    glGetFloatv(GL_PROJECTION_MATRIX, project);

    float w = pt.z * project[11];
    if(w>0)
        return true;
    return false;
}

Point3D
Converter::getDisplaySpacePoint(float x, float y, float z) const
{
    float center_x,center_y,center_z;
    cloud_ptr_->getCenter(center_x,center_y,center_z);
    Point3D pt;
    pt.x -= center_x;
    pt.y -= center_y;
    pt.z -= center_z;
    x = cloud_ptr_->getMatrix()[0] * pt.x +
            cloud_ptr_->getMatrix()[4] * pt.y +
            cloud_ptr_->getMatrix()[8] * pt.z +
            cloud_ptr_->getMatrix()[12];
    y = cloud_ptr_->getMatrix()[1] * pt.x +
            cloud_ptr_->getMatrix()[5] * pt.y +
            cloud_ptr_->getMatrix()[9] * pt.z +
            cloud_ptr_->getMatrix()[13];
    z = cloud_ptr_->getMatrix()[2] * pt.x +
            cloud_ptr_->getMatrix()[6] * pt.y +
            cloud_ptr_->getMatrix()[10] * pt.z +
            cloud_ptr_->getMatrix()[14];
    pt.x = x * cloud_ptr_->getScalingFactor();
    pt.y = y * cloud_ptr_->getScalingFactor();
    pt.z = z * cloud_ptr_->getScalingFactor();
    //    pt.z += cloud_ptr_->getDISPLAYZTRANSLATION();

    return (pt);
}

Point3D
Converter::getObjectSpacePoint(float x, float y, float z) const
{
    float center_x,center_y,center_z;
    cloud_ptr_->getCenter(center_x,center_y,center_z);
    Point3D pt;
    pt.x -= center_x;
    pt.y -= center_y;
    pt.z -= center_z;
    x = cloud_ptr_->getMatrix()[0] * pt.x +
            cloud_ptr_->getMatrix()[4] * pt.y +
            cloud_ptr_->getMatrix()[8] * pt.z +
            cloud_ptr_->getMatrix()[12];
    y = cloud_ptr_->getMatrix()[1] * pt.x +
            cloud_ptr_->getMatrix()[5] * pt.y +
            cloud_ptr_->getMatrix()[9] * pt.z +
            cloud_ptr_->getMatrix()[13];
    z = cloud_ptr_->getMatrix()[2] * pt.x +
            cloud_ptr_->getMatrix()[6] * pt.y +
            cloud_ptr_->getMatrix()[10] * pt.z +
            cloud_ptr_->getMatrix()[14];
    pt.x = x;
    pt.y = y;
    pt.z = z;
    return (pt);
}

