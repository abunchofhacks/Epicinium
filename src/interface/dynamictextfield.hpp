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
#pragma once
#include "header.hpp"

#include "verticallayout.hpp"
#include "textstyle.hpp"
#include "animator.hpp"

struct Color;


class DynamicTextField : public VerticalLayout, private Animator
{
private:
	struct Fragment
	{
		std::string str;
		TextStyle style;
		size_t reveal;
		const char* icon;
	};

public:
	DynamicTextField(int fontsize);

private:
	TextStyle _style;
	HorizontalAlignment _alignment;
	std::vector<Fragment> _fragments;
	float _cumulDelay = 0;

	void init();
	void generate(int w, bool fake);

public:
	virtual void refresh() override;

	virtual void setWidth(int w) override;
	virtual void setHeight(int h) override;
	virtual bool resizableWidth() override { return !_fixedWidth; }
	virtual bool resizableHeight() override { return false; }
	virtual void settleWidth() override;
	virtual void settleHeight() override;

	virtual std::string text() const override;
	virtual void setText(const std::string& str) override;
	virtual void addText(const std::string& str) override;
	virtual void addText(const std::string& str, int fontsize,
		const Paint& color) override;
	virtual void addText(const std::string& str, float revealDelay,
		float initDelay = 0) override;
	virtual void addText(const std::string& str, int fontsize,
		const Paint& color, float revealDelay, float initDelay = 0) override;
	virtual void addIcon(const char* spritename) override;
	virtual void addIcon(const char* spritename,
		const Paint& color) override;

	virtual void align(HorizontalAlignment alignment) override
		{ _alignment = alignment; }
};
