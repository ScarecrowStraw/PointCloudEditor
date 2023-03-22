#include <pcl/apps/point_cloud_editor/displayDepthValue.h>
#include <pcl/apps/point_cloud_editor/cloud.h>
#include <pcl/apps/point_cloud_editor/screenpointconverter.h>
#include <QApplication>
#include <QToolTip>
DisplayDepthValue::DisplayDepthValue()
{
}

DisplayDepthValue::~DisplayDepthValue()
{}


// First of all, it should be judged whether the mouse event is stopped
void
DisplayDepthValue::getDepthValue(int x, int y,const QPointF screen_pos,boost::shared_ptr<Converter> convert)
{
    Point3D point;
    if(convert->getDepthValue(x,y,point)){
        qDebug("depth value:%f",point.z);
        QString str("depth value:");
        str.append(QString::number(point.z));
        QToolTip::showText(screen_pos.toPoint(),str);
        // convert->Test(point);
    }
    else
    {
        qDebug("no points selected");
    }
}
