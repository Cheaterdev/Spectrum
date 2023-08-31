module Graphics:NRDDenoiser;
import :FrameGraphContext;
import :MipMapGenerator;
import HAL;
import <RenderSystem.h>;
import FrameGraph;


using namespace FrameGraph;
using namespace HAL;

import streamline;


NRDDenoiser::NRDDenoiser()
{

}



void NRDDenoiser::generate(Graph& graph)
{

	auto& frame = graph.get_context<ViewportInfo>();
	auto size = frame.frame_size;

	//size/=2;

	{
		struct ShadowDenoiser_PrepareData
		{
			Handlers::Texture H(RTXDebug);

		};

		graph.pass<ShadowDenoiser_PrepareData>("ShadowDenoiser_Prepare", [this, &graph](ShadowDenoiser_PrepareData& data, TaskBuilder& builder) {
			builder.need(data.RTXDebug, ResourceFlags::UnorderedAccess);
			//	builder.create(data.ShadowDenoiser_TileBuffer, { tileCount }, ResourceFlags::UnorderedAccess);

			return true;
			}, [this, &graph, size](ShadowDenoiser_PrepareData& data, FrameContext& _context) {
				auto& list = *_context.get_list();
				auto& compute = list.get_compute();

				//			compute.set_pipeline<PSOS::DenoiserShadow_Prepare>();

					auto& cam = graph.get_context<CameraInfo>();

				
// call slNRDSetConstants with populated NRDConstants object




				sl::NRDConstants nrdConsts = {};
				// Depending on what type of denoising we are doing, set the method mask
		/*		if (enableAo)
				{
					nrdConsts.methodMask |= (1 << sl::NRDMethods::eNRDMethodReblurDiffuseOcclusion);
				}
				else if (enableDiffuse && enableSpecular)
				{
					nrdConsts.methodMask |= (1 << sl::NRDMethods::eNRDMethodReblurDiffuseSpecular);
				}
				else if (enableDiffuse)
				{*/
					nrdConsts.methodMask |= (1 << static_cast<uint>(sl::NRDMethods::eSigmaShadow));
			/*	}
				else if (enableSpecular)
				{
					nrdConsts.methodMask |= (1 << sl::NRDMethods::eNRDMethodReblurSpecular);
				}*/

				// Various camera matrices must be provided

					memcpy(&nrdConsts.clipToWorld,cam.cam->camera_cb.current.GetInvProj().raw(),sizeof(sl::float4x4) );
					memcpy(&nrdConsts.clipToWorldPrev,cam.cam->camera_cb.prev.GetInvProj().raw(),sizeof(sl::float4x4) );
		//		memcpy(&nrdConsts.worldToViewMatrix,cam.cam->camera_cb.current.GetView().raw(),sizeof(sl::float4x4) );


			//	nrdConsts.clipToWorld = cam.cam->camera_cb.current.GetInvProj().raw();
			//	nrdConsts.clipToWorldPrev = cam.cam->camera_cb.prev.GetInvProj().raw(); // clipToWorld from the previous frame
				nrdConsts.common = {};
			//	nrdConsts.common.worldToViewMatrix = cam.cam->camera_cb.current.GetView().raw();
				nrdConsts.common.motionVectorScale[0] = 1; // Normally 1
				nrdConsts.common.motionVectorScale[1] = 1; // Normally 1
				nrdConsts.common.resolutionScale[0] = 1; // Are we rendering within a larger render target? If so set scale, otherwise set to 1.0f
				nrdConsts.common.resolutionScale[1] = 1; // Are we rendering within a larger render target? If so set scale, otherwise set to 1.0f
				nrdConsts.common.denoisingRange = 1;//myDenoisingRange;
				nrdConsts.common.splitScreen = 0;//mySplitScreen;
				nrdConsts.common.accumulationMode = sl::NRDAccumulationMode::CONTINUE;//myAccumMode; // Reset or keep history etc

				// Now set the specific settings for the method(s) chosen above (defaults are OK for 99% of the applications)
				sl::NRDReblurSettings& reblurSettings = nrdConsts.reblurSettings;
				reblurSettings = {};


				static uint myFrameIndex = 0;
				myFrameIndex++;
					sl::ViewportHandle viewportHandle{ 0 };

				auto hr = slNRDSetConstants(viewportHandle, nrdConsts);
				    sl::FrameToken* frameToken;
    slGetNewFrameToken(frameToken, &myFrameIndex);
 
	{
		sl::ViewportHandle viewportHandle{ 0 };
						sl::Constants consts = {};
// Set motion vector scaling based on your setup
consts.mvecScale = {1,1}; // Values in eMotionVectors are in [-1,1] range
//consts.mvecScale = {1.0f / 1,1.0f / 1}; // Values in eMotionVectors are in pixel space
//consts.mvecScale = 1; // Custom scaling to ensure values end up in [-1,1] range
// Set all other constants here
auto hr3 = slSetConstants(consts, *frameToken,viewportHandle); // constants are changing per frame so frame index is required

	}
	list.compiler.func([frameToken](API::CommandList& list)
					{
						auto l= list.get_native();
							sl::ViewportHandle viewportHandle{ 0 };
							const sl::BaseStructure* structs=&viewportHandle;
						auto hr2 = slEvaluateFeature(sl::kFeatureNRD, *frameToken, &structs, 1, l.Get());

					});


//	

			}/*, PassFlags::Compute*/);
	}


}

