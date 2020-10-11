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
#include "multitextfield.hpp"
#include "source.hpp"

#include "color.hpp"
#include "textfield.hpp"
#include "paint.hpp"
#include "colorname.hpp"


MultiTextField::MultiTextField(const std::string& str, int fontsize) :
	MultiTextField(str, fontsize, ColorName::TEXT800)
{}

MultiTextField::MultiTextField(const std::string& str, int fontsize,
		const Paint& color) :
	_alignment(HorizontalAlignment::LEFT),
	_style(fontsize, color),
	_text(str)
{
	init();
}

void MultiTextField::init()
{
	reset();
	add("0", new TextField(_text, _style));
	_width = _elements[0]->width();
	calculateHeight();
	place(_topleft);
}

void MultiTextField::generate(int w)
{
	if (w == 0) return init();

	reset();

	std::string leftover = _text;
	size_t line = 0;
	while (!leftover.empty() && line < 10)
	{
		std::unique_ptr<TextField> textfield(
			new TextField(leftover, _style));
		textfield->setWidth(w);
		leftover = textfield->textLeftover();
		add(std::to_string(line), std::move(textfield));
		line++;
	}
	_width = w;

	calculateHeight();
	place(_topleft);
}

void MultiTextField::added()
{}

void MultiTextField::removed()
{}

void MultiTextField::setWidth(int w)
{
	if (w < 0 && !_fixedWidth)
	{
		// This is a little trick that we can use specifically on MultiTextFields:
		// when you settle their height, they return to only one line,
		// which allows you to get their natural width.
		// In contrast, settleWidth() tries to get the width as small as possible
		// because otherwise it cannot stretch while settling everything else.
		init();
		return;
	}

	generate(w);
}

void MultiTextField::setHeight(int)
{
	LOGW << "Trying to set height of a MultiTextField.";
	DEBUG_ASSERT(false);
}

void MultiTextField::settleWidth()
{
	if (_fixedWidth) return;

	_width = std::min(_width, 200 * InterfaceElement::scale());
	generate(_width);
}

std::string MultiTextField::text() const
{
	return _text;
}

void MultiTextField::setText(const std::string& str)
{
	if (str == _text) return;

	_text = str;

	init();
}

void MultiTextField::setTextColor(const Paint& color)
{
	_style.textcolor = color;

	generate(_width);
}
