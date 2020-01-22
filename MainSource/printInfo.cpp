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
#include <iterator>

int printInfo(void)
{
    std::cout << "Lib Version: v" << SoapySDR::getLibVersion() << std::endl;
    std::cout << "API Version: v" << SoapySDR::getAPIVersion() << std::endl;
    std::cout << "ABI Version: v" << SoapySDR::getABIVersion() << std::endl;
    std::cout << "Install root: " << SoapySDR::getRootPath() << std::endl;
    
    std::vector<std::string> path=SoapySDR::listSearchPaths();
    for(size_t i=0;i<path.size();++i){
        std::cout << "Search path: " << path[i] << std::endl;
    }
    
    std::vector<std::string> mod=SoapySDR::listModules();
    
    for (size_t k=0;k<mod.size();++k)
    {
        std::cout << "Module found: " << mod[k];
        const auto &errMsg = SoapySDR::loadModule(mod[k]);
        if (not errMsg.empty()) std::cout << "\n  " << errMsg;
        std::cout << std::endl;
    }
    if (mod.empty()) std::cout << "No modules found!" << std::endl;
    
    
    return 0;
}


