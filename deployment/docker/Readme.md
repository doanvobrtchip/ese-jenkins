# Introduction
Provide an overview about how to build an image and run a container
Can customize the Dockerfile to suit your needs

========

# Docker

To build an image, in the project source directory, run:

```
docker build -t ese-image -f deployment/docker/Dockerfile deployment/docker
```
or add network configuration option `--network "Default Switch"` 
```
docker build -t ese-image -f deployment/docker/Dockerfile deployment/docker --network "Default Switch"  
```  
if there is similar error observed: 
![image](https://user-images.githubusercontent.com/13127756/210039572-d51f58bb-8187-413b-af83-7ad4d556d76c.png)


To run an container, in the project source directory, run:

```
docker run --rm --mount type=bind,source="%cd%",target="%cd%/prj" -w "%cd%/prj" ese-image build.bat
```

### Note
#### Please ensure the container your Docker Desktop is windows container , not linux container:  

![image](https://user-images.githubusercontent.com/13127756/210039718-79e71338-e981-4b69-ae28-f8e8047efc95.png)

#### You may need change the source path and working directory to run the container correctly: 

![image](https://user-images.githubusercontent.com/13127756/210046859-a5c4d20f-a91d-40fd-a985-a92de95e3cf3.png)


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
