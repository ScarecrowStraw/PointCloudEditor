#include <pcl/apps/point_cloud_editor/crackidentity.h>


using namespace Eigen;


pcl::PointCloud<pcl::PointXYZI>::Ptr cloudRGB2GRAY(pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud) {

    pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_gray(new pcl::PointCloud<pcl::PointXYZI>);
    cloud_gray->height = cloud->height;
    cloud_gray->width = cloud->width;

    for (pcl::PointCloud<pcl::PointXYZRGB>::iterator it = cloud->begin(); it != cloud->end(); it++) {
        // Color conversion
        cv::Mat pixel(1, 1, CV_8UC3, cv::Scalar(it->r, it->g, it->b));
        cv::Mat temp;
        cv::cvtColor(pixel, temp, CV_RGB2GRAY);

        pcl::PointXYZI pointI;
        pointI.x = it->x;
        pointI.y = it->y;
        pointI.z = it->z;
        pointI.intensity = temp.at<uchar>(0, 0);

        cloud_gray->push_back(pointI);

    }
    return cloud_gray;
}

pcl::PointCloud<pcl::PointXYZI>::Ptr cloudRGB2S(Cloud3D::Ptr cloud) {
    pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_gray(new pcl::PointCloud<pcl::PointXYZI>);
    cloud_gray->height = cloud->height;
    cloud_gray->width = cloud->width;

    for (Cloud3D::iterator it = cloud->begin(); it != cloud->end(); it++) {
        // Color conversion
        cv::Mat pixel(1, 1, CV_8UC3, cv::Scalar(it->r, it->g, it->b));
        cv::Mat temp;
        cv::cvtColor(pixel, temp, CV_RGB2HSV);

        pcl::PointXYZI pointI;
        pointI.x = it->x;
        pointI.y = it->y;
        pointI.z = it->z;
        pointI.intensity = temp.at<uchar>(0, 1);

        cloud_gray->push_back(pointI);

    }
    return cloud_gray;
}

Eigen::Matrix4f CreateRotateMatrix(Eigen::Vector3f before,Eigen::Vector3f after)
{
    before.normalize();
    after.normalize();

    float angle = acos(before.dot(after));
    Eigen::Vector3f p_rotate =before.cross(after);
    p_rotate.normalize();

    Eigen::Matrix4f rotationMatrix = Eigen::Matrix4f::Identity();
    rotationMatrix(0, 0) = cos(angle) + p_rotate[0] * p_rotate[0] * (1 - cos(angle));
    rotationMatrix(0, 1) = p_rotate[0] * p_rotate[1] * (1 - cos(angle) - p_rotate[2] * sin(angle));
    rotationMatrix(0, 2) = p_rotate[1] * sin(angle) + p_rotate[0] * p_rotate[2] * (1 - cos(angle));


    rotationMatrix(1, 0) = p_rotate[2] * sin(angle) + p_rotate[0] * p_rotate[1] * (1 - cos(angle));
    rotationMatrix(1, 1) = cos(angle) + p_rotate[1] * p_rotate[1] * (1 - cos(angle));
    rotationMatrix(1, 2) = -p_rotate[0] * sin(angle) + p_rotate[1] * p_rotate[2] * (1 - cos(angle));


    rotationMatrix(2, 0) = -p_rotate[1] * sin(angle) +p_rotate[0] * p_rotate[2] * (1 - cos(angle));
    rotationMatrix(2, 1) = p_rotate[0] * sin(angle) + p_rotate[1] * p_rotate[2] * (1 - cos(angle));
    rotationMatrix(2, 2) = cos(angle) + p_rotate[2] * p_rotate[2] * (1 - cos(angle));

    return rotationMatrix;
}


