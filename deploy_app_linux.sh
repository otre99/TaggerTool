QT_VERSION=6.9.2
export QT_QMAKE_EXECUTABLE=/home/programs/QtSDK/${QT_VERSION}/gcc_64/bin/qmake
export QT_DIR=/home/programs/QtSDK/${QT_VERSION}/gcc_64/lib/cmake/Qt6
export CMAKE_PREFIX_PATH=/home/programs/QtSDK/${QT_VERSION}/gcc_64
export PATH=/home/programs/QtSDK/${QT_VERSION}/gcc_64/bin/:$PATH

CURR_DIR=`pwd`
LINUX_DEPLOY=${CURR_DIR}/AppBinaries/linux64

mkdir -p ${LINUX_DEPLOY} 
mkdir -p ${CURR_DIR}/AppBinaries/build_linux && cd ${CURR_DIR}/AppBinaries/build_linux 

qt-cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${LINUX_DEPLOY} ${CURR_DIR}/src
make -j6 && make install 

cd ${CURR_DIR} 

## CREATE THE RUN script
printf "#!/bin/sh\n" > ${LINUX_DEPLOY}/CVTaggerTool.sh 
printf 'execname=$PWD/bin/TaggerTool\n' >> ${LINUX_DEPLOY}/CVTaggerTool.sh 
printf 'LD_LIBRARY_PATH=${PWD}:${PWD}/lib\n' >> ${LINUX_DEPLOY}/CVTaggerTool.sh 
printf 'export LD_LIBRARY_PATH\n' >> ${LINUX_DEPLOY}/CVTaggerTool.sh 
printf '${execname}\n' >> ${LINUX_DEPLOY}/CVTaggerTool.sh 

chmod a+x ${LINUX_DEPLOY}/CVTaggerTool.sh
