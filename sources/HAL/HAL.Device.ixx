export module HAL:Device;

import IdGenerator;
import Singleton;
import StateContext;
import Data;
import :API.Device;

export namespace HAL
{

	class Queue;
		class Device : public Singleton<Device>, public API::Device
		{
			friend class API::Device;
			friend class Singleton<Device>;
		private:
			
			HAL::DeviceProperties properties;

			HAL::Adapter::ptr adapter;
			enum_array<HAL::CommandListType, std::shared_ptr<HAL::Queue>> queues;
			IdGenerator<Thread::Lockable> id_generator;
			friend class CommandList;
			bool rtx = false;
		
		public:
			Device(HAL::DeviceDesc desc);
			virtual ~Device();

			static std::shared_ptr<Device> create_singleton();

			void stop_all();
		
			const HAL::DeviceProperties& get_properties() const { return properties;}
			ContextGenerator context_generator;
			std::shared_ptr<CommandList> get_upload_list();
	
			std::shared_ptr<HAL::Queue>& get_queue(HAL::CommandListType type);

			bool is_rtx_supported() { return rtx; }

			mutable bool alive = true;
		};

	


}