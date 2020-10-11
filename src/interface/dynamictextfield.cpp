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
#include "dynamictextfield.hpp"
#include "source.hpp"

#include "color.hpp"
#include "horizontallayout.hpp"
#include "textfield.hpp"
#include "paint.hpp"
#include "colorname.hpp"
#include "image.hpp"
#include "padding.hpp"


DynamicTextField::DynamicTextField(int fontsize) :
	_style(fontsize, Paint(ColorName::TEXT800))
{}

void DynamicTextField::init()
{
	VerticalLayout::reset();
	add("0", new HorizontalLayout());
	InterfaceElement& currentLine = (*this)["0"];
	for (size_t i = 0; i < _fragments.size(); i++)
	{
		Fragment& frag = _fragments[i];
		currentLine.add(std::to_string(i), new TextField(frag.str, frag.style));
	}
	_width = currentLine.width();
	calculateHeight();
	place(_topleft);
}

void DynamicTextField::generate(int w, bool fake)
{
	if (w == 0) return init();

	size_t oldNrLines = fake ? 0 : _elements.size();
	VerticalLayout::reset();

	int spacewidth = InterfaceElement::textW(_style, " ");
	int minwidth = InterfaceElement::textW(_style, "abcde");

	add("0", new HorizontalLayout());
	size_t line = 0;
	for (size_t i = 0; i < _fragments.size() && line < 10; i++)
	{
		Fragment& frag = _fragments[i];

		InterfaceElement& currentLine = (*this)[std::to_string(line)];
		currentLine.settleWidth();
		int space = w - currentLine.width();

		if (frag.icon != nullptr)
		{
			std::unique_ptr<InterfaceElement> icon(new Padding());
			icon->put(
				new Image(frag.icon));
			icon->setPaddingLeft(1 * InterfaceElement::scale());
			icon->setPaddingRight(1 * InterfaceElement::scale());
			icon->setColor(0, frag.style.textcolor);
			if (space >= icon->width() + spacewidth + minwidth)
			{
				currentLine.add(std::to_string(i), std::move(icon));
				currentLine.align(VerticalAlignment::MIDDLE);
			}
			else
			{
				line++;
				add(std::to_string(line), new HorizontalLayout());
				InterfaceElement& newLine = (*this)[std::to_string(line)];
				newLine.add(std::to_string(i), std::move(icon));
				newLine.align(VerticalAlignment::MIDDLE);
			}
			continue;
		}

		std::string leftover =
			fake ? frag.str : frag.str.substr(0, frag.reveal);

		if (space > spacewidth)
		{
			std::unique_ptr<TextField> textfield(
				new TextField(leftover, frag.style));
			if (textfield->width() > space) textfield->setWidth(space);
			leftover = textfield->textLeftover();
			currentLine.add(std::to_string(i), std::move(textfield));
		}

		while (!leftover.empty() && line < 10)
		{
			line++;
			add(std::to_string(line), new HorizontalLayout());
			InterfaceElement& newLine = (*this)[std::to_string(line)];
			std::unique_ptr<TextField> textfield(
				new TextField(leftover, frag.style));
			if (textfield->width() > w) textfield->setWidth(w);
			leftover = textfield->textLeftover();
			newLine.add(std::to_string(i), std::move(textfield));
		}
	}
	_width = w;

	// restore the old number of lines, to make sure that the height never
	// unexpectedly decreases
	while (++line < oldNrLines)
	{
		add(std::to_string(line), new HorizontalLayout());
		InterfaceElement& currentLine = (*this)[std::to_string(line)];
		std::unique_ptr<TextField> textfield(new TextField("", _style));
		currentLine.add("filler", std::move(textfield));
	}

	calculateHeight();
	place(_topleft);
}

void DynamicTextField::refresh()
{
	Animator::update();
	VerticalLayout::refresh();
}

void DynamicTextField::setWidth(int w)
{
	if (w < 0 && !_fixedWidth)
	{
		// This is a little trick that we can use specifically on DynamicTextFields:
		// when you settle their height, they return to only one line,
		// which allows you to get their natural width.
		// In contrast, settleWidth() tries to get the width as small as possible
		// because otherwise it cannot stretch while settling everything else.
		init();
		return;
	}

	//_elements.clear();
	generate(w, true);
}

void DynamicTextField::setHeight(int)
{
	LOGW << "Trying to set height of a DynamicTextField.";
	DEBUG_ASSERT(false);
}

void DynamicTextField::settleWidth()
{
	if (_fixedWidth) return;

	_width = std::min(_width, 200 * InterfaceElement::scale());
	generate(_width, true);
}

void DynamicTextField::settleHeight()
{
	return InterfaceElement::settleHeight();
}

std::string DynamicTextField::text() const
{
	std::string result;
	for (const Fragment& fragment : _fragments)
	{
		result += fragment.str;
	}
	return result;
}

void DynamicTextField::setText(const std::string& str)
{
	Animator::reset();
	_fragments.clear();
	_fragments.emplace_back(Fragment{str, _style, str.length(), nullptr});
	generate(_width, true);
}

void DynamicTextField::addText(const std::string& str)
{
	addText(str, _style.size, _style.textcolor);
}

void DynamicTextField::addText(const std::string& str, int fontsize,
	const Paint& color)
{
	_fragments.emplace_back(Fragment{str, {fontsize, color}, str.length(), nullptr});
	generate(_width, true);
}

void DynamicTextField::addText(const std::string& str, float revealDelay,
	float initDelay)
{
	addText(str, _style.size, _style.textcolor, revealDelay, initDelay);
}

void DynamicTextField::addText(const std::string& str, int fontsize,
	const Paint& color, float revealDelay, float initDelay)
{
	_fragments.emplace_back(Fragment{str, {fontsize, color}, 0, nullptr});
	size_t idx = _fragments.size() - 1;
	addAnimation(Animation(nullptr, [this, idx](float progress) {

		Fragment& fragment = _fragments[idx];
		size_t newReveal = progress * fragment.str.length();
		if (fragment.reveal == newReveal) return;
		fragment.reveal = newReveal;
		generate(_width, false);

	}, revealDelay * str.length(), _cumulDelay + initDelay));
	_cumulDelay += initDelay + revealDelay * str.length();
}

void DynamicTextField::addIcon(const char* spritename)
{
	addIcon(spritename, _style.textcolor);
}

void DynamicTextField::addIcon(const char* spritename,
	const Paint& color)
{
	_fragments.emplace_back(Fragment{spritename, {_style.size, color}, 0,
		spritename});
	generate(_width, true);
}