std::vector<unsigned int> getIntensityIndicies(const Cloud3D::Ptr& cloud_orig )
{
    pcl::PointCloud <pcl::PointXYZI>::Ptr cloud = cloudRGB2S(cloud_orig);

    cv::Mat gray_values(1, cloud->size(), CV_8U);
    cv::Mat temp;

    int counter = 0;
    for (pcl::PointCloud<pcl::PointXYZI>::iterator it = cloud->begin(); it != cloud->end(); it++) {
        gray_values.at<uchar>(0, counter) = it->intensity;
        counter++;
    }

    double thres_v = cv::threshold(gray_values, temp, 0, 255, CV_THRESH_OTSU);
    std::cout << "Otsu threshold value = " << thres_v << std::endl;
    unsigned int i=0;
    std::vector<unsigned int> indicies;
    for (pcl::PointCloud<pcl::PointXYZI>::iterator it = cloud->begin(); it != cloud->end(); it++) {
        float v = it->intensity;
        if (v >= thres_v)
        {
            indicies.push_back(i);
        }
        i++;
    }
    return indicies;
}




std::string intToString(int v)
{
    char buf[32] = {0};
    snprintf(buf, sizeof(buf), "%u", v);

    std::string str = buf;
    return str;
}


pcl::PointXYZRGBA findNearestPointsZDepth(std::vector<pcl::PointXYZRGBA> &points)
{
    float x,y,z;
    x=y=z=0;
    int r,g,b,a;
    pcl::PointXYZRGBA point;

    for(int i=0;i<points.size();i++)
    {
        x+=points[i].x;
        y+=points[i].y;
        z+=points[i].z;
        r+=points[i].r;
        g+=points[i].g;
        b+=points[i].b;
        a+=points[i].a;
    }

    point.x=x/points.size();
    point.y=y/points.size();
    point.z=z/points.size();
    point.r=r/points.size();
    point.g=g/points.size();
    point.b=b/points.size();
    point.a=a/points.size();
    return point;
}
std::vector<unsigned int> getEdgeIndex(const Cloud3D::Ptr& cloud,int knumbersneighborofedge)
{
    //   int KNumbersNeighbor = 80; // numbers of neighbors 7 , 120
    std::vector<int> NeighborsKNSearch(knumbersneighborofedge);
    std::vector<float> NeighborsKNSquaredDistance(knumbersneighborofedge);

    int* NumbersNeighbor = new  int [cloud ->points.size ()];
    pcl::KdTreeFLANN<pcl::PointXYZRGBA> kdtree;
    kdtree.setInputCloud (cloud);
    pcl::PointXYZRGBA searchPoint;

    double* Sigma = new  double [cloud->points.size()];

    //  ************ All the Points of the cloud *******************
    for (int i = 0; i < cloud ->points.size (); ++i) {

        searchPoint.x =   cloud->points[i].x;
        searchPoint.y =   cloud->points[i].y;
        searchPoint.z =   cloud->points[i].z;

        if ( kdtree.nearestKSearch (searchPoint, knumbersneighborofedge, NeighborsKNSearch, NeighborsKNSquaredDistance) > 0 ) {
            NumbersNeighbor[i]= NeighborsKNSearch.size (); }
        else { NumbersNeighbor[i] = 0; }

        float Xmean; float Ymean; float Zmean;
        float sum= 0.00;
        // Computing Covariance Matrix
        for (int ii = 0; ii < NeighborsKNSearch.size (); ++ii){
            sum += cloud->points[ NeighborsKNSearch[ii] ].x;
        }
        Xmean = sum / NumbersNeighbor[i] ;
        sum= 0.00;
        for (int ii = 0; ii < NeighborsKNSearch.size (); ++ii){
            sum += cloud->points[NeighborsKNSearch[ii] ].y;
            //sum += cloud->points[NeighborsKNSearch[ii] ].g;
        }
        Ymean = sum / NumbersNeighbor[i] ;
        sum= 0.00;
        for (int ii = 0; ii < NeighborsKNSearch.size (); ++ii){
            sum += cloud->points[NeighborsKNSearch[ii] ].z;
            //sum += cloud->points[NeighborsKNSearch[ii] ].b;
        }
        Zmean = sum / NumbersNeighbor[i] ;

        float	CovXX;  float CovXY; float CovXZ; float CovYX; float CovYY; float CovYZ; float CovZX; float CovZY; float CovZZ;

        sum = 0.00 ;
        for (int ii = 0; ii < NeighborsKNSearch.size (); ++ii){
            sum += ( (cloud->points[NeighborsKNSearch[ii] ].x - Xmean ) * ( cloud->points[NeighborsKNSearch[ii] ].x - Xmean )  );
            //sum += ( (cloud->points[NeighborsKNSearch[ii] ].r - Xmean ) * ( cloud->points[NeighborsKNSearch[ii] ].r - Xmean )  );
        }
        CovXX = sum / ( NumbersNeighbor[i]-1) ;

        sum = 0.00 ;
        for (int ii = 0; ii < NeighborsKNSearch.size (); ++ii){
            sum += ( (cloud->points[NeighborsKNSearch[ii] ].x - Xmean ) * ( cloud->points[NeighborsKNSearch[ii] ].y - Ymean )  );
            //sum += ( (cloud->points[NeighborsKNSearch[ii] ].r - Xmean ) * ( cloud->points[NeighborsKNSearch[ii] ].g - Ymean )  );
        }
        CovXY = sum / ( NumbersNeighbor[i]-1) ;

        CovYX = CovXY ;

        sum = 0.00 ;
        for (int ii = 0; ii < NeighborsKNSearch.size (); ++ii){
            sum += ( (cloud->points[NeighborsKNSearch[ii] ].x - Xmean ) * ( cloud->points[NeighborsKNSearch[ii] ].z - Zmean )  );
            //sum += ( (cloud->points[NeighborsKNSearch[ii] ].r - Xmean ) * ( cloud->points[NeighborsKNSearch[ii] ].b - Zmean )  );
        }
        CovXZ= sum / ( NumbersNeighbor[i]-1) ;

        CovZX = CovXZ;

        sum = 0.00 ;
        for (int ii = 0; ii < NeighborsKNSearch.size (); ++ii){
            sum += ( (cloud->points[NeighborsKNSearch[ii] ].y - Ymean ) * ( cloud->points[NeighborsKNSearch[ii] ].y - Ymean )  );
            //sum += ( (cloud->points[NeighborsKNSearch[ii] ].g - Ymean ) * ( cloud->points[NeighborsKNSearch[ii] ].g - Ymean )  );
        }
        CovYY = sum / ( NumbersNeighbor[i]-1) ;

        sum = 0.00 ;
        for (int ii = 0; ii < NeighborsKNSearch.size (); ++ii){
            sum += ( (cloud->points[NeighborsKNSearch[ii] ].y - Ymean ) * ( cloud->points[NeighborsKNSearch[ii] ].z - Zmean )  );
            //sum += ( (cloud->points[NeighborsKNSearch[ii] ].g - Ymean ) * ( cloud->points[NeighborsKNSearch[ii] ].b - Zmean )  );
        }
        CovYZ = sum / ( NumbersNeighbor[i]-1) ;

        CovZY = CovYZ;

        sum = 0.00 ;
        for (int ii = 0; ii < NeighborsKNSearch.size (); ++ii){
            sum += ( (cloud->points[NeighborsKNSearch[ii] ].z - Zmean ) * ( cloud->points[NeighborsKNSearch[ii] ].z - Zmean )  );
            //sum += ( (cloud->points[NeighborsKNSearch[ii] ].b - Zmean ) * ( cloud->points[NeighborsKNSearch[ii] ].b - Zmean )  );
        }
        CovZZ = sum / ( NumbersNeighbor[i]-1) ;

        // Computing Eigenvalue and EigenVector
        Matrix3f Cov;
        Cov << CovXX, CovXY, CovXZ, CovYX, CovYY, CovYZ, CovZX, CovZY, CovZZ;

        SelfAdjointEigenSolver<Matrix3f> eigensolver(Cov);
        if (eigensolver.info() != Success) abort();

        double EigenValue1 = eigensolver.eigenvalues()[0];
        double EigenValue2 = eigensolver.eigenvalues()[1];
        double EigenValue3 = eigensolver.eigenvalues()[2];

        double Smallest = 0.00; double Middle = 0.00; double Largest= 0.00;
        if (EigenValue1<  EigenValue2 ) { Smallest =  EigenValue1 ; } else { Smallest = EigenValue2 ; }
        if (EigenValue3<  Smallest ) { Smallest =  EigenValue3 ; }


        if(EigenValue1 <= EigenValue2 && EigenValue1 <= EigenValue3) {
            Smallest = EigenValue1;
            if(EigenValue2 <= EigenValue3) {Middle = EigenValue2; Largest = EigenValue3;}
            else {Middle = EigenValue3; Largest = EigenValue2;}
        }

        if(EigenValue1 >= EigenValue2 && EigenValue1 >= EigenValue3)
        {
            Largest = EigenValue1;
            if(EigenValue2 <= EigenValue3) { Smallest = EigenValue2; Middle = EigenValue3; }
            else {Smallest = EigenValue3; Middle = EigenValue2;}
        }

        if ((EigenValue1 >= EigenValue2 && EigenValue1 <= EigenValue3) || (EigenValue1 <= EigenValue2 && EigenValue1 >= EigenValue3))
        {
            Middle = EigenValue1;
            if(EigenValue2 >= EigenValue3){Largest = EigenValue2; Smallest = EigenValue3;}
            else{Largest = EigenValue3; Smallest = EigenValue2;}
        }


        Sigma[i] = (Smallest) / ( Smallest+ Middle+ Largest) ;

    } // For each point of the cloud

    std::cout<< " Computing Sigma is Done! " << std::endl;
    // Color Map For the difference of the eigen values

    double MaxD=0.00 ;
    double MinD= cloud ->points.size ();
    int Ncolors=256;

    for (int i = 0; i < cloud ->points.size (); ++i) {
        if (  Sigma [i] < MinD) MinD= Sigma [i];
        if (  Sigma[i] > MaxD) MaxD = Sigma [i];
    }

    //    for(int i=0;i<cloud->points.size();i++)
    //    {
    //        if(cloud->cloud.r+cloud->cloud.g+cloud->cloud.b<100)
    //        {
    //            cloud->cloud.r=255;
    //            cloud->cloud.g=0;
    //            cloud->cloud.b=0;
    //        }
    //    }
    std::cout<< " Minimum is :" << MinD<< std::endl;
    std::cout<< " Maximum  is :" << MaxD << std::endl;

    //   *****************************************
    int Edgepoints = 0;

    std::vector<unsigned int> indexes;
    float step = ( ( MaxD -  MinD) / Ncolors ) ;
    for (unsigned int i = 0; i < cloud ->points.size (); ++i) {
        if ( Sigma [i] > ( MinD + ( 8* step) ) ) {  //6*step

            if(cloud->points[i].r+cloud->points[i].g+cloud->points[i].b<200){
                indexes.push_back(i);
                Edgepoints ++;
            }
        }
    }
    std::cout<< " Number of Edge points  is :" << Edgepoints << std::endl;
    return indexes;

    //   *****************************************

}
std::vector<unsigned int> getColorChangedIndex(const Cloud3D::Ptr& cloud,int knumbersneighborofcolor)
{
    // int KNumbersNeighbor = 80; // numbers of neighbors 7 , 120
    std::vector<int> NeighborsKNSearch(knumbersneighborofcolor);
    std::vector<float> NeighborsKNSquaredDistance(knumbersneighborofcolor);

    int* NumbersNeighbor = new  int [cloud ->points.size ()];
    pcl::KdTreeFLANN<pcl::PointXYZRGBA> kdtree;
    kdtree.setInputCloud (cloud);
    pcl::PointXYZRGBA searchPoint;

    double* Sigma = new  double [cloud->points.size()];

    //  ************ All the Points of the cloud *******************
    for (int i = 0; i < cloud ->points.size (); ++i) {

        searchPoint.x =   cloud->points[i].x;
        searchPoint.y =   cloud->points[i].y;
        searchPoint.z =   cloud->points[i].z;

        if ( kdtree.nearestKSearch (searchPoint, knumbersneighborofcolor, NeighborsKNSearch, NeighborsKNSquaredDistance) > 0 ) {
            NumbersNeighbor[i]= NeighborsKNSearch.size (); }
        else { NumbersNeighbor[i] = 0; }

        float Xmean; float Ymean; float Zmean;
        float sum= 0.00;
        // Computing Covariance Matrix
        for (int ii = 0; ii < NeighborsKNSearch.size (); ++ii){
            //   sum += cloud->points[ NeighborsKNSearch[ii] ].x;
            sum += cloud->points[ NeighborsKNSearch[ii] ].r;
        }
        Xmean = sum / NumbersNeighbor[i] ;
        sum= 0.00;
        for (int ii = 0; ii < NeighborsKNSearch.size (); ++ii){
            //  sum += cloud->points[NeighborsKNSearch[ii] ].y;
            sum += cloud->points[NeighborsKNSearch[ii] ].g;
        }
        Ymean = sum / NumbersNeighbor[i] ;
        sum= 0.00;
        for (int ii = 0; ii < NeighborsKNSearch.size (); ++ii){
            //    sum += cloud->points[NeighborsKNSearch[ii] ].z;
            sum += cloud->points[NeighborsKNSearch[ii] ].b;
        }
        Zmean = sum / NumbersNeighbor[i] ;

        float	CovXX;  float CovXY; float CovXZ; float CovYX; float CovYY; float CovYZ; float CovZX; float CovZY; float CovZZ;

        sum = 0.00 ;
        for (int ii = 0; ii < NeighborsKNSearch.size (); ++ii){
            //      sum += ( (cloud->points[NeighborsKNSearch[ii] ].x - Xmean ) * ( cloud->points[NeighborsKNSearch[ii] ].x - Xmean )  );
            sum += ( (cloud->points[NeighborsKNSearch[ii] ].r - Xmean ) * ( cloud->points[NeighborsKNSearch[ii] ].r - Xmean )  );
        }
        CovXX = sum / ( NumbersNeighbor[i]-1) ;

        sum = 0.00 ;
        for (int ii = 0; ii < NeighborsKNSearch.size (); ++ii){
            // sum += ( (cloud->points[NeighborsKNSearch[ii] ].x - Xmean ) * ( cloud->points[NeighborsKNSearch[ii] ].y - Ymean )  );
            sum += ( (cloud->points[NeighborsKNSearch[ii] ].r - Xmean ) * ( cloud->points[NeighborsKNSearch[ii] ].g - Ymean )  );
        }
        CovXY = sum / ( NumbersNeighbor[i]-1) ;

        CovYX = CovXY ;

        sum = 0.00 ;
        for (int ii = 0; ii < NeighborsKNSearch.size (); ++ii){
            //      sum += ( (cloud->points[NeighborsKNSearch[ii] ].x - Xmean ) * ( cloud->points[NeighborsKNSearch[ii] ].z - Zmean )  );
            sum += ( (cloud->points[NeighborsKNSearch[ii] ].r - Xmean ) * ( cloud->points[NeighborsKNSearch[ii] ].b - Zmean )  );
        }
        CovXZ= sum / ( NumbersNeighbor[i]-1) ;

        CovZX = CovXZ;

        sum = 0.00 ;
        for (int ii = 0; ii < NeighborsKNSearch.size (); ++ii){
            //   sum += ( (cloud->points[NeighborsKNSearch[ii] ].y - Ymean ) * ( cloud->points[NeighborsKNSearch[ii] ].y - Ymean )  );
            sum += ( (cloud->points[NeighborsKNSearch[ii] ].g - Ymean ) * ( cloud->points[NeighborsKNSearch[ii] ].g - Ymean )  );
        }
        CovYY = sum / ( NumbersNeighbor[i]-1) ;

        sum = 0.00 ;
        for (int ii = 0; ii < NeighborsKNSearch.size (); ++ii){
            //           sum += ( (cloud->points[NeighborsKNSearch[ii] ].y - Ymean ) * ( cloud->points[NeighborsKNSearch[ii] ].z - Zmean )  );
            sum += ( (cloud->points[NeighborsKNSearch[ii] ].g - Ymean ) * ( cloud->points[NeighborsKNSearch[ii] ].b - Zmean )  );
        }
        CovYZ = sum / ( NumbersNeighbor[i]-1) ;

        CovZY = CovYZ;

        sum = 0.00 ;
        for (int ii = 0; ii < NeighborsKNSearch.size (); ++ii){
            //         sum += ( (cloud->points[NeighborsKNSearch[ii] ].z - Zmean ) * ( cloud->points[NeighborsKNSearch[ii] ].z - Zmean )  );
            sum += ( (cloud->points[NeighborsKNSearch[ii] ].b - Zmean ) * ( cloud->points[NeighborsKNSearch[ii] ].b - Zmean )  );
        }
        CovZZ = sum / ( NumbersNeighbor[i]-1) ;

        // Computing Eigenvalue and EigenVector
        Matrix3f Cov;
        Cov << CovXX, CovXY, CovXZ, CovYX, CovYY, CovYZ, CovZX, CovZY, CovZZ;

        SelfAdjointEigenSolver<Matrix3f> eigensolver(Cov);
        if (eigensolver.info() != Success) abort();

        double EigenValue1 = eigensolver.eigenvalues()[0];
        double EigenValue2 = eigensolver.eigenvalues()[1];
        double EigenValue3 = eigensolver.eigenvalues()[2];

        double Smallest = 0.00; double Middle = 0.00; double Largest= 0.00;
        if (EigenValue1<  EigenValue2 ) { Smallest =  EigenValue1 ; } else { Smallest = EigenValue2 ; }
        if (EigenValue3<  Smallest ) { Smallest =  EigenValue3 ; }


        if(EigenValue1 <= EigenValue2 && EigenValue1 <= EigenValue3) {
            Smallest = EigenValue1;
            if(EigenValue2 <= EigenValue3) {Middle = EigenValue2; Largest = EigenValue3;}
            else {Middle = EigenValue3; Largest = EigenValue2;}
        }

        if(EigenValue1 >= EigenValue2 && EigenValue1 >= EigenValue3)
        {
            Largest = EigenValue1;
            if(EigenValue2 <= EigenValue3) { Smallest = EigenValue2; Middle = EigenValue3; }
            else {Smallest = EigenValue3; Middle = EigenValue2;}
        }

        if ((EigenValue1 >= EigenValue2 && EigenValue1 <= EigenValue3) || (EigenValue1 <= EigenValue2 && EigenValue1 >= EigenValue3))
        {
            Middle = EigenValue1;
            if(EigenValue2 >= EigenValue3){Largest = EigenValue2; Smallest = EigenValue3;}
            else{Largest = EigenValue3; Smallest = EigenValue2;}
        }


        Sigma[i] = (Smallest) / ( Smallest+ Middle+ Largest) ;

    } // For each point of the cloud

    std::cout<< " Computing Sigma is Done! " << std::endl;
    // Color Map For the difference of the eigen values

    double MaxD=0.00 ;
    double MinD= cloud ->points.size ();
    int Ncolors=256;

    for (int i = 0; i < cloud ->points.size (); ++i) {
        if (  Sigma [i] < MinD) MinD= Sigma [i];
        if (  Sigma[i] > MaxD) MaxD = Sigma [i];
    }

    //    for(int i=0;i<cloud->points.size();i++)
    //    {
    //        if(cloud->cloud.r+cloud->cloud.g+cloud->cloud.b<100)
    //        {
    //            cloud->cloud.r=255;
    //            cloud->cloud.g=0;
    //            cloud->cloud.b=0;
    //        }
    //    }
    std::cout<< " Minimum is :" << MinD<< std::endl;
    std::cout<< " Maximum  is :" << MaxD << std::endl;

    //   *****************************************
    int Edgepoints = 0;

    std::vector<unsigned int> indexes;
    float step = ( ( MaxD -  MinD) / Ncolors ) ;
    for (unsigned int i = 0; i < cloud ->points.size (); ++i) {
        if ( Sigma [i] > ( MinD + ( 8* step) ) ) {  //6*step
            if(cloud->points[i].r+cloud->points[i].g+cloud->points[i].b<200){
                indexes.push_back(i);
                Edgepoints ++;
            }
        }
    }
    std::cout<< " Number of Color Changed points  is :" << Edgepoints << std::endl;
    return indexes;

    //   *****************************************

}




