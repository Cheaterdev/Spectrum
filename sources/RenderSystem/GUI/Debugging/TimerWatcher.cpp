#include "pch_render.h"
#include "TimerWatcher.h"
#include "GUI/Elements/Image.h"

void GUI::Elements::Debug::TimedLabel::think(float dt)
{
	color.w = std::chrono::duration<double>(std::chrono::system_clock::now() - last_time).count() > 1 ? 128 :255;
}

GUI::Elements::Debug::TimedLabel* GUI::Elements::Debug::TimedLabel::set(TimedBlock* timer)
{
				
	docking = dock::LEFT;
	thinkable = true;
	//	time_text->text = convert(elem->get_name());
	magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
	margin = { 5, 0, 0, 0 };

	//	wptr weak = get_ptr<TimedLabel>();
	timer->cpu_counter.on_time.register_handler(this, [this](const float2 & time)
	{
		//	auto ptr = weak.lock();
		//	if (!ptr) return;
		//run_on_ui([this, time]() {
		text = std::string("GPU:") + std::to_string(time.x * 1000) + " CPU:" + std::to_string(time.y * 1000);
		last_time = std::chrono::system_clock::now();

		//	});

	});
				
	return this;
}

void GUI::Elements::Debug::TimeCreator::init_element(tree_element<TimedBlock>* tree, TimedBlock* elem)
{
	auto timer = dynamic_cast<TimedBlock*>(elem);
	base::ptr l(new line(tree));
	toogle_icon::ptr	open_icon(new toogle_icon());
	open_icon->texture = HAL::Texture::null;
	open_icon->size = { 16, 16 }; // size_type::MATCH_PARENT;
	open_icon->docking = dock::LEFT;
	//open_icon->visible = false;
	l->add_child(open_icon);
	image::ptr icon(new image());
	//
	icon->size = { 16, 16 }; // size_type::MATCH_PARENT;
	icon->docking = dock::LEFT;
	icon->width_size = size_type::FIXED;
	icon->height_size = size_type::FIXED;
	l->add_child(icon);
	{
		label::ptr label_text(new label());
		label_text->docking = dock::LEFT;
		label_text->text = convert(elem->get_name());
		label_text->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
		label_text->margin = { 5, 0, 0, 0 };
		l->add_child(label_text);
	}
	tree->add_child(l);
	{
		TimedLabel::ptr time_text((new TimedLabel));
		time_text->set(timer);
		l->add_child(time_text);
		tree->add_child(l);
                         
	}
}

GUI::Elements::Debug::TimerWatcher::TimerWatcher(): GUI::Elements::tree<TimedBlock>(tree_creator<TimedBlock>::ptr(new TimeCreator))
{
}
