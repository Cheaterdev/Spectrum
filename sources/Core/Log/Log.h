#pragma once

import Singletons;
import Logs;


#define EVENT(x) {Log::get()<<"EVENT: "<<#x<< " time: "<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count()<<Log::endl;}