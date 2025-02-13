
########################################################################################
#################### !!! this is lib makefile !!! ##################################
########################################################################################

#设置项目名称
set(LibName "Utility")

#静态连接库, 连接器->输入->附加依赖项
set(LibItems "")


################这里面所有路径都要填相对路径, 相对于当前目录所在的路径 begin ##########################


#设置Include目录, Properties->C/C++->General->Additional Include Directories
set(IncludeDirs
		"."
	)

#静态连接库,  连接器->常规->附加库目录
set(LibDirs
	"${SlnDir}/Libs"
	"${SlnAutoGenLibsDir}"
	)

#设置Output目录, Properties->General->Output Directory
#静态文件一般都输出到和sln同级的Libs目录中.
set(OutputDir "${SlnAutoGenLibsDir}")


GenLib( 
		"${LibName}" 
		"${CMAKE_CURRENT_SOURCE_DIR}" 
		"${IncludeDirs}" 
		"${LibDirs}" 
		"${OutputDir}" 
		"${LibItems}" 
	  )
