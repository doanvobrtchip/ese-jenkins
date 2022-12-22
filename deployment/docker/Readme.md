# Introduction
Provide an overview about how to build an image and run a container
Can customize the Dockerfile to suit your needs

========

# Docker
To build an image, in the project source directory, run:

```
docker build -t ese-image -f deployment/docker/Dockerfile deployment/docker
```

To run an container, in the project source directory, run:

```
docker run --rm --mount type=bind,source="%cd%",target="%cd%/prj" -w "%cd%/prj" ese-image build.bat
```

# Version of the software
PYTHON_VERSION=3.10.0
7ZIP_VERSION=19.0
GIT_VERSION=2.24.0
QBS_VERSION=1.23.1
CMAKE_VERSION=3.25.1
QT_VERSION=6.4.0
INNOSETUP_VERSION=6.2.1

#Toolchain-type:
MSVC2019_64(win64_msvc2019_64)
#Components of toolchain-type: 
qtbase qttools qtdeclarative qtscript qtsvg qt5compat opengl32sw
