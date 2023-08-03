QT_VERSION=6.5.2
export QT_QMAKE_EXECUTABLE=/home/programs/Qt/${QT_VERSION}/gcc_64/bin/qmake
export QT_DIR=/home/programs/Qt/${QT_VERSION}/gcc_64/lib/cmake/Qt6
export CMAKE_PREFIX_PATH=/home/programs/Qt/${QT_VERSION}/gcc_64
export PATH=/home/programs/Qt/${QT_VERSION}/gcc_64/bin/:$PATH

CURR_DIR=$PWD 


rm -rf deploy app_bin
mkdir -p deploy && cd deploy 
qt-cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../app_bin ../src
make -j6 && make install 

# deb packing 
cpack -G DEB
