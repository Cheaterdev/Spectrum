#pragma once

import Exceptions;

#ifdef SPECTRUM_ENABLE_EXEPTIONS
#define SPECTRUM_TRY try{
#define SPECTRUM_CATCH }catch (const std::system_error& e){Log::get() << Log::LEVEL_ERROR << e.what() << Log::endl;}
#else
#define SPECTRUM_TRY {
#define SPECTRUM_CATCH }
#endif