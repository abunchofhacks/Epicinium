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
#include "skineditor.hpp"
#include "source.hpp"

#include "skin.hpp"
#include "paint.hpp"
#include "parseerror.hpp"


void SkinEditor::updateIfEnabled()
{
	if (Input::get()->isDebugKeyHeld())
	{
		if (ImGui::Begin("Windows", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Checkbox("Skin Editor", &_show);
		}
		ImGui::End();
	}

	if (_show)
	{
		updatePalettes();
		updateSkins();
	}
}

void SkinEditor::updatePalettes()
{
	if (ImGui::Begin("Master Palette"))
	{
		if (ImGui::BeginChild("indexed master palettes", ImVec2(0, 100)))
		{
			for (const std::string& name : Palette::indexedNames())
			{
				if (ImGui::Selectable(name.c_str()))
				{
					_activepalettename = name;
					Palette::install(Palette::load(_activepalettename));
				}
			}
		}
		ImGui::EndChild();

		ImGui::Separator();

		static ImVec4 headercolor(0.2f, 1.0f, 0.4f, 1.0f);

		ImGui::InputText("Name", &_activepalettename);

		if (ImGui::Button("Load"))
		{
			Palette::install(Palette::load(_activepalettename));
		}
		ImGui::SameLine();
		if (ImGui::Button("Save"))
		{
			Palette::saveInstalledAs(_activepalettename);
		}
		ImGui::SameLine();
		if (_activepalettename == "default")
		{
			ImGui::Text("(default)");
		}
		else
		{
			if (ImGui::Button("Add to list"))
			{
				Palette::addToIndex(_activepalettename);
				Palette::saveIndex();
			}
			ImGui::SameLine();
			if (ImGui::Button("Set as default"))
			{
				Palette::saveInstalledAs("default");
			}
		}

		static ImGuiTextFilter filter;
		filter.Draw();

		if (ImGui::BeginChild("master palette colors"))
		{
			for (size_t i = 0; i < COLORNAME_SIZE; i++)
			{
				ColorName colorname = (ColorName) i;
				if (!filter.PassFilter(::stringify(colorname))) continue;

				Color color = Palette::get(colorname);
				ImVec4 editable = ImColor(color.r, color.g, color.b, color.a);
				ImGui::ColorEdit4(::stringify(colorname), (float*)&editable);
				Color newcolor = Color(int(255 * editable.x + 0.5f),
					int(255 * editable.y + 0.5f),
					int(255 * editable.z + 0.5f),
					int(255 * editable.w + 0.5f));
				if (newcolor != color)
				{
					Palette::set(colorname, newcolor);
				}
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void SkinEditor::updateSkins()
{
	if (ImGui::Begin("Skins"))
	{
		static ImGuiTextFilter filter;
		filter.Draw();

		static const char* SKINS[] = {
			"figures/rifleman",
			"figures/militia",
			"figures/sapper",
			"figures/gunner",
			"figures/settler",
			"figures/tank",
			"figures/zeppelin",
			"figures/city",
			"figures/industry",
			"figures/barracks",
			"figures/airfield",
			"figures/town",
			"figures/settlement",
			"figures/outpost",
			"figures/farm",
			"figures/fence",
			"figures/mountain",
			"figures/birch",
			"figures/oak",
			"figures/palm",
			"figures/poplar",
			"figures/spruce",
			"figures/ridge",
			"surfaces/airfield",
			"surfaces/barracks",
			"surfaces/border",
			"surfaces/city",
			"surfaces/crops",
			"surfaces/desert",
			"surfaces/dirt",
			"surfaces/farm",
			"surfaces/forest",
			"surfaces/grass",
			"surfaces/home",
			"surfaces/industry",
			"surfaces/mountain",
			"surfaces/outpost",
			"surfaces/rubble",
			"surfaces/settlement",
			"surfaces/soil",
			"surfaces/stone",
			"surfaces/town",
			"surfaces/trenches",
			"surfaces/water",
			"surfaces/ridge",
			"ui/phase_graphic",
		};
		for (size_t i = 0; i < array_size(SKINS); i++)
		{
			const char* skinname = SKINS[i];
			if (!filter.PassFilter(skinname))
			{
				continue;
			}
			if (ImGui::Selectable(skinname))
			{
				_activeskin = Skin::load(skinname);
				_activeskinname = skinname;
			}
		}
	}
	ImGui::End();

	if (_activeskin)
	{
		updateActiveSkin(*_activeskin);
	}
}

void SkinEditor::updateActiveSkin(Skin& skin)
{
	if (ImGui::Begin("Edit Skin"))
	{
		static ImVec4 headercolor(0.2f, 1.0f, 0.4f, 1.0f);
		ImGui::TextColored(headercolor, "%s", _activeskinname.c_str());

		if (skin.name != _activeskinname)
		{
			ImGui::Text("Linked to: ");
			ImGui::SameLine();
			if (ImGui::Selectable(skin.name.c_str()))
			{
				_activeskin = Skin::load(skin.name);
				_activeskinname = skin.name;
			}

			if (ImGui::Button("Unlink"))
			{
				_activeskin = skin.clone(_activeskinname);
			}

			ImGui::End();
			return;
		}

		if (ImGui::Button("Load"))
		{
			// This changes `skin`.
			_activeskin = Skin::load(skin.name);
			_activeskinname = skin.name;
		}
		ImGui::SameLine();
		if (ImGui::Button("Save"))
		{
			skin.save(skin.name);
		}

		// TODO clone skin

		// TODO edit skin properties other than colors

		for (size_t p = 0; p < skin.palettenames.size(); p++)
		{
			if (ImGui::TreeNode(skin.palettenames[p].c_str()))
			{
				for (int i = 0; i < skin.palettesize; i++)
				{
					std::string itext = std::to_string(i + 1);
					itext.insert(itext.begin(), 2 - itext.size(), ' ');
					Paint& paint = skin.palettes[p][i];
					std::string oldtext = (paint)
						? paint.toJson().asString()
						: std::string("");
					std::string newtext = oldtext;
					ImGui::InputTextWithHint(itext.c_str(),
						"(not used)", &newtext);
					Color color;
					std::string errortext;
					if (newtext != oldtext)
					{
						try
						{
							Json::Value json = newtext.empty()
								? Json::nullValue
								: Json::Value(newtext);
							paint = Paint::parse(json);
							color = (Color) paint;
						}
						catch (const ParseError& error)
						{
							errortext = error.what();
						}
						catch (const std::exception& error)
						{
							LOGE << "Unhandled error: " << error.what();
							errortext = error.what();
						}
					}
					else
					{
						color = (Color) paint;
					}
					if (color)
					{
						ImGui::SameLine();
						ImGui::ColorButton(color.toString().c_str(),
							ImColor(color.r, color.g, color.b, color.a));
					}

					if (!errortext.empty())
					{
						static ImVec4 invalidcolor(1.0f, 0.4f, 0.2f, 1.0f);
						ImGui::TextColored(invalidcolor, "%s",
							errortext.c_str());
					}
				}
				ImGui::TreePop();
			}
		}

		int maxmaxused = -1;
		for (size_t p = 0; p < skin.palettenames.size(); p++)
		{
			int maxused = -1;
			for (int i = 0; i < skin.palettesize; i++)
			{
				if (skin.palettes[p][i])
				{
					maxused = i;
				}
			}
			if (maxused > maxmaxused)
			{
				maxmaxused = maxused;
			}
		}
		// TODO load sprite and compare with Sprite::paletteSize()
		if (maxmaxused > 0 && maxmaxused + 1 < skin.palettesize)
		{
			if (ImGui::Button("Trim palette to remove unused colors"))
			{
				// Lower the palettesize but do NOT realign the palettedata,
				// because Figures and Surfaces have pointers with cached size.
				skin.palettesize = maxmaxused + 1;
			}
		}
	}
	ImGui::End();
}
