#include "pch.h"


	void ContextGenerator::generate(StateContext* context)
	{

		context->local_id = generator.get();
		context->global_id = global_id++;
		context->owner = this;
	}

	void ContextGenerator::free(StateContext* context)
	{
		generator.put(context->local_id);
	}

	void StateContext::release()
	{
		owner->free(this);
		owner = nullptr;
	}


	