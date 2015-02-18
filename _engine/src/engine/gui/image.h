#pragma once

#include "engine/gui/panel.h"
#include "engine/render/graph/tex_manager.h"

class GUIImage : public GUIPanel
{
	protected:
		NYTexFile * _Tex;

	public:
		GUIImage() : GUIPanel()
		{					
			_Tex = NULL;
		}

		void setPos(uint16 x, uint16 y)
		{
			X = x;
			Y = y;
		}
		
		void loadImage(std::string & pngFile)
		{
			_Tex = NYTexManager::getInstance()->loadTexture(pngFile);
			Width = _Tex->SizeX;
			Height = _Tex->SizeY;
		}

				
		sint8 mouseCallback(int x, int y, uint16 click, sint16 wheel, uint16 zorder, bool focusAvailable, uint32 elapsed)
		{
			return GUIPanel::mouseCallback(x,y,click,wheel,zorder,focusAvailable,elapsed);
		}

		void render(uint16 zorder)
		{
			if(Visible && zorder == ZOrder)
			{
				if(_Tex)
					NYTexManager::getInstance()->drawTex2D(X,Y,1,1,0,0,*_Tex);
			}
		}
};