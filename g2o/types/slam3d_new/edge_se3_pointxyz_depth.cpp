#include "edge_se3_pointxyz_depth.h"

namespace Slam3dNew {
  using namespace g2o;
  using namespace std;


  // point to camera projection, monocular
  EdgeSE3PointXYZDepth::EdgeSE3PointXYZDepth() : BaseBinaryEdge<3, Vector3d, VertexSE3, VertexPointXYZ>() {
    resizeParameters(1);
    installParameter(params, 0);
    information().setIdentity();
    information()(2,2)=100;
    J.fill(0);
    J.block<3,3>(0,0) = -Eigen::Matrix3d::Identity();
  }

  bool EdgeSE3PointXYZDepth::resolveCaches(){
    ParameterVector pv(1);
    pv[0]=params;
    resolveCache(cache, (OptimizableGraph::Vertex*)_vertices[0],"CACHE_CAMERA_NEW",pv);
    return cache;
  }




  bool EdgeSE3PointXYZDepth::read(std::istream& is) {
    int pid;
    is >> pid;
    setParameterId(0,pid);

    // measured keypoint
    Vector3d meas;
    for (int i=0; i<3; i++) is >> meas[i];
    setMeasurement(meas);
    // don't need this if we don't use it in error calculation (???)
    // information matrix is the identity for features, could be changed to allow arbitrary covariances    
    if (is.bad()) {
      return false;
    }
    for ( int i=0; i<information().rows() && is.good(); i++)
      for (int j=i; j<information().cols() && is.good(); j++){
  is >> information()(i,j);
  if (i!=j)
    information()(j,i)=information()(i,j);
      }
    if (is.bad()) {
      //  we overwrite the information matrix
      information().setIdentity();
      information()(2,2)=10/_measurement(2); // scale the info by the inverse of the measured depth
    } 
    return true;
  }

  bool EdgeSE3PointXYZDepth::write(std::ostream& os) const {
    os << params->id() << " ";
    for (int i=0; i<3; i++) os  << measurement()[i] << " ";
    for (int i=0; i<information().rows(); i++)
      for (int j=i; j<information().cols(); j++) {
        os <<  information()(i,j) << " ";
      }
    return os.good();
  }


  void EdgeSE3PointXYZDepth::computeError() {
    // from cam to point (track)
    //VertexSE3 *cam = static_cast<VertexSE3*>(_vertices[0]);
    VertexPointXYZ *point = static_cast<VertexPointXYZ*>(_vertices[1]);

    Eigen::Vector3d p = cache->w2i() * point->estimate();
    Eigen::Vector3d perr;
    perr.head<2>() = p.head<2>()/p(2);
    perr(2) = p(2);

    // error, which is backwards from the normal observed - calculated
    // _measurement is the measured projection
    _error = perr - _measurement;
    //    std::cout << _error << std::endl << std::endl;
  }

  void EdgeSE3PointXYZDepth::linearizeOplus() {
    //VertexSE3 *cam = static_cast<VertexSE3 *>(_vertices[0]);
    VertexPointXYZ *vp = static_cast<VertexPointXYZ *>(_vertices[1]);

    const Eigen::Vector3d& pt = vp->estimate();

    Eigen::Vector3d Zcam = cache->w2l() * pt;

    //  J(0,3) = -0.0;
    J(0,4) = -2*Zcam(2);
    J(0,5) = 2*Zcam(1);

    J(1,3) = 2*Zcam(2);
    //  J(1,4) = -0.0;
    J(1,5) = -2*Zcam(0);

    J(2,3) = -2*Zcam(1);
    J(2,4) = 2*Zcam(0);
    //  J(2,5) = -0.0;

    J.block<3,3>(0,6) = cache->w2l().rotation();

    Eigen::Matrix<double,3,9> Jprime = params->Kcam_inverseOffsetR()  * J;
    Eigen::Vector3d Zprime = cache->w2i() * pt;

    Eigen::Matrix<double, 3, 9> Jhom;
    Jhom.block<2,9>(0,0) = 1/(Zprime(2)*Zprime(2)) * (Jprime.block<2,9>(0,0)*Zprime(2) - Zprime.head<2>() * Jprime.block<1,9>(2,0));
    Jhom.block<1,9>(2,0) = Jprime.block<1,9>(2,0);

    _jacobianOplusXi = Jhom.block<3,6>(0,0);
    _jacobianOplusXj = Jhom.block<3,3>(0,6);
  }


  bool EdgeSE3PointXYZDepth::setMeasurementFromState(){
    //VertexSE3 *cam = static_cast<VertexSE3*>(_vertices[0]);
    VertexPointXYZ *point = static_cast<VertexPointXYZ*>(_vertices[1]);

    // calculate the projection
    const Vector3d& pt = point->estimate();

    Eigen::Vector3d p = cache->w2i() * pt;
    Eigen::Vector3d perr;
    perr.head<2>() = p.head<2>()/p(2);
    perr(2) = p(2);
    _measurement = perr;
    return true;
  }


  void EdgeSE3PointXYZDepth::initialEstimate(const OptimizableGraph::VertexSet& from, OptimizableGraph::Vertex* /*to_*/)
  {
    (void) from;
    assert(from.size() == 1 && from.count(_vertices[0]) == 1 && "Can not initialize VertexDepthCam position by VertexTrackXYZ");

    VertexSE3 *cam = dynamic_cast<VertexSE3*>(_vertices[0]);
    VertexPointXYZ *point = dynamic_cast<VertexPointXYZ*>(_vertices[1]);
    const Eigen::Matrix<double, 3, 3>& invKcam = params->invKcam();
    Eigen::Vector3d p;
    p(2) = _measurement(2);
    p.head<2>() = _measurement.head<2>()*p(2);
    p=invKcam*p;
    point->setEstimate(cam->estimate() * (params->offset() * p));
  }

}
