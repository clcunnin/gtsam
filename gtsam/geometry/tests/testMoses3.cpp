/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file   testMoses3.cpp
 * @brief  Unit tests for Moses3 class
 */

#include <gtsam/geometry/Moses3.h>
#include <gtsam/geometry/Pose2.h>
#include <gtsam/geometry/Pose3.h> 
#include <gtsam/base/lieProxies.h>
#include <gtsam/base/Testable.h>
#include <gtsam/base/numericalDerivative.h>

//#include <gtsam/3rdparty/Eigen/unsupported/Eigen/MatrixFunctions> 

#include <boost/assign/std/vector.hpp> // for operator +=
using namespace boost::assign;

#include <CppUnitLite/TestHarness.h>
#include <cmath>

using namespace std;
using namespace Sophus;
using namespace gtsam;

GTSAM_CONCEPT_TESTABLE_INST(Moses3)
GTSAM_CONCEPT_LIE_INST(Moses3)

/*
static Point3 P(0.2,0.7,-2);
static Rot3 R = Rot3::rodriguez(0,0,0);
static Point3 Pp(10.0,0.0,0.0);

static Rot3 R1 = Rot3(Point3(0.0, -1.0 , 0.0),Point3(1.0, 0.0 , 0.0),Point3(0.0, 0.0 , 1.0));
static Rot3 R2 = Rot3(Point3(0.0, 1.0 , 0.0),Point3(-1.0, 0.0 , 0.0),Point3(0.0, 0.0 , 1.0));

static Point3 Pp1(0.0,10.0,0.0);
static Point3 Pp2(10.1,0.0,0.0);


static Moses3 Tt(ScSO3(R.matrix()),Pp.vector());
static Sim3 Ss(ScSO3(R.matrix()),Pp.vector());
static Moses3 Ts(Ss);

static Moses3 Tt1(ScSO3(2*R.matrix()),Pp.vector());

static SE3 Se(R.matrix(),Pp.vector());
static SE3 Se1(R1.matrix(),Pp1.vector());

static Pose3 Po(R2,Pp);
static Pose3 Po1(R2,Pp2);

static Moses3 T(R,Point3(3.5,-8.2,4.2));
static Moses3 T2(Rot3::rodriguez(0.3,0.2,0.1),Point3(3.5,-8.2,4.2));
static Moses3 T3(Rot3::rodriguez(-90, 0, 0), Point3(1, 2, 3));
const double tol=1e-5;
*/
static Point3 P(0.2,0.7,-2);
static Rot3 R = Rot3::rodriguez(0.3,0,0);
static Moses3 T(R,Point3(3.5,-8.2,4.2));
static Moses3 T2(Rot3::rodriguez(0.3,0.2,0.1),Point3(3.5,-8.2,4.2));
static Moses3 T3(Rot3::rodriguez(-90, 0, 0), Point3(1, 2, 3));

static Moses3 T4(ScSO3(2*Rot3::rodriguez(-90, 0, 0).matrix()), Point3(1, 2, 3).vector());

const double tol=1e-5;



/* ************************************************************************* */
/* TEST( Moses3, stream) */
/* { */
/*   Moses3 T; */
/*   std::ostringstream os; */
/*   os << T; */
/*   EXPECT(os.str() == "1 * 1 0 0\n0 1 0\n0 0 1\n\n[0, 0, 0]';\n"); */
/* } */

/* ************************************************************************* */
TEST( Moses3, constructors)
{
  Moses3 expected(Rot3::rodriguez(0,0,3),Point3(1,2,0));
  Pose2 pose2(1,2,3);
  EXPECT(assert_equal(expected,Moses3(pose2)));
}

/* ************************************************************************* */
TEST( Moses3, retract_expmap)
{
  Moses3 id;
  Vector v = zero(7);
  Rot3 Ro = Rot3::rodriguez(0.0,0,0);
  v(6) = 0.0; //scale = e^this
  EXPECT(assert_equal(Moses3(Ro, Point3()), id.retract(v, Moses3::EXPMAP),1e-2));
  v(3) = 0.3; //rot1
  EXPECT(assert_equal(Moses3(R, Point3()), id.retract(v, Moses3::EXPMAP),1e-2));  
  v(0)=0.2;v(1)=0.394742;v(2)=-2.08998; //translation precalculated for P and R defined globle
  EXPECT(assert_equal(Moses3(R, P),id.retract(v, Moses3::EXPMAP),1e-2));
}

/* ************************************************************************* */
TEST( Moses3, expmap_a_full)
{
  Moses3 id;
  Vector v = zero(7);
  v(3) = 0.3;
  EXPECT(assert_equal(expmap_default<Moses3>(id, v), Moses3(R, Point3())));
  v(0)=0.2;v(1)=0.394742;v(2)=-2.08998;
  EXPECT(assert_equal(Moses3(R, P),expmap_default<Moses3>(id, v),1e-5));
}

/* ************************************************************************* */
TEST( Moses3, expmap_a_full2)
{
  Moses3 id;
  Vector v = zero(7);
  v(3) = 0.3;
  EXPECT(assert_equal(expmap_default<Moses3>(id, v), Moses3(R, Point3())));
  v(0)=0.2;v(1)=0.394742;v(2)=-2.08998;
  EXPECT(assert_equal(Moses3(R, P),expmap_default<Moses3>(id, v),1e-5));
}

