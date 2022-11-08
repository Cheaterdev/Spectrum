module Graphics:RTX;
import HAL;


void RTX::prepare(HAL::CommandList::ptr& list)
{
	rtx.prepare(list);
}
