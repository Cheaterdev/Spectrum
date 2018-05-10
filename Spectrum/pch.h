// PROJECT includes
#pragma once
#define ALTERNATIVE_SCHEDULER

#include "../Core/pch.h"
#include "../FileSystem/pch.h"

#include "../RenderSystem/pch.h"
#include "../GameFramework/pch.h"
#include "../FlowGraph/pch.h"

#include "Application/Application.h"
#include "Platform/Window.h"

#include "Engine/SystemInfo.h"
#include "Engine/TaskManager.h"
#include "Engine/Scheduler.h"
#include "Engine/Engine.h"

#include "Threads/thread_pool.h"

// CURRENT includes
#if defined( DEBUG ) | defined( _DEBUG )
#include "Tests/MathTesting.h"
#include "Tests/ThreadTests.h"
#endif