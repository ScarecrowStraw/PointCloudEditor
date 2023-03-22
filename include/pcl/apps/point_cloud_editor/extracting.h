#ifndef EXTRACTING_H
#define EXTRACTING_H
#include <pcl/apps/point_cloud_editor/toolInterface.h>
#include <vector>
#include <QList>
#include <QLine>
#include <QPoint>
#include <pcl/apps/point_cloud_editor/screenpointconverter.h>

class Extracting:public ToolInterface
{
public:
    void start(int x, int y, BitMask modifiers, BitMask buttons) override ;
    void update (int x, int y, BitMask modifiers, BitMask buttons) override;
    void end (int x, int y, BitMask modifiers, BitMask buttons) override;
    void draw() const override;
    Extracting(CloudPtr cloud_ptr_,bool isColored,boost::shared_ptr<Converter> converter);
    ~Extracting();
    const Cloud3D& getInternalCloud();
private:

    Cloud3D cloud;
    std::vector<QPoint> screenPoints;
    QList<QLine> lines;
    CloudPtr cloud_ptr_;
    bool isInSelectBox(QPoint point);
    void createLine(QPoint p1,QPoint p2);
    void createLines();
    
    void checkPoints();
    int start_x=-1;
    int start_y=-1;
    bool isColored;
    boost::shared_ptr<Converter> converter;

};

#endif // EXTRACTING_H
