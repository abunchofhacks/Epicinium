/**
 * Part of Epicinium
 * developed by A Bunch of Hacks.
 *
 * Copyright (c) 2017-2020 A Bunch of Hacks
 *
 * Epicinium is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Epicinium is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * [authors:]
 * Sander in 't Veld (sander@abunchofhacks.coop)
 *
 * [based on code snippet from:]
 * Gerber Yannick (https://github.com/gerberya)
 */
#include "editortheme.hpp"
#include "source.hpp"

#include "color.hpp"
#include "paint.hpp"
#include "colorname.hpp"


static Color Black				= Color(0x00, 0x00, 0x00);
static Color White				= Color(0xFF, 0xFF, 0xFF);

static uint8_t Alpha20			= 0x33;
static uint8_t Alpha40			= 0x66;
static uint8_t Alpha50			= 0x80;
static uint8_t Alpha60			= 0x99;
static uint8_t Alpha80			= 0xCC;
static uint8_t Alpha90			= 0xE6;

static inline ImVec4 vec(const Color& color)
{
	return ImVec4(
		color.r / (float) (0xFF),
		color.g / (float) (0xFF),
		color.b / (float) (0xFF),
		color.a / (float) (0xFF));
}

void EditorTheme::apply()
{
	ImVec4* colors = ImGui::GetStyle().Colors;

	colors[ImGuiCol_Text]					= vec(
		Paint(ColorName::EDITORTEXT));
	colors[ImGuiCol_TextDisabled]			= vec(
		Paint::blend(ColorName::EDITORTEXT, ColorName::DISABLEDBLEND));
	colors[ImGuiCol_WindowBg]				= vec(
		Paint(ColorName::EDITORFRAME));
	colors[ImGuiCol_ChildBg]				= vec(
		Paint::alpha(Black, Alpha20));
	colors[ImGuiCol_PopupBg]				= vec(
		Paint::alpha(ColorName::EDITORFRAME, Alpha90));
	colors[ImGuiCol_Border]					= vec(
		Paint::blend(ColorName::EDITORFRAME, ColorName::SHINEBLEND));
	colors[ImGuiCol_BorderShadow]			= vec(
		Paint::alpha(Black, Alpha20));
	colors[ImGuiCol_FrameBg]				= vec(
		Paint::alpha(ColorName::EDITORACCENT, Alpha40));
	colors[ImGuiCol_FrameBgHovered]			= vec(
		Paint::blend(
			Paint::alpha(ColorName::EDITORACCENT, Alpha40),
			ColorName::HOVEREDBLEND));
	colors[ImGuiCol_FrameBgActive]			= vec(
		Paint::blend(ColorName::EDITORACCENT, ColorName::HOVEREDBLEND, 0.5f));
	colors[ImGuiCol_TitleBg]				= vec(
		Paint::alpha(ColorName::EDITORFRAME, Alpha80));
	colors[ImGuiCol_TitleBgActive]			= vec(
		Paint::blend(ColorName::EDITORFRAME,
			ColorName::HOVEREDBLEND, 0.5f));
	colors[ImGuiCol_TitleBgCollapsed]		= vec(
		Paint::blend(
			Paint::alpha(ColorName::EDITORFRAME, Alpha80),
			ColorName::PRESSEDBLEND));
	colors[ImGuiCol_MenuBarBg]				= vec(
		Paint::blend(ColorName::EDITORFRAME, ColorName::SHADEBLEND));
	colors[ImGuiCol_ScrollbarBg]			= vec(
		Paint::blend(
			Paint::alpha(ColorName::EDITORFRAME, Alpha50),
			ColorName::SHINEBLEND));
	colors[ImGuiCol_ScrollbarGrab]			= vec(
		Paint::blend(ColorName::EDITORFRAME, ColorName::SHINEBLEND));
	colors[ImGuiCol_ScrollbarGrabHovered]	= vec(
		Paint::blend(
			Paint::blend(ColorName::EDITORFRAME, ColorName::SHINEBLEND),
			ColorName::HOVEREDBLEND));
	colors[ImGuiCol_ScrollbarGrabActive]	= vec(
		Paint::blend(
			Paint::blend(ColorName::EDITORFRAME, ColorName::SHINEBLEND),
			ColorName::HOVEREDBLEND, 0.5f));
	colors[ImGuiCol_CheckMark]				= vec(
		Paint(ColorName::EDITORHIGHLIGHT));
	colors[ImGuiCol_SliderGrab]				= vec(
		Paint(ColorName::EDITORHIGHLIGHT));
	colors[ImGuiCol_SliderGrabActive]		= vec(
		Paint::blend(ColorName::EDITORHIGHLIGHT,
			ColorName::HOVEREDBLEND, 0.5f));
	colors[ImGuiCol_Button]					= vec(
		Paint::alpha(ColorName::EDITORMAIN, Alpha80));
	colors[ImGuiCol_ButtonHovered]			= vec(
		Paint::blend(
			Paint::alpha(ColorName::EDITORMAIN, Alpha80),
			ColorName::HOVEREDBLEND));
	colors[ImGuiCol_ButtonActive]			= vec(
		Paint::blend(ColorName::EDITORMAIN, ColorName::HOVEREDBLEND, 0.5f));
	colors[ImGuiCol_Header]					= vec(
		Paint::alpha(ColorName::EDITORACCENT, Alpha80));
	colors[ImGuiCol_HeaderHovered]			= vec(
		Paint::blend(
			Paint::alpha(ColorName::EDITORACCENT, Alpha80),
			ColorName::HOVEREDBLEND));
	colors[ImGuiCol_HeaderActive]			= vec(
		Paint::blend(ColorName::EDITORACCENT, ColorName::HOVEREDBLEND, 0.5f));
	colors[ImGuiCol_Separator]				= vec(
		Paint::blend(ColorName::EDITORFRAME, ColorName::SHINEBLEND));
	colors[ImGuiCol_SeparatorHovered]		= vec(
		Paint::blend(
			Paint::blend(ColorName::EDITORFRAME, ColorName::SHINEBLEND),
			ColorName::HOVEREDBLEND));
	colors[ImGuiCol_SeparatorActive]		= vec(
		Paint::blend(
			Paint::blend(ColorName::EDITORFRAME, ColorName::SHINEBLEND),
			ColorName::HOVEREDBLEND, 0.5f));
	colors[ImGuiCol_ResizeGrip]				= vec(
		Paint::alpha(ColorName::EDITORMAIN, Alpha20));
	colors[ImGuiCol_ResizeGripHovered]		= vec(
		Paint::blend(
			Paint::alpha(ColorName::EDITORMAIN, Alpha20),
			ColorName::HOVEREDBLEND));
	colors[ImGuiCol_ResizeGripActive]		= vec(
		Paint::blend(ColorName::EDITORMAIN, ColorName::HOVEREDBLEND, 0.5f));
	colors[ImGuiCol_Tab]					= vec(
		Paint::alpha(ColorName::EDITORMAIN, Alpha60));
	colors[ImGuiCol_TabHovered]				= vec(
		Paint::blend(
			Paint::alpha(ColorName::EDITORMAIN, Alpha60),
			ColorName::HOVEREDBLEND));
	colors[ImGuiCol_TabActive]				= vec(
		Paint::blend(ColorName::EDITORMAIN, ColorName::HOVEREDBLEND, 0.5f));
	colors[ImGuiCol_TabUnfocused]			= vec(
		Paint::alpha(ColorName::EDITORMAIN, Alpha60));
	colors[ImGuiCol_TabUnfocusedActive]		= vec(
		Paint::blend(ColorName::EDITORMAIN, ColorName::HOVEREDBLEND, 0.5f));
	colors[ImGuiCol_PlotLines]				= vec(
		Paint(ColorName::EDITORHIGHLIGHT));
	colors[ImGuiCol_PlotLinesHovered]		= vec(
		Paint::blend(ColorName::EDITORHIGHLIGHT, ColorName::HOVEREDBLEND));
	colors[ImGuiCol_PlotHistogram]			= vec(
		Paint(ColorName::EDITORHIGHLIGHT));
	colors[ImGuiCol_PlotHistogramHovered]	= vec(
		Paint::blend(ColorName::EDITORHIGHLIGHT, ColorName::HOVEREDBLEND));
	colors[ImGuiCol_TextSelectedBg]			= vec(
		Paint::alpha(ColorName::EDITORHIGHLIGHT, Alpha40));
	colors[ImGuiCol_DragDropTarget]			= vec(
		Paint::alpha(ColorName::EDITORHIGHLIGHT, Alpha80));
	colors[ImGuiCol_NavHighlight]			= vec(White);
	colors[ImGuiCol_NavWindowingHighlight]	= vec(Paint::alpha(White, Alpha80));
	colors[ImGuiCol_NavWindowingDimBg]		= vec(Paint::alpha(White, Alpha20));
	colors[ImGuiCol_ModalWindowDimBg]		= vec(Paint::alpha(Black, Alpha60));
}
