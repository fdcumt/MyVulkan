
#根据相对路径获取绝对路径
function(GetAbslutePaths AbslutePaths RelativePaths BaseDir)
    if(MSVC)
		set(NewAbslutePaths)
		foreach(RelativePath IN LISTS RelativePaths)
			string(COMPARE EQUAL ${RelativePath} "" bEqual)
			if(${bEqual} EQUAL 0)
				get_filename_component(AbsPath "${RelativePath}" REALPATH BASE_DIR "${BaseDir}")
				list(APPEND NewAbslutePaths ${AbsPath})
			endif()
        endforeach()
		set(${AbslutePaths} ${NewAbslutePaths} PARENT_SCOPE)
    endif(MSVC)
endfunction(GetAbslutePaths)

#按目录结构加入文件
function(AddFiltersForVS FullPathFileNames CurDir)
    if(MSVC)
        foreach(FullPathFileName IN LISTS FullPathFileNames)
            # 求出相对路径
            string(REPLACE "${CurDir}/" "" RelativePathFileName "${FullPathFileName}")
			get_filename_component(RelativePath "${RelativePathFileName}" DIRECTORY)
            source_group("${RelativePath}" FILES "${FullPathFileName}")
        endforeach(FullPathFileName)
    endif(MSVC)
endfunction(AddFiltersForVS)

function(GenIncludeABSDir IncludeAbsDirs IncludeDirs CurDir)
	set(IncludeAbsDirsTemp)
    foreach(IncludeDir IN LISTS IncludeDirs)
		string(COMPARE EQUAL "${IncludeDir}" "" bEqual)
		if(bEqual)
			continue()
		endif()
		STRING(REGEX REPLACE "^[.]($|/)" "${CurDir}/" IncludeDir1 "${IncludeDir}")
		STRING(REGEX REPLACE "^[.][.][/]?" "${CurDir}/.." IncludeDir2 "${IncludeDir1}")
		string(SUBSTRING "${IncludeDir2}" 1 1 char)
		string(COMPARE EQUAL "${char}" ":" bEqual)
		if(bEqual)
			list(APPEND IncludeAbsDirsTemp "${IncludeDir2}")
			continue()
		else()
			STRING(REGEX MATCH "^[a-zA-Z0-9_]+" CurLibName "${IncludeDir2}")
			foreach(libName Path IN ZIP_LISTS LibNames LibPaths)
				string(COMPARE EQUAL "${libName}" "${CurLibName}" bEqual)
				if(bEqual)
					STRING(REGEX REPLACE "^[a-zA-Z0-9_]+/?" "" tailPath "${IncludeDir2}")
					list(APPEND IncludeAbsDirsTemp "${Path}/${tailPath}")
					break()
				endif()
			endforeach()
		endif()
    endforeach()
	set(${IncludeAbsDirs} ${IncludeAbsDirsTemp} PARENT_SCOPE)
	#message("IncludeAbsDirs:${IncludeAbsDirsTemp}")
endfunction(GenIncludeABSDir)

