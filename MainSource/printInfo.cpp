//
//  printInfo.cpp
//  SdrGlut
//
//  Created by Dale on 1/21/20.
//  Copyright © 2020 Dale Ranta. All rights reserved.
//

#include <SoapySDR/Version.hpp>
#include <SoapySDR/Modules.hpp>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>
#include <vector>
#include <iostream>

int printInfo(void)
{
    std::cout << "Lib Version: v" << SoapySDR::getLibVersion() << std::endl;
    std::cout << "API Version: v" << SoapySDR::getAPIVersion() << std::endl;
    std::cout << "ABI Version: v" << SoapySDR::getABIVersion() << std::endl;
    std::cout << "Install root: " << SoapySDR::getRootPath() << std::endl;
    
    for (const auto &path : SoapySDR::listSearchPaths())
        std::cout << "Search path: " << path << std::endl;
    
    //get a list of module and calculate the max path length
    const auto modules = SoapySDR::listModules();
    size_t maxModulePathLen(0);
    for (const auto &mod : modules) maxModulePathLen = std::max(maxModulePathLen, mod.size());
    
    //load each module and print information
    for (const auto &mod : modules)
    {
        std::cout << "Module found: " << mod;
        const auto &errMsg = SoapySDR::loadModule(mod);
        if (not errMsg.empty()) std::cout << "\n  " << errMsg;
        const auto version = SoapySDR::getModuleVersion(mod);
        if (not version.empty()) std::cout << std::string(maxModulePathLen-mod.size(), ' ') << " (" << version << ")";
        std::cout << std::endl;
    }
    if (modules.empty()) std::cout << "No modules found!" << std::endl;
    
    
    return EXIT_SUCCESS;
}