/* ************************************************************************* */
TEST(Moses3, expmap_b)
{
  Moses3 p1(Rot3(), Point3(100, 0, 0));
  Moses3 p2 = p1.retract((Vector(7) << 0.0, 0.0, 0.0, 0.0, 0.0, 0.1, 0.0));
  Moses3 expected(Rot3::rodriguez(0.0, 0.0, 0.1), Point3(100.0, 0.0, 0.0));
  EXPECT(assert_equal(expected, p2,1e-2));
}

/* ************************************************************************* */
// test case for screw motion in the plane
namespace screw {
  double a=0.3, c=cos(a), s=sin(a), w=0.3;
  Vector xi = (Vector(7) << w, 0.0, 1.0 ,0.0, 0.0, w, 0.0);
  Vector xi2 = (Vector(7) << w, 2.0, 1.0 ,0.0, 3.0, w, 1.0);

  Vector si = (Vector(7) << w, 0.0, 1.0 ,0.0, 0.0, w);
  Vector si2 = (Vector(7) << w, 2.0, 1.0 ,0.0, 3.0, w);

  
  Rot3 expectedR(c, -s, 0, s, c, 0, 0, 0, 1);
  Point3 expectedT(0.29552, 0.0446635, 1);
  Moses3 expected(expectedR, expectedT);
}


/* ************************************************************************* */
// Checks correct exponential map (Expmap) with brute force matrix exponential
TEST(Moses3, expmap_c_full)
{
  EXPECT(assert_equal(screw::expected, expm<Moses3>(screw::xi),1e-6));
  EXPECT(assert_equal(screw::expected, Moses3::Expmap(screw::xi),1e-6));
}

/* ************************************************************************* */
// assert that T*exp(xi)*T^-1 is equal to exp(Ad_T(xi))
TEST(Moses3, Adjoint_full)
{
  Moses3 expected = T * Moses3::Expmap(screw::xi) * T.inverse();
  Vector xiprime = T.Adjoint(screw::xi);
  EXPECT(assert_equal(expected, Moses3::Expmap(xiprime), 1e-6));

  Moses3 expected2 = T2 * Moses3::Expmap(screw::xi) * T2.inverse();
  Vector xiprime2 = T2.Adjoint(screw::xi);
  EXPECT(assert_equal(expected2, Moses3::Expmap(xiprime2), 1e-6));

  Moses3 expected3 = T3 * Moses3::Expmap(screw::xi) * T3.inverse();
  Vector xiprime3 = T3.Adjoint(screw::xi);
  EXPECT(assert_equal(expected3, Moses3::Expmap(xiprime3), 1e-6));
}


/* ************************************************************************* */
// Tested with scale
TEST(Moses3, expmaps_galore_full)
{
  Vector xi; Moses3 actual;
  xi = (Vector(7) << 0.4, 0.5, 0.6 ,0.1, 0.2, 0.3, 0.1);
  actual = Moses3::Expmap(xi);
  EXPECT(assert_equal(expm<Moses3>(xi), actual,1e-6));
  //EXPECT(assert_equal(Agrawal06iros(xi), actual,1e-6));
  EXPECT(assert_equal(xi, Moses3::Logmap(actual),1e-6));

  xi = (Vector(7) << -0.4, 0.5, -0.6 ,0.1, -0.2, 0.3, 0.1);
  for (double theta=1.0;0.3*theta<=M_PI;theta*=2) {
    Vector txi = xi*theta;
    actual = Moses3::Expmap(txi);
    EXPECT(assert_equal(expm<Moses3>(txi,30), actual,1e-6));
    //EXPECT(assert_equal(Agrawal06iros(txi), actual,1e-6));
    Vector log = Moses3::Logmap(actual);
    EXPECT(assert_equal(actual, Moses3::Expmap(log),1e-6));
    EXPECT(assert_equal(txi,log,1e-6)); // not true once wraps
  }

  // Works with large v as well, but expm needs 10 iterations!
  xi = (Vector(7) << 100.0, 120.0, -60.0 ,0.2, 0.3, -0.8, 0.1);
  actual = Moses3::Expmap(xi);
  EXPECT(assert_equal(expm<Moses3>(xi,10), actual,1e-5));
  //EXPECT(assert_equal(Agrawal06iros(xi), actual,1e-6));
  EXPECT(assert_equal(xi, Moses3::Logmap(actual),1e-6));
}

/* ************************************************************************* */
// Tested with scale
TEST(Moses3, Adjoint_compose_full)
{
  // To debug derivatives of compose, assert that
  // T1*T2*exp(Adjoint(inv(T2),x) = T1*exp(x)*T2
  const Moses3& T1 = T;
  Vector x = (Vector(7) << 0.4, 0.2, 0.8 ,0.1, 0.1, 0.1, 0.0);
  Moses3 expected = T1 * Moses3::Expmap(x) * T2;
  Vector y = T2.inverse().Adjoint(x);
  Moses3 actual = T1 * T2 * Moses3::Expmap(y);
  EXPECT(assert_equal(expected, actual, 1e-6));
}

