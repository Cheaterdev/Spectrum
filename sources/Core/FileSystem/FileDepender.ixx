export module Core:FileDepender;

import :Utils;
import :Log;
import :Singleton;
import :Scheduler;

import stl.filesystem;
import stl.memory;
import windows;

import :serialization;
import :FileSystem;

export
{


	class resource_file_depender
	{
		struct depender
		{
			std::filesystem::path file_name;
			std::filesystem::file_time_type modify_time;

			bool need_update() const;

            bool operator<(const  depender& r) const
            {
	            return file_name<r.file_name;
            }
		private:

			inline const std::wstring get_name() const { return file_name.wstring(); }
			SERIALIZE()
			{
				ar& NVP(file_name)& NVP(modify_time);
			}
		};
		std::set<depender> files;
	public:

		inline const std::set<depender>& get_files() const { return files; }

		void add_depend(std::shared_ptr<file> _file);
		bool need_update() const;
		void clear();
		bool depends_on(std::string v) const;
	private:
		SERIALIZE()
		{
			ar& NVP(files);
		}
	};

}

