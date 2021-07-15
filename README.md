# Epicinium

Epicinium is a multiplayer strategy game with simultaneous turns where nature is a finite resource,
available for Windows, Mac and Linux.

Epicinium was released on October 12th, 2020 as a free game on [Steam](https://store.steampowered.com/app/1286730/Epicinium/) and [itch.io](https://abunchofhacks.itch.io/epicinium).
In this repository, the full source code (excluding proprietary Steam integrations) is made available under the AGPL-3.0 License.

Epicinium is being developed by [A Bunch of Hacks](https://abunchofhacks.coop),
a worker cooperative for video game and software development from the Netherlands.
Contact us at [info@epicinium.nl](mailto:info@epicinium.nl).

## Support

Epicinium is and will remain free software. If you wish to support Epicinium and A Bunch of Hacks, you have the option to [name-your-own-price](https://abunchofhacks.itch.io/epicinium/purchase) or [buy the game's soundtrack](https://store.steampowered.com/app/1442600/Epicinium__Extended_Soundtrack/).

## Contents

*  `src/` contains Epicinium's source files
*  `libs/` contains header files for included dependencies
*  `docs/` contains attribution and license files
*  `archive/` contains sets of precompiled shared libraries (64-bit, as well as deprecated 32-bit)
*  `translations/` contains translation files populated by the [Epicinium community translation project](https://www.localizor.com/epicinium)

## External dependencies

*  [OpenGL](https://www.opengl.org//)
*  [CMake](https://cmake.org/download/) (on Windows)
*  [Visual Studio](https://visualstudio.microsoft.com/downloads/) (on Windows)
*  [GNU Make](https://www.gnu.org/software/make/) (on Mac or Linux)

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

## Getting started (Windows)

1. Open this project in Visual Studio.
2. Right-click _CMakeLists.txt_ in the Visual Studio's Solution Explorer and select "Add Debug Configuration". This opens a file called _launch.vs.json_. Change or add the fields "projectTarget", "name" and "args" as below:
```json
{
  "version": "0.2.1",
  "defaults": {},
  "configurations": [
    {
        "type": "default",
        "project": "CMakeLists.txt",
        "projectTarget": "epicinium.exe",
        "name": "epicinium.exe",
        "args": [
            "--font-filename=C:\\PATH\\TO\\FONT",
            "--resource-root=C:\\PATH\\TO\\RESOURCES"
        ]
    }
  ]
}
```
3. Replace `C:\\PATH\\TO\\RESOURCES` to a folder containing Epicinium's assets (see _Locating the assets_ above).
4. Replace `C:\\PATH\\TO\\FONT` to the path of a TTF font file. If using assets downloaded from Steam or itch.io, you may instead remove that line altogether, which causes the font included with the assets to be used.
5. Compile and run the target named "epicinium.exe" from within Visual Studio.

## Getting started (Mac or Linux)

1. Create _settings.json_ by copying _settings.template.json_, leaving "config-root" as is.
2. Edit _settings.json_ so that "resource-root" is set to a folder containing Epicinium's assets (see _Locating the assets_ above).
3. Edit _settings.json_ so that "font-filename" is set to the path of a TTF font file. If using assets downloaded from Steam or itch.io, you may instead remove the setting from _settings.json_ altogether, which causes the font included with the assets to be used.
5. Compile the game with `make`.
6. Run the resulting launcher: `./epicinium`.

## License

Epicinium was created by [A Bunch of Hacks](https://abunchofhacks.coop).
It is made available to you under the AGPL-3.0 License,
as specified in `LICENSE.txt`.

Epicinium is free software; you can redistribute it and/or modify it under the terms of the GNU Affero General Public License (AGPL) as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

Epicinium is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

## Related repositories

*  [Epicinium documentation](https://github.com/abunchofhacks/epicinium-documentation), which includes a wiki and a tutorial for Epicinium
*  [Epicinium assets](https://github.com/abunchofhacks/Epicinium-assets), assets for Epicinium including audio, sprites, maps, rulesets and other resources
*  [Epicinium-NeuralNewt](https://github.com/abunchofhacks/Epicinium-NeuralNewt), a libtorch framework for training neural networks to play Epicinium via NeuralNewt, a parameterized decision tree AI, with evolutionary training techniques
