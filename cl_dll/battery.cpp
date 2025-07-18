/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// battery.cpp
//
// implementation of CHudBattery class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

DECLARE_MESSAGE(m_Battery, Battery)

bool CHudBattery::Init()
{
	m_iBat = 0;
	m_fFade = 0;
	m_iFlags = 0;

	HOOK_MESSAGE(Battery);

	gHUD.AddHudElem(this);

	return true;
}


bool CHudBattery::VidInit()
{
	int HUD_suit_empty = gHUD.GetSpriteIndex("suit_empty");
	int HUD_suit_full = gHUD.GetSpriteIndex("suit_full");

	m_hSprite1 = m_hSprite2 = 0; // delaying get sprite handles until we know the sprites are loaded
	m_prc1 = &gHUD.GetSpriteRect(HUD_suit_empty);
	m_prc2 = &gHUD.GetSpriteRect(HUD_suit_full);
	m_iHeight = m_prc2->bottom - m_prc1->top;
	m_fFade = 0;
	return true;
}

bool CHudBattery::MsgFunc_Battery(const char* pszName, int iSize, void* pbuf)
{
	m_iFlags |= HUD_ACTIVE;

	BEGIN_READ(pbuf, iSize);
	int x = READ_SHORT();

	if (x != m_iBat)
	{
		m_fFade = FADE_TIME;
		m_iBat = x;
	}

	return true;
}


bool CHudBattery::Draw(float flTime)
{
	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH) != 0)
		return true;

	int r, g, b, x, y, a;
	Rect rc;

	rc = *m_prc2;
	
	// ############ hu3lifezado ############ //
	// Redimensionado para a nova armadura 666
	//rc.top += m_iHeight * ((float)(100 - (V_min(100, m_iBat))) * 0.01); // battery can go from 0 to 100 so * 0.01 goes from 0 to 1
	rc.top += m_iHeight * ((float)(100 - (V_min(100, (m_iBat / 6 - 5)))) * 0.01);	// battery can go from 0 to 100 so * 0.01 goes from 0 to 1
	// ############ //

	// ############ hu3lifezado ############ //
	// Mudei a cor do HUD (RGB_YELLOWISH)
	UnpackRGB(r, g, b, RGB_WHITEISH);
	// ############ //

	if (!gHUD.HasSuit())
		return true;

	// Has health changed? Flash the health #
	if (0 != m_fFade)
	{
		if (m_fFade > FADE_TIME)
			m_fFade = FADE_TIME;

		m_fFade -= (gHUD.m_flTimeDelta * 20);
		if (m_fFade <= 0)
		{
			a = 128;
			m_fFade = 0;
		}

		// Fade the health number back to dim

		a = MIN_ALPHA + (m_fFade / FADE_TIME) * 128;
	}
	else
		a = MIN_ALPHA;

	ScaleColors(r, g, b, a);

	int iOffset = (m_prc1->bottom - m_prc1->top) / 6;

	// ############ hu3lifezado ############ //
	// Movi a armadura para baixo do sangue
	//y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2;

	int width = (m_prc1->right - m_prc1->left);

	//// this used to just be ScreenWidth/5 (4 on Updated) but that caused real issues at higher resolutions. Instead, base it on the width of this sprite.
	//x = 3 * width;
	y = ScreenHeight - 2 * gHUD.m_iFontHeight;
	x = 12;
	// ############ //

	// make sure we have the right sprite handles
	if (0 == m_hSprite1)
		m_hSprite1 = gHUD.GetSprite(gHUD.GetSpriteIndex("suit_empty"));
	if (0 == m_hSprite2)
		m_hSprite2 = gHUD.GetSprite(gHUD.GetSpriteIndex("suit_full"));

	SPR_Set(m_hSprite1, r, g, b);
	SPR_DrawAdditive(0, x, y - iOffset, m_prc1);

	if (rc.bottom > rc.top)
	{
		SPR_Set(m_hSprite2, r, g, b);
		SPR_DrawAdditive(0, x, y - iOffset + (rc.top - m_prc2->top), &rc);
	}

	// ############ hu3lifezado ############ //
	// Afastei os numeros um pouco para longe do icone de bateria
	//x += width;
	//y += (int)(gHUD.m_iFontHeight * 0.2f);
	x += (m_prc1->right - m_prc1->left) + 4;
	// ############ //
	x = gHUD.DrawHudNumber(x, y, DHN_3DIGITS | DHN_DRAWZERO, m_iBat, r, g, b);

	return true;
}
