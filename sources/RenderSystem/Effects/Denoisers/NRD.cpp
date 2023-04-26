module Graphics:NRDDenoiser;
import :FrameGraphContext;
import :MipMapGenerator;
import HAL;


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
				// Note that we use method mask as a unique id
				// This allows us to evaluate different NRD denoisers in the same or different viewports but at the different stages with the same frame
				//if (!slSetFeatureConstants(sl::kFeatureNRD, &nrdConsts, myFrameIndex, nrdConsts.methodMask))
				//{
				//	// Handle error here, check the logs
				//}


			}/*, PassFlags::Compute*/);
	}


}

