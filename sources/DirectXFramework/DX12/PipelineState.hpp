
template<class Archive>
void DX12::PipelineStateDesc::serialize(Archive& ar, const unsigned int)
{
	ar& NVP(name);

	ar& NVP(topology);
	ar& NVP(blend);
	ar& NVP(rasterizer);
	ar& NVP(rtv);


	if constexpr (Archive::is_saving::value)
	{

		auto sig = dynamic_cast<RootLayout*>(root_signature.get());
		ar& NVP(sig->layout);
		auto save_header = [&](auto& shader) {

			bool has_header = !!shader;
			ar& NVP(has_header);

			if (has_header)
				ar& NVP(shader->get_header());
		};

		save_header(pixel);
		save_header(vertex);
		save_header(geometry);
		save_header(hull);
		save_header(domain);

		//	save_header(mesh);
		//	save_header(amplification);

	}
	else
	{
		Layouts l;

		ar& NVP(l);

		root_signature = get_Signature(l);
		auto load_header = [&]<class T>(std::shared_ptr<T>&shader) {

			using Type = decltype(*shader.get());
			bool has_header;
			ar& NVP(has_header);

			if (has_header)
			{
				D3D::shader_header header;
				ar& NVP(header);

				shader = Type::get_resource(header);
			}

		};



		load_header(pixel);
		load_header(vertex);
		load_header(geometry);
		load_header(hull);
		load_header(domain);

		//	load_header(mesh);
		//	load_header(amplification);
	}
}


template<class Archive>
void DX12::GraphicsPipelineStateDesc::serialize(Archive& ar, const unsigned int)
{
	ar& NVP(name);

	ar& NVP(topology);
	ar& NVP(blend);
	ar& NVP(rasterizer);
	ar& NVP(rtv);


	if constexpr (Archive::is_saving::value)
	{

		auto sig = dynamic_cast<RootLayout*>(root_signature.get());
		ar& NVP(sig->layout);
		auto save_header = [&](auto& shader) {

			bool has_header = !!shader;
			ar& NVP(has_header);

			if (has_header)
				ar& NVP(shader->get_header());
		};


		save_header(mesh);
		save_header(amplification);

	}
	else
	{
		Layouts l;

		ar& NVP(l);

		root_signature = get_Signature(l);
		auto load_header = [&]<class T>(std::shared_ptr<T>&shader) {

			using Type = decltype(*shader.get());
			bool has_header;
			ar& NVP(has_header);

			if (has_header)
			{
				D3D::shader_header header;
				ar& NVP(header);

				shader = Type::get_resource(header);
			}

		};


		load_header(mesh);
		load_header(amplification);
	}
}