SDK_FIREFLY_PATH?=/opt/rk3588_sdk
export CONFIG_NAME?=latest
BUILD_TYPE?=Release
BUILD_DIR?=Release
export PATH := ${SDK_FIREFLY_PATH}/buildroot/output/${CONFIG_NAME}/host/bin/:${PATH}

TOOLCHAIN=${SDK_FIREFLY_PATH}/buildroot/output/${CONFIG_NAME}/host/share/buildroot/toolchainfile.cmake

INSTALL_PREFIX?=/opt/pkrv/

.PHONY: all

all:
	cmake -S ./ -B${BUILD_DIR} \
	-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN} \
	-DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	cmake --build ./${BUILD_DIR} --parallel 4
	echo "ПО успешно собрано"

clean:
	rm -rf ./${BUILD_DIR} 

install:
	cmake --install ${BUILD_DIR} --prefix ${INSTALL_PREFIX} --component lightning_gpu
	cmake --install ${BUILD_DIR} --prefix ${INSTALL_PREFIX} --component gpuConfig.xml