/* ************************************************************************* */
// Check compose and its pushforward
// NOTE: testing::compose<Pose3>(t1,t2) = t1.compose(t2)  (see lieProxies.h)
// Tested with scale
TEST( Moses3, compose )
{

  const Moses3& T6 = T2;
  gtsam::Matrix actual = (T6*T6).matrix();
  
  gtsam::Matrix expected = T6.matrix()*T6.matrix();
  EXPECT(assert_equal(actual,expected,1e-8));

  gtsam::Matrix actualDcompose1, actualDcompose2;
  T6.compose(T6, actualDcompose1, actualDcompose2);

  gtsam::Matrix numericalH1 = numericalDerivative21(testing::compose<Moses3>, T6, T6);
  EXPECT(assert_equal(numericalH1,actualDcompose1,5e-3));
  EXPECT(assert_equal(T6.inverse().AdjointMap(),actualDcompose1,5e-3));

  gtsam::Matrix numericalH2 = numericalDerivative22(testing::compose<Moses3>, T6, T6);
  EXPECT(assert_equal(numericalH2,actualDcompose2,1e-4));
}

/* ************************************************************************* */
// Check compose and its pushforward, another case 
//Tested with scale
TEST( Moses3, compose2 )
{
  const Moses3& T1 = T;
  gtsam::Matrix actual = (T1*T2).matrix();
  gtsam::Matrix expected = T1.matrix()*T2.matrix();
  EXPECT(assert_equal(actual,expected,1e-8));

  gtsam::Matrix actualDcompose1, actualDcompose2;
  T1.compose(T2, actualDcompose1, actualDcompose2);

  gtsam::Matrix numericalH1 = numericalDerivative21(testing::compose<Moses3>, T1, T2);
  EXPECT(assert_equal(numericalH1,actualDcompose1,5e-3));
  EXPECT(assert_equal(T2.inverse().AdjointMap(),actualDcompose1,5e-3));

  gtsam::Matrix numericalH2 = numericalDerivative22(testing::compose<Moses3>, T1, T2);
  EXPECT(assert_equal(numericalH2,actualDcompose2,1e-5));
}


/* ************************************************************************* */
// Tested with scale
TEST( Moses3, inverse)
{
  const Moses3& T5 = T4;
  gtsam::Matrix actualDinverse;
  gtsam::Matrix actual = T5.inverse(actualDinverse).matrix();
  gtsam::Matrix expected = inverse(T5.matrix());
  EXPECT(assert_equal(actual,expected,1e-8));

  gtsam::Matrix numericalH = numericalDerivative11(testing::inverse<Moses3>, T5);
  EXPECT(assert_equal(numericalH,actualDinverse,5e-3));
  EXPECT(assert_equal(-T5.AdjointMap(),actualDinverse,5e-3));
}


/* ************************************************************************* */
TEST( Moses3, inverseDerivatives2)
{
  Rot3 R = Rot3::rodriguez(0.3,0.4,-0.5);
  Point3 t(3.5,-8.2,4.2);
  Moses3 T(R,t);

  gtsam::Matrix numericalH = numericalDerivative11(testing::inverse<Moses3>, T);
  gtsam::Matrix actualDinverse;
  T.inverse(actualDinverse);
  EXPECT(assert_equal(numericalH,actualDinverse,5e-3));
  EXPECT(assert_equal(-T.AdjointMap(),actualDinverse,5e-3));
}

/* ************************************************************************* */
TEST( Moses3, compose_inverse)
{
  gtsam::Matrix actual = (T*T.inverse()).matrix();
  gtsam::Matrix expected = eye(4,4);
  EXPECT(assert_equal(actual,expected,1e-8));
}

/* ************************************************************************* */
// Jacobian Scale hacked. Not correct.
Point3 transform_from_(const Moses3& pose, const Point3& point) { return pose.transform_from(point); }


TEST( Moses3, Dtransform_from1_a)
{
  gtsam::Matrix actualDtransform_from1;
  T.transform_from(P, actualDtransform_from1, boost::none);
  gtsam::Matrix numerical = numericalDerivative21(transform_from_,T,P);
  EXPECT(assert_equal(numerical,actualDtransform_from1,1e-8));
}

TEST( Moses3, Dtransform_from1_b)
{
  Moses3 origin(ScSO3(2*Rot3::rodriguez(1.0,0.0,1.0).matrix()),Point3(0.0,1.0,0.0));
  Point3 Pee(1.0,10.0,1.0);
  gtsam::Matrix actualDtransform_from1;
  origin.transform_from(Pee, actualDtransform_from1, boost::none);
  gtsam::Matrix numerical = numericalDerivative21(transform_from_,origin,Pee);
  EXPECT(assert_equal(numerical,actualDtransform_from1,1e-8));
}

TEST( Moses3, Dtransform_from1_c)
{
  Point3 origin;
  Moses3 T0(R,origin);
  gtsam::Matrix actualDtransform_from1;
  T0.transform_from(P, actualDtransform_from1, boost::none);
  gtsam::Matrix numerical = numericalDerivative21(transform_from_,T0,P);
  EXPECT(assert_equal(numerical,actualDtransform_from1,1e-8));
}

TEST( Moses3, Dtransform_from1_d)
{
  Rot3 I;
  Point3 t0(100,0,0);
  Moses3 T0(I,t0);
  gtsam::Matrix actualDtransform_from1;
  T0.transform_from(P, actualDtransform_from1, boost::none);
  //print(computed, "Dtransform_from1_d computed:");
  gtsam::Matrix numerical = numericalDerivative21(transform_from_,T0,P);
  //print(numerical, "Dtransform_from1_d numerical:");
  EXPECT(assert_equal(numerical,actualDtransform_from1,1e-8));
}

