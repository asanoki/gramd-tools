/*
 * log.cpp
 *
 *  Created on: Mar 31, 2012
 *      Author: asanoki
 */

#include "log.h"
#include <iostream>

namespace jovislab {

boost::mutex Log::m_mutex;

Log Log::debug(std::wcout);
Log Log::info(std::wcout);
Log Log::message(std::wcout);
Log Log::notice(std::wcout);
Log Log::warning(std::wcerr);
Log Log::error(std::wcerr);

}