//std::vector<unsigned int>
//crackdetect(const Cloud3D::Ptr& cloud,int knumbersneighborofcolor,int knumbersneighborofedge)
//{
//    clock_t startTime,endTime;
//    startTime = clock();
//    // 提取出平面
//    pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients);
//    pcl::PointIndices::Ptr inliers (new pcl::PointIndices);
//    // Create the segmentation object
//    pcl::SACSegmentation<pcl::PointXYZRGBA> seg;
//    // Optional
//    seg.setOptimizeCoefficients (true);
//    // Mandatory
//    seg.setModelType (pcl::SACMODEL_PLANE);
//    seg.setMethodType (pcl::SAC_RANSAC);
//    seg.setMaxIterations(1000);
//    seg.setDistanceThreshold (0.5);

//    //pcl::io::savePCDFileBinary("No_ground.pcd", *cloud_filtered);
//    std::vector<std::vector<int>> crackindicies;
//    std::vector<unsigned int> indexOtsu(getIntensityIndicies(cloud));
//    std::vector<unsigned int> indexedge(getEdgeIndex(cloud,knumbersneighborofedge));
//    //  std::vector<unsigned int> indexcolor(getColorChangedIndex(cloud,knumbersneighborofcolor));
//    // std::cerr<<"index edge size is "<<indexedge.size()<<std::endl;
//    std::vector<unsigned int> result,temp;

