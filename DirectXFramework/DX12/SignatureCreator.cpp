#include "pch.h"


DX12::RootSignature::ptr DX12::SignatureCreator::create_root()
{
	return std::make_shared<RootSignature>(desc);
}

DX12::Signature::Signature(DX12::SignatureDataSetter* context) : context(context)
{

}
