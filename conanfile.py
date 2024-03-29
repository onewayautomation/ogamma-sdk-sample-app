from conans import ConanFile, CMake, tools, errors
from conans.model.version import Version
import sys
import os
from shutil import copyfile

class sampleApp(ConanFile):
    requires = [
        ("botan/2.19.2"),
        ("boost/1.79.0"),
        ("spdlog/1.10.0"),
        ("pugixml/1.12.1"),
    ]
    
    name = "sampleApp"
    version = "0.2"
    license = "Proprietary"
    url = "https://onewayautomation.com/opcua-sdk-docs/html/index.html"
    description = "ogamma OPC UA C++ SDK Sample Application"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False}
    generators = "cmake"
    verbose = True
    cmake = None

    def export_sources(self):
        self.output.info("Executing export_sources() method")
  
        self.copy("conanfile.py")
        self.copy("CMakeLists.txt")      
        
        self.copy("main.cpp", src="sampleApp", dst="sampleApp")
        self.copy("CMakeLists.txt", src="sampleApp", dst="sampleApp")
        
        # Download library file from the Internet:
        zipFileName = "OpcUaSdk.zip"
        libUrl = "https://onewayautomation.com/opcua-binaries/sdk/vs2022-OpcUaSdk-1.2.4-demo.zip"
        libFolderName = "ogamma-sdk/lib/"
        
        if tools.OSInfo().is_linux:
            if tools.OSInfo().linux_distro == "ubuntu":
                if tools.OSInfo().os_version == "18.04":
                    libUrl = "https://onewayautomation.com/opcua-binaries/sdk/ubuntu1804-OpcUaSdk-1.1.2-demo.zip"
                    libFileName = "ogamma-sdk/lib/libOpcUaSdk.a"
                else:
                    libUrl = "https://onewayautomation.com/opcua-binaries/sdk/ubuntu2004-OpcUaSdk-1.1.2-demo.zip"
                    libFileName = "ogamma-sdk/lib/libOpcUaSdk.a"
            elif tools.OSInfo().linux_distro == "debian":
                libUrl = "https://onewayautomation.com/opcua-binaries/sdk/debian1010-OpcUaSdk-1.1.2-demo.zip"
                libFileName = "ogamma-sdk/lib/libOpcUaSdk.a"
            elif tools.OSInfo().linux_distro == "rhel":
                libUrl = "https://onewayautomation.com/opcua-binaries/sdk/rhel84-OpcUaSdk-1.1.2-demo.zip"
                libFileName = "ogamma-sdk/lib/libOpcUaSdk.a"                
        else:
            libFileName = "ogamma-sdk/lib/OpcUaSdk.lib"
            
        
        fullPathToLibFile = "{}/{}".format(self.source_folder, libFileName);
        
        if  not os.path.exists(fullPathToLibFile):
            try:
                print ("Trying to download library file from {}.".format(libUrl))
                tools.download(libUrl, zipFileName, overwrite=True)
                tools.unzip(zipFileName, libFolderName)
            except errors.ConanException:
                print ("Failed to get the library file. Ignoring, probably zip file exists.")
            except:
                print("Failed to get the library file, unexpected error: ", sys.exc_info()[0])
                raise
        else:
            print("Detected library file {}".format(fullPathToLibFile))
        #End of downlaoding library.

        # Copy ogamma-sdk source code and downloaded library file to the build folder:
        self.copy("*", src="ogamma-sdk", dst="ogamma-sdk")   
            
    def configure(self):
        if self.settings.os == "Linux":
            self.settings.compiler.libcxx="libstdc++11"

        self.options["botan"].amalgamation = False
        self.options["botan"].shared = False
        self.options["boost"].shared = False
        self.options["boost"].multithreading = True
           
    def _configure_cmake(self):
        if self.cmake:
            return self.cmake
        self.cmake = CMake(self)
        self.cmake.verbose = True
        version = Version(self.version)

        self.cmake.configure()
        return self.cmake
        
    def build(self):
        cmake = self._configure_cmake()
        
        if self.settings.os == "Windows":
            cmake.definitions["_WIN32_WINNT"] = "0x0601"
        
        cmake.configure()
        cmake.build()
        
        try:
            xmlFileName = "{}/ogamma-sdk/bin/Opc.Ua.xml".format(self.build_folder)
            dstFileName = "{}/bin/Opc.Ua.xml".format(self.build_folder)
            if not os.path.exists(dstFileName):
                copyfile(xmlFileName, dstFileName)
        except:
            print("Warning: failed to copy file {}".format(xmlFileName))
    
    def package(self):
        self.copy("sampleApp.*", src="bin", dst="bin")
        self.copy("Opc.Ua.xml", src="ogamma-sdk/bin", dst="bin")
