
class FSR :public Events::prop_handler, public FrameGraphGenerator
{
public:
	using ptr = std::shared_ptr<FSR>;
	virtual void generate(FrameGraph& graph) override;
};
