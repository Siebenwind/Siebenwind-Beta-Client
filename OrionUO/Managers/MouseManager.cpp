﻿// MIT License
// Copyright (C) August 2016 Hotride

#include "MouseManager.h"
#include "ColorManager.h"
#include "MapManager.h"
#include "ConfigManager.h"
#include "CustomHousesManager.h"
#include "../OrionUO.h"
#include "../ToolTip.h"
#include "../Target.h"
#include "../PressedObject.h"
#include "../Profiler.h"
#include "../SelectedObject.h"
#include "../GameObjects/ObjectOnCursor.h"
#include "../GameObjects/GamePlayer.h"
#include "../Gumps/GumpCustomHouse.h"
#include "../Walker/PathFinder.h"

CMouseManager g_MouseManager;

uint16_t g_CursorData[2][16] = { { 0x206A,
                                   0x206B,
                                   0x206C,
                                   0x206D,
                                   0x206E,
                                   0x206F,
                                   0x2070,
                                   0x2071,
                                   0x2072,
                                   0x2073,
                                   0x2074,
                                   0x2075,
                                   0x2076,
                                   0x2077,
                                   0x2078,
                                   0x2079 },
                                 { 0x2053,
                                   0x2054,
                                   0x2055,
                                   0x2056,
                                   0x2057,
                                   0x2058,
                                   0x2059,
                                   0x205A,
                                   0x205B,
                                   0x205C,
                                   0x205D,
                                   0x205E,
                                   0x205F,
                                   0x2060,
                                   0x2061,
                                   0x2062 } };

int CMouseManager::Sgn(int val)
{
    return static_cast<int>(0 < val) - static_cast<int>(val < 0);
}

int CMouseManager::GetFacing(int x1, int y1, int to_x, int to_y, int current_facing)
{
    DEBUG_TRACE_FUNCTION;
    int shiftX = to_x - x1;
    int shiftY = to_y - y1;

    int hashf = 100 * (Sgn(shiftX) + 2) + 10 * (Sgn(shiftY) + 2);

    if ((shiftX != 0) && (shiftY != 0))
    {
        shiftX = std::abs(shiftX);
        shiftY = std::abs(shiftY);

        if ((shiftY * 5) <= (shiftX * 2))
        {
            hashf = hashf + 1;
        }
        else if ((shiftY * 2) >= (shiftX * 5))
        {
            hashf = hashf + 3;
        }
        else
        {
            hashf = hashf + 2;
        }
    }
    else if (shiftX == 0)
    {
        if (shiftY == 0)
        {
            return current_facing;
        }
    }

    switch (hashf)
    {
        case 111:
            return DT_W; // W
        case 112:
            return DT_NW; // NW
        case 113:
            return DT_N; // N
        case 120:
            return DT_W; // W
        case 131:
            return DT_W; // W
        case 132:
            return DT_SW; // SW
        case 133:
            return DT_S; // S
        case 210:
            return DT_N; // N
        case 230:
            return DT_S; // S
        case 311:
            return DT_E; // E
        case 312:
            return DT_NE; // NE
        case 313:
            return DT_N; // N
        case 320:
            return DT_E; // E
        case 331:
            return DT_E; // E
        case 332:
            return DT_SE; // SE
        case 333:
            return DT_S; // S
        default:
            break;
    }

    return current_facing;
}

uint16_t CMouseManager::GetGameCursor()
{
    DEBUG_TRACE_FUNCTION;
    int war = (int)(g_Player != nullptr && g_Player->Warmode);
    uint16_t result = g_CursorData[war][9]; //Main Gump mouse cursor

    if (g_Target.IsTargeting() && !g_ObjectInHand.Enabled)
    {
        return g_CursorData[war][12]; //Targetting cursor
    }

    bool mouseInWindow =
        !(Position.X < g_ConfigManager.GameWindowX || Position.Y < g_ConfigManager.GameWindowY ||
          Position.X > (g_ConfigManager.GameWindowX + g_ConfigManager.GameWindowWidth) ||
          Position.Y > (g_ConfigManager.GameWindowY + g_ConfigManager.GameWindowHeight));

    //bool gumpChecked = (g_LastSelectedGump || (g_LastSelectedObject && g_LastObjectType != SOT_GAME_OBJECT && g_LastObjectType != SOT_STATIC_OBJECT && g_LastObjectType != SOT_LAND_OBJECT && g_LastObjectType != SOT_TEXT_OBJECT));

    if (!mouseInWindow || g_SelectedObject.Gump != nullptr || g_PressedObject.LeftGump != nullptr)
    {
        return result;
    }

    int gameWindowCenterX = g_ConfigManager.GameWindowX + (g_ConfigManager.GameWindowWidth / 2);
    int gameWindowCenterY = g_ConfigManager.GameWindowY + (g_ConfigManager.GameWindowHeight / 2);

    return g_CursorData[war]
                       [GetFacing(gameWindowCenterX, gameWindowCenterY, Position.X, Position.Y, 1)];
}

