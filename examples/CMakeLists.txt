set (excluded_examples
    DiscreteBayesNet_FG.cpp
    UGM_chain.cpp
    UGM_small.cpp
    elaboratePoint2KalmanFilter.cpp
)


SET(EIGEN3_INCLUDE_DIR "/home/clcunnin/git/gtsam_sim3/gtsam/3rdparty/Eigen/")

SET (INCLUDE_DIRS ${EIGEN3_INCLUDE_DIR})
INCLUDE_DIRECTORIES( ${EIGEN3_INCLUDE_DIR} )


gtsamAddExamplesGlob("*.cpp" "${excluded_examples}" "gtsam;${Boost_PROGRAM_OPTIONS_LIBRARY};${INCLUDE_DIRS}")
