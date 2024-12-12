# Sample Application built using ``ogamma-sdk``

This repository contains source code for the Sample OPC UA Client Application built using ``ogamma-sdk``, OPC UA Client C++ SDK library from One-Way Automation: https://onewayautomation.com/opcua-sdk

## Prerequisites

### ogamma-sdk (OPC UA Client C++ SDK)

The sample application uses ``ogamma-sdk`` as a dependency library. You can download it from our online store at https://onewayautomation.com/online-store

There is a free demo version of the distributable package (no credit card is required to download it), as well as commercial version is available at this online store.

To use ``ogamma-sdk`` library as a dependency in your C++ projects you need only its own header files, there is no need to access header files of its dependency libraries such as `boost``. Binary format distribution package of the sdk has pre-built binaries of the sdk itself and its dependencies. This makes easy to integrate the library into your applications.

Download the sdk distribution package from our online store and unzip its content into the folder ``ogamma-sdk``, located at the same level as this project's source code. If you move it to some other location, adjust path to the library in the file ``CMakeLists.txt`` (variable ``OGAMMA_SDK_PATH``).

Folder structure should be like this:

  ```
  < Folder for Source Code >
            |
            |--- ogamma-sdk
                     |
                     |--- bin
                     |--- include
                     |--- lib
            |--- ogamma-sdk-sample-app
                     |
                     |--- sampleApp
  ```

### Build tools for the target platform.

#### Windows

- Visual Studio 2019 or 2022. Community Edition is enough.
- CMake

#### Linux

- GCC compiler
- CMake

## Building of the sample application

- Open terminal and navigate to the source code repository root folder.
- Create build folder, for eample, ``build``, and navigate to it:

    ```
    mkdir build
    cd build
    ```
- Configure build files by running the command:

    ```
    cmake ../
    ```

- Build the project:

    - Windows

        Open solution ``sampleApp.sln`` and build it.

        The program executable should be created at ``build\sampleApp\Release\sampleApp.exe``

    - Linux

        Run command:
        
        ```
        make
        ```

        The program executable should be created at ``build/sampleApp/sampleApp``.

## Running of the sample application.

- The application requires standard OPC UA model definition file ``Opc.Ua.xml`` at the process's current working directory. You can copy it from ogamma SDK's ``bin`` sub-folder (``ogamma-sdk/bin``). 
- At the first start it will create default application instance certificate, under folder ``build/sampleApp/data/PKI``. 

    Note that to connect to OPC UA servers in secured mode, you will need to move server's application instance certificate or its issuer certificate to the ``trusted/certs`` sub-folder under ``build/sampleApp/data/PKI``. In certificate chain has many issuers, they must be copied either into the trusted folder or issuers folder.

## ``ogamma-sdk`` Developer's Guide

For more information please refer OPC UA C++ SDK Developer's Guide at https://onewayautomation.com/opcua-sdk-docs/html