void CMouseManager::ProcessWalking()
{
    PROFILER_EVENT();
    DEBUG_TRACE_FUNCTION;
    bool mouseInWindow = true;

    if (Position.X < g_ConfigManager.GameWindowX || Position.Y < g_ConfigManager.GameWindowY ||
        Position.X > (g_ConfigManager.GameWindowX + g_ConfigManager.GameWindowWidth) ||
        Position.Y > (g_ConfigManager.GameWindowY + g_ConfigManager.GameWindowHeight))
    {
        mouseInWindow = false;
    }

    if ((g_MovingFromMouse || (mouseInWindow && g_AutoMoving)) &&
        g_PressedObject.RightGump == nullptr &&
        !((g_ShiftPressed && !g_CtrlPressed && !g_AltPressed) &&
          g_ConfigManager.HoldShiftForEnablePathfind && g_ConfigManager.EnablePathfind) &&
        !(g_SelectedGameObjectHandle == g_PressedObject.RightSerial &&
          g_PressedObject.RightObject != nullptr && g_PressedObject.RightObject->IsGameObject()))
    {
        int gameWindowCenterX = g_ConfigManager.GameWindowX + (g_ConfigManager.GameWindowWidth / 2);
        int gameWindowCenterY =
            g_ConfigManager.GameWindowY + (g_ConfigManager.GameWindowHeight / 2);

        int facing = GetFacing(gameWindowCenterX, gameWindowCenterY, Position.X, Position.Y, 1);

        float mouse_range = std::hypotf(
            (float)(gameWindowCenterX - Position.X), (float)(gameWindowCenterY - Position.Y));

        int dir = facing;

        if (dir == 0)
        {
            dir = 8;
        }

        bool run = (mouse_range >= 190.0f);

        if (!g_PathFinder.AutoWalking)
        {
            g_PathFinder.Walk(run, dir - 1);
        }
    }
}

bool CMouseManager::LoadCursorTextures()
{
    DEBUG_TRACE_FUNCTION;
    bool result = true;

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            uint16_t id = g_CursorData[i][j];

            CGLTexture *pth = g_Orion.ExecuteStaticArt(id);

            if (i == 0)
            {
                if (pth != nullptr)
                {
                    float OffsX = 0.0f;
                    float OffsY = 0.0f;

                    float DW = (float)pth->Width;
                    float DH = (float)pth->Height;

                    if (id == 0x206A)
                    {
                        OffsX = -4.0f;
                    }
                    else if (id == 0x206B)
                    {
                        OffsX = -DW + 3.0f;
                    }
                    else if (id == 0x206C)
                    {
                        OffsX = -DW + 3.0f;
                        OffsY = -(DH / 2.0f);
                    }
                    else if (id == 0x206D)
                    {
                        OffsX = -DW;
                        OffsY = -DH;
                    }
                    else if (id == 0x206E)
                    {
                        OffsX = -(DW * 0.66f);
                        OffsY = -DH;
                    }
                    else if (id == 0x206F)
                    {
                        OffsY = ((-DH) + 4.0f);
                    }
                    else if (id == 0x2070)
                    {
                        OffsY = ((-DH) + 4.0f);
                    }
                    else if (id == 0x2075)
                    {
                        OffsY = -4.0f;
                    }
                    else if (id == 0x2076)
                    {
                        OffsX = -12.0f;
                        OffsY = -14.0f;
                    }
                    else if (id == 0x2077)
                    {
                        OffsX = -(DW / 2.0f);
                        OffsY = -(DH / 2.0f);
                    }
                    else if (id == 0x2078)
                    {
                        OffsY = -(DH * 0.66f);
                    }
                    else if (id == 0x2079)
                    {
                        OffsY = -(DH / 2.0f);
                    }

                    switch (id)
                    {
                        case 0x206B:
                            OffsX = -29;
                            OffsY = -1;
                            break;
                        case 0x206C:
                            OffsX = -41;
                            OffsY = -9;
                            break;
                        case 0x206D:
                            OffsX = -36;
                            OffsY = -25;
                            break;
                        case 0x206E:
                            OffsX = -14;
                            OffsY = -33;
                            break;
                        case 0x206F:
                            OffsX = -2;
                            OffsY = -26;
                            break;
                        case 0x2070:
                            OffsX = -3;
                            OffsY = -8;
                            break;
                        case 0x2071:
                            OffsX = -1;
                            OffsY = -1;
                            break;
                        case 0x206A:
                            OffsX = -4;
                            OffsY = -2;
                            break;
                        case 0x2075:
                            OffsX = -2;
                            OffsY = -10;
                            break;
                        default:
                            break;
                    }

                    m_CursorOffset[0][j] = (int)OffsX; //X
                    m_CursorOffset[1][j] = (int)OffsY; //Y
                }
                else
                {
                    m_CursorOffset[0][j] = 0; //X
                    m_CursorOffset[1][j] = 0; //Y
                }
            }
        }
    }

    return result;
}

