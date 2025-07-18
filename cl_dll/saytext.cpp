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
// saytext.cpp
//
// implementation of CHudSayText class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

#include "vgui_TeamFortressViewport.h"

extern float* GetClientColor(int clientIndex);

// ############ hu3lifezado ############ //
// Numero de linhas aumentado de 5 para 8
#define MAX_LINES 8
// ############ //
#define MAX_CHARS_PER_LINE 256 /* it can be less than this, depending on char size */

// ############ hu3lifezado ############ //
// Suporte para impressao com mais cores
const Vector g_ColorBlue = { 0.6f, 0.8f, 1.0f };
const Vector g_ColorRed = { 1.00f, 0.3f, 0.3f };
const Vector g_ColorGreen = { 0.6f, 1.0f, 0.6f };
const Vector g_ColorGrey = { 0.8f, 0.8f, 0.8f };
// ############ //

// allow 20 pixels on either side of the text
#define MAX_LINE_WIDTH (ScreenWidth - 40)
#define LINE_START 10
static float SCROLL_SPEED = 5;

static char g_szLineBuffer[MAX_LINES + 1][MAX_CHARS_PER_LINE];
static float* g_pflNameColors[MAX_LINES + 1];
static int g_iNameLengths[MAX_LINES + 1];
static float flScrollTime = 0; // the time at which the lines next scroll up

static int Y_START = 0;
static int line_height = 0;

DECLARE_MESSAGE(m_SayText, SayText);

bool CHudSayText::Init()
{
	gHUD.AddHudElem(this);

	HOOK_MESSAGE(SayText);

	InitHUDData();

	m_HUD_saytext = gEngfuncs.pfnRegisterVariable("hud_saytext", "1", 0);
	m_HUD_saytext_time = gEngfuncs.pfnRegisterVariable("hud_saytext_time", "5", 0);
	m_con_color = gEngfuncs.pfnGetCvarPointer("con_color");

	m_iFlags |= HUD_INTERMISSION; // is always drawn during an intermission

	return true;
}


void CHudSayText::InitHUDData()
{
	memset(g_szLineBuffer, 0, sizeof g_szLineBuffer);
	memset(g_pflNameColors, 0, sizeof g_pflNameColors);
	memset(g_iNameLengths, 0, sizeof g_iNameLengths);
}

bool CHudSayText::VidInit()
{
	return true;
}


int ScrollTextUp()
{
	g_szLineBuffer[MAX_LINES][0] = 0;
	memmove(g_szLineBuffer[0], g_szLineBuffer[1], sizeof(g_szLineBuffer) - sizeof(g_szLineBuffer[0])); // overwrite the first line
	memmove(&g_pflNameColors[0], &g_pflNameColors[1], sizeof(g_pflNameColors) - sizeof(g_pflNameColors[0]));
	memmove(&g_iNameLengths[0], &g_iNameLengths[1], sizeof(g_iNameLengths) - sizeof(g_iNameLengths[0]));
	g_szLineBuffer[MAX_LINES - 1][0] = 0;

	if (g_szLineBuffer[0][0] == ' ') // also scroll up following lines
	{
		g_szLineBuffer[0][0] = 2;
		return 1 + ScrollTextUp();
	}

	return 1;
}

