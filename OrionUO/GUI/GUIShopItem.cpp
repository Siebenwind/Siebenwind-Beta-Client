﻿// MIT License
// Copyright (C) August 2016 Hotride

#include "GUIShopItem.h"
#include <SDL_timer.h>
#include "../OrionUO.h"
#include "../Managers/AnimationManager.h"
#include "../Managers/MouseManager.h"
#include "../Managers/ColorManager.h"
#include "../Managers/FontsManager.h"

CGUIShopItem::CGUIShopItem(
    int serial,
    uint16_t graphic,
    uint16_t color,
    int count,
    int price,
    const string &name,
    int x,
    int y)
    : CBaseGUI(GOT_SHOPITEM, serial, graphic, color, x, y)
    , Count(count)
    , Price(price)
    , Name(name)
{
    DEBUG_TRACE_FUNCTION;
    MoveOnDrag = true;

    CreateCountText(0);
    CreateNameText();
    UpdateOffsets();
}

CGUIShopItem::~CGUIShopItem()
{
    DEBUG_TRACE_FUNCTION;
    m_NameText.Clear();
    m_CountText.Clear();
}

void CGUIShopItem::UpdateOffsets()
{
    if (Serial >= 0x40000000)
    {
        CGLTexture *th = g_Orion.ExecuteStaticArt(Graphic);

        if (th != nullptr)
        {
            m_MaxOffset = th->Height;
        }
    }
    else
    {
        uint8_t group = 0;

        switch (g_AnimationManager.GetGroupIndex(Graphic))
        {
            case AG_LOW:
            {
                group = LAG_STAND;
                break;
            }
            case AG_HIGHT:
            {
                group = HAG_STAND;
                break;
            }
            case AG_PEOPLE:
            {
                group = PAG_STAND;
                break;
            }
            default:
                break;
        }

        ANIMATION_DIMENSIONS dims =
            g_AnimationManager.GetAnimationDimensions(0, Graphic, 1, group, false);

        if (dims.Height != 0)
        {
            m_MaxOffset = dims.Height;

            if (m_MaxOffset > 35)
            {
                m_MaxOffset = 35;
            }
        }
    }

    if (m_MaxOffset < m_NameText.Height)
    {
        m_ImageOffset = ((m_NameText.Height - m_MaxOffset) / 2);
        m_MaxOffset = m_NameText.Height;
    }
    else
    {
        m_TextOffset = ((m_MaxOffset - m_NameText.Height) / 2);
    }
}

void CGUIShopItem::OnClick()
{
    DEBUG_TRACE_FUNCTION;
    Selected = false;

    for (CBaseGUI *item = this; item != nullptr; item = (CBaseGUI *)item->m_Next)
    {
        if (item->Type == GOT_SHOPITEM && ((CGUIShopItem *)item)->Selected)
        {
            ((CGUIShopItem *)item)->Selected = false;
            ((CGUIShopItem *)item)->CreateNameText();
        }
    }

    for (CBaseGUI *item = this; item != nullptr; item = (CBaseGUI *)item->m_Prev)
    {
        if (item->Type == GOT_SHOPITEM && ((CGUIShopItem *)item)->Selected)
        {
            ((CGUIShopItem *)item)->Selected = false;
            ((CGUIShopItem *)item)->CreateNameText();
        }
    }

    Selected = true;
    CreateNameText();
}

void CGUIShopItem::CreateNameText()
{
    DEBUG_TRACE_FUNCTION;
    uint16_t textColor = 0x021F;

    if (Selected)
    {
        textColor = 0x0021;
    }

    string str = Name + " für " + std::to_string(Price) + " Dukaten";

    // Convert the name of the item from ISO-8859-1 to UTF16. This is not ideal,
    // in the long run we should make the Siebenwind Server send UTF16.
    wstring unicode = SiebenwindClient::Iso8859ToUtf16(str);
    g_FontManager.GenerateW(1, m_NameText, unicode, textColor, 30, 120);
}

void CGUIShopItem::CreateCountText(int lostCount)
{
    DEBUG_TRACE_FUNCTION;
    uint16_t textColor = 0x021F;

    if (Selected)
    {
        textColor = 0x0021;
    }

    g_FontManager.GenerateA(9, m_CountText, std::to_string(Count - lostCount), textColor);
}