void CMouseManager::Draw(uint16_t id)
{
    PROFILER_EVENT();
    DEBUG_TRACE_FUNCTION;
    if (g_GameState >= GS_GAME)
    {
        if (g_CustomHouseGump != nullptr && (g_CustomHouseGump->SelectedGraphic != 0u))
        {
            uint16_t color = 0;

            vector<CBuildObject> list;
            CUSTOM_HOUSE_BUILD_TYPE type;

            if (!g_CustomHouseGump->CanBuildHere(
                    list, (CRenderWorldObject *)g_SelectedObject.Object, type))
            {
                color = 0x0021;
            }

            if (color != 0)
            {
                g_ColorizerShader.Use();
            }

            if (static_cast<unsigned int>(!list.empty()) != 0u)
            {
                for (const CBuildObject &item : list)
                {
                    int x = g_MouseManager.Position.X + (item.X - item.Y) * 22;
                    int y = g_MouseManager.Position.Y + (item.X + item.Y) * 22 - (item.Z * 4);

                    g_Orion.DrawStaticArt(item.Graphic, color, x, y, false);
                }
            }
            else
            {
                g_Orion.DrawStaticArtInContainer(
                    g_CustomHouseGump->SelectedGraphic,
                    color,
                    g_MouseManager.Position.X,
                    g_MouseManager.Position.Y,
                    false,
                    true);
            }

            if (color != 0)
            {
                UnuseShader();
            }
        }
        else if (g_ObjectInHand.Enabled)
        {
            bool doubleDraw = false;
            uint16_t ohGraphic = g_ObjectInHand.GetDrawGraphic(doubleDraw);

            uint16_t ohColor = g_ObjectInHand.Color;
            doubleDraw =
                ((CGameObject::IsGold(g_ObjectInHand.Graphic) == 0) &&
                 IsStackable(g_ObjectInHand.TiledataPtr->Flags) && g_ObjectInHand.Count > 1);

            if (ohColor != 0)
            {
                g_ColorizerShader.Use();
            }

            if (g_ObjectInHand.IsGameFigure)
            {
                ohGraphic -= GAME_FIGURE_GUMP_OFFSET;

                CGLTexture *to = g_Orion.ExecuteGump(ohGraphic);

                if (to != nullptr)
                {
                    g_Orion.DrawGump(
                        ohGraphic,
                        ohColor,
                        g_MouseManager.Position.X - (to->Width / 2),
                        g_MouseManager.Position.Y - (to->Height / 2));
                }
            }
            else
            {
                g_Orion.DrawStaticArtInContainer(
                    ohGraphic,
                    ohColor,
                    g_MouseManager.Position.X,
                    g_MouseManager.Position.Y,
                    false,
                    true);

                if (doubleDraw)
                {
                    g_Orion.DrawStaticArtInContainer(
                        ohGraphic,
                        ohColor,
                        g_MouseManager.Position.X + 5,
                        g_MouseManager.Position.Y + 5,
                        false,
                        true);
                }
            }

            if (ohColor != 0)
            {
                UnuseShader();
            }
        }
    }

    CGLTexture *th = g_Orion.ExecuteStaticArt(id);

    if (th != nullptr)
    {
        uint16_t color = 0;

        if (id < 0x206A)
        {
            id -= 0x2053;
        }
        else
        {
            id -= 0x206A;

            if (g_GameState >= GS_GAME && (g_MapManager.GetActualMap() != 0))
            {
                color = 0x0033;
            }
        }

        if (id < 16)
        {
            g_ToolTip.Draw(th->Width, th->Height);

            int x = Position.X + m_CursorOffset[0][id];
            int y = Position.Y + m_CursorOffset[1][id];

            if (color != 0u)
            {
                g_ColorizerShader.Use();

                g_ColorManager.SendColorsToShader(color);

                glUniform1iARB(g_ShaderDrawMode, SDM_COLORED);
            }

            th->Draw(x, y);

            if (color != 0u)
            {
                UnuseShader();
            }

            if (g_Target.Targeting && g_ConfigManager.HighlightTargetByType)
            {
                uint32_t auraColor = 0;

                if (g_Target.CursorType == 0)
                {
                    auraColor = g_ColorManager.GetPolygoneColor(16, 0x03B2);
                }
                else if (g_Target.CursorType == 1)
                {
                    auraColor = g_ColorManager.GetPolygoneColor(16, 0x0023);
                }
                else if (g_Target.CursorType == 2)
                {
                    auraColor = g_ColorManager.GetPolygoneColor(16, 0x005A);
                }

                if (auraColor != 0u)
                {
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

                    glColor4ub(ToColorR(auraColor), ToColorG(auraColor), ToColorB(auraColor), 0xFF);

                    glUniform1iARB(g_ShaderDrawMode, SDM_NO_COLOR);

                    CGLTexture tex;
                    tex.Texture = g_AuraTexture.Texture;
                    tex.Width = 35;
                    tex.Height = 35;

                    g_GL.GL1_Draw(tex, x - 6, y - 2);

                    tex.Texture = 0;

                    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                    glDisable(GL_BLEND);
                }
            }
        }
    }
}