bool CHudSayText::Draw(float flTime)
{
	int y = Y_START;

	if ((gViewPort && !gViewPort->AllowedToPrintText()) || 0 == m_HUD_saytext->value)
		return true;

	// make sure the scrolltime is within reasonable bounds,  to guard against the clock being reset
	flScrollTime = V_min(flScrollTime, flTime + m_HUD_saytext_time->value);

	// make sure the scrolltime is within reasonable bounds,  to guard against the clock being reset
	flScrollTime = V_min(flScrollTime, flTime + m_HUD_saytext_time->value);

	if (flScrollTime <= flTime)
	{
		if ('\0' != *g_szLineBuffer[0])
		{
			flScrollTime = flTime + m_HUD_saytext_time->value;
			// push the console up
			ScrollTextUp();
		}
		else
		{ // buffer is empty,  just disable drawing of this section
			m_iFlags &= ~HUD_ACTIVE;
		}
	}

	//Set text color to con_color cvar value before drawing to ensure consistent color.
	//The engine resets this color to that value after drawing a single string.
	if (int r, g, b; sscanf(m_con_color->string, "%i %i %i", &r, &g, &b) == 3)
	{
		gEngfuncs.pfnDrawSetTextColor(r / 255.0f, g / 255.0f, b / 255.0f);
	}

	char line[MAX_CHARS_PER_LINE]{};

	for (int i = 0; i < MAX_LINES; i++)
	{
		if ('\0' != *g_szLineBuffer[i])
		{
			// ############ hu3lifezado ############ //
			// Suporte para impressao com mais cores
			//
			// 2 modos estao disponiveis:
			//
			// -- Mensagens de funcao:
			// ---- Aparecem apenas para o jogador atual e nao mostram o nome deste;
			// ---- Sao geradas por funcoes dentro do codigo do jogo.
			// -- Mensagens de chat:
			// ---- Aparecem para todos os jogadores e exibem o nome do jogador que escreveu a mensagem;
			// ---- Podem se comportar como mensagens de funcao (isso eh util para uso em certos comandos do Hu3);
			// ---- Sao geradas pela escrita dos jogadores no chat do jogo.
			//
			// A cor eh controlada pela adicao de caracteres especiais no final das mensagens:
			// -- |b
			// ---- Deixa a mensagem azul (Blue)
			// -- |r
			// ---- Deixa a mensagem vermelha (Red)
			// -- |g
			// ---- Deixa a mensagem verde (Green)
			// -- |y
			// ---- Deixa a mensagem cinza (graY) (foda-se)
			//
			// ** Se esses caracteres forem omitidos, a cor fica amarela;
			// ** Se a letra apos o "|" estiver maiuscula, a mensagem de chat sai no modo funcao.

			// draw the first x characters in the player color
			const std::size_t playerNameEndIndex = V_min(g_iNameLengths[i], MAX_PLAYER_NAME_LENGTH + 31);

			Vector color = {g_pflNameColors[i][0], g_pflNameColors[i][1], g_pflNameColors[i][2]};
			int forceNormal = 0; // 0 = Mensagem de chat; 1 = Mensagem de chat se comportando como de funcao; -1 = Mensagem de funcao
			int saytextFix = 0; // Necessario para que eu possa usar o mesmo codigo com chamadas por funcao e via chat

			// Mensagem de chat
			if (*g_szLineBuffer[i] == 2 && g_pflNameColors[i])
			{
				saytextFix = 1;
				strncpy(line, g_szLineBuffer[i] + playerNameEndIndex, strlen(g_szLineBuffer[i]));
			}
			else
			// Mensagem de funcao
			{
				strncpy(line, g_szLineBuffer[i], strlen(g_szLineBuffer[i]) + 1);
				forceNormal = -1;
			}

			// Caracter especial
			if (line[strlen(g_szLineBuffer[i] + playerNameEndIndex) - 2 - saytextFix] == '|')
			{
				// Caracter da cor
				char ch = line[strlen(g_szLineBuffer[i] + playerNameEndIndex) - 1 - saytextFix];

				// Atribuicao de cores
				if (( ch == 'b') || (ch == 'B'))
					color = g_ColorBlue;
				else if ((ch == 'r') || (ch == 'R'))
					color = g_ColorRed;
				else if ((ch == 'g') || (ch == 'G'))
					color = g_ColorGreen;
				else if ((ch == 'y') || (ch == 'Y'))
					color = g_ColorGrey;

				// Se a cor estiver com letra maiuscula, ponho a mensagem no modo funcao
				if ((ch >= 65 && ch <= 90) && forceNormal != -1)
				{
					forceNormal = 1;
					strncpy(line, g_szLineBuffer[i] + playerNameEndIndex + 2, strlen(g_szLineBuffer[i]));
				}
			}
			
			// Impressao apenas para mensagens no modo Chat
			if ((*g_szLineBuffer[i] == 2 && g_pflNameColors[i]) && ! forceNormal)
			// if (*g_szLineBuffer[i] == 2 && g_pflNameColors[i])
			{
				//Make a copy we can freely modify
				strncpy(line, g_szLineBuffer[i], sizeof(line) - 1);
				line[sizeof(line) - 1] = '\0';

				//Cut off the actual text so we can print player name
				line[playerNameEndIndex] = '\0';

				gEngfuncs.pfnDrawSetTextColor(g_pflNameColors[i][0], g_pflNameColors[i][1], g_pflNameColors[i][2]);
				const int x = DrawConsoleString(LINE_START, y, line + 1); // don't draw the control code at the start

				//Reset last character
				line[playerNameEndIndex] = g_szLineBuffer[i][playerNameEndIndex];

				// Remocao dos caracteres especiais caso estejam presentes, se nao so preciso fechar a string
				if (line[strlen(g_szLineBuffer[i]) - 3] == '|')
					line[strlen(g_szLineBuffer[i]) - 3] = '\0';

				// Aplicacao da cor
				gEngfuncs.pfnDrawSetTextColor(color[0], color[1], color[2]);

				// color is reset after each string draw
				//Print the text without player name
				DrawConsoleString(x, y, line + playerNameEndIndex);
				//DrawConsoleString(x, y, line + g_iNameLengths[i]);
			}
			// Impressao apenas para mensagens no modo Funcao
			else
			{
				// Remocao dos caracteres especiais caso estejam presentes
				if (line[strlen(g_szLineBuffer[i] + playerNameEndIndex) - 5] == '|') // Mensagem de chat no modo Funcao
					line[strlen(g_szLineBuffer[i] + playerNameEndIndex) - 5] = '\0';
				else if (line[strlen(g_szLineBuffer[i] + playerNameEndIndex) - 2] == '|') // Mensagem de Funcao
					line[strlen(g_szLineBuffer[i] + playerNameEndIndex) - 2] = '\0';
				else
					line[sizeof(line) - 1] = '\0';

				// Aplicacao da cor
				gEngfuncs.pfnDrawSetTextColor(color[0], color[1], color[2]);

				// normal draw
				DrawConsoleString(LINE_START, y, line);
			}
			// ############ //
		}

		y += line_height;
	}

	return true;
}