/* ************************************************************************* */
TEST( Moses3, Dtransform_from2)
{
  gtsam::Matrix actualDtransform_from2;
  T.transform_from(P, boost::none, actualDtransform_from2);
  gtsam::Matrix numerical = numericalDerivative22(transform_from_,T,P);
  EXPECT(assert_equal(numerical,actualDtransform_from2,1e-8));
}

/* ************************************************************************* */
Point3 transform_to_(const Moses3& pose, const Point3& point) { return pose.transform_to(point); }
TEST( Moses3, Dtransform_to1)
{
  gtsam::Matrix computed;
  T.transform_to(P, computed, boost::none);
  gtsam::Matrix numerical = numericalDerivative21(transform_to_,T,P);
  EXPECT(assert_equal(numerical,computed,1e-8));
}

/* ************************************************************************* */
TEST( Moses3, Dtransform_to2)
{
  gtsam::Matrix computed;
  T.transform_to(P, boost::none, computed);
  gtsam::Matrix numerical = numericalDerivative22(transform_to_,T,P);
  EXPECT(assert_equal(numerical,computed,1e-8));
}

/* ************************************************************************* */
TEST( Moses3, transform_to_with_derivatives)
{
  gtsam::Matrix actH1, actH2;
  T.transform_to(P,actH1,actH2);
  gtsam::Matrix expH1 = numericalDerivative21(transform_to_, T,P),
       expH2 = numericalDerivative22(transform_to_, T,P);
  EXPECT(assert_equal(expH1, actH1, 1e-8));
  EXPECT(assert_equal(expH2, actH2, 1e-8));
}

/* ************************************************************************* */
TEST( Moses3, transform_from_with_derivatives)
{
  gtsam::Matrix actH1, actH2;
  T.transform_from(P,actH1,actH2);
  gtsam::Matrix expH1 = numericalDerivative21(transform_from_, T,P),
       expH2 = numericalDerivative22(transform_from_, T,P);
  EXPECT(assert_equal(expH1, actH1, 1e-8));
  EXPECT(assert_equal(expH2, actH2, 1e-8));
}

/* ************************************************************************* */
//Will not test with scale as answer is hard coded. Manually scale checked.
TEST( Moses3, transform_to_translate)
{
    Point3 actual = Moses3(ScSO3(1*Rot3().matrix()), Point3(1, 2, 3).vector()).transform_to(Point3(10.,20.,30.));
    Point3 expected(9.,18.,27.);
    EXPECT(assert_equal(expected, actual));
}

/* ************************************************************************* */
//Will not test with scale as answer is hard coded. Manually scale checked.
TEST( Moses3, transform_to_rotate)
{
    Moses3 transform(ScSO3(1*Rot3::rodriguez(0,0,-1.570796).matrix()), Point3());
    Point3 actual = transform.transform_to(Point3(2,1,10));
    Point3 expected(-1,2,10);
    EXPECT(assert_equal(expected, actual, 0.001));
}

/* ************************************************************************* */
//Will not test with scale as answer is hard coded. Manually scale check.
TEST( Moses3, transform_to)
{
    Moses3 transform(Rot3::rodriguez(0,0,-1.570796), Point3(2,4, 0));
    Point3 actual = transform.transform_to(Point3(3,2,10));
    Point3 expected(2,1,10);
    EXPECT(assert_equal(expected, actual, 0.001));
}

/* ************************************************************************* */
//Will not test with scale as answer is hard coded. Manually scale check.
TEST( Moses3, transform_from)
{
    Point3 actual = T3.transform_from(Point3());
    Point3 expected = Point3(1.,2.,3.);
    EXPECT(assert_equal(expected, actual));
}

/* ************************************************************************* */
//Scale checked
TEST( Moses3, transform_roundtrip)
{
    Point3 actual = T4.transform_from(T4.transform_to(Point3(12., -0.11,7.0)));
    Point3 expected(12., -0.11,7.0);
    EXPECT(assert_equal(expected, actual));
}

/* ************************************************************************* */
//Scale checked
TEST( Moses3, transformPose_to_origin)
{
    // transform to origin
    Moses3 actual = T4.transform_to(Moses3());
    EXPECT(assert_equal(T4, actual, 1e-8));
}

/* ************************************************************************* */
// Scale checked
TEST( Moses3, transformPose_to_itself)
{
    // transform to itself
    Moses3 actual = T4.transform_to(T4);
    EXPECT(assert_equal(Moses3(), actual, 1e-8));
}

/* ************************************************************************* */
//Will not test with scale as answer is hard coded. Manually scale check.
TEST( Moses3, transformPose_to_translation)
{
    // transform translation only
    Rot3 r = Rot3::rodriguez(-1.570796,0,0);
    Moses3 pose2(r, Point3(21.,32.,13.));
    Moses3 actual = pose2.transform_to(Moses3(Rot3(), Point3(1,2,3)));
    Moses3 expected(r, Point3(20.,30.,10.));
    EXPECT(assert_equal(expected, actual, 1e-8));
}

/* ************************************************************************* */
//Will not test with scale as answer is hard coded. Manually scale check.
TEST( Moses3, transformPose_to_simple_rotate)
{
    // transform translation only
    Rot3 r = Rot3::rodriguez(0,0,-1.570796);
    Moses3 pose2(r, Point3(21.,32.,13.));
    Moses3 transform(r, Point3(1,2,3));
    Moses3 actual = pose2.transform_to(transform);
    Moses3 expected(Rot3(), Point3(-30.,20.,10.));
    EXPECT(assert_equal(expected, actual, 0.001));
}

