#pragma once
#ifndef MODEL_H
#define MODEL_H
#include "DSdataType.h"


// * Each rect's data structure.
// * tlwh: topleft point & (w,h)
// * confidence: detection confidence.
// * feature: the rect's 128d feature.
// */
class DETECTION_ROW
{
public:
    DETECTBOX tlwh;
    float confidence;
    FEATURE feature;
    DETECTBOX to_xyah() const;
    DETECTBOX to_tlbr() const; 
    float cls;
};

typedef std::vector<DETECTION_ROW> DETECTIONS;



#endif // MODEL_H