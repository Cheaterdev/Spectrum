module Graphics:RTX;
import HAL;
import Core;


void RTX::prepare(HAL::CommandList::ptr& list)
{
	rtx.prepare(list);
}

void RTX::update()
{
	PROFILE(L"RTX::update");

	// TEMPORARY (task R.1): how many distinct hit libraries the materials about to be added have.
	{
		std::vector<const void*> libs;
		{
			std::lock_guard<std::mutex> g(rtx.m);
			for (auto& [mat, info] : rtx.new_materials)
				libs.push_back(mat->raytracing_lib.operator->());
		}
		if (!libs.empty())
		{
			const size_t pending = libs.size();
			std::sort(libs.begin(), libs.end());
			libs.erase(std::unique(libs.begin(), libs.end()), libs.end());

			std::fstream temp("rtx_libs.temp", std::ios::out | std::ios::app);
			temp << "pending materials " << pending << ", distinct raytracing_lib " << libs.size() << "\n";
		}
	}

	rtx.update();
}
