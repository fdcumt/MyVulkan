
########################################################################################
#################### !!! this is slution makefile !!! ##################################
########################################################################################

#指定cmake的最小版本
cmake_minimum_required(VERSION 3.18)


#设置项目名称
set(SlnName "VulkanExample")


#在顶层引入通用函数模块
INCLUDE("${CMAKE_BINARY_DIR}/AutoGenHeader.cmake")  
INCLUDE("${CMAKE_SOURCE_DIR}/../CMake/Utility.cmake")  

set(SlnDir "${CMAKE_SOURCE_DIR}/..")
set(SlnAutoGenLibsDir "${CMAKE_BINARY_DIR}/Libs")


#开启项目文件夹
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Z_CMakePredefinedTargets ")  # 设置为空

project("${SlnName}")
	
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")

AddProjectToSln("${CMAKE_SOURCE_DIR}")