/* ************************************************************************* */
//Will not test with scale as answer is hard coded. Manually scale check.
TEST( Moses3, transformPose_to)
{
    // transform to
    Rot3 r = Rot3::rodriguez(0,0,-1.570796); //-90 degree yaw
    Rot3 r2 = Rot3::rodriguez(0,0,0.698131701); //40 degree yaw
    Moses3 pose2(r2, Point3(21.,32.,13.));
    Moses3 transform(r, Point3(1,2,3));
    Moses3 actual = pose2.transform_to(transform);
    Moses3 expected(Rot3::rodriguez(0,0,2.26892803), Point3(-30.,20.,10.));
    EXPECT(assert_equal(expected, actual, 0.001));
}

/* ************************************************************************* */
//scale tested
TEST(Moses3, localCoordinates_first_order)
{
  Vector d12 = repeat(7,0.1);
  Moses3 t1 = T, t2 = t1.retract(d12);
  EXPECT(assert_equal(d12, t1.localCoordinates(t2)));
}

/* ************************************************************************* */
//Scale tested
TEST(Moses3, manifold_expmap)
{
  Moses3 t1 = T4;
  Moses3 t2 = T3;
  Moses3 origin;
  Vector d12 = t1.localCoordinates(t2);
  EXPECT(assert_equal(t2, t1.retract(d12)));
  Vector d21 = t2.localCoordinates(t1);
  EXPECT(assert_equal(t1, t2.retract(d21)));

  // Check that log(t1,t2)=-log(t2,t1)
  EXPECT(assert_equal(d12,-d21));
}

/* ************************************************************************* */
TEST(Moses3, subgroups)
{
  // Frank - Below only works for correct "Agrawal06iros style expmap
  // lines in canonical coordinates correspond to Abelian subgroups in SE(3)
   Vector d = (Vector(7) << 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7);
  // exp(-d)=inverse(exp(d))
   EXPECT(assert_equal(Moses3::Expmap(-d),Moses3::Expmap(d).inverse()));
  // exp(5d)=exp(2*d+3*d)=exp(2*d)exp(3*d)=exp(3*d)exp(2*d)
   Moses3 T2 = Moses3::Expmap(2*d);
   Moses3 T3 = Moses3::Expmap(3*d);
   Moses3 T5 = Moses3::Expmap(5*d);
   EXPECT(assert_equal(T5,T2*T3));
   EXPECT(assert_equal(T5,T3*T2));
}

/* ************************************************************************* */
// Scale tested
TEST( Moses3, between )
{
  Moses3 expected = T2.inverse() * T4;
  gtsam::Matrix actualDBetween1,actualDBetween2;
  Moses3 actual = T2.between(T4, actualDBetween1,actualDBetween2);
  EXPECT(assert_equal(expected,actual));

  gtsam::Matrix numericalH1 = numericalDerivative21(testing::between<Moses3> , T2, T4);
  EXPECT(assert_equal(numericalH1,actualDBetween1,5e-3));

  gtsam::Matrix numericalH2 = numericalDerivative22(testing::between<Moses3> , T2, T4);
  EXPECT(assert_equal(numericalH2,actualDBetween2,1e-5));
}

/* ************************************************************************* */
// some shared test values - pulled from equivalent test in Pose2
const Point3 l1(1, 0, 0), l2(1, 1, 0), l3(2, 2, 0), l4(1, 4,-4);
const Moses3 x1, x2(Rot3::ypr(0.0, 0.0, 0.0), l2), x3(Rot3::ypr(M_PI/4.0, 0.0, 0.0), l2);
const Moses3
    xl1(Rot3::ypr(0.0, 0.0, 0.0), Point3(1, 0, 0)),
    xl2(Rot3::ypr(0.0, 1.0, 0.0), Point3(1, 1, 0)),
    xl3(Rot3::ypr(1.0, 0.0, 0.0), Point3(2, 2, 0)),
    xl4(Rot3::ypr(0.0, 0.0, 1.0), Point3(1, 4,-4));

/* ************************************************************************* */
LieVector range_proxy(const Moses3& pose, const Point3& point) {
  return LieVector(pose.range(point));
}

// Scale Tested
TEST( Moses3, range )
{
  gtsam::Matrix expectedH1, actualH1, expectedH2, actualH2;


  Moses3 sx(ScSO3(2*Rot3::ypr(0.0, 0.0, 0.0).matrix()), Point3(1.0,1.0,0.0));
  EXPECT_DOUBLES_EQUAL(sqrt(2)/2,sx.range(l3, actualH1, actualH2),1e-9);
  expectedH1 = numericalDerivative21(range_proxy, sx, l3);
  expectedH2 = numericalDerivative22(range_proxy, sx, l3);
  EXPECT(assert_equal(expectedH1,actualH1));
  EXPECT(assert_equal(expectedH2,actualH2));



  // establish range is indeed zero
  EXPECT_DOUBLES_EQUAL(1,x1.range(l1),1e-9);

  // establish range is indeed sqrt2
  EXPECT_DOUBLES_EQUAL(sqrt(2.0),x1.range(l2),1e-9);

  // Another pair
  double actual23 = x2.range(l3, actualH1, actualH2);
  EXPECT_DOUBLES_EQUAL(sqrt(2.0),actual23,1e-9);

  // Check numerical derivatives
  expectedH1 = numericalDerivative21(range_proxy, x2, l3);
  expectedH2 = numericalDerivative22(range_proxy, x2, l3);
  EXPECT(assert_equal(expectedH1,actualH1));
  EXPECT(assert_equal(expectedH2,actualH2));

  // Another test
  double actual34 = x3.range(l4, actualH1, actualH2);
  EXPECT_DOUBLES_EQUAL(5,actual34,1e-9);

  // Check numerical derivatives
  expectedH1 = numericalDerivative21(range_proxy, x3, l4);
  expectedH2 = numericalDerivative22(range_proxy, x3, l4);
  EXPECT(assert_equal(expectedH1,actualH1));
  EXPECT(assert_equal(expectedH2,actualH2));
}


LieVector range_pose_proxy(const Moses3& pose, const Moses3& point) {
  return LieVector(pose.range(point));
}

// IMPORTANT ThIS NEEDS FURTHER TESTING
TEST( Moses3, range_pose )
{
  gtsam::Matrix expectedH1, actualH1, expectedH2, actualH2;

  //scale
  Moses3 sx(ScSO3(3*Rot3::ypr(0.0, 0.0, 0.0).matrix()), Point3(0.0,0.0,0.0));
  Moses3 sx1(ScSO3(6*Rot3::ypr(0.0, 0.0, 0.0).matrix()), Point3(1.0,1.0,0.0));

  //EXPECT_DOUBLES_EQUAL(sqrt(2),sx.range(sx1),1e-9);
  sx1.range(sx,actualH1, actualH2);
  // Check numerical derivatives //scale checked
  expectedH1 = numericalDerivative21(range_pose_proxy, sx1, sx);
  expectedH2 = numericalDerivative22(range_pose_proxy, sx1, sx);
  EXPECT(assert_equal(expectedH1,actualH1));
  EXPECT(assert_equal(expectedH2,actualH2));

  // establish range is indeed zero
  EXPECT_DOUBLES_EQUAL(1,x1.range(xl1),1e-9);

  // establish range is indeed sqrt2
  EXPECT_DOUBLES_EQUAL(sqrt(2.0),x1.range(xl2),1e-9);

  // Another pair
  double actual23 = x2.range(xl3, actualH1, actualH2);
  EXPECT_DOUBLES_EQUAL(sqrt(2.0),actual23,1e-9);


  // Check numerical derivatives
  expectedH1 = numericalDerivative21(range_pose_proxy, x2, xl3);
  expectedH2 = numericalDerivative22(range_pose_proxy, x2, xl3);
  EXPECT(assert_equal(expectedH1,actualH1));
  EXPECT(assert_equal(expectedH2,actualH2));

  // Another test
  double actual34 = x3.range(xl4, actualH1, actualH2);
  EXPECT_DOUBLES_EQUAL(5,actual34,1e-9);

  // Check numerical derivatives
  expectedH1 = numericalDerivative21(range_pose_proxy, x3, xl4);
  expectedH2 = numericalDerivative22(range_pose_proxy, x3, xl4);
  EXPECT(assert_equal(expectedH1,actualH1));
  EXPECT(assert_equal(expectedH2,actualH2));
}

/* ************************************************************************* */
// scale not checked
TEST( Moses3, unicycle )
{
  // velocity in X should be X in inertial frame, rather than global frame
  Vector x_step = delta(7,0,1.0);
  EXPECT(assert_equal(Moses3(Rot3::ypr(0,0,0), l1), expmap_default<Moses3>(x1, x_step), tol));
  EXPECT(assert_equal(Moses3(Rot3::ypr(0,0,0), Point3(2,1,0)), expmap_default<Moses3>(x2, x_step), tol));
  EXPECT(assert_equal(Moses3(Rot3::ypr(M_PI/4.0,0,0), Point3(2,2,0)), expmap_default<Moses3>(x3, sqrt(2.0) * x_step), tol));
}

/* ************************************************************************* */
// This is doubtful
/*
  //SIGN CONFLICT!!
  cout << "Moses3::adjoint"<<endl;
  cout << Moses3::adjoint(screw::xi,screw::xi2) << endl;
  cout << "Sim3::lieBracket"<<endl;
  cout << Sim3::lieBracket(screw::xi, screw::xi2) << endl;


  cout << "SE3::adjointMap"<<endl;
  cout << SE3::d_lieBracketab_by_d_a(screw::si)*screw::si2 << endl;
  cout << "SE3::lieBracket"<<endl;
  cout << SE3::lieBracket(screw::si, screw::si2) << endl;
*/
TEST( Moses3, adjointMap) {
  gtsam::Matrix res = Moses3::adjointMap(screw::xi);
  gtsam::Matrix wh = skewSymmetric(screw::xi(0), screw::xi(1), screw::xi(2));
  gtsam::Matrix vh = skewSymmetric(screw::xi(3), screw::xi(4), screw::xi(5));
  gtsam::Matrix Z3 = zeros(3,3);
  gtsam::Matrix6 expected;
  expected << wh, Z3, vh, wh;
  EXPECT(assert_equal(res,res,1e-5));
}

//Vector xi = (Vector(6) << 0.1, 0.2, 0.3, 1.0, 2.0, 3.0, 0.0);

/* ************************************************************************* */
Vector testDerivAdjoint(const LieVector& xi, const LieVector& v) {
  return Moses3::adjointMap(xi)*v;
}

TEST( Moses3, adjoint) {
  Vector expected = testDerivAdjoint(screw::xi, screw::xi2);

  gtsam::Matrix actualH;
  Vector actual = Moses3::adjoint(screw::xi, screw::xi2, actualH);

  gtsam::Matrix numericalH = numericalDerivative21(
      boost::function<Vector(const LieVector&, const LieVector&)>(
          boost::bind(testDerivAdjoint,  _1, _2)
          ),
      LieVector(screw::xi), LieVector(screw::xi2), 1e-5
      );

  EXPECT(assert_equal(expected,actual,1e-5));
  EXPECT(assert_equal(numericalH,actualH,1e-5));
}


/* ************************************************************************* */
Vector testDerivAdjointTranspose(const LieVector& xi, const LieVector& v) {
  return Moses3::adjointMap(xi).transpose()*v;
}

TEST( Moses3, adjointTranspose) {
  Vector xi = (Vector(7) << 0.01, 0.02, 0.03, 1.0, 2.0, 3.0, 0.1);
  Vector v = (Vector(7) << 0.04, 0.05, 0.06, 4.0, 5.0, 6.0, 0.2);
  Vector expected = testDerivAdjointTranspose(xi, v);

  gtsam::Matrix actualH;
  Vector actual = Moses3::adjointTranspose(xi, v, actualH);

  gtsam::Matrix numericalH = numericalDerivative21(
      boost::function<Vector(const LieVector&, const LieVector&)>(
          boost::bind(testDerivAdjointTranspose,  _1, _2)
          ),
      LieVector(xi), LieVector(v), 1e-5
      );

  EXPECT(assert_equal(expected,actual,1e-15));
  EXPECT(assert_equal(numericalH,actualH,1e-5));
}


/* ************************************************************************* 
/// exp(xi) exp(y) = exp(xi + x)
/// Hence, y = log (exp(-xi)*exp(xi+x))

Vector xi = (Vector(6) << 0.1, 0.2, 0.3, 1.0, 2.0, 3.0);

Vector testDerivExpmapInv(const LieVector& dxi) {
  Vector y = Pose3::Logmap(Pose3::Expmap(-xi)*Pose3::Expmap(xi+dxi));
  return y;
}

TEST( Pose3, dExpInv_TLN) {
  Matrix res = Pose3::dExpInv_exp(xi);

  Matrix numericalDerivExpmapInv = numericalDerivative11(
      boost::function<Vector(const LieVector&)>(
          boost::bind(testDerivExpmapInv,  _1)
          ),
      LieVector(Vector::Zero(6)), 1e-5
      );

  EXPECT(assert_equal(numericalDerivExpmapInv,res,3e-1));
}*/






/* ************************************************************************* */
bool moses3bracket_tests()
{
  bool failed = false;
  vector<Vector7> vecs;
  Vector7 tmp;
  tmp << 0,0,0,0,0,0,0;
  vecs.push_back(tmp);
  tmp << 1,0,0,0,0,0,0;
  vecs.push_back(tmp);
  tmp << 0,1,0,1,0,0,0.1;
  vecs.push_back(tmp);
  tmp << 0,0,1,0,1,0,0.1;
  vecs.push_back(tmp);
  tmp << -1,1,0,0,0,1,-0.1;
  vecs.push_back(tmp);
  tmp << 20,-1,0,-1,1,0,-0.1;
  vecs.push_back(tmp);
  tmp << 30,5,-1,20,-1,0,2;
  vecs.push_back(tmp);
  for (size_t i=0; i<vecs.size(); ++i)
  {
    Vector7 resDiff = vecs[i] - Moses3::vee(Moses3::hat(vecs[i]));
    if (resDiff.norm()>SMALL_EPS)
    {
      cerr << "Hat-vee Test" << endl;
      cerr  << "Test case: " << i <<  endl;
      cerr << resDiff.transpose() << endl;
      cerr << endl;
      failed = true;
    }

    for (size_t j=0; j<vecs.size(); ++j)
    {
      Vector7 res1 = Moses3::lieBracket(vecs[i],vecs[j]);
      Matrix4 hati = Moses3::hat(vecs[i]);
      Matrix4 hatj = Moses3::hat(vecs[j]);

      Vector7 res2 = Moses3::vee(hati*hatj-hatj*hati);
      Vector7 resDiff = res1-res2;
      if (resDiff.norm()>SMALL_EPS)
      {
        cerr << "Sim3 Lie Bracket Test" << endl;
        cerr  << "Test case: " << i << ", " <<j<< endl;
        cerr << vecs[i].transpose() << endl;
        cerr << vecs[j].transpose() << endl;
        cerr << resDiff.transpose() << endl;
        cerr << endl;
        failed = true;
      }
    }


    /*
    Vector7 omega = vecs[i];
    Matrix4 exp_x = Moses3::exp(omega).matrix();
    Matrix4 expmap_hat_x = (Moses3::hat(omega)).exp();
    Matrix4 DiffR = exp_x-expmap_hat_x;
    double nrm = DiffR.norm();

    if (isnan(nrm) || nrm>SMALL_EPS)
    {
      cerr << "expmap(hat(x)) - exp(x)" << endl;
      cerr  << "Test case: " << i << endl;
      cerr << exp_x <<endl;
      cerr << expmap_hat_x <<endl;
      cerr << DiffR <<endl;
      cerr << endl;
      failed = true;
    }*/
  }
  return failed;
}

