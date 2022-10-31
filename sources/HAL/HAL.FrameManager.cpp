module HAL:FrameManager;

import Utils;
import StateContext;
import Profiling;
import Threading;
import Exceptions;
import :Types;
import Math;
import Data;
import Singleton;
import Debug;

import stl.core;
import stl.memory;

import HAL;

namespace HAL {

	FrameResources::ptr FrameResourceManager::begin_frame()
	{

		auto result = std::make_shared<FrameResources>();


		result->frame_number = frame_number++;

		return result;
	}

	std::shared_ptr<CommandList> FrameResources::start_list(std::string name, CommandListType type)
	{
		auto list = (HAL::Device::get().get_queue(type)->get_free_list());
		list->begin(name);
		list->frame_resources = get_ptr();
		return list;
	}

}