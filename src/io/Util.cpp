#include "io/Util.hpp"

const glm::mat4 HERMITE_MTX(
    2.0f, -2.0f, 1.0f, 1.0f,
    -3.0f, 3.0f, -2.0f, -1.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    1.0f, 0.0f, 0.0f, 0.0f
);

float InterpolateHermite(float factor, float timeA, float valueA, float outTangent, float timeB, float valueB, float inTangent){
    float framesBetweenKeys = timeB - timeA;
    
    glm::vec4 timeParameters = glm::vec4(factor * factor * factor, factor * factor, factor, 1.0f);
    glm::vec4 valueParameters = glm::vec4(valueA, valueB, outTangent * framesBetweenKeys, inTangent * framesBetweenKeys);

    glm::vec4 transform = HERMITE_MTX * timeParameters;
    glm::vec4 result = transform * valueParameters;

    return result.x + result.y + result.z + result.w;
}

float MixTrack(LTrackCommon& track, float time, uint32_t& previousKey, uint32_t& nextKey, bool adjustSlope){
	if(track.mKeys.size() == 1) track.mFrames[track.mKeys[0]];
	if(nextKey < track.mKeys.size()){
		float slopeOut = track.mFrames[track.mKeys[previousKey]].outslope;
		float slopeIn = track.mFrames[track.mKeys[nextKey]].inslope;
		if(adjustSlope){
			slopeOut = slopeOut * (track.mKeys[nextKey] - track.mKeys[previousKey]);
			slopeIn = slopeIn * (track.mKeys[nextKey] - track.mKeys[previousKey]);
		}
		float v = InterpolateHermite(
			(time - track.mFrames[track.mKeys[previousKey]].frame) / (track.mFrames[track.mKeys[nextKey]].frame - track.mFrames[track.mKeys[previousKey]].frame),
			track.mFrames[track.mKeys[previousKey]].frame,
			track.mFrames[track.mKeys[previousKey]].value,
			slopeOut,
			track.mFrames[track.mKeys[nextKey]].frame,
			track.mFrames[track.mKeys[nextKey]].value,
			slopeIn
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