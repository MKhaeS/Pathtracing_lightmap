#pragma once
#include "Component.h"
#include "DirectXMath.h"

class Button : public Component
{
public:

				 Button             (const int& x,		    const int& y, 
				 		             const int& width,	    const int& height, 
				 		             const XMFLOAT3& color                      );

protected:
    
    virtual std::unique_ptr<std::vector<float>>
                                GetVertexColorData (const int&	index, const int& width,
                                                    const int& height, const float& depth,
                                                    int* numOfVerts, int* stride ) const override;
};