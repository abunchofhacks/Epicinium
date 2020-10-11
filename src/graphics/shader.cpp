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
 * Daan Mulder (daan@abunchofhacks.coop)
 */
#include "shader.hpp"
#include "source.hpp"




const char* getShaderName(Shader shader)
{
	switch (shader)
	{
		case Shader::STANDARD:         return "Standard";
		case Shader::PICTURE:          return "Picture";
		case Shader::TEXT:             return "Text";
		case Shader::GRAYSCALE:        return "Grayscale";
		case Shader::PURESEPIA:        return "Pure Sepia";
		case Shader::SEPIA:            return "Mild Sepia";
		case Shader::GAMEBOY:          return "Gameboy";
		case Shader::PROTANOPIA:       return "Protanopia";
		case Shader::DEUTERANOPIA:     return "Deuteranopia";
		case Shader::TRITANOPIA:       return "Tritanopia";
		case Shader::PROTANOMALY:      return "Protanomaly";
		case Shader::DEUTERANOMALY:    return "Deuteranomaly";
		case Shader::TRITANOMALY:      return "Tritanomaly";
		case Shader::BUILDUP:          return "Buildup";
		case Shader::COMPRESSED:       return "Compressed";
	}

	return nullptr;
}

const char* getShaderSource(Shader shader)
{
	switch (shader)
	{
		case Shader::STANDARD:
		{
			return
			#include "shader_head.glsl"
			"void main()"
			"{"
			#include "shader_mask.glsl"
			#include "shader_color_sprite.glsl"
			#include "shader_body.glsl"
			#include "shader_end.glsl"
			"}"
			;
		}
		break;

		case Shader::PICTURE:
		{
			return
			#include "shader_head.glsl"
			"void main()"
			"{"
			#include "shader_mask.glsl"
			#include "shader_color_picture.glsl"
			#include "shader_body.glsl"
			#include "shader_end.glsl"
			"}"
			;
		}
		break;

		case Shader::TEXT:
		{
			return
			#include "shader_text.glsl"
			;
		}
		break;

		case Shader::GRAYSCALE:
		{
			return
			#include "shader_head.glsl"
			"void main()"
			"{"
			#include "shader_mask.glsl"
			#include "shader_color_sprite.glsl"
			#include "shader_body.glsl"
			#include "shader_modifier_grayscale.glsl"
			#include "shader_end.glsl"
			"}"
			;
		}
		break;

		case Shader::PURESEPIA:
		{
			return
			#include "shader_head.glsl"
			"void main()"
			"{"
			#include "shader_mask.glsl"
			#include "shader_color_sprite.glsl"
			#include "shader_body.glsl"
			#include "shader_modifier_puresepia.glsl"
			#include "shader_end.glsl"
			"}"
			;
		}
		break;

		case Shader::SEPIA:
		{
			return
			#include "shader_head.glsl"
			"void main()"
			"{"
			#include "shader_mask.glsl"
			#include "shader_color_sprite.glsl"
			#include "shader_body.glsl"
			#include "shader_modifier_sepia.glsl"
			#include "shader_end.glsl"
			"}"
			;
		}
		break;

		case Shader::GAMEBOY:
		{
			return
			#include "shader_head.glsl"
			"void main()"
			"{"
			#include "shader_mask.glsl"
			#include "shader_color_sprite.glsl"
			#include "shader_body.glsl"
			#include "shader_modifier_gameboy.glsl"
			#include "shader_end.glsl"
			"}"
			;
		}
		break;

		case Shader::PROTANOPIA:
		{
			return
			#include "shader_head.glsl"
			"void main()"
			"{"
			#include "shader_mask.glsl"
			#include "shader_color_sprite.glsl"
			#include "shader_body.glsl"
			#include "shader_modifier_protanopia.glsl"
			#include "shader_end.glsl"
			"}"
			;
		}
		break;

		case Shader::DEUTERANOPIA:
		{
			return
			#include "shader_head.glsl"
			"void main()"
			"{"
			#include "shader_mask.glsl"
			#include "shader_color_sprite.glsl"
			#include "shader_body.glsl"
			#include "shader_modifier_deuteranopia.glsl"
			#include "shader_end.glsl"
			"}"
			;
		}
		break;

		case Shader::TRITANOPIA:
		{
			return
			#include "shader_head.glsl"
			"void main()"
			"{"
			#include "shader_mask.glsl"
			#include "shader_color_sprite.glsl"
			#include "shader_body.glsl"
			#include "shader_modifier_tritanopia.glsl"
			#include "shader_end.glsl"
			"}"
			;
		}
		break;

		case Shader::PROTANOMALY:
		{
			return
			#include "shader_head.glsl"
			"void main()"
			"{"
			#include "shader_mask.glsl"
			#include "shader_color_sprite.glsl"
			#include "shader_body.glsl"
			#include "shader_modifier_protanomaly.glsl"
			#include "shader_end.glsl"
			"}"
			;
		}
		break;

		case Shader::DEUTERANOMALY:
		{
			return
			#include "shader_head.glsl"
			"void main()"
			"{"
			#include "shader_mask.glsl"
			#include "shader_color_sprite.glsl"
			#include "shader_body.glsl"
			#include "shader_modifier_deuteranomaly.glsl"
			#include "shader_end.glsl"
			"}"
			;
		}
		break;

		case Shader::TRITANOMALY:
		{
			return
			#include "shader_head.glsl"
			"void main()"
			"{"
			#include "shader_mask.glsl"
			#include "shader_color_sprite.glsl"
			#include "shader_body.glsl"
			#include "shader_modifier_tritanomaly.glsl"
			#include "shader_end.glsl"
			"}"
			;
		}
		break;

		case Shader::BUILDUP:
		{
			return
			#include "shader_head.glsl"
			"void main()"
			"{"
			#include "shader_mask.glsl"
			#include "shader_color_sprite.glsl"
			#include "shader_body.glsl"
			#include "shader_modifier_buildup.glsl"
			#include "shader_end.glsl"
			"}"
			;
		}
		break;

		case Shader::COMPRESSED:
		{
			return
			#include "shader_head.glsl"
			"void main()"
			"{"
			#include "shader_mask.glsl"
			#include "shader_color_sprite.glsl"
			#include "shader_body.glsl"
			#include "shader_modifier_compressed.glsl"
			#include "shader_end.glsl"
			"}"
			;
		}
		break;
	}

	return nullptr;
}
