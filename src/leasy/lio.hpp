#pragma once

#include <iostream>

#define printerr(X) std::cout << "[leasy.Stderr]: " << __func__ << "(*)\t" << X << std::endl;
#define printinf(X) std::cout << "[leasy.Stdout]: " << __func__ << "(*)\t" << X << std::endl;