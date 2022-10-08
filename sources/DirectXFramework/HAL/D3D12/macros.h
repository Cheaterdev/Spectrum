#pragma once
#define STRINGIZE(x) #x
#define TEST(device, x) device->process_result(x,STRINGIZE(x))