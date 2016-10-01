/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file  Moses3.cpp
 * @brief 3D Pose
 */


#include <gtsam/geometry/Moses3.h>
#include <gtsam/geometry/Pose2.h>
#include <gtsam/geometry/concepts.h>
#include <gtsam/base/Lie-inl.h>
#include <boost/foreach.hpp>
#include <iostream>
#include <cmath>

using namespace std;
using namespace Sophus;

namespace gtsam {

/** Explicit instantiation of base class to export members */
INSTANTIATE_LIE(Moses3);

/** instantiate concept checks */
GTSAM_CONCEPT_POSE_INST(Moses3);

static const Matrix3 I3 = eye(3), Z3 = zeros(3, 3), _I3 = -I3;
static const Matrix6 I6 = eye(6);
static const Matrix7 I7 = eye(7);



/* ************************************************************************* */
Moses3::Moses3(const Pose2& pose2) :
    Sim3(RxSO3(Rot3::rodriguez(0, 0, pose2.theta()).matrix()), (
        Point3(pose2.x(), pose2.y(), 0)).vector()) {
}

/* ************************************************************************* */
// Calculate Adjoint map
// Ad_pose is 6*6 matrix that when applied to twist xi, returns Ad_pose(xi)
// Experimental - unit tests of derivatives based on it do not check out yet
Matrix7 Moses3::AdjointMap() const {
  return this->Adj();  
}

/* ************************************************************************* */
// Calculate Adjoint map
// From Sim3
Matrix7 Moses3::AdjointMap7() const {
  return this->Adj();
}


/* ************************************************************************* */
//Remove
Matrix7 Moses3::adjointMap(const Vector& xi) {
  /*
  Matrix3 w_hat = skewSymmetric(xi(0), xi(1), xi(2));
  Matrix3 v_hat = skewSymmetric(xi(3), xi(4), xi(5));
  Matrix6 adj;
  adj << w_hat, Z3, v_hat, w_hat;*/

  return adjointMap7(xi);
}


/* ************************************************************************* */
Matrix7 Moses3::adjointMap7(const Vector& xi) {
  /*
  Matrix3 w_hat = skewSymmetric(xi(0), xi(1), xi(2));
  Matrix3 v_hat = skewSymmetric(xi(3), xi(4), xi(5));
  Matrix6 adj;
  adj << w_hat, Z3, v_hat, w_hat;*/

  return d_lieBracketab_by_d_a(xi);
}







/* ************************************************************************* */
Vector Moses3::adjoint(const Vector& xi, const Vector& y,
    boost::optional<Matrix&> H) {
  if (H) {
    *H = zeros(7, 7);
    for (int i = 0; i < 7; ++i) {
      Vector dxi = zero(7);
      dxi(i) = 1.0;
      Matrix Gi = adjointMap7(dxi);
      (*H).col(i) = Gi * y;
    }
  }
  return adjointMap7(xi) * y; // Should it be = Sim3::lieBracket(xi, y) or Sim3::lieBracket(y, xi). [a,b] = d/dt(adj(a)*b).
}


/* ************************************************************************* */
Vector Moses3::adjointTranspose(const Vector& xi, const Vector& y,
    boost::optional<Matrix&> H) {
  if (H) {
    *H = zeros(7, 7);
    for (int i = 0; i < 7; ++i) {
      Vector dxi = zero(7);
      dxi(i) = 1.0;
      Matrix GTi = adjointMap7(dxi).transpose();
      (*H).col(i) = GTi * y;
    }
  }
  return adjointMap7(xi).transpose() * y;
}



/* ************************************************************************* 
Matrix6 Moses3::dExpInv_exp(const Vector& xi) {
  // Bernoulli numbers, from Wikipedia
  static const Vector B = (Vector(9) << 1.0, -1.0 / 2.0, 1. / 6., 0.0, -1.0 / 30.0,
      0.0, 1.0 / 42.0, 0.0, -1.0 / 30);
  static const int N = 5; // order of approximation
  Matrix res = I6;
  Matrix6 ad_i = I6;
  Matrix6 ad_xi = adjointMap(xi);
  double fac = 1.0;
  for (int i = 1; i < N; ++i) {
    ad_i = ad_xi * ad_i;
    fac = fac * i;
    res = res + B(i) / fac * ad_i;
  }
  return res;
}*/

/* ************************************************************************* */
void Moses3::print(const string& s) const {
  cout << s;
  cout << *this; //DOES *this WORK here ??? Defined in Sim3
}

/* ************************************************************************* */
bool Moses3::equals(const Moses3& pose, double tol) const {

 const Matrix3 & Rself = this->rotationMatrix();  
 const Matrix3 & Rpose = pose.rotationMatrix();  
    

  return (Rot3(Rself)).equals(Rot3(Rpose), tol) && (Point3(this->Sim3::translation())).equals(Point3(pose.Sim3::translation()), tol) && (fabs(this->scale() - pose.scale()) < tol) ;
}

/* ************************************************************************* */
/** Test test */
Moses3 Moses3::Expmap(const Vector& xi) {
	return Moses3(Sim3::exp(xi));
}

/* **************REMOVE*********************************************************** */
Vector7 Moses3::Logmap(const Moses3& p) {
	Vector7 vec;
	return  Sim3::log(Sim3(p.rxso3(), p.Sim3::translation()));
}


/* btak remove************************************************************************* */
Vector7  Moses3::Logmap7(const Moses3& p) {
	Vector7 vec;
	return  Sim3::log(Sim3(p.rxso3(), p.Sim3::translation()));
}



/* ************************************************************************* */
// Different versions of retract
Moses3 Moses3::retract(const Vector& xi, Moses3::CoordinatesMode mode) const {
  if (mode == Moses3::EXPMAP) {
    // Lie group exponential map, traces out geodesic
    return compose(Expmap(xi));

  } /*else if (mode == Moses3::FIRST_ORDER) {
    // First order
    return retractFirstOrder(xi);
  

  }*/
  else {
    assert(false);
    exit(1);
  }
}

/* ************************************************************************* */
// different versions of localCoordinates
Vector7 Moses3::localCoordinates(const Moses3& T,
    Moses3::CoordinatesMode mode) const {
  if (mode == Moses3::EXPMAP) {
    // Lie group logarithm map, exact inverse of exponential map
    return Logmap7(between(T));
  } else {
    assert(false);
    exit(1);
  }
}

/* ************************************************************************* */
Matrix4 Moses3::matrix() const {
  return this->Sim3::matrix();
}

/* ************************************************************************* */
// Check scale man
Moses3 Moses3::transform_to(const Moses3& pose) const {

	const Point3 Tt(this->Sim3::translation());


  RxSO3 cRv = (rxso3_) * pose.rxso3().inverse();// Scale should not be lost here!! Using scso3. So this has scale included.
  Point3 t = pose.transform_to(Tt); // But this has scale included too. WTF.
  return Moses3(cRv, t);
}

/* ************************************************************************* */
Point3 Moses3::transform_from(const Point3& p, boost::optional<Matrix&> Dpose,
    boost::optional<Matrix&> Dpoint) const {
  


    const Matrix3 & Rself = this->rotationMatrix();  
	const Rot3 Rr(Rself);

	const double Ss(this->Sim3::scale());
	const Point3 Tt(this->Sim3::translation());

  if (Dpose) {
    const Matrix R = Rr.matrix();
    Matrix DR = R * skewSymmetric(-p.x(), -p.y(), -p.z()); //Should it have scso3 or just so3 ? It should have scso3 coz y = sRx + st. dy/dx = sR
    Dpose->resize(3, 7);
    Vector3 scaleCol=(Ss*(R*p.vector()));//TODO This needs checking. Formula obtained by matching with numerical derivative values. Make sure it theritically makes sense.
    //(scaleCol) << 0.2, 1.25977596, -1.70380883;//Bullshit hack REMOVE
    (*Dpose) <<  Ss*R, (Ss)*DR, scaleCol; //TODO This needs checking. Mostly incorrect. //should the scale component of jacobi be zero ? or 0 0 1 ?
  }
  if (Dpoint)
    *Dpoint = Ss*Rr.matrix(); //TODO This needs checking. Mostly incorrect.
  return Point3(static_cast<const Sim3&>(*this)*p.vector()); //Using Sim3 group action. //TODO This needs checking. Mostly correct.
}

/* ************************************************************************* */
// check scale man
// unrotate(p - t_)*s_ : P_world of point. t_ translation of new frame wrt world cordi in world coordi. Multiply (or divide?) with scale (axis strech).
Point3 Moses3::transform_to(const Point3& p, boost::optional<Matrix&> Dpose,
    boost::optional<Matrix&> Dpoint) const {

    const Matrix3 & Rself = this->rotationMatrix();  
	const Rot3 Rr(Rself);
	const double Ss(this->Sim3::scale());
	const Point3 Tt(this->Sim3::translation());

  const Point3 result = Rr.unrotate((p - Tt)/Ss); // should it be (p - 1/s_*t_) or 1/s_*(p - t_). Multiply or divide by scale. ? 
  												// We are multiplying in transform_from (t_ + s*Rot(P)), so here we divide. //TODO This needs checking. Mostly correct.
  if (Dpose) {
    const Point3& q = result;
    Matrix DR = skewSymmetric(q.x(), q.y(), q.z());
    Dpose->resize(3, 7); 
    Vector3 scaleCol=(-1)*q.vector();//TODO This needs checking. Mostly incorrect.
    (*Dpose) << _I3, DR, scaleCol; //TODO This needs checking. Mostly incorrect.
  }
  if (Dpoint)
    *Dpoint = (1/Ss)*Rr.transpose();  //TODO This needs checking. Mostly incorrect.
  return result;
}

/* ************************************************************************* */
Moses3 Moses3::compose(const Moses3& p2, boost::optional<Matrix&> H1,
    boost::optional<Matrix&> H2) const {
  if (H1)
    *H1 = p2.inverse().AdjointMap();
  if (H2)
    *H2 = I7;
  return (*this) * p2;
}

/* ************************************************************************* 
 // TODO: Test
  //	   Implement Derivatives
  */
Moses3 Moses3::inverse(boost::optional<Matrix&> H1) const {
  if (H1)
    *H1 = -AdjointMap(); // TODO Replace with correct function here
  Moses3 inv(this->Sim3::inverse());
  return inv;
}

/* ************************************************************************* */
// between = compose(p2,inverse(p1));
Moses3 Moses3::between(const Moses3& p2, boost::optional<Matrix&> H1,
    boost::optional<Matrix&> H2) const {
  Moses3 result = inverse() * p2;
  if (H1)
    *H1 = -result.inverse().AdjointMap();
  if (H2)
    *H2 = I7;
  return result;
}

/* ************************************************************************* */
double Moses3::range(const Point3& point, boost::optional<Matrix&> H1,
    boost::optional<Matrix&> H2) const {
  if (!H1 && !H2)
    return transform_to(point).norm();
  Point3 d = transform_to(point, H1, H2);
  double x = d.x(), y = d.y(), z = d.z(), d2 = x * x + y * y + z * z, n = sqrt(
      d2);
  Matrix D_result_d = (Matrix(1, 3) << x / n, y / n, z / n);
  if (H1)
    *H1 = D_result_d * (*H1);
  if (H2)
    *H2 = D_result_d * (*H2);
  return n;
}

/* ************************************************************************* */
// This needs proper testing
double Moses3::range(const Moses3& point, boost::optional<Matrix&> H1,
    boost::optional<Matrix&> H2) const {
  double r = range(Point3(point.Sim3::translation()), H1, H2); //IMPORTANT!!! Should it be just translation or scaled translation ?? Scaled translation spoils derivatives.
  if (H2) {
    Matrix H2_ = *H2 * point.rxso3().matrix(); //Checked via numerical derivatives
    *H2 = zeros(1, 7);
    insertSub(*H2, H2_, 0, 0);
  }
  return r;
}

/* ************************************************************************* */

/*
boost::optional<Moses3> align(const vector<Point3Pair>& pairs) {
  const size_t n = pairs.size();
  if (n < 3)
    return boost::none; // we need at least three pairs

  // calculate centroids
  Vector cp = zero(3), cq = zero(3);
  BOOST_FOREACH(const Point3Pair& pair, pairs){
  cp += pair.first.vector();
  cq += pair.second.vector();
}
  double f = 1.0 / n;
  cp *= f;
  cq *= f;

  // Add to form H matrix
  Matrix H = zeros(3, 3);
  BOOST_FOREACH(const Point3Pair& pair, pairs){
  Vector dp = pair.first.vector() - cp;
  Vector dq = pair.second.vector() - cq;
  H += dp * dq.transpose();
}

// Compute SVD
  Matrix U, V;
  Vector S;
  svd(H, U, S, V);

  // Recover transform with correction from Eggert97machinevisionandapplications
  Matrix UVtranspose = U * V.transpose();
  Matrix detWeighting = eye(3, 3);
  detWeighting(2, 2) = UVtranspose.determinant();
  Rot3 R(Matrix(V * detWeighting * U.transpose()));
  Point3 t = Point3(cq) - R * Point3(cp);
  return Moses3(R, t);
}

*/
/* ************************************************************************* */
std::ostream &operator<<(std::ostream &os, const Moses3& pose) {


  const Matrix3 & Rself = pose.rotationMatrix();  
  Rot3 rots(Rself);

  os << rots << "\n" << pose.scale() << "\n" << Point3(pose.translation()) << endl;
  return os;
}

} // namespace gtsam
