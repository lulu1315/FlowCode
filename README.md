# FloCode

utilities to convert from .flo format (optical flow) to .exr format

* flo2exr : convert .flo to .exr

    usage: ./flo2exr [-quiet] in.flo out.exr

* exr2flo : convert .exr to .flo

    usage: ./exr2flo [-quiet] in.exr out.flo
    
* flo2png : visualize .flo file into a png

    usage: ./flo2png [-quiet] in.flo out.png [maxmotion] 

Compile :

    git clone --recursive https://github.com/lulu1315/FloCode.git
    cd FloCode
    mkdir build;cd build
    cmake ..
    make

* tinyexr : https://github.com/syoyo/tinyexr
* imageLib https://github.com/dscharstein/imageLib
* original Code : http://vision.middlebury.edu/flow/data/
* twixtor format : http://revisionfx.com/faq/motion_vector/