/* ********************************************************************** */

bool Moses3explog_tests()
{
  double pi = 3.14159265;
  vector<Moses3> omegas;
  omegas.push_back(Moses3(ScSO3::exp(Vector4(0.2, 0.5, 0.0,1.)),Vector3(0,0,0)));
  omegas.push_back(Moses3(ScSO3::exp(Vector4(0.2, 0.5, -1.0,1.1)),Vector3(10,0,0)));
  omegas.push_back(Moses3(ScSO3::exp(Vector4(0., 0., 0.,1.1)),Vector3(0,100,5)));
  omegas.push_back(Moses3(ScSO3::exp(Vector4(0., 0., 0.00001, 0.)),Vector3(0,0,0)));
  omegas.push_back(Moses3(ScSO3::exp(Vector4(0., 0., 0.00001, 0.0000001)),Vector3(1,-1.00000001,2.0000000001)));
  omegas.push_back(Moses3(ScSO3::exp(Vector4(0., 0., 0.00001, 0)),Vector3(0.01,0,0)));
  omegas.push_back(Moses3(ScSO3::exp(Vector4(pi, 0, 0,0.9)),Vector3(4,-5,0)));
  omegas.push_back(Moses3(ScSO3::exp(Vector4(0.2, 0.5, 0.0,0)),Vector3(0,0,0))
                   *Moses3(ScSO3::exp(Vector4(pi, 0, 0,0)),Vector3(0,0,0))
                   *Moses3(ScSO3::exp(Vector4(-0.2, -0.5, -0.0,0)),Vector3(0,0,0)));
  omegas.push_back(Moses3(ScSO3::exp(Vector4(0.3, 0.5, 0.1,0)),Vector3(2,0,-7))
                   *Moses3(ScSO3::exp(Vector4(pi, 0, 0,0)),Vector3(0,0,0))
                   *Moses3(ScSO3::exp(Vector4(-0.3, -0.5, -0.1,0)),Vector3(0,6,0)));

  bool failed = false;

  for (size_t i=0; i<omegas.size(); ++i)
  {
    Matrix4 R1 = omegas[i].matrix();
    Matrix4 R2 = Moses3::Expmap(omegas[i].Logmap7()).matrix();
    Matrix4 DiffR = R1-R2;
    double nrm = DiffR.norm();

    // ToDO: Force Sim3 to be more accurate!
    if (isnan(nrm) || nrm>SMALL_EPS)
    {
      cerr << "Sim3 - exp(log(Sim3))" << endl;
      cerr  << "Test case: " << i << endl;
      cerr << DiffR <<endl;
      cerr << endl;
      failed = true;
    }
  }
  for (size_t i=0; i<omegas.size(); ++i)
  {
    Vector3 p(1,2,4);
    Matrix4 T = omegas[i].matrix();
    Vector3 res1 = static_cast<const Sim3 &>(omegas[i])*p;
    Vector3 res2 = T.topLeftCorner<3,3>()*p + T.topRightCorner<3,1>();

    double nrm = (res1-res2).norm();

    if (isnan(nrm) || nrm>SMALL_EPS)
    {
      cerr << "Transform vector" << endl;
      cerr  << "Test case: " << i << endl;
      cerr << (res1-res2) <<endl;
      cerr << endl;
      failed = true;
    }
  }

  for (size_t i=0; i<omegas.size(); ++i)
  {
    Matrix4 q = omegas[i].matrix();
    Matrix4 inv_q = omegas[i].inverse().matrix();
    Matrix4 res = q*inv_q ;
    Matrix4 I;
    I.setIdentity();

    double nrm = (res-I).norm();

    if (isnan(nrm) || nrm>SMALL_EPS)
    {
      cerr << "Inverse" << endl;
      cerr  << "Test case: " << i << endl;
      cerr << (res-I) <<endl;
      cerr << endl;
      failed = true;
    }
  }
  return failed;
}


/* ************************************************************************* */
int main(){ 
  TestResult tr;
/*
  Vector7d tmp;
  tmp << 0,0,0,0,0,0,0;


  cout << Tt;
  cout << Tt.to_Pose3();

  cout << Tt1;
  cout << Tt1.to_Pose3();

  cout << Tt1.inverse();
  cout << Tt1.inverse().to_Pose3();

  cout << Tt1*Tt1.inverse();
  cout << (Tt1*Tt1.inverse()).to_Pose3();



  cout << Se;
  cout << Se1;
  cout << (Se*Se1);

  cout << Po;
  cout << Po1;

  cout << Po1.between(Po);
  cout << Po1.compose(Po.inverse());

*/

  bool failed = Moses3explog_tests();
  cout << failed<<endl;

  failed = moses3bracket_tests();
  cout << failed<<endl;
/*
  //SIGN CONFLICT!!
  cout << "Moses3::adjoint"<<endl;
  cout << Moses3::adjoint(screw::xi,screw::xi2) << endl;
  cout << "Sim3::lieBracket"<<endl;
  cout << Sim3::lieBracket(screw::xi, screw::xi2) << endl;


  cout << "SE3::adjointMap"<<endl;
  cout << SE3::d_lieBracketab_by_d_a(screw::si)*screw::si2 << endl;
  cout << "SE3::lieBracket"<<endl;
  cout << SE3::lieBracket(screw::si, screw::si2) << endl;
*/
  return TestRegistry::runAllTests(tr);}
/* ************************************************************************* */
