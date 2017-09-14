#pragma once

#include "fontFace.h"

namespace alfons {

class AppleFontFace : public FontFace {

public:

    AppleFontFace(FreetypeHelper& _ft, FaceID faceId, const Descriptor& descriptor, float baseSize);

    bool load() override;

};

}
