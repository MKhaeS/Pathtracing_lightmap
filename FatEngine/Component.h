#pragma once
#include <memory>
#include <vector>
#include "FatDXFramework.h"
#include "DirectXMath.h"
#include "EventHandler.h"


class Component {

public :
	
	void SetPosition (int x, int y);
	void SetSize (int w, int h);

    EventHandler OnMouseClick;

protected:
	int m_Pos_x_;
	int m_Pos_y_;
	int m_Width_;
	int m_Height_;
	bool m_bVisible_ = true;
	bool m_bEnable_ = true;
    XMFLOAT3 m_Color_;
	std::shared_ptr<Component> m_Parent_;

	mutable int m_ViewIndex_;

	virtual std::unique_ptr<std::vector<float>> 
                GetVertexColorData (const int&  index,          const int&      width, 
                                    const int&  height,         const float&    depth,
                                    int*        numOfVerts,     int*            stride ) const = 0;

    

private:
	friend class Form;	
};

//Base components
//class Button;		// "Button.h"