#include "Model.hpp"

LModel::LModel()
{

}

bool LModel::LoadBIN(bStream::CMemoryStream* stream)
{
	stream->readInt8();
	mName = std::string(stream->readString(11));

	return true;
}

bool LModel::LoadMDL(bStream::CMemoryStream* stream)
{
	return true;
}

void LModel::Render()
{

}