#生成lib库
function(GenLib LibName CurDir IncludeDirs LibDirs OutputDir)
	#message("Begin Generate lib:${LibName}")
	GenIncludeABSDir(IncludeAbsDirs "${IncludeDirs}" "${CurDir}" )
	GetAbslutePaths(LibAbsDirs "${LibDirs}" "${CurDir}")
	GetAbslutePaths(OutputAbsDir "${OutputDir}" "${CurDir}")

	#所有文件保存在一个变量中		
	file(GLOB_RECURSE all_files *.h *.cpp *.c *.cc)
	AddFiltersForVS("${all_files}" "${CurDir}")
	
	#********这种方法设置后, Output目录后边不会添加Release/debug***********
	#把这种方法命名为 OutputNoDebugOrRelase
	#如果启用了该方法, 则方法OutputWithDebugOrRelase永远不会生效
	# Properties->General->Output Directory
	if(MSVC)
		#开启这几行, 可以去除output尾部的debug, 但是指定目录就会失效
		#https://stackoverflow.com/questions/543203/cmake-runtime-output-directory-on-windows
		#官方链接https://cmake.org/cmake/help/latest/prop_tgt/LIBRARY_OUTPUT_DIRECTORY_CONFIG.html
		#官方链接https://cmake.org/cmake/help/latest/variable/CMAKE_LIBRARY_OUTPUT_DIRECTORY_CONFIG.html#variable:CMAKE_LIBRARY_OUTPUT_DIRECTORY_%3CCONFIG%3E
		SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OutputAbsDir}")
		SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OutputAbsDir}")
		SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${OutputAbsDir}")
		SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${OutputAbsDir}")
		SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${OutputAbsDir}")
		SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${OutputAbsDir}")
	endif(MSVC)
	
	add_library("${LibName}" "${all_files}")
	
	# Properties->C/C++->General->Additional Include Directories
	include_directories("${IncludeAbsDirs}")
	
	
	#添加静态链接库, 即xxx.lib
	target_link_libraries("${LibName}" "${LibItems}")
	
	#静态连接库,  连接器->常规->附加库目录
	target_link_directories("${LibName}" PRIVATE "${LibAbsDirs}")
	target_link_directories("${LibName}" PUBLIC "${LibAbsDirs}")
	
	# Creates a folder "libraries" and adds target this project under it
	set_property(TARGET "${LibName}" PROPERTY FOLDER "Libraries")
	
	#message("Succeed Generate lib:${LibName}")
endfunction(GenLib)

#生成exe项目
function(GenEXE ExeName CurDir IncludeDirs LibDirs OutputDir)
	#message("Begin Generate EXE:${ExeName}")
	GenIncludeABSDir(IncludeAbsDirs "${IncludeDirs}" "${CurDir}")
	GetAbslutePaths(LibAbsDirs "${LibDirs}" "${CurDir}")
	GetAbslutePaths(OutputAbsDir "${OutputDir}" "${CurDir}")

	#所有文件保存在一个变量中		
	file(GLOB_RECURSE all_files *.h *.cpp *.c *.cc)
	AddFiltersForVS("${all_files}" "${CurDir}")

	add_executable("${ExeName}" "${all_files}")
	
	# Properties->C/C++->General->Additional Include Directories
	include_directories("${IncludeAbsDirs}")

	#添加静态链接库, 即xxx.lib
	target_link_libraries("${ExeName}" "${LibItems}")
	
	#静态连接库,  连接器->常规->附加库目录
	target_link_directories("${ExeName}" PRIVATE "${LibAbsDirs}")
	target_link_directories("${ExeName}" PUBLIC "${LibAbsDirs}")
	
	#Creates a folder "libraries" and adds target project (math.vcproj) under it
	set_property(TARGET "${ExeName}" PROPERTY FOLDER "Executables")

	#Properties->General->Output Directory
	set_target_properties("${ExeName}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OutputAbsDir}")

	#message("Succeed Generate EXE:${ExeName}")
endfunction(GenEXE)

#function(AddSlnSubDir CurDir)
#	#遍历sln文件夹,获取文件夹下所有含有CMakeLists.txt的项目
#	file(GLOB ChildDirs RELATIVE "${CurDir}" "${CurDir}/*")
#	foreach(child IN LISTS ChildDirs)
#		if(IS_DIRECTORY "${CurDir}/${child}")
#			if(EXISTS "${CurDir}/${child}/CMakeLists.txt")
#				add_subdirectory("${child}")
#			endif()
#		endif()
#	endforeach()
#endfunction(AddSlnSubDir)

function(AddSlnSubDir CurDir)
	#遍历sln文件夹,获取文件夹下所有含有CMakeLists.txt的项目
	#file(GLOB_RECURSE CMakeFiles RELATIVE "${CurDir}" CMakeLists.txt)
	file(GLOB_RECURSE CMakeFiles CMakeLists.txt)
	foreach(CMakeFile IN LISTS CMakeFiles)
		get_filename_component(FileDir "${CMakeFile}" DIRECTORY)
		string(COMPARE EQUAL "${FileDir}" "${CurDir}" bEqual)
		if(NOT bEqual)
			add_subdirectory("${FileDir}")
		endif(NOT bEqual)
	endforeach()
endfunction(AddSlnSubDir)
