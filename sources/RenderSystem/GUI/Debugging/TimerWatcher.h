#pragma once

import GUI;
//#include "GUI/Elements/Tree.h"

namespace GUI
{
	namespace Elements
	{
		namespace Debug
		{
			class TimedLabel : public label
			{
				std::chrono::time_point<std::chrono::system_clock>  last_time;

				void think(float dt) override;
			public:
				using ptr = s_ptr<TimedLabel>;
				using wptr = w_ptr<TimedLabel>;

				TimedLabel* set(TimedBlock* timer);
			};

			class TimeCreator : public tree_creator<TimedBlock>
			{
			public:
				virtual void init_element(tree_element<TimedBlock>* tree, TimedBlock* elem) override;
			};

			class TimerWatcher : public  GUI::Elements::tree<TimedBlock>
			{
			public:
				using ptr = s_ptr<TimerWatcher>;

				TimerWatcher();
			};
		}
	}
}