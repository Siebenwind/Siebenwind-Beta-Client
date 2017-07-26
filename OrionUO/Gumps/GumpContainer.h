﻿/***********************************************************************************
**
** GumpContainer.h
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#ifndef GUMPCONTAINER_H
#define GUMPCONTAINER_H
//----------------------------------------------------------------------------------
#include "Gump.h"
#include "../TextEngine/TextRenderer.h"
//----------------------------------------------------------------------------------
class CGumpContainer : public CGump
{
	SETGET(bool, IsGameBoard, false);

private:
	uint m_CorpseEyesTicks{ 0 };
	uchar m_CorpseEyesOffset{ 0 };

	static const uint ID_GC_LOCK_MOVING = 0xFFFFFFFE;
	static const uint ID_GC_MINIMIZE = 0xFFFFFFFF;

	CTextRenderer m_TextRenderer{ CTextRenderer() };

	CGUIGumppic *m_CorpseEyes{ NULL };
	CGUIDataBox *m_DataBox{ NULL };

protected:
	virtual void CalculateGumpState();

public:
	CGumpContainer(uint serial, uint id, short x, short y);
	virtual ~CGumpContainer();

	CGUIGumppic *m_BodyGump{ NULL };
	
	void UpdateItemCoordinates(class CGameObject *item);

	CTextRenderer *GetTextRenderer() { return &m_TextRenderer; }

	virtual void PrepareTextures();

	virtual void PrepareContent();

	virtual void UpdateContent();

	virtual void InitToolTip();

	virtual void Draw();
	virtual CRenderObject *Select();

	GUMP_BUTTON_EVENT_H;

	virtual void OnLeftMouseButtonUp();
	virtual bool OnLeftMouseButtonDoubleClick();
};
//----------------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------------
