#ifndef POINT_CLOUD_H
#define POINT_CLOUD_H

#include <vector>

#include <pcl/point_cloud.h>
#include <opencv2/core/core.hpp>
#include <lemon/list_graph.h>
#include <flann/flann.hpp>

#include "graph_map.h"
#include "parameter_map.h"
#include "types.h"


class PointCloud: public pcl::PointCloud<Point>
{
public:
    typedef lemon::ListGraph DeformationGraph;

public:
    PointCloud();
    virtual ~PointCloud();

    void load(const std::string &file, bool flag = true); // true for structured, false for unstructured
    Point getPointFromDepthMap(int u, int v);

    void binding(); // construct deformation graph

    void setNodeNum(size_t node_num);

    inline size_t getNodeNum() {
        return node_num_;
    }

    inline DeformationGraph *getDeformationGraph() {
        return deformation_graph_;
    }

    inline GraphMap *getGraphMap() {

        return graph_map_;
    }

    inline ParameterMap *getParameterMap() {
        return parameter_map_;
    }

    inline Eigen::Vector3d& getMassCenter() {
        return mass_center_;
    }
    
    inline cv::Mat& getDepthMap(){
        return depth_map_;
    }

    void transform();
    void transform(size_t index);
    void update();
     
    static void print(PointCloud *pointcloud) {
        for (size_t i = 0, i_end = pointcloud->size(); i < i_end; i ++) {
            const Point &point = pointcloud->at(i);
            std::cout << "x:" << point.x << " y:" << point.y << " z:" << point.z << std::endl;
        }
    }
    
    void setColor(size_t r, size_t g, size_t b);
    
    void getCorrespondenceByKnn(pcl::PointCloud<Point>::Ptr target_knn, PointCloud* target);
    void smoothDependency();
    
private:
    virtual void sampling();
    void connecting();
    void parameterize();

    void kNearestSearch(const int k);
    void evaluateNormal();
    void evaluateMassCenter();

    Point localTransform(size_t j);
    Point globalTransform(const Point &point);
    bool isGraphNode(size_t j);

public:
    Eigen::Vector3d rigid_rot_;  // axis-angle form, only need three parameters
    Eigen::Vector3d rigid_trans_;

private:
    cv::Mat depth_map_;

    size_t node_num_;
    DeformationGraph *deformation_graph_;
    GraphMap *graph_map_;
    ParameterMap *parameter_map_;
    std::map<size_t, double *> unknowns_map_;

    static const int k_ = 4;
    flann::Matrix<int> *nearest_neighbors_;
    flann::Matrix<double> *neighbor_dists_;
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> dependency_weights_;

    Eigen::Vector3d mass_center_;

};


// to remove same edges because lemon can only support the mode of adding edges between same nodes
struct Edge {
    Edge(int source, int target): _source(source), _target(target) {}
    int _source;
    int _target;
};

// compare function for user-defined std::set
// must have strict weak ordering
struct CompareEdge {
    bool operator()(const Edge &edge1, const Edge &edge2) {

        if ((edge1._source == edge2._source && edge1._target == edge2._target)
                || (edge1._source == edge2._target && edge1._target == edge2._source)) {
            return false;
        }

        if ((edge1._source == edge2._source && edge1._target < edge2._target)
                || (edge1._source < edge2._source)) {
            return true;
        }

        if ((edge1._source == edge2._source && edge1._target > edge2._target)
                || (edge1._source > edge2._source)) {
            return false;
        }
    }
};

#endif //POINT_CLOUD_H
