#include "pch.h"




GUI::Elements::resizable::resizable()
{
	resizers[2].reset(new resizer());
	resizers[2]->docking = dock::BOTTOM;
	resizers[2]->dir = direction::BOTTOM;
	resizers[2]->target = this;
	add_child(resizers[2]);

	resizers[1].reset(new resizer());
	resizers[1]->docking = dock::LEFT;
	resizers[1]->dir = direction::LEFT_BOTTOM;
	resizers[1]->target = this;
	resizers[2]->add_child(resizers[1]);


	resizers[3].reset(new resizer());
	resizers[3]->docking = dock::RIGHT;
	resizers[3]->dir = direction::RIGHT_BOTTOM;
	resizers[3]->target = this;
	resizers[2]->add_child(resizers[3]);


	resizers[8].reset(new resizer());
	resizers[8]->docking = dock::TOP;
	resizers[8]->dir = direction::TOP;
	resizers[8]->target = this;
	add_child(resizers[8]);



	resizers[7].reset(new resizer());
	resizers[7]->docking = dock::LEFT;
	resizers[7]->dir = direction::LEFT_TOP;
	resizers[7]->target = this;
	resizers[8]->add_child(resizers[7]);



	resizers[9].reset(new resizer());
	resizers[9]->docking = dock::RIGHT;
	resizers[9]->dir = direction::RIGHT_TOP;
	resizers[9]->target = this;
	resizers[8]->add_child(resizers[9]);


	resizers[4].reset(new resizer());
	resizers[4]->docking = dock::LEFT;
	resizers[4]->dir = direction::LEFT;
	resizers[4]->target = this;
	add_child(resizers[4]);

	resizers[6].reset(new resizer());
	resizers[6]->docking = dock::RIGHT;
	resizers[6]->dir = direction::RIGHT;
	resizers[6]->target = this;
	add_child(resizers[6]);
}
