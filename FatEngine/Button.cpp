
#include "Button.h"

Button::Button (const int& x,           const int& y, 
                const int& width,       const int& height, 
                const XMFLOAT3& color                      ) {
	this->m_Pos_x_ = x;
	this->m_Pos_y_ = y;
	this->m_Width_ = width;
	this->m_Height_ = height;
    this->m_Color_ = color;
}


std::unique_ptr<std::vector<float>> Button::GetVertexColorData (const int &     index,      const int&      width,
                                                                const int&      height,     const float&    depth,
                                                                int*            numOfVerts, int*            stride) const {
    this->m_ViewIndex_ = index;
    float x = 2 * static_cast<float>(m_Pos_x_) / width - 1;
    float y = 1 - 2*static_cast<float>(m_Pos_y_) / height;
    float z = depth;
    float w = 2*static_cast<float>(m_Width_) / width;
    float h = 2*static_cast<float>(m_Height_) / height;
    float r = m_Color_.x;
    float g = m_Color_.y;
    float b = m_Color_.z;
	
    auto vdata = std::make_unique<std::vector<float>> (std::vector<float> {
        x, y, z, r, g, b, 1.0f,
        x + w, y, z, r, g, b, 1.0f,
        x, y - h, z, r, g, b, 1.0f,
        x + w, y - h, z, r, g, b, 1.0f,
        x, y - h, z, r, g, b, 1.0f,
        x + w, y, z, r, g, b, 1.0f
    });
    *numOfVerts = 6;
    *stride = 28;
    return std::move (vdata);
}
