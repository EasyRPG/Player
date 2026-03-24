#pragma once

#include <iostream>

#define LDBG(X) std::cout << "[leasy.Debug]: " << __FILE__ << ":" << __LINE__ << ":" << __func__ << "\t" << X << std::endl;
#define LDBG_RECORD(X) LDBG("[RECORD]: " #X); X;