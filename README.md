OpenCV 2.4 implementation of "egbis" segmentation algorithm
=====================================================
OpenCV (2.4) Implementation of Efficient Graph-Based Image Segmentation algorithm developed
by Pedro F. Felzenszwalb and Daniel P. Huttenlocher.

Heavily borrowed from:

- https://github.com/christofferholmstedt/opencv-wrapper-egbis
- https://github.com/saliksyed/OpenCVGraphSegmentation

How To: 

    cd <project_root_directory>/
    mkdir build
    cd build/
    cmake ../
    make

    **To run**
    ./main [path to image]
