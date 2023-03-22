#pragma once

#include <qgl.h>
#include <pcl/apps/point_cloud_editor/toolInterface.h>
#include <pcl/apps/point_cloud_editor/localTypes.h>
#include <pcl/apps/point_cloud_editor/screenpointconverter.h>
#include <QPointF>
class DisplayDepthValue
{
public:
    DisplayDepthValue();
    ~DisplayDepthValue();
    void getDepthValue(int x, int y,const QPointF pos,boost::shared_ptr<Converter> convert);
private:
    CloudPtr cloud_ptr_;
    float screen_pos_x;
    float screen_pos_y;
    float mistake_dis=0.01;
};
