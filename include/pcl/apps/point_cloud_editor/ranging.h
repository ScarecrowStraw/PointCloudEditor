#pragma once

#include <qgl.h>
#include <pcl/apps/point_cloud_editor/toolInterface.h>
#include <pcl/apps/point_cloud_editor/localTypes.h>
#include <pcl/apps/point_cloud_editor/screenpointconverter.h>
#include <QDialog>
#include <QPushButton>
#include <QLayout>
#include <QLabel>
#include <QGLWidget>
#include <vector>
#include <QList>
#include <pcl/apps/point_cloud_editor/toolInterface.h>
#include <pcl/apps/point_cloud_editor/bestfitplane.h>

enum Plane
{
    XOY,
    YOZ,
    XOZ,
};

struct Point
{
public:
    Point(float X,float Y,float Z):x(X),y(Y),z(Z){}
    Point(){}
    float x;
    float y;
    float z;

};
class Ranging:public ToolInterface
{
public:
    Ranging(boost::shared_ptr<Converter> converter,CloudPtr cloud_ptr_,boost::shared_ptr<HightLightPoints> highLighter);
    ~Ranging();
    void reset();
    void undo();
    void start(int x, int y, BitMask modifiers, BitMask buttons) override ;
    void update (int x, int y, BitMask modifiers, BitMask buttons) override;
    void end (int x, int y, BitMask modifiers, BitMask buttons) override;
    void draw() const override;

    QList<QLine> getLines() const;
private:
    bool isClosed=false;
    int times=0;
    std::vector<QPoint> screenPositions;

    std::vector<Point> points;
    std::vector<int> indicies;
    void drawLines();
    void drawLine(QPoint point1,QPoint point2);
    void calculateArea();
    void calculatePerimeter();/
    CloudPtr cloud_ptr_;
    void highLight(int index);
    void update();
    float getDistance(Point point1,Point point2);
    boost::shared_ptr<Converter> converter;
    void getPoint3D(int x,int y);
    Point calculateCrossProduct(Point p1,Point p2);
    Point getProjectPoint(Point normal,Point original,Point p);
    boost::shared_ptr<HightLightPoints> highLighter;
    float getAreaOfTriangle(Point p1,Point p2);
    int mouse_x=0;
    int mouse_y=0;

    QList<QLine> lines;

};