//    std::set_intersection(indexedge.begin(),indexedge.end(),indexOtsu.begin(),indexOtsu.end(),std::back_inserter(result));
//    //    if(temp.size()!=0){

//    //        std::set_intersection(temp.begin(),temp.end(),indexOtsu.begin(),indexOtsu.end(),std::back_inserter(result));
//    ////        if(result.size()==0)
//    ////        {
//    ////            std::cerr<<result.size()<<std::endl;
//    ////        }
//    ////        else{
//    ////            crackindicies.push_back(result);
//    ////        }
//    //        //crackindicies.push_back(indexedge);
//    //    }
//    //    else
//    //    {
//    //        std::cerr<<"temp has a length of 0"<<std::endl;
//    //    }

//    endTime=clock();
//    cout << "total time spent : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;

//    return result;
//}


void
crackdetect(const Cloud3D::Ptr& cloud,int k,float thresh,Cloud3D::Ptr& cloud_filtered)
{
    clock_t startTime,endTime;
    startTime = clock();
    // std::vector<unsigned int> indexOtsu(getIntensityIndicies(cloud));
    pcl::PointIndices::Ptr indexOtsu(new pcl::PointIndices);
    Cloud3D::Ptr cloud_ptr(new Cloud3D);
    *cloud_ptr=*cloud;
    IndexVector indices = getIntensityIndicies(cloud);

    for(IndexVector::iterator iter=indices.begin();iter!=indices.end();iter++)
    {
        indexOtsu->indices.push_back((*iter));
    }
    //   return indexOtsu;

    //TODO European split

    //TODO:Extract point
    pcl::ExtractIndices<Point3D> extract;
    extract.setNegative(false);
    extract.setInputCloud (cloud_ptr);
    extract.setIndices (indexOtsu);
    extract.filter(*cloud_filtered);
    //TODO: remove outliers
    pcl::StatisticalOutlierRemoval<Point3D> sor;
    sor.setInputCloud (cloud_filtered);
    sor.setMeanK (k);
    sor.setStddevMulThresh (thresh);
    sor.filter (*cloud_filtered);

    endTime=clock();
    cout << "total time spent : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
}


