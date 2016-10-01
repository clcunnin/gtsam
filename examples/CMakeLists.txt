set (excluded_examples
    DiscreteBayesNet_FG.cpp
    UGM_chain.cpp
    UGM_small.cpp
    elaboratePoint2KalmanFilter.cpp
)



SET(EIGEN3_INCLUDE_DIR "/home/clcunnin/git/gtsam_sim3/gtsam/3rdparty/Eigen/")
SET( Sophus_INCLUDE_DIR "/home/clcunnin/catkin_ws/src/lsd_slam_core/thirdparty/Sophus/") 
SET (INCLUDE_DIRS ${EIGEN3_INCLUDE_DIR} ${Sophus_INCLUDE_DIR})
INCLUDE_DIRECTORIES( ${INCLUDE_DIRS} )




gtsamAddExamplesGlob("*.cpp" "${excluded_examples}" "gtsam;${Boost_PROGRAM_OPTIONS_LIBRARY};${INCLUDE_DIRS}")