void CGUIShopItem::PrepareTextures()
{
    DEBUG_TRACE_FUNCTION;

    if (Serial >= 0x40000000)
    {
        g_Orion.ExecuteStaticArt(Graphic);
    }
    else
    {
        uint8_t group = 0;

        switch (g_AnimationManager.GetGroupIndex(Graphic))
        {
            case AG_LOW:
            {
                group = LAG_STAND;
                break;
            }
            case AG_HIGHT:
            {
                group = HAG_STAND;
                break;
            }
            case AG_PEOPLE:
            {
                group = PAG_STAND;
                break;
            }
            default:
                break;
        }

        CTextureAnimationDirection &direction =
            g_AnimationManager.m_DataIndex[Graphic].m_Groups[group].m_Direction[1];
        direction.LastAccessTime = SDL_GetTicks() + 60000;
        g_AnimationManager.AnimID = Graphic;
        g_AnimationManager.AnimGroup = group;
        g_AnimationManager.Direction = 1;

        if (direction.FrameCount == 0)
        {
            g_AnimationManager.LoadDirectionGroup(direction);
        }
    }

    g_Orion.ExecuteGump(0x0039);
    g_Orion.ExecuteGump(0x003A);
    g_Orion.ExecuteGump(0x003B);
}

void CGUIShopItem::SetShaderMode()
{
    DEBUG_TRACE_FUNCTION;

    if (Color != 0)
    {
        if (PartialHue)
        {
            glUniform1iARB(g_ShaderDrawMode, SDM_PARTIAL_HUE);
        }
        else
        {
            glUniform1iARB(g_ShaderDrawMode, SDM_COLORED);
        }

        g_ColorManager.SendColorsToShader(Color);
    }
    else
    {
        glUniform1iARB(g_ShaderDrawMode, SDM_NO_COLOR);
    }
}

void CGUIShopItem::Draw(bool checktrans)
{
    DEBUG_TRACE_FUNCTION;
    CGLTexture *th = nullptr;

    glTranslatef((GLfloat)m_X, (GLfloat)m_Y, 0.0f);

    glUniform1iARB(g_ShaderDrawMode, SDM_NO_COLOR);

    m_NameText.Draw(52, m_TextOffset);
    m_CountText.Draw(196 - m_CountText.Width, (m_MaxOffset / 2) - (m_CountText.Height / 2));

    SetShaderMode();

    if (Serial >= 0x40000000)
    {
        th = g_Orion.ExecuteStaticArt(Graphic);

        if (th != nullptr)
        {
            th->Draw(2, m_ImageOffset, checktrans);
        }
    }
    else
    {
        uint8_t group = 0;

        switch (g_AnimationManager.GetGroupIndex(Graphic))
        {
            case AG_LOW:
            {
                group = LAG_STAND;
                break;
            }
            case AG_HIGHT:
            {
                group = HAG_STAND;
                break;
            }
            case AG_PEOPLE:
            {
                group = PAG_STAND;
                break;
            }
            default:
                break;
        }

        CTextureAnimationDirection &direction =
            g_AnimationManager.m_DataIndex[Graphic].m_Groups[group].m_Direction[1];

        if (direction.FrameCount != 0)
        {
            CGLTexture &originalTexture = direction.m_Frames[0];

            if (originalTexture.Texture != 0u)
            {
                CGLTexture tex;
                tex.Texture = originalTexture.Texture;

                if (originalTexture.Width > 35)
                {
                    tex.Width = 35;
                }
                else
                {
                    tex.Width = originalTexture.Width;
                }

                if (originalTexture.Height > 35)
                {
                    tex.Height = 35;
                }
                else
                {
                    tex.Height = originalTexture.Height;
                }

                g_GL.GL1_Draw(tex, 2, m_ImageOffset);

                tex.Texture = 0;

                //originalTexture.Draw(2, m_ImageOffset, checktrans);
            }
        }
    }

    glUniform1iARB(g_ShaderDrawMode, SDM_NO_COLOR);

    th = g_Orion.ExecuteGump(0x0039);

    if (th != nullptr)
    {
        th->Draw(2, m_MaxOffset, checktrans);
    }

    th = g_Orion.ExecuteGump(0x003A);

    if (th != nullptr)
    {
        th->Draw(32, m_MaxOffset, 140, 0, checktrans);
    }

    th = g_Orion.ExecuteGump(0x003B);

    if (th != nullptr)
    {
        th->Draw(166, m_MaxOffset, checktrans);
    }

    glTranslatef((GLfloat)-m_X, (GLfloat)-m_Y, 0.0f);
}

bool CGUIShopItem::Select()
{
    DEBUG_TRACE_FUNCTION;
    int x = g_MouseManager.Position.X - m_X;
    int y = g_MouseManager.Position.Y - m_Y;

    return (x >= 0 && y >= -10 && x < 200 && y < m_MaxOffset);
}
