﻿/***********************************************************************************
**
** PartyObject.h
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#ifndef PARTYOBJECT_H
#define PARTYOBJECT_H
//----------------------------------------------------------------------------------
#include "Globals.h"
#include "Game objects/GameCharacter.h"

//---------------------------------------------------------------------------
//Объект группы
class CPartyObject
{
	SETGET(uint, Serial, 0);

public:
	CPartyObject();
	~CPartyObject() {}
	CGameCharacter *Character{ NULL };
	string GetName(const int &index);
};
//----------------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------------
