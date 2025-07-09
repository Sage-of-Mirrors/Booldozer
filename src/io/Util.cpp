#include "io/Util.hpp"

float InterpolateHermite(float factor, float timeA, float valueA, float outTangent, float timeB, float valueB, float inTangent){
	float a = factor - timeA;
	float b = a * (1.0f / (timeB - timeA));
	float c = b - 1.0f;
	float d = (3.0f + -2.0f * b) * (b * b);

	float cab = c * a * b;
	float coeffx3 = cab * inTangent;
	float cca = c * c * a;
	float coeffc2 = cca * outTangent;

	return b * c * a * inTangent + a * c * c * outTangent + (1.0f - d) * valueA + d * valueB;
}

float MixTrack(LTrackCommon& track, uint32_t frameCount, float time, uint32_t& previousKey, uint32_t& nextKey){
	if(nextKey < track.mKeys.size()){
		float v = InterpolateHermite(
			(time - track.mFrames[track.mKeys[previousKey]].frame) / (track.mFrames[track.mKeys[nextKey]].frame - track.mFrames[track.mKeys[previousKey]].frame),
			track.mFrames[track.mKeys[previousKey]].frame,
			track.mFrames[track.mKeys[previousKey]].value,
			track.mFrames[track.mKeys[previousKey]].outslope,
			track.mFrames[track.mKeys[nextKey]].frame,
			track.mFrames[track.mKeys[nextKey]].value,
			track.mFrames[track.mKeys[nextKey]].inslope
		);

		if(time >= track.mFrames[track.mKeys[nextKey]].frame){
			nextKey += 1;
			previousKey +=1;
		}
		return v;
	} else {
		return track.mFrames[track.mKeys[previousKey]].value;
	}
}