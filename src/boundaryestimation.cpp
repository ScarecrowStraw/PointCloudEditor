#include <pcl/apps/point_cloud_editor/boundaryestimation.h>
#include <QDebug>

std::vector<unsigned int> BoundaryEstimation::getBoundary(PclCloudPtr cloud)
{
    pcl::PointCloud<pcl::Normal>::Ptr normals (new pcl::PointCloud<pcl::Normal>);
    pcl::PointCloud<pcl::Boundary> boundaries;
    pcl::BoundaryEstimation<Point3D,pcl::Normal,pcl::Boundary> est;
    pcl::search::KdTree<Point3D>::Ptr tree(new pcl::search::KdTree<Point3D>());

    pcl::NormalEstimation<Point3D,pcl::Normal> normEst; 
    normEst.setInputCloud(cloud);
    normEst.setSearchMethod(tree);

    normEst.setKSearch(20); 
    normEst.compute(*normals);


    est.setInputCloud(cloud);
    est.setInputNormals(normals);
    //  est.setAngleThreshold(90);
    //   est.setSearchMethod (pcl::search::KdTree<Point3D>::Ptr (new pcl::search::KdTree<Point3D>));
    est.setSearchMethod (tree);
    est.setKSearch(100); 
    //  est.setRadiusSearch(everagedistance);  
    est.compute (boundaries);
    std::vector<unsigned int> boundariesIndex;
    for (unsigned int i=0; i<cloud->size(); i++){
        uint8_t x = (boundaries.points[i].boundary_point);
        int a = static_cast<int>(x); 
        if ( a == 1)
        {
            boundariesIndex.push_back(i);
        }
    }
    return boundariesIndex;
}
std::vector<unsigned int>
BoundaryEstimation::getBoundary(Cloud3D cloud)
{
    PclCloudPtr cloud_ptr(new Cloud3D(cloud));
    return getBoundary(cloud_ptr);
}
