import os
import sys
import re

class FMakefileInfos:
    def __init__(self, rootDir, targetPathFile):
        self.mRootDir = rootDir
        self.mTargetPathFile = targetPathFile
        self.mModuleName2Path = {}

    def __GetLibName(self, fullPathName):
        for line in open(fullPathName, 'r', encoding='UTF-8'):
            libName = self.__TryExtractLibName(line)
            if libName!=None:
                return libName
        return None

    def __TryExtractLibName(self, content):
        matchObj = re.match(r'^[ ]*(set|SET)[ ]*\(LibName[ ]+("[a-zA-Z0-9_]+"|[a-zA-Z0-9_]+)[ ]*\)[ ]*([ ]*$|#.*$)', content)
        if matchObj:
            return matchObj.group(2)
        else:
            return None

    def __ExportToFile(self):
        strLibNames = "list(APPEND LibNames\n" 
        strLibPaths = "list(APPEND LibPaths\n" 
        for libName, path in self.mModuleName2Path.items():
            strLibNames = '{}    {}\n'.format(strLibNames, libName)
            strLibPaths = '{}    \"{}\"\n'.format(strLibPaths, path.replace('\\', "/"))
        strLibNames = strLibNames+"    )\n\n"
        strLibPaths = strLibPaths+"    )\n"

        f = open(self.mTargetPathFile, 'w')
        f.write(strLibNames+strLibPaths)
        f.close()

    def __InitLibName2Path(self):
        for root, dirs, fileNames in os.walk(self.mRootDir):  
            for fileName in fileNames:
                if fileName=="CMakeLists.txt":
                    fullPathName = os.path.join(root, fileName)
                    libName = self.__GetLibName(fullPathName)
                    if libName!=None:
                        self.mModuleName2Path[libName] = root

    def Run(self):
        self.__InitLibName2Path()
        self.__ExportToFile()



def Main(rootDir, targetPathFile):
    makefileInfos = FMakefileInfos(rootDir, targetPathFile)
    makefileInfos.Run()

if __name__ == '__main__' :
    Main(sys.argv[1], sys.argv[2])

