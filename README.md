# Epicinium
Epicinium is a multiplayer strategy game with simultaneous turns where nature is a finite resource, available for Windows, Mac and Linux. It is released 12 October 2020 on [Steam](https://store.steampowered.com/app/1286730/Epicinium/) and [itch.io](https://abunchofhacks.itch.io/epicinium).

Epicinium is being developed by [A Bunch of Hacks](https://abunchofhacks.coop),
a worker cooperative for video game and software development from the Netherlands.
Contact us at [info@epicinium.nl](mailto:info@epicinium.nl).

## Contents

*  `src/` contains Epicinium's source files
*  `libs/` contains header files for included dependencies
*  `docs/` contains attribution and license files
*  `archive/` contains sets of precompiled shared libraries (64-bit, as well as deprecated 32-bit)
*  `translations/` contains translation files populated by the [Epicinium community translation project](https://www.localizor.com/epicinium)

## External dependencies

*  [GNU Make](https://www.gnu.org/software/make/)
*  [GnuWin32](http://gnuwin32.sourceforge.net/) (on Windows)

## Locating the assets

In order to run the game, you must acquire a valid copy of Epicinium's assets.

The preferred way is to clone the [Epicinium-assets](https://github.com/abunchofhacks/Epicinium-assets) repository.
Note that the assets in the Epicinium-assets repository have their own license, separate from that of the source code (this repository).
Epicinum-assets does not come with any fonts, so you will need to download a suitable font separately, such as [GNU Unifont](https://unifoundry.com/unifont/index.html). Again note that fonts have their own separate licenses.

If you don't intend to modify the assets, you can also use a copy of the game downloaded from Steam or itch.io for this purpose. If for example you are on Windows and you have previously installed Epicinium through Steam, the assets might be stored here:

> C:\Program Files (x86)\Steam\steamapps\common\Epicinium

On Mac, the assets are stored inside the application bundle:

> [...]/epicinium.app/Contents/Resources

However you cannot redistribute assets that came with the game to anyone other than yourself, as they do not come with a license.
Also beware that running anything other than the steps from _Getting started_ below may overwrite and thus corrupt the assets.

## Getting started

1. Create _settings.json_ by copying _settings.template.json_, leaving "config-root" as is.
2. Edit _settings.json_ so that "resource-root" is set to a folder containing Epicinium's assets (see _Locating the assets_ above).
3. Edit _settings.json_ so that "font-filename" is set to the path of a TTF font file. If using assets downloaded from Steam or itch.io, you may instead remove the setting from _settings.json_ altogether, which causes the font included with the assets to be used.
5. Compile the game with `make` (or `make -j` to speed up compilation).
6. Run the resulting launcher: `.\epicinium.exe` on Windows or `./epicinium` on Mac and Linux.

## License

Epicinium was created by [A Bunch of Hacks](https://abunchofhacks.coop).
It is made available to you under the AGPL-3.0 License,
as specified in `LICENSE.txt`.

Epicinium is free software; you can redistribute it and/or modify it under the terms of the GNU Affero General Public License (AGPL) as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

Epicinium is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

## Related repositories

*  [Epicinium documentation](https://github.com/abunchofhacks/epicinium-documentation), which includes a wiki and a tutorial for Epicinium