bool CHudSayText::MsgFunc_SayText(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int client_index = READ_BYTE(); // the client who spoke the message
	SayTextPrint(READ_STRING(), iSize - 1, client_index);

	return true;
}

void CHudSayText::SayTextPrint(const char* pszBuf, int iBufSize, int clientIndex)
{
	// Print it straight to the console
	ConsolePrint(pszBuf);

	if (gViewPort && gViewPort->AllowedToPrintText() == false)
	{
		return;
	}

	int i;
	// find an empty string slot
	for (i = 0; i < MAX_LINES; i++)
	{
		if ('\0' == *g_szLineBuffer[i])
			break;
	}
	if (i == MAX_LINES)
	{
		// force scroll buffer up
		ScrollTextUp();
		i = MAX_LINES - 1;
	}

	g_iNameLengths[i] = 0;
	g_pflNameColors[i] = NULL;

	// if it's a say message, search for the players name in the string
	if (*pszBuf == 2 && clientIndex > 0)
	{
		gEngfuncs.pfnGetPlayerInfo(clientIndex, &g_PlayerInfoList[clientIndex]);
		const char* pName = g_PlayerInfoList[clientIndex].name;

		if (pName)
		{
			const char* nameInString = strstr(pszBuf, pName);

			if (nameInString)
			{
				g_iNameLengths[i] = strlen(pName) + (nameInString - pszBuf);
				g_pflNameColors[i] = GetClientColor(clientIndex);
			}
		}
	}

	strncpy(g_szLineBuffer[i], pszBuf, MAX_CHARS_PER_LINE);

	// make sure the text fits in one line
	EnsureTextFitsInOneLineAndWrapIfHaveTo(i);

	// Set scroll time
	if (i == 0)
	{
		flScrollTime = gHUD.m_flTime + m_HUD_saytext_time->value;
	}

	m_iFlags |= HUD_ACTIVE;
	PlaySound("misc/talk.wav", 1);

	// ############ hu3lifezado ############ //
	// Mensagens de chat agoram aparecem 40 unidades acima do normal
	Y_START = ScreenHeight - 60 - 40 - (line_height * (MAX_LINES + 2));
	//Y_START = ScreenHeight - 60 - (line_height * (MAX_LINES + 2));
	// ############ //
}

void CHudSayText::EnsureTextFitsInOneLineAndWrapIfHaveTo(int line)
{
	int line_width = 0;
	GetConsoleStringSize(g_szLineBuffer[line], &line_width, &line_height);

	if ((line_width + LINE_START) > MAX_LINE_WIDTH)
	{ // string is too long to fit on line
		// scan the string until we find what word is too long,  and wrap the end of the sentence after the word
		int length = LINE_START;
		int tmp_len = 0;
		char* last_break = NULL;
		for (char* x = g_szLineBuffer[line]; *x != 0; x++)
		{
			// check for a color change, if so skip past it
			if (x[0] == '/' && x[1] == '(')
			{
				x += 2;
				// skip forward until past mode specifier
				while (*x != 0 && *x != ')')
					x++;

				if (*x != 0)
					x++;

				if (*x == 0)
					break;
			}

			char buf[2];
			buf[1] = 0;

			if (*x == ' ' && x != g_szLineBuffer[line]) // store each line break,  except for the very first character
				last_break = x;

			buf[0] = *x; // get the length of the current character
			GetConsoleStringSize(buf, &tmp_len, &line_height);
			length += tmp_len;

			if (length > MAX_LINE_WIDTH)
			{ // needs to be broken up
				if (!last_break)
					last_break = x - 1;

				x = last_break;

				// find an empty string slot
				int j;
				do
				{
					for (j = 0; j < MAX_LINES; j++)
					{
						if ('\0' == *g_szLineBuffer[j])
							break;
					}
					if (j == MAX_LINES)
					{
						// need to make more room to display text, scroll stuff up then fix the pointers
						int linesmoved = ScrollTextUp();
						line -= linesmoved;
						last_break = last_break - (sizeof(g_szLineBuffer[0]) * linesmoved);
					}
				} while (j == MAX_LINES);

				// copy remaining string into next buffer,  making sure it starts with a space character
				if ((char)*last_break == (char)' ')
				{
					int linelen = strlen(g_szLineBuffer[j]);
					int remaininglen = strlen(last_break);

					if ((linelen - remaininglen) <= MAX_CHARS_PER_LINE)
						strcat(g_szLineBuffer[j], last_break);
				}
				else
				{
					if ((strlen(g_szLineBuffer[j]) - strlen(last_break) - 2) < MAX_CHARS_PER_LINE)
					{
						strcat(g_szLineBuffer[j], " ");
						strcat(g_szLineBuffer[j], last_break);
					}
				}

				*last_break = 0; // cut off the last string

				EnsureTextFitsInOneLineAndWrapIfHaveTo(j);
				break;
			}
		}
	}
}
