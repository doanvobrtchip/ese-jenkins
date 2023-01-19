# Introduction
Provide the instructions on how to build a docker image and run it in a container to build ESE and create the installation package

# Docker

## Build Docker Image
To pull the existing image from the Docker hub, run:
```
docker pull thanglie1/qt640_msvc2019
```

Or build an image, in the *project source directory*, run:

```
docker build -t thanglie1/qt640_msvc2019 -f deployment/docker/Dockerfile deployment/docker
```
or add network configuration option `--network "Default Switch"` 
```
docker build -t thanglie1/qt640_msvc2019 -f deployment/docker/Dockerfile deployment/docker --network "Default Switch"  
```  
if there is similar error observed: 
![image](https://user-images.githubusercontent.com/13127756/210039572-d51f58bb-8187-413b-af83-7ad4d556d76c.png)


## Run the container:
To run the created docker image, in the project source directory, run:

```
docker run --rm --mount type=bind,source="%cd%",target="%cd%/prj" -w "%cd%/prj" thanglie1/qt640_msvc2019 build.bat
```

in the windows command prompt , other than power shell with administrator privilege : 
![image](https://user-images.githubusercontent.com/13127756/210320783-029c99eb-c4d5-4cfe-83b3-da4a5379a2b3.png)


### Note  

Please ensure the container your Docker Desktop is windows container , not linux container:  

![image](https://user-images.githubusercontent.com/13127756/210039718-79e71338-e981-4b69-ae28-f8e8047efc95.png)

# Version of the software
PYTHON_VERSION=3.10.0  
7ZIP_VERSION=19.0   
GIT_VERSION=2.24.0  
QBS_VERSION=1.23.1  
CMAKE_VERSION=3.25.1  
QT_VERSION=6.4.0  
INNOSETUP_VERSION=6.2.1  

# Toolchain-type:
MSVC2019_64(win64_msvc2019_64)  
# Components of toolchain-type: 
qtbase qttools qtdeclarative qtscript qtsvg qt5compat opengl32sw

# How to get the output files from the container?  
The output files are in folder: $(Project Source Code directory)/deployment/ese/Ouput
