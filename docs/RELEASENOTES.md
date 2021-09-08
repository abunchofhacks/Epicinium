<!-- SUMMARY
Added Steam Workshop support.
Added a Campaign made by community member Zanath.
Frostbite now causes units to become Chilled.
-->

# Release 1.1.0 (2021-09-08)

### Content
- Added support for Steam Workshop. Steam Workshop allows sharing maps, challenges, rulesets and color palettes made using the Map Editor. More information about how publish and play Steam Workshop maps can be found on [this wiki page](https://epicinium.fandom.com/wiki/Steam_Workshop).
- Added a Campaign, made by community member Zanath.
- All challenges can now be selected from the "Challenges" submenu.
- Added a Ruleset Editor to the Map Editor.
- Added a Palette Editor to the Map Editor. The Palette Editor also opens when selecting the color palette "custom..." in the Settings menu.

### Gameplay
- *Reworked Frostbite:* Frostbite no longer deals damage. Instead, units that remain in Frostbite in the Decay phase become **Chilled**. Chilled units cannot attack or use abilities until they move at least one tile. This effect wears off at the next Decay phase.
- *Reworked Firestorm:* Firestorm now has a percentage chance of appearing, instead of being placed on random tiles. Highly flammable tiles (Grass, Forest, Soil, Crops and Trenches) have a 20 percent chance of receiving Firestorm after the second stage of global warming, increased to 40 percent if the tile has no humidity left. After the fourth stage of global warming this becomes a 50 percent chance (70 for tiles without humidity), and structural tiles (City, Town, Outpost, Barracks, Industry, Airfield and Farm) without humidity have a 10 percent chance of receiving Firestorm.

### Visuals & User Interface
- Redesigned the multiplayer menu.
- The map in the Map Editor uses a smaller scale than the rest of the game, giving a more zoomed out overview.
- Added option to claim Host privileges to the lobby screen. When a player becomes the Host, only they can change lobby settings and start the game.
- Added a "Ruleset" dropdown to the lobby. In custom lobbies, this can be used to change the rules from *v1.1.0* (this version) to a different set of rules, such as one from the Steam Workshop.
- Added 5-second option back to the planning timer dropdown.

### Audio
- The alternative title theme now also plays inside the Map Editor.
- Reduced the relative volume of the alternative title theme to 50%.

### Bugfixes
- Fixed a bug in the Steam version where music would stop playing after clicking the "PLAY" button.
- Fixed a bug in the Map Editor where 'Copy' did the same thing as 'Open'.
- Fixed a bug in the Map Editor where 'Play Versus AI' did not work correctly for challenge maps.
- Fixed a crash that could occur in the Map Editor when selecting 'Crop Top' or 'Crop Left'.
- Fixed a bug in the Map Editor where resigning from a test game could cause a tile or unit to be placed because the left mouse button was still held down.
- Fixed a bug in the Map Editor where neutral units could not be placed after placing non-neutral units.
- Fixed a bug in the Map Editor where ground units could be place in the air and vice versa.

### Technical
- Made it easier to compile Epicinium's source code on Windows by adding CMake.


## Patch 1.0.12 (2021-05-17)

*This is a server-only patch that is compatible with release 1.0.0.*

### Content
- Added more user-made maps.

### Gameplay
- Players that are defeated before the game ends are granted global vision.

### Bugfixes
- Fixed a bug where rejoining a lobby after resigning as a player would not grant global vision.
- Fixed a bug where, after a bypass attack, the bypassed unit was not involved in the retaliation damage.


## Patch 1.0.11 (2021-03-12)

*This is a server-only patch that is compatible with release 1.0.0.*

### Content
- Added various user-made maps.

### Bugfixes
- Fixed a bug where NeuralNewt AI could be selected on maps bigger than 20x13, resulting in a server crash.


## Patch 1.0.10 (2020-12-24)

*This is a Windows-only patch that is compatible with release 1.0.0.*

### Content
- Added the Map Editor to the main menu.
- Increased the size of *small8ffa* and *cornered8ffa* to 32 by 32 by adding Water tiles in order to balance out the advance of global warming.
- Updated the credits.

### Visuals & User Interface
- Updated the panel pictures in the multiplayer menu that showed outdated graphics.

### Bugfixes
- Fixed a bug where tooltips for Frostbite could show when hovering a tile in the fog of war.


## Patch 1.0.9 (2020-11-27)

*This is a server-only patch that is compatible with release 1.0.0.*

### Accounts
- Ending a game as the winner with a score of 0 also causes your rating to increase by 0.1, instead of decrease by a percentage.

### Content
- Added the ChargingCheetah AI. This aggressive AI made by Zanath is based on HungryHippo but uses Gunners and Sappers in an early rush strategy.

### Bugfixes
- Fixed a bug where HungryHippo would not create Rifleman units.

### Technical
- Improved NeuralNewt's brain with additional training.


## Patch 1.0.7 (2020-11-09)

*This is a Windows-only patch that is compatible with release 1.0.0.*

### Visuals & User Interface
- The game asks for confirmation after changing screen resolution or other visual settings and reverts automatically if not confirmed.


## Patch 1.0.6 (2020-10-22)

*This is a Windows-only patch that is compatible with release 1.0.0.*

### Audio
- Lowered volume of explosion and bombard sound effects.

### Techninal
- Restored translation support on Windows.


## Patch 1.0.5 (2020-10-20)

*This is a server-only hotfix that is compatible with release 1.0.0.*

### Accounts
- Ending a game with a score that is lower than your current rating (but higher than 0) now causes your rating to increase by 0.1, instead of decrease by a percentage of the difference.

### Bugfixes
- Fixed a bug where clients were kicked for no reason if they tried to Copy an AI opponent.


## Patch 1.0.4 (2020-10-16)

*This is a Windows-only patch that is compatible with release 1.0.0.*

### Bugfixes
- Fixed an issue where the game was unplayable on Windows if the AppData path contained a non-ASCII character, such as in the user's name.
- Temporarily disabled translation support on Windows as it is not yet fully functional, but instead causes incomplete translations to be shown instead of English.


## Patch 1.0.2 (2020-10-13)

*This is a server-only hotfix that is compatible with release 1.0.0.*

### Bugfixes
- Fixed a crash in the server that would occur if a player resigned from an FFA game while all other players were done with their planning phase.


# Release 1.0.0 (2020-10-12)

### Accounts
- Added Steam integration. When launching the game from Steam, you may be asked to enter a new username if your Steam nickname does not suffice. Beyond that, no registration is required.

### Social
- Added Steam Rich Presence integration, including Join Game functionality.

### Gameplay
- Decreased the self-upgrade cost of the Town tile to 4, down from 10.

### Content
- Updated the credits.

### Visuals & User Interface
- Reworked the main menu with brand new art. Menu art panning can be disabled in the Settings menu.
- Added tooltip cards when hovering an order in the order context menu.
- Improved the default color palette.
- Added a high constrast color palette, especially geared towards colorblind players. This can be enabled in the Settings menu.
- Added a color palette option to the Settings menu.
- Added language options to the Settings menu. Language options other than English are thanks to [our online community translation effort](https://www.localizor.com/epicinium) and may be incomplete.
- Added a scrollbar to scrollable layouts that exceed their frame height.
- Reduced the transparency of various forms and tooltips.
- Changed font to GNU Unifont.
- Changed default font size to 16 (down from 20).
- Added a option in the Settings menu to increase font size to 150%.

### Audio
- Revamped title theme.
- An alternative version of the title theme now plays on the multiplayer menu.
- Added new in-game music that plays during the Planning phase based on the current season.
- Added new in-game music that players during the Action phase.
- Added additional in-game sound effects.
- Added sound effects to menu buttons.

### Bugfixes
- Fixed an issue where the lower half of unit figures could be obscured by City buildings.
- Fixed an errant pixel in the phase graphic.
- Fixed an issue where a segment of the tutorial could be skipped accidentally.
- Fixed some layout issues on smaller resolutions.
- Fixed a bug where windowed mode could default to an unusably small resolution.

### Technical
- Added support for Unicode characters in UI and chat.
- Improved line wrapping in overlong text fields.
- Added a version string to the bottom of the feedback menu to help with debugging.


# Release 0.35.0 (2020-08-11)

### Accounts
- Lowered the rating increase percentage for two-player games played outside of One vs One lobbies to 5% per human player, down from 10%. Also lowered the rating decrease percentage for these matches to 5%, down from 10%.
- Made it possible to change usernames upon request.

### Content
- Added the NeuralNewt AI. This AI uses a neural network that been trained using neuro-evolution.
- Added an overview map that can be used to view all tile types, unit types and hazardous weather effects in the game. It can be opened from the multiplayer menu.
- Players in One vs One lobbies can no longer add bots, choose non-*1v1* maps or custom maps, change the planning phase timer or grant players global vision.

### Visuals & User Interface
- Added a button in the multiplayer menu that opens a detailed guide of all structures, units, abilities, construction paths and tile properties in the default browser.
- Replaced the "Free For All" panel with a "Custom Lobby" panel. Clicking this panel no longer automatically joins open lobbies on *ffa* maps.
- Lobbies are now tagged with "1v1" if they are One vs One lobbies, and with "custom" if they have custom maps enabled.

### Bugfixes
- Fixed a bug where the map would not fade in properly on maps that were exactly 20 columns wide and 13 rows high.
- Fixed a bug where an AI could give an invalid Lockdown order to a Gunner unit if it was placed on the edge of the *oasis1v1* map.
- Fixed a bug where "Welcome back" and "Your rating is now X" messages did not appear when returning to the multiplayer menu after a game has ended.
- Fixed a missing icon on the info card of Crops.

### Technical
- Reimplemented the server architecture to be more flexible and more stress-resistant.
- Changed message length endianness to be explicitly network byte order (big-endian) instead of host byte order (usually little-endian).
- The game gets the latest leaderboard from our website instead of from the server.


# Release 0.34.1 (2020-07-19)

### Content
- Games can now be played on custom maps by selecting "Custom maps" in the lobby settings. Custom maps may be unbalanced and therefore these games are not rated.
- Added the old *snowcut1v1* map to the custom map pool.
- Added the *challenge_morale* map to the custom map pool. Games played on this map use the Morale ruleset. The HungryHippo AI is not available on this map.
- Added twelve maps made by Overlord_Vadim to the custom map pool.

### Visuals & User Interface
- Further reworked the in-game UI.
- Double-clicking a lobby in the multiplayer menu now joins that lobby.
- Removed the "create lobby" and "join lobby" buttons in the multiplayer menu.

### Bugfixes
- Fixed a crash that could occur when a produced unit attempts to rally into an adjacent space under lockdown but dies due to an attack of opportunity.
- Fixed a bug where the info boxes would briefly appear when the left mouse button was pressed during animations.
- Fixed a bug where the Escape key did not work while animations were playing.


# Release 0.34.0 (2020-06-25)

### Accounts
- During the Indigo 2020 brawl event on June 26, new users can log in as a guest using their Discord username if the Discord desktop application is running. Ratings and stars are not recorded when playing as a guest.

### Gameplay
- During this event, the One vs One gamemode is played with the Morale ruleset on a special map, and all gamemodes other than One vs One are disabled.

### Visuals & User Interface
- Reworked the in-game UI.


## Patch 0.33.1 (2020-05-30)

*This is a server-only hotfix that is compatible with release 0.33.0.*

### Bugfixes
- Fixed an issue where RampantRhino built Outposts as if they were Towns, instead of as forward bases.
- Fixed an issue where MacOS clients could no longer connect to the server.


# Release 0.33.0 (2020-05-11)

### Gameplay
- Removed the Death global warming effect.
- City tiles no longer benefit from nearby snowed tiles, but always fully benefit from nearby Forest tiles, even if they are covered with snow.
- Decreased the cost of the Rifleman unit to 5, down from 10.
- Decreased the cost of the Settler unit to 1, down from 5.
- Decreased the cost of the Industry tile to 5, down from 10.
- The Outpost tile can no longer upgrade to a Town or Farm.
- The Settler unit can no longer directly settle a City.
- Upgrade orders are no longer postponed if the tile is occupied by a friendly ground unit.
- Players can give empty orders by dragging, causing subsequent orders to be artificially delayed. Empty orders are collapsed when giving a new order.

### Content
- Adjusted all maps to ensure that at least one City for each player can become fully powered, even in winter.
- Added the Morale Challenge. In this challenge money is replaced with "morale", which is gained when units are killed and tiles are destroyed or captured.
- Disabled the planning timer when creating a "Versus AI" lobby.
- Adjusted the RampantRhino AI to avoid issuing orders it cannot afford.
- Updated the credits.

### Visuals & User Interface
- Improved building, tree and mountain sprites.
- Improved grass texture.
- Improved the color palette.
- Trees change color when a tile loses all humidity.
- Improved the automatic camera panning, in particular during Bombard orders.
- Decreased the maximum distance the camera can extend past the edge of the map when scrolling manually.
- Added separate icons in the multiplayer menu for replay lobbies, lobbies that are already in progress, players that are watching a replay and players that spectating a match in progress.

### Bugfixes
- Fixed multiple bugs resulting in Discord Rich Presence not activating.
- Fixed a bug where Discord Rich Presence reported an incorrect party size.
- Fixed a bug causing the HungryHippo AI to issue invalid Move orders.
- Fixed a segfault when using Selection Mode: Context Menu.
- Fixed disappearing text in the Feedback menu.
- Fixed an issue where pressing the "One vs One" button could result in joining a match in progress as a spectator.
- Fixed a bug where the chat could be flooded with "Alice is online" messages when joining the server.
- Fixed a bug where rearranging orders in the order list did not update the Ready button.
- Fixed a bug where orders of killed units were not disabled in the order list.
- Fixed a bug where tooltips did not appear despite UI elements being hovered.
- Fixed a bug in measuring in-game distances.
- Fixed a bug where the team color palettes were flipped for yellow/teal and black/pink when saving and flipped back when loading.

### Technical
- Transitioned MacOS build to 64-bit. MacOS 32-bit is no longer supported.
- Improved performance when contacting the server or downloading files.
- Added support for automatic server migration.
- Replaced message-based patching with HTTP-based patching.
- Removed the need for a patch primer file.
- The client prevents connection to the server when there are storage issues.
- Minor graphical performance improvements.
- Lowered maximum map width and height to 32.


# Release 0.32.0 (2019-04-19)

### Gameplay
- The Weather phase now immediately follows the Night phase, instead of alternating between Weather and Night phases. This means each season only consists of one Action phase instead of two, and each year contains four Action phases instead of eight. Various costs have changed to offset the increased speed at which buildings generate income.
- Weather effects no longer update at the end of the Night phase.
- Moved the Dig Trenches ability from the Gunner unit to the Rifleman unit.
- The Gunner unit instead has a new **Lockdown** ability. After executing this order, the Gunner unit stays in lockdown mode for the remainder of the Action phase and will immediately fire upon any enemy unit that enters the target space, interrupting the movement of that unit.
- Units no longer keep their Focus order indefinitely.
- The Gunner unit and the Sapper unit can now capture enemy tiles.
- Added the **Outpost** tile. The Outpost tile can produce Settler and Militia units and can upgrade to a Town or Farm tile.
- The Settler unit and the Militia unit can settle Outposts at no cost.
- The City tile can no longer produce Rifleman units.
- The City tile can instead produce the Militia unit for a cost of 10.
- Increased the maximum size of the Militia unit to 3.
- Increased the cost of the Rifleman unit to 10, up from 5.
- Increased the cost of the Settler unit to 5, up from 1.
- Increased the cost of the Gunner unit to 15, up from 10.
- Increased the cost of the Sapper unit to 15, up from 10.
- Increased the cost of the Tank unit to 25, up from 15.
- Increased the cost of the Zeppelin unit to 25, up from 20.
- Increased the cost of the City tile to 50, up from 20.
- Increased the cost of the Town tile to 10, up from 5.
- Increased the cost of the Farm tile to 5, up from 2.
- Increased the cost of the Industry tile to 10, up from 3.
- Increased the cost of the Barracks tile to 5, up from 3.
- Decreased the upgrade cost of the Barracks tile to 25, down from 30.
- Increased the upgrade cost of the Industry tile to 50, up from 30.
- Increased the cost of upgrading a Town tile to a City tile to 40, up from 15.
- The Town tile no longer automatically increase in size beyond 3 buildings. Town tiles can be upgraded twice to increase their size, for a cost of 10. The Town tile retains its size when upgrading to a City tile.
- Upgrading an Industry, Barracks or Town tile now costs 1 power, instead of depleting all of its power.
- Tiles contribute half as much to global warming per Weather phase in Autumn, to offset the increased speed at which seasons change.
- Grass and Forest tiles have a 25% change to gain a tree each Weather phase in Spring, down from 50%.
- Aridification appears on 4 tiles each Weather phase in Autumn, down from 10.
- Aridification now prefers Grass, Dirt, Soil, Crops and Rubble tiles and avoids other tiles if possible.
- When a player resigns, they are defeated at the start of the next Action phase. If only one player remains undefeated, the game immediately proceeds to the Action phase and the remaining player is victorious.

### Content
- Removed the starting Militia units from all maps except *tutorial*, replacing them with a single Militia unit in each City tile. The maps *small8ffa* and *cornered8ffa* do not have starting Militia units.
- Added extra Grass tiles near the top and bottom of *cornered8ffa*.
- Removed TutorialTurtle and QuickQuack from the AI pool. The default AI is now RampantRhino.
- Updated the HungryHippo and RampantRhino AIs for the new gameplay changes.
- Changed default AI difficulty to medium.
- Added the Economic Growth Challenge. In this challenge, there are no opponents to defeat. Instead, the game ends after the second winter night, and you are scored for how much money you have at the end.

### Visuals & User Interface
- Added unit and tile names below the icons in the order context menu.
- Swapped the icons of the Focus and Lockdown orders.
- Added a message in the chat when a player resigns.
- The multiplayer menu chat window now also shows all messages that were received while playing a match.

### Bugfixes
- Fixed a bug where Industry tiles did not lower the humidity of surrounding tiles in Autumn.
- Fixed a bug where aridification did not lower the humidity of surrounding tiles.
- Fixed a bug where the order of a killed unit could be executed by a unit that moved to the same space later in the round.
- Fixed various issues with orders appearing in the order list that did not belong to any unit.
- Fixed some asymmetries in the RampantRhino AI, causing it to lose more when starting on the left side of the map.
- Fixed a bug where quitting the game with Alt+F4 would not cause the player to resign from a match.
- Fixed a crash in the settings menu when switching from fullscreen to windowed mode.

### Technical
- Network messages are now compressed to reduce bandwidth usage.
- Updated libssl and libcrypto to 1.1.1a in Linux for security and compatibility reasons.
- Added internal moddability features that make it easier for us to try out new gameplay changes.
- Improved code quality of various parts of the codebase.


## Patch 0.31.1 (2019-04-19)

*This is a transitional version that only exists to facilitate patching.*

### Visuals & User Interface
- While downloading a patch, the Play button is replaced with a tooltip.
- Added a progress counter to the tooltip when downloading large files.
- When patching through transitional versions (such as this one), the game automatically downloads and installs the next patch.

### Bugfixes
- Fixed a bug where downloading a symbolic link would create a copy of the original file instead.

### Technical
- Instead of using ping and pong messages to keep the connection with the server alive, the client now sends cheaper pulse messages.
- Patch files are now compressed to improve download speeds.
- Each patch is now accompanied by a signed manifest to prevent downgrade attacks.
- Disabled unused wide string support for log files under Windows.


# Release 0.31.0 (2018-11-28)

### Accounts
- For the remainder of the Kickstarter campaign, i.e. until December 1st 22:59 UTC, the beta is open for everyone.
- During this time, new users can log in as a guest using their Discord username if the Discord desktop application is running. Ratings and stars are not recorded when playing as a guest.

### Gameplay
- Giving an Upgrade order to a tile with max stacks (using Advanced Controls) now discards the order instead of spending money and power for nothing.

### Content
- Brought back the Tank Derby Challenge.
- Replay lobbies are now made private when the replay starts.

### Visuals & User Interface
- Added a star to the surface texture of City tiles.
- Added icons to the tutorial texts.

### Bugfixes
- Fixed some asymmetries on the *beetle4ffa* map.
- Fixed a bug where orders could occasionally be ignored.
- Fixed a bug where bypassing a unit would visually de-occupy a tile.
- Fixed a bug causing idle animations to be synchronized instead of random.
- Fixed a bug where a message about downloaded files would overwrite the server connection status and Play button tooltip.


## Hotfix 0.30.1 (2018-10-17)

### Bugfixes
- Fixed severe screen flickering issues on Windows.


# Release 0.30.0 (2018-10-17)

### Gameplay
- Firestorm occurs at random on 4% of the map for every 40% of the map that is covered by chaos, instead of 24% for every 40%.
- Firestorm now prefers Grass, Forest, Soil, Crops and Trenches tiles and avoids other tiles if possible.
- Building a tile now removes any Firestorm present if the newly built tile is not flammable.
- Building a tile or digging Trenches now removes any Frostbite present if Frostbite would not naturally occur on the newly built tile.
- Frostbite no longer occurs on Water tiles.

### Visuals & User Interface
- Added additional visual indicators for damaging weather effects such as Frostbite and Firestorm.
- Added a flicker effect when tile buildings gain or lose power.
- When a tile is occupied, the lights in its buildings are dimmed.
- Made snow more transparent, in particular on Grass and Mountain tiles.
- Added additional tree types. Different maps can feature different types of trees.
- Added highlights to valid targets while giving a targeted order in the Planning phase.
- Added highlights to active tiles and units in the Action and Night phases.
- Added lighting effects for gunfire and explosions.
- Add animations to the in-game wallet.
- The wallet blinks red if a unit or tile cannot execute its orders due to insufficient funds.
- Improved the in-game tooltips for Snow and Firestorm.
- Animated the logo on the main menu.
- Added the possibility to show a stamp on the logo with tooltip and link (to be used during our crowdfunding campaign).

### Bugfixes
- Fixed a pathing bug when unit figures spawn.
- Fixed a bug where mountains would not be fully contained within their borders.
- Fixed a bug where incorrect amounts of money were displayed when joining a replay in progress.
- Fixed interface layout issues on smaller resolutions.
- Fixed a text rendering bug.
- Fixed a bug where the RampantRhino AI would give multiple orders to the same unit or tile within a single planning phase.
- Fixed a bug where Discord Rich Presence would contain a "Ask to Join" button instead of a "Spectate" button while spectating.
- Fixed a bug where the mouse cursor would be stuck at ibeam if a text input would disappear while hovered.

### Technical
- Reduced the filesize of various animated sprites.
- Improved timing of screen flip.
- Added *settings.json* to the log files that can be attached to a bug report.


## Hotfix 0.29.1 (2018-09-21)

*This is a server-only hotfix that is compatible with release 0.29.0.*

### Bugfixes
- Fixed an issue preventing itch.io and gamejolt users from logging in.
- Fixed an issue causing users to appear multiple times in the user list.


# Release 0.29.0 (2018-09-21)

### Accounts
- This version marks the start of the Closed Beta. A key is required to gain access to this Closed Beta.

### Social
- If integration with Discord is enabled, the game can now automatically link your Epicinium username to your Discord account.

### Content
- Added the High Speed Challenge. In this challenge, you play a normal match on *oasis1v1* except all units have their movement speed increased by 2. To earn 1 star, get at least 1 point. To earn 2 stars, get at least 30 points. To earn 3 stars, get at least 80 points.

### Visuals & User Interface
- Figures now walk around buildings and trees instead of through them.
- Added a key activation page.
- Added text selection to text input fields.
- Added copy and paste functionality to text input fields.

### Bugfixes
- Fixed an issue with text rendering.
- Fixed an issue where player doing tutorial or challenge would be listed as in lobby instead of in game.
- Fixed an issue where the sound effect of a collapsing building started before the animation did.

### Technical
- Improved performance of the main menu.


## Hotfix 0.28.3 (2018-09-03)

*This is a server-only hotfix that is compatible with release 0.28.2.*

### Bugfixes
- Fixed a bug where Discord Rich Presence users could not post an Invite to Spectate link, despite the lobby being public.


# Release 0.28.2 (2018-08-31)

### Visuals & User Interface
- Animations can now be sped up by holding down the left mouse button, and paused temporarily by holding down the right mouse button.
- Made the impact animation of attacks more noticeable and easy to read.
- Updated the tutorial wording to correspond to the new UI.

### Bugfixes
- Fixed a bug where Discord Rich Presence was updated too frequently, causing it to fail to display at all.
- Fixed a bug where rejoining a game in progress after disconnecting would break the timer and disable view panning.
- Fixed a bug where lobbies whose game had ended were not locked even though they cannot be joined.
- Fixed a bug where the "One vs One", "Free For All" and "Versus A.I." buttons remained clickable when the server is shutting down.
- Fixed a bug in the settings menu where the Advanced Controls settings would not save and load correctly.

### Technical
- Updated libssl and libcrypto to 1.1.0i in Windows and 1.0.2p in OS X for security. Linux still uses the system libraries.


## Patch 0.28.1 (2018-08-28)

*This is a server-only patch that is compatible with release 0.28.0.*

### Social
- The server now communicates with a Discord bot instead of posting information about who is online directly to a Discord channel.


# Release 0.28.0 (2018-08-25)

### Social
- Added Discord Rich Presence integration.

### Content
- Added the Tank Derby Challenge. In this challenge, you and three AI opponents fight with Tank units on a tiny map covered with Crops tiles. To earn 1 star, defeat all three opponents. To earn 2 stars, get at least 4 points. To earn 3 stars, get at least 10 points.
- The RampantRhino, TutorialTurtle and BrawlingBear AIs now avoid settling Town, Farm or City tiles directly adjacent to their own City tiles.

### Visuals & User Interface
- Added a Discord status icon to the top right of the main menu.
- Lobbies in the lobby list now show the current and maximum number of players.
- Added "One vs One" and "Free For All" buttons that automatically join a lobby of the chosen type if available, and create such a lobby otherwise.
- Added a "Versus A.I." button that creates a lobby and immediately adds an A.I. opponent.
- Added a faded map preview to the back of the observer list.
- Reworked the order list elements with new icons for all units, tiles and abilities.
- Replaced the unit and tile names within the order context menu with matching icons.
- Replaced the blue text within the order guides with matching icons.
- Made the valid target guide and invalid target cursor more prominent.
- Added numbers to the back of the order list elements that match the sequence in which the orders will be executed during the Action phase.
- Double-clicking an order in the order list causes the view to pan to that unit or tile.
- Manual view scrolling is now disabled during the action phase and while panning. This can be reenabled by turning on Advanced Controls in the settings menu.
- The UI now prohibits giving orders to tiles without power and giving redundant Stop, Cultivate and Upgrade orders. This can be reenabled by turning on Advanced Controls in the settings menu.
- Made text in disabled interface elements slightly transparent.
- Changed the style of various UI elements to be more consistent.
- While spectating, if a player loses a challenge the popup now says "The player was defeated" instead of "A player was victorious".
- Pressing Escape while the order context menu is open, closes it instead of showing the Quit popup. The same holds for the targeting cursor.

### Bugfixes
- Pressing Escape while the in-game chat window is open, closes it but no longer also shows the Quit popup. The same holds for the player list, the newspaper and the mission briefing.
- Fixed a bug where the initiative list was incomplete or incorrect when an observer joined a game in progress.
- Fixed a server crash that could occur when a game client sent a broken sequence of messages.
- Fixed a server crash that could occur due to multithreading issues.


## Patch 0.27.1 (2018-08-25)

*This is a transitional version that only exists to facilitate patching.*

### Bugfixes
- Added msvcr120.dll in the Windows releases, fixing an issue where users without the Visual C++ Redistributable could not launch the game.

### Technical
- The game can now patch itself without relying on a separate patcher.
- The game can now patch the launcher.


# Release 0.27.0 (2018-08-06)

### Content
- Added the Everything is Free Challenge. In this challenge, you play a normal match except that all unit and tile costs have been reduced to 0. To earn 1 star, get at least 1 point. To earn 2 stars, get at least 30 points. To earn 3 stars, get at least 80 points.

### Visuals & User Interface
- Changed the size and layout of the phase list to put more emphasis on the Planning and Action phases.
- Made the unit info boxes and the initiative box partially transparent.
- The UI can now handle different rulesets (such as those from the challenges) and adapt as needed.

### Technical
- The server now sends lobby info before player info, so that the lobby ids are known before you hear which lobbies are joined.


## Hotfix 0.26.1 (2018-07-24)

*This hotfix only applies to debian64 and is compatible with version 0.26.0.*

### Bugfixes
- Fixed a bug where the debian64 version of the game would connect with an internal server instead of with the live server.


# Release 0.26.0 (2018-07-20)

### Accounts
- Challenge maps no longer affect rating.

### Content
- Challenge lobbies are no longer private.
- Added the Elimination Challenge. To earn 1 star, eliminate all enemy units. For 2 stars, also get at least 25 points. For 3 stars, also avoid destroying any City, Farm or Industry tiles.

### Visuals & User Interface
- Added an image background to the Tutorial and Challenge buttons.
- The name and image background of the Challenge button will reflect the currently active challenge.

### Bugfixes
- Fixed a mix-up where feedback text would be unnecessarily truncated before being sent to the devs, while the untruncated text would be sent to STOMT and rejected for being too long.

### Technical
- Added functionality for the client to download images and other small files from the server without needing to download an entire patch.
- Improved text rendering.


# Release 0.25.0 (2018-07-06)

### Accounts
- Players can now earn stars by playing weekly challenge maps. Each challenge can award up to three stars based on requirements given in a mission briefing. Players can retry challenges in order to get more stars; only the best attempt counts.

### Content
- Added the first challenge to the multiplayer menu. In this challenge, you start with three Rifleman units and no City tiles; having your City tiles occupied does not cause you to lose. To earn 1 star, win with at least 1 point. For 2 stars, get at least 25 points. For 3 stars, get at least 40 points.
- The RampantRhino and TutorialTurtle AIs now move their units out of the way if they are blocking another unit.

### Visuals & User Interface
- The figures of a newly produced unit now spawn inside the buildings of the tile that produced them.
- Added grass particles when a Grass tile is destroyed.
- Added dust particles when a tile is built or upgraded.
- Added particles when a Zeppelins drops mustard gas.
- A Shell or Bombard explosion can no longer appear right next to the figure of a unit if it did not hit that unit.
- Added a "Feedback?" button to the bottom of the main menu that can be used to post feedback to [our STOMT page](https://www.stomt.com/epicinium) or to submit a bug report alongside your Epicinium logs.
- Also added a "Feedback?" tab to the top right corner of the multiplayer menu.
- The notice underneath the registration form now links to our new [privacy statement](https://epicinium.nl/privacy.php).
- When less than 4 players are online, a link to our Discord is added to the player list to help players find opponents.

### Bugfixes
- Fixed a bug in the mailing list preferences popup where the functionality of "yes, email me about the campaign" was swapped with that of "yes, and also keep me up to date on Epicinium".
- Fixed a bug that could result in frames being dropped unnecessarily.
- Fixed a bug where tile textures would change too soon while animating a tile being transformed or destroyed.
- Fixed a bug where the RampantRhino and TutorialTurtle AIs could not produce Settler units while defending their City tiles.

### Technical
- Reduced video memory usage by approximately 15%.

### Other
- Added the privacy statement to the archive as a markdown file.
- Added a text file with install instructions for those who have trouble getting the game to run.


# Release 0.24.0 (2018-06-01)

### Accounts
- Players no longer lose rating points if they resign before the third action phase has started.

### Gameplay
- The City tile can now produce the Rifleman unit for a cost of 10.
- The City tile can no longer produce Militia units.
- Lowered the movement speed of the Rifleman unit to 2, down from 3.

### Content
- Changed the RampantRhino and TutorialTurtle AIs to produce units earlier in the game.
- Updated the QuickQuack and HungryHippo AIs for the new gameplay changes.

### Visuals & User Interface
- Improved the layout of the in-game UI on very small screens.

### Bugfixes
- Fixed a bug where players rejoining a match after a disconnect could see the enemy base through the fog of war.
- Fixed a bug where Tank units dealing trample damage to Soil or Crops tiles could leave behind ghost tanks.
- Fixed a bug that caused units to disappear and reappear on the same tile.

### Technical
- Improved the stability of the server when the connection with a client deteriorates.


# Release 0.23.1 (2018-05-25)

### Content
- Updated the tutorial to match the gameplay changes introduced in 0.23.0.

### Visuals & User Interface
- Made a higher definition version of the title screen logo.
- The camera now rests longer before showing a tile being captured to make it more clear what is happening.
- Slightly tweaked the registration procedure to comply with GDPR.

### Bugfixes
- Fixed a bug where the camera could jerk to the side for no reason at lower framerates.
- Fixed a bug where a player leaving a game in progress and rejoining as an observer would cause that player to always receive gameplay updates about that game even if they were not in that lobby anymore and even if they were playing a different game.
- Fixed a bug where the server would finish a game at full speed when one player disconnected and one player resigned.
- Fixed a bug where a locked lobby could appear unlocked.


# Release 0.23.0 (2018-05-21)

### Accounts
- Slightly changed the way the score you obtain during a match affects your rating. Every match you play where you obtain a score higher than your current rating now causes your rating to increase by a percentage of the difference, but at least 0.1. Every match you play where you obtain a score lower than your current rating causes your rating to decrease by a percentage of the difference, but at least 0.1.
- Lowered the rating increase percentage for free-for-all matches involving at least two human players to 1% per human player, down from 10%. Lowered the rating decrease percentage for these matches to 1%, down from 10%.
- Lowered the rating increase and decrease percentages for matches versus AI players to 1%, down from 10%.
- While your rating is below 10, the rating increase percentage is at least a minimum percentage; this minimum scales from 10% at a rating of 0 through 5% at a rating of 5 down to 1% at a rating of 10.

### Gameplay
- Added the **Militia** unit. Its maximum size is 5 and it has a movement speed of 3. Each figure of a Militia unit deals 1 damage and has only 1 hitpoint. It can be produced by City and Farm tiles for a cost of 5.
- The Rifleman unit is no longer produced by City or Farm tiles, but instead by Barracks tiles for a cost of 5.
- The Tank unit can no longer attack and must rely on its Shell ability to deal damage.
- While a tile is occupied by an enemy unit, the tile cannot execute any orders except Stop orders and does not gain power or generate income at night.
- At the end of the decay phase, if all City tiles controlled by a player are occupied by enemy units, that player loses control of those City tiles and is defeated.
- When a player is defeated, their tiles and units lose all old orders.
- Tiles can now be given an order to produce a unit at an adjacent space. When the unit is produced, it will move to that space if possible. If a friendly unit is occupying the producing tile, the produced unit will bypass that unit to get to its destination. The produced unit will not attack if its destination space is occupied by an enemy unit, but it can be subject to an attack of opportunity if it is not bypassing a friendly unit.

### Content
- All maps have been adjusted by replacing the starting Rifleman units with  Militia units.
- Added the maps *oasis3ffa* and *beetle4ffa* back to the map pool.
- Added the map **cornered8ffa** to the map pool.
- Improved the RampantRhino and TutorialTurtle AIs to be better at defending their City tiles and avoiding weather effects such as Frostbite.
- Improved the HungryHippo AI to use Militia, Rifleman and Tank units.
- Updated the QuickQuack AI to use Militia units instead of Rifleman units.

### Bugfixes
- Fixed a bug where Airfield tiles would not gain power when adjacent to an Industry tile that just gained power that night.
- Fixed a bug where the camera could no longer be moved with WASD or the arrow keys if the newspaper announcement appeared while the chat window was open.


## Hotfix 0.22.2 (2018-05-15)

*This is a server-only hotfix that is compatible with release 0.22.0.*

### Bugfixes
- Fixed a bug where vision was not calculated properly, which could result in invisible units or units leaving behind ghosts when disappearing in the fog of war.

### Technical
- When a user attempts to reconnect after a crash, the server might believe that they are still online. Instead of rejecting the attempt, it will now be put on hold for a few moments until the old connection has been dropped.


## Hotfix 0.22.1 (2018-05-14)

*This is a server-only hotfix that is compatible with release 0.22.0.*

### Bugfixes
- Fixed an issue that caused the server to freeze for several minutes when a player disconnected unexpectedly.

### Technical
- Lowered the server-side connection timeout to 2 minutes, down from 10 minutes.


# Release 0.22.0 (2018-05-12)

### Gameplay
- When a unit performs a bypass attack past an entrenched unit and an enemy unit retaliates, the entrenched unit is no longer involved in the resulting damage step.
- All players receive an announcement whenever global warming crosses a threshold, i.e. whenever another 20% of the map is covered by chaos.
- Reenabled fog of war by default, except in the tutorial.

### Content
- Improved the HungryHippo AI so it behaves symmetrically on all maps.
- Improved the RampantRhino AI so it makes better use of its units and capitalizes on an income lead by building more City tiles.

### Visuals & User Interface
- Joining a match in progress is now instantaneous.
- In the Winter growth phase, Snow also appears on tiles in the fog of war whose last known humidity is high enough.
- The chaometer has been removed in favor of a newspaper; this newspaper appears whenever global warming crosses a threshold. The last issue of the newspaper can also be brought back by clicking the date in the bottom left.
- The panels detailing unit and tile stats have been significantly simplified and now only show size, power and hitpoints.
- The hygrometer and weather markers are now placed above these panels.
- The wallet and the diamond button that reveals the player info panel have been unified.
- Added option in the lobby to create a copy of an AI player.
- In Fullscreen mode, the resolutions list in the settings menu will only contain resolutions the monitor supports (as far as we can detect).
- In Fullscreen Desktop mode, no resolution can be specified because it will be the same as the user's desktop.

### Bugfixes
- Fixed a bug where a Trenches tile would provide its defense bonus during the retaliation attack that takes place when a unit performs a bypass attack past an entrenched unit.
- Added a missing delay between the sound effects of unit figures dying as a result of Death.
- The UPDATE button is now properly disabled while downloading a patch.
- Fixed a bug where certain particles would only render at scale 2.
- Fixed a bug where the mouse position was incorrectly matched to its position on the screen under certain resolutions, preventing buttons from being clicked.
- Fixed a bug in Windows where in Windowed Borderless the window would be offset and the cursor position misaligned.
- Fixed a server crash that could occur when a player left the server and a lobby was disbanded at the same time.
- Fixed a bug where normal lobbies could be started on the tutorial map.

### Technical
- Improved native resolution detection.
- Added support for multiple monitors. The game will not use the primary monitor by default. The monitor can be selected in the settings menu.
- In windowed mode and borderless windowed mode, the position of the window can be changed using the settings menu.
- Fullscreen and Fullscreen Desktop now behave as intended, without any undesired stretching or clipping.
- Made sure the game window is never displayed at a resolution the monitor does not support in Fullscreen mode.


## Release 0.21.1 (2018-05-02)

*This is a Linux-only release that fixes compatibility with Ubuntu 18.04.*

### Bugfixes
- Fixed a compatibility issue that caused the game not to start on certain Linux distributions due to differences in libcurl.


# Release 0.21.0 (2018-04-26)

### Content
- Added the maps **small3ffa**, **small4ffa** and **small8ffa**. Unlike the old free-for-all maps whose size scaled with the number of players, these maps are roughly the same size as the *small1v1* map. The chaos thresholds now scale with the size of the map instead of with the number of players, so expect a lot of global warming on these maps.
- Removed the maps *oasis3ffa* and *beetle4ffa* from the map pool.

### Visuals & User Interface
- When an order is given to a unit or tile, the order appears below that unit or tile and floats to the order list.

### Bugfixes
- Fixed a bug where non-existing tile buildings could remain on the screen indefinitely after the tile had been revealed.
- Fixed a bug where question marks appeared above units that still had old orders to execute.
- Fixed a bug where no sound was played when an Industry or Barracks tile was upgraded.
- Fixed a bug where giving an order after pressing the Ready button caused that order to remain in the order list when the action phase started.
- Fixed a bug where shadows from dragged players in the lobby could persist when the game was started.
- Fixed a bug in the lobby menu where changing a player's color could move them to the observers.
- Fixed a bug where a disconnected player could leave behind a ghost in the lobby.
- Fixed bug where draggable items would remain dragged if clicked with a very low framerate.
- Fixed a compatibility issue on Arch Linux.


# Release 0.20.0 (2018-04-14)

### Gameplay
- Disabled fog of war for human players. Fog of war can be reenabled per player by switching between "global vision" (no fog of war) and "normal vision" (fog of war).

### Visuals & User Interface
- Added an in-game panel that displays which player goes first each round.
- Orders can be rearranged by clicking and dragging them. (**Issue #1**)
- Added a blue button to orders in the order list. Clicking the button on a new order revokes the order. Clicking the button on an old order issues a Stop order to override the old order. (**Issue #2**)
- The in-game camera starts out with your tiles and units in view.
- Added a texture and decorative mountains to the edges of the map.
- In the lobby, players can be dragged to the observers list and back.
- The "PLAY" button on the main menu now turns into an "UPDATE" button when a patch is available to be downloaded.
- The "PLAY" button's tooltip is now more informative and appropriate to the specific situation.
- In case you forgot your password, you can now reset it from the main menu.
- The login form now displays a confirmation of a succesful registration or password reset.
- If joining the server fails due to a locked account, the login form now displays the reason.

### Bugfixes
- Fixed a bug where the game does not fade in from black.
- Fixed a bug where the caret did not appear when a text input was selected.
- Fixed a bug where the "PLAY" button would be enabled inappropriately.
- Fixed a bug where the server would accept clients with old version to join.

### Technical
- Login sessions are now invalidated after a manual logout.
- Login sessions that should not be remembered are now always invalidated after logout or game close.
- You will now receive an email if you are resetting your password or when your account is locked from too many failed login attempts.


# Release 0.19.0 (2018-04-05)

### Accounts
- Users can no longer join the server multiple times at once with the same account.

### Visuals & User Interface
- Added a leaderboard tab to the multiplayer menu.
- Renamed "Camera Scrolling" to "View Scrolling" in the settings menu.
- The tutorial is less rigid and will allow you to continue as long as you have given a correct order, without having to also select that unit.
- Increased the animation speed of the Farm tile's cultivate ability.
- Improved error handling when joining server: the reason for failure (currently: session expired or already joined) is displayed in a login prompt.
- Changed the wording of the rating change notification.
- Added tooltip to start button when it is grayed out.
- Added coin icon before cost in the order context menu.
- Fixed the placement of text on different scale settings.
- Added some margin to the tooltips in the multiplayer menu.

### Bugfixes
- Fixed a bug in the animation timing of units that are attacked from within the fog of war.
- Fixed a bug where trees and buildings from the fog of war would remain on the screen for too long after the real tile was revealed.
- Fixed an issue where the rating change notification would sometimes not appear after a lobby was disbanded.
- Prevent crash when quickly starting new threads through repeated reconnection, login or registration.
- Fixed a bug where the players were too spaced out in the lobby after joining a replay lobby.
- Fixed a bug where sprites would show the wrong frame once every couple of draw steps in 32-bit Windows versions of the game.


## Hotfix 0.18.1 (2018-03-26)

*This is a server-only hotfix that is compatible with release 0.18.0.*

### Gameplay
- First player advantage is now properly randomized.

### Content
- Lowered the aggressiveness of the Easy RampantRhino, and increased the aggressiveness of the Hard RampantRhino.
- Improved the RampantRhino AI to prevent its units from blocking each other.


# Release 0.18.0 (2018-03-21)

### Accounts
- The game now requires an Epicinium account with username, email and password. This allows you to log in with the same account on different computers. (**Issue #33**)
- Usernames are now restricted to alphanumeric ASCII characters and the characters '_', '-', '.' and '~'. Support for usernames with non-ASCII characters (such as Cyrillic or Chinese) will be added in the future.
- Some players had issues with files not being saved correctly and were forced to create multiple accounts with slightly different usernames. We will manually merge the ranks of these accounts into a new Epicinium account.

### Gameplay
- Decreased the hitpoints of City, Town and Farm tiles to 2, down from 3.
- Decreased the cost of the City tile to 20, down from 30.
- Decreased the cost of upgrading a Town tile to a City tile to 15, down from 30.
- Industry and Barracks tiles can be upgraded to increase their size, for a cost of 30. At night, each Industry or Barracks tile gains power if the total power of surrounding City tiles is greater than its current power, but this does not automatically create a new building.
- Frostbite only appears on Grass, Dirt, Desert, Rubble, Soil, Crops and Water tiles.
- Frostbite no longer lowers hitpoints, and instead deals 2 shots of 1 damage to ground units in the Decay phase.
- Dirt tiles without humidity transform to Desert tiles if at least 40% of the map is covered by chaos, up from 20%.

### Content
- Reworked the *toad1v1*, *spruce1v1* and *oceanside1v1* maps to make expansion areas more interactive.
- Updated the Medium and Hard TutorialTurtle to be more difficult to beat, and to allow them to use Sappers.
- Added the RampantRhino AI, an aggressive variant of the TutorialTurtle.

### Visuals & User Interface
- Reworked the main menu and added login and registration functionality.
- The "Waiting for response from server..." notice time is changed from 5 to 10 seconds after sending ping.
- New notice "Connection to server resumed" added after receiving pong after the aforementioned 10 seconds.
- Added "Authenticating..." notice to multiplayer menu while waiting for confirmation from server.

### Technical
- A warning now appears in the main menu if the game detects that it cannot properly save configuration files on the system.
- Communication with the server now uses our new domain name, server.epicinium.nl, instead of using the IP address directly.
- Login and registration uses login.epicinium.nl.
- Login now works with sessions: a session token (with temporary validity) is issued while logging in, which is validated when joining the server.
- Account files are now used to (optionally) remember sessions.


## Hotfix 0.17.1 (2018-03-12)

*This is a server-only hotfix that is compatible with release 0.17.0.*

### Bugfixes
- Fixed an issue where replays from version 0.17.0 could not be viewed due to missing data.
- Fixed an issue where missing data while viewing a replay caused the lobby to crash.
- Fixed an issue where a single lobby crash caused the server to crash.


# Release 0.17.0 (2018-03-01)

*Due to a bug in the patcher, Windows users may need to update manually.*

### Gameplay
- Snow, Frostbite, Firestorm, Bonedrought and Death also update at the end of every Night phase.
- Death is placed on one random tile per player per Weather or Night phase, instead of two random tiles per player per Weather phase.

### Content
- Altered the bottom of the *spruce1v1* map slightly.

### Visuals & User Interface
- Added UI elements that show the hitpoints, movement speed, vision radius, attack damage, ability volleys, ability range and income of units and tiles under the cursor.
- At night, clouds of Gas appear transparent instead of invisible.
- Before a unit fires, its figures show a short aim animation.
- When a unit is triggered to fire because of a Focus Attack of another unit or because of an Attack of Opportunity, an exclamation mark pops up above each of its figures.
- When a moving unit is part of an Attack of Opportunity, it briefly stops moving.
- Units without orders have a question mark pop up until they are selected.
- Target guides now blink to help differentiate them from order guides.
- The color of humid grass is now consistent within each map, but varies between maps.
- Adjusted the colorization of dry grass to be more readable across all maps.
- The pause icon that appears in the tutorial can now be clicked to unpause the game.
- In the multiplayer menu's playerlist and lobby, the player's own username is now colored dark red to distinguish it from other usernames.

### Audio
- Added titlescreen music. (**Issue #11**)
- Increased overall loudness of the game.
- Decreased relative loudness of explosions and collapses.

### Bugfixes
- Fixed some bugfixes where the main menu would give incorrect or misleading information regarding the connection status.
- Fixed a bug where the settings would not be saved if settings.json was missing.

### Technical
- Added a notification for when the server takes more than 5 seconds to respond so the player knows what is going on.
- The game will now attempt to reconnect after the server has not responded for 33 seconds, instead of 66 seconds.
- The game will now always attempt to reconnect when the connection closes unexpectedly, instead of only in the case where the server seems up but does not respond to ping.
- The game will now keep attempting to reconnect after losing connection.
- Improved behavior and information when the server is shutting down for maintenance.
- Made the audio mixer more efficient to prevent audio playback issues on slow machines.
- Fixed a bug that prevented patches from being installed correctly under Windows.


# Release 0.16.1 (2018-02-16)

### Visuals & User Interface
- Added a settings menu. (**Issue #3**)

### Technical
- Removed unused rulesets from the *rulesets* folder.
- Reduced RAM usage by about 75%.


# Release 0.16.0 (2018-02-09)

### Gameplay
- Mountain tiles now cause nearby spaces to start with 4 humidity.
- Water tiles no longer cause nearby spaces to start with more than 3 humidity.
- Snow also occurs in Spring, Summer and Autumn on spaces with 4 humidity.
- Aridification also lowers the humidity of surrounding tiles.
- Firestorm occurs in Summer at random on 24% of the map for every 40% of the map that is covered by chaos, instead of 8% for every 20%.
- Bonedrought occurs on Desert tiles if at least 60% of the map is covered by chaos, up from 40%.
- Bonedrought also occurs on City, Town, Farm, Barracks, Industry, Airfield and Rubble tiles if at least 80% of the map is covered by chaos.
- Death is placed on two random City, Town, Farm, Barracks, Industry or Airfield tiles for every player, up from one per player.
- If a tile or unit is affected by multiple hitpoint modifiers, such as Frostbite and Bonedrought, its hitpoints can be lowered by more than 1, but not below 1.

### Visuals & User Interface
- Swapped the positions of the chaometer and the weather marker gauge.
- Added an in-game panel that displays which player is playing as what color. Access this panel by clicking on the diamond button in the top left of the screen. For observers, a similar panel can be accessed by clicking the coin button; this panel also shows how much money each player has.
- The "start game" button is disabled when there are not enough players to start a game.
- Added text to main menu that displays when you have been disconnected from the server and tells you to press 'PLAY' to rejoin.

### Audio
- Added a delay between building destruction sounds to reduce the loudness of Upgrade and Death animations.

### Bugfixes
- Fixed GPU memory leak that could cause the game to crash. (**Issue #32**)
- Fixed bug where the chaometer indicated values slightly lower then intended.
- Fixed bug where the chaometer could grow beyond its container.
- Fixed bug where Dirt turns to Desert before 20% of the map is covered by chaos.
- Fixed bug where AI difficulty was not synchronized correctly when a player joined a lobby.
- Fixed bug where orders would sometimes not be sent at the end of the planning phase if a player did not press the Ready button.

### Technical
- If the game was installed via the Itch.io desktop application or via the GameJolt desktop application, the game no longer patches itself to prevent extraneous patching.


## Hotfix 0.15.1 (2018-02-02)

*This is a server-only hotfix that is compatible with release 0.15.0.*

### Bugfixes
- ~~Fixed bug where Dirt turns to Desert before 20% of the map is covered by chaos.~~

### Technical
- When all players are disconnected from a non-tutorial game in progress, the lobby is not disbanded immediately but remains alive for ten minutes.


# Release 0.15.0 (2018-01-27)

### Gameplay
- Removed temperature from the game.
- Humidity ranges from 0 to 4, instead of from 0 to 100. Water and Mountain tiles cause nearby spaces to start with more than 2 humidity. Desert tiles can cause nearby spaces to start with only 1 humidity.
- Each space can have up to 1 chaos counter. Chaos counters no longer influence the weather on the spaces they are on; only the total number of chaos counters matters. When a chaos counter would be placed on a space that already has a chaos counter, it is placed on a random other space instead.
- Aridification occurs at random on 8% of the map for every 20% of the map that is covered by chaos.
- Aridification lowers humidity by 1, instead of by 5 percent point.
- Aridification no longer lowers the humidity of surrounding tiles.
- Industry tiles lower humidity of surrounding tiles by 1, instead of by 10 percent point. This only occurs in Autumn.
- Gas lowers humidity by 1, instead of by 10 percent point.
- Degradation occurs when a Grass tile has no humidity.
- Desertification occurs when a Dirt tile has no humidity and at least 20% of the map is covered by chaos.
- Snow occurs in Winter on all spaces with at least 1 humidity.
- Frostbite occurs in Winter on spaces without buildings or trees on them if at least 20% of the map is covered by chaos, on spaces with 1 building or tree on them if at least 40% of the map is covered, etcetera.
- Firestorm occurs in Summer at random on 8% of the map for every 20% of the map that is covered by chaos. Firestorm avoids City, Town, Farm, Barracks, Industry and Airfield tiles if possible.
- Bonedrought occurs on Desert tiles if at least 40% of the map is covered by chaos.
- Once 100% of the map is covered by chaos, Death is placed at random on one random City, Town, Farm, Barracks, Industry or Airfield for every player every Growth phase. Once placed, Death never disappears.
- Reduced chaos emission of Industry tiles to 5, down from 25.
- Reduced chaos emission of City tiles to 2, down from 5.
- Reduced chaos emission of Town, Farm, Barracks, Airfield and Rubble tiles to 1, down from 2.

### Content
- Altered the *spruce1v1* map slightly to make the top area less restrictive.
- Cropped the *oasis3ffa* map and expanded the *beetle4ffa* map so they have the correct number of tiles for the new chaos system.

### Audio
- Unit spawn sounds are no longer played during a rejoin.
- Foot soldier spawn is now three footsteps instead of one.
- Separate volumes for gameplay and music.
- Made volume behave exponentially.
- Made panning behave in accordance to a constant-power rule.

### Visuals & User Interface
- Removed the thermometer.
- The hygrometer now has four segments to indicate how many humidity a space has.
- A new "chaometer" shows a known lower bound and possible upper bound of the total number of chaos counters, based on your current vision of the map.
- The camera no longer pans to show enemy cities gaining power.
- The chat window expands to cover most of the screen, and only shows a small preview when collapsed.

### Bugfixes
- Fixed a bug where buildings and trees could be improperly placed.


# Release 0.14.0 (2018-01-20)

### Gameplay
- The City tile now generates income.
- The Industry tile generates three income per powered stack, instead of just one per powered stack.
- Increased the cost of the City tile to 30, up from 20.
- Increased the cost of the Industry tile to 3, up from 0.
- Increased the cost of the Barracks tile to 3, up from 2.
- Increased the cost of the Rifleman unit to 10, up from 3.
- Increased the cost of the Gunner unit to 10, up from 6.
- Increased the cost of the Sapper unit to 10, up from 8.
- Increased the cost of the Tank unit to 15, up from 5.
- Increased the starting income to 20, up from 10.

### Content
- Improved phrasing of certain prompts in the in-game tutorial, and added new prompts about building and using Barracks.

### Audio
- Improved the timing of the the tank engine sound.

### Visuals & User Interface
- Unit figures are positioned more towards the center of the tile.
- Guides that appear when a Capture, Shape, Settle, Upgrade or Produce order is given are now blue instead of orange.

### Bugfixes
- Fixed an issue where joining a replay in progress would skip through the entire recording at once.
- Made the audio code thread-safe, preventing some rare crashes.
- Fixed issue where after playing the tutorial, the tutorial map would be listed in the map picker.
- Fixed a bug where clicking on unpathable tiles would generate invalid move orders.

### Technical
- When a player reconnects to the server after being disconnected from a game against human opponents, the player automatically rejoins the game.
- Framerate and audio volume can now be adjusted by editing *settings.json*.
- The client and the server now ping each other more often to keep the TCP connection alive.


## Hotfix 0.13.0 (2018-01-12)

*Due to a bug in the patcher, updating from v0.11.0 or earlier to v0.12.0 would result in a broken version of the game. This hotfix should allow players to first patch to v0.12.1 and then to v0.13.0.*

### Bugfixes
- Fixed an issue where the patcher could not install the audio folder.

### Technical
- The game is now able to patch the patcher before the patcher patches the game.


# Release 0.12.0 (2018-01-11)

### Gameplay
- Decreased the maximum size of the Rifleman unit to 3, down from 5.
- Decreased the cost of the Farm tile to 2, down from 5, and increased its maximum size to 2, up from 1. The Farm tile can now produce Rifleman units.
- Settling a Farm tile now immediately cultivates surrounding tiles into Soil tiles. Once built, a Farm tile can still be given a cultivate order to repair any destroyed Soil tiles.
- Decreased the cost of the Town tile to 5, down from 10.
- Decreased the cost of the Industry tile to 0, down from 2.
- Decreased the cost of the Tank unit to 5, down from 10.
- Decreased the starting income to 10, down from 20.
- Crops tiles are now considered natural (like Water tiles) when evaluating if a City, Town of Farm tile gains power.
- Aridification (the humidity loss that occurs in cells with extreme heat) also lowers the humidity of all surrounding cells, but decreased the humidity loss for humid cells to 5, down from 10.
- Ground pollution (the humidity loss caused by Industry tiles) now occurs in all seasons, not just in Autumn.

### Content
- Players no longer start with a Town tile but instead have an extra Rifleman unit of size 1, and City tiles start with 1 power instead of 2.
- The in-game tutorial now has prompts to guide the player and takes place on a new map with a new AI. (**Issue #7**)
- Updated QuickQuack to do farming because gameplay changes made it necessary to do economy.
- Updated HungryHippo to produce and control tanks.

### Visuals & User Interface
- Added auto-pathing assistance; when issuing a Move order to a ground unit, it will now attempt to find a way around any unpathable terrain.
- A message now appears for all players and observers whenever a player is defeated.
- Added tooltips for the players' in-game and in-lobby icons in the multiplayer menu.

### Audio
- Added sound effects during gameplay. (**Issue #12**)
- Added cue for beginning of planning phase. (**Issue #16**)

### Bugfixes
- Fixed a bug where replays from version 0.10.0 and older did not work.
- Fixed a rare bug where valid orders were discarded for no reason.
- Fixed an issue in free-for-all games where the server would wait for defeated players to ready up.
- Fixed a bug where players could still control their units locally after being defeated.
- Fixed a bug where players could not send messages with quotes in them.
- Fixed a bug where the camera would fly past a tile being captured instead of stopping and showing it.


# Release 0.11.0 (2017-12-23)

### Content
- Added the map **oasis1v1**.
- Added the free-for-all maps **oasis3ffa** and **beetle4ffa**.

### Visuals & User Interface
- Reworked Rifleman and Gunner unit sprites.
- Reworked surface textures and added blending between surfaces.
- Increased the speed at which the diamond icons appeared.

### Bugfixes
- Fixed a bug where unit figures would keep moving instead of dying when taking lethal damage.
- Fixed a bug where players were able to generate broken orders by clicking on unpathable tiles, which were then discarded by the server. (**Issue #30**)
- Fixed a bug where a starting a game while a chat message was being typed would cause that message to be sent once that player started typing again. The message is now discarded. (**Issue #29**)
- Fixed a bug where observers watching a replay that ends in the planning phase, would not see the "game over" message.
- Fixed a bug where HungryHippo would issue invalid Focus orders.

### Technical
- Reduced the size of the data sent to observers by about 80 percent.


## Hotfix 0.10.1 (2017-12-18)

*This is a server-only hotfix that is compatible with release 0.10.0.*

### Bugfixes
- Fixed issue where the tutorial opponent would be QuickQuack instead of DemoDuck.


# Release 0.10.0 (2017-12-15)

### Gameplay
- When a player leaves a game in progress, they resign. Once resigned, a player can rejoin the lobby as an observer but can no longer win.

### Content
- Reworked the maps *toad1v1*, *spruce1v1*, *small1v1* and *oceanside1v1* to be all have the same number of starting City tiles, Town tiles and Rifleman units.
- In a lobby, it is now possible to change the AI and difficulty after adding an AI opponent.
- Updated both AIs slightly, mostly to respond to difficulty setting.

### Visuals & User Interface
- After a match has ended, a diamond icon pops up over each tile that is worth points.

### Bugfixes
- Fixed a bug where the camera would not always pan when Crops tiles generated income at night.
- Fixed bug where player ratings in the multiplayer menu would not always update.

### Technical
- Improved server responsiveness when a player joins a game in-progress.
- Reduced the size of the data sent by the server at the start of a game by about 40 percent.
- The game ceases drawing if its window is hidden or minimized. (**Issue #28**)


# Release 0.9.1 (2017-12-07)

### Gameplay
- After the game ends, the entire map is revealed to all players.

### Visuals & User Interface
- Added an extra bar with icons for each of the weather effects. These icons light up when that weather effect is active and show a tooltip explaining their effects when hovered.
- Made thermometer (temperature gauge) and hygrometer (humidity gauge) larger.
- Added tooltips to the various markings on the thermometer and hygrometer.
- The in-game chat box can now be clicked to activate it.
- Increased the visual difference between 1 stack of Gas and 2 stacks of Gas.
- Tweaked the colors of the Gunner's machine gun.

### Bugfixes
- Fixed a bug where the camera would not scroll down when the cursor was at the bottom of the screen on certain resolutions. (**Issue #24**)
- Fixed a bug where the cursor displayed a waiting animation at the end of a game.
- Fixed a bug where Industry tiles could lower the humidity of surrounding tiles below 35 percent.


# Release 0.9.0 (2017-12-04)

### Gameplay
- The **Town** tile replaces the Settlement tile. The Town tile can have up to 5 buildings, generates income and can produce Settler units, but cannot produce Rifleman units.
- Added the **Farm** tile. The Farm tile can produce Settler units and can turn all tiles around it into **Soil** tiles. In the growth phase, a Soil tile transforms into a **Crops** tile if the temperature and humidity support plant growth. The Crops tile is consumed in the night phase, generating 1 income for its owner and turning back into a Soil tile. Soil tiles and Crops tiles leave Dirt behind when destroyed.
- The Settler unit no longer builds Settlement tiles. Instead, it can settle either a Town tile, a City tile or a Farm tile.
- The City tile no longer generates income.
- Barracks and Industry tiles require nearby City tiles to grow, and thus no longer benefit from nearby Settlement/Town tiles.
- City tiles and other structure tiles now start with 1 power when they are created, instead of 0.
- Increased the cost of City tiles to 20, up from 10.
- Decreased the cost of Barracks and Industry tiles to 2, down from 5.
- Decreased the starting income to 20, down from 50.
- City tiles and other structure tiles now leave **Rubble** behind when destroyed, instead of Stone. Rubble cannot be built upon.
- Desert can no longer be built upon.
- Added **Ridge** tiles. Ridge tiles function the same as Mountain tiles except they are not as cold by default.
- In the spring growth phase, Grass no longer regrows from Dirt.
- Increased the maximum temperature at which trees and crops will grow to 39 degrees, up from 29 degrees.
- Reduced randomness in the starting values of temperature and humidity so they are more reliable.
- A Rifleman unit now always succeeds when capturing a tile, regardless of its size.
- When a tile produces a non-Settler unit, the tile now loses all of its power, not just 1.

### Content
- Reworked the maps *toad1v1*, *spruce1v1* and *small1v1* to be less cold and less humid by replacing some Water and Mountain tiles with Ridge tiles.
- Removed the maps *peaktorn1v1* and *snowcut1v1* from the map pool.
- Added the map **oceanside1v1**.
- Updated the AIs for new gameplay changes and wrote new one that farms (HungryHippo).

### Visuals & User Interface
- Added impact shake to various attacks and damage abilities.
- Added camera shake on various attacks and abilities.
- Improved timing of the Shell ability of Tank units with less than three tanks.
- Differences in grass color (caused by temperature or humidity differences) are now more distinct, which should improve their readability.
- Tweaked the colors of Dirt, Desert and Mountain tiles.
- When displaying simultaneous effects such as the hearts and coins at night, if not all effects fit on the screen at once, the camera will pan multiple times.

### Bugfixes
- Fixed bug where a player could appear to remain online indefinitely after their game crashed. (**Issue #25**)
- Fixed bug where the planning phase would last its entire duration even though both players were ready. (**Issue #27**)
- Properly fixed a bug where a figure could disappear before its death animation had finished.
- Fixed bug where Tank death animation would not play if the tank had just finished moving.
- Fixed bug where user's rating would not update in the UI after a game was over.
- Fixed bug where the multiplayer menu would be in an invalid state after returning to main menu and going back.
- Fixed minor bug where the chat indicatator would keep saying NAME after renaming lobby.


# Release 0.8.0 (2017-11-23)

### Accounts
- The game will ask you for a username the first time you log in with version 0.8.0 and automatically log you in with that username afterwards. To log in with that username on a different computer, you need to copy the *.acc* file from the *accounts* folder. To log in as a different user, you need to override the username in the settings.
- The server now tracks **rating points** for each player. All players start with 0 rating points. Every game you play causes you to gain rating points if the score you obtain at the end is higher than your rating, and lose points if your score is lower.

### Gameplay
- Tank units now deal trample damage to tiles when they move onto them, turning Grass to Dirt and destroying trees. A Tank unit with three tanks can even bring down buildings.
- Lowered default humidity from 65 percent to 55 percent.
- Lowered the maximum temperature at which grass and trees will grow from 39 degrees to 29 degrees.
- Stone tiles now cause residual global warming.

### Content
- Reworked the maps *toad1v1*, *peaktorn1v1*, *snowcut1v1*, *spruce1v1*, *small1v1* and the tutorial map, making them more compact and fixing the total number of Grass and Forest tiles on each map to be exactly 100.
- It is now possible to see and send 'All' chat in game as well.

### Visuals & User Interface
- Added title logo to the main menu.
- Added connection icon to the top right of the main menu that indicates whether the game is connected to the server and whether a patch is available for download.
- Added icons to indicate whether a user is in a lobby or in a game.
- Added an icon to indicate whether a lobby is private.
- Added tooltips to the phase icons and the order icons. (**Issue #22**)
- Tweaked how colors look at night.

### Bugfixes
- ~~Fixed a bug where a figure could disappear before its death animation had finished.~~
- Fixed a bug where night phase mood indicators (e.g. pink hearts) were not displayed in observer mode and replay mode.
- Fixed a bug where disconnected observers could not rejoin the lobby if it was private.

### Technical
- The game is now able to patch itself. If a patch is available, the connection icon in the top right of the main menu will show an exclamation mark. Clicking this icon will cause the game to download the necessary files. Once the download is complete, clicking the icon again installs the patch and restarts the game.
- In AI vs AI games and replays, the server now waits between action phases until all observers have finished animating.
- Improved the responsiveness of rejoining a match in progress.
- Improved the stability and responsiveness of replay mode.
- Improved resolution detection when resolution is not specified in the settings. (**Issue #23**)

### Other
- We now have a Discord bot that posts when users go online or offline.


## Hotfix 0.7.2 (2017-11-07)

*This is a server-only hotfix that is compatible with release 0.7.0.*

### Content
- Modified the tutorial map and AI to be less overwhelming.

### Bugfixes
- Fixed issue where replays were available before the match had finished.
- Fixed possible issue that could cause the game to crash when receiving multiple messages from the server simultaneously.


## Hotfix 0.7.1 (2017-11-05)

*This is a server-only hotfix that is compatible with release 0.7.0.*

### Bugfixes
- Fixed a bug where the player could spawn on the wrong side of the map when playing the tutorial online, which made it more difficult than intended.
- Fixed a bug where colors assigned to an AI player would be permanently unavailable after the AI player was removed from the lobby.


# Release 0.7.0 (2017-11-04)

### Content
- Removed singleplayer modes from the main menu.
- Tutorial mode can be accessed by clicking the "start tutorial" button once connected to the server.
- Added AI opponents. Once inside a lobby, add an AI player by clicking the blue plus and selecting "Add AI player". To remove them, click the blue dots and select "Remove AI player".
- Added observer mode. To turn a player into an observer, click the blue dots and select "Move to observers". To move them back, click the blue dots again and select "Move to players". Users that join a lobby after a match has already begun automatically become observers.
- Added replay mode. Click the "watch replay" button to create a Replay lobby.
- Added lobby privacy. Private lobbies cannot be joined by other users.
- New lobbies are now automatically assigned a name by the server, but can be renamed with the "rename" button.
- Separated "Lobby" chat from "All" chat. During matches "All" chat is disabled.
- Renamed *tiny1v1* map to *small1v1*.

### Visuals & User Interface
- Revamped the lobby screen.
- Added dropdown boxes for selecting map, planning time and player colors.
- Tweaked the style and colors of various interface elements.

### Bugfixes
- The game now no longer continues after a player has been declared victorious.
- Fixed visual issues when rejoining a match in progress.
- Fixed issue where tiles destroyed during combat would rebuild themselves at night.
- Fixed issue where new trees popped up either with a very large delay or with no delay at all.
- Fixed issue where clicking an interface element also caused units and interface elements behind it to be clicked.
- Fixed issue where the text of an interface element was drawn over an interface element in front of it.


# Release 0.6.0 (2017-10-20)

### Gameplay
- A Tank unit executing a Shell order now fires an additional volley after the first.
- Simplified the final score: each Grass and Forest tile is worth 1 point.
- Dirt, Desert and Mountain tiles are no longer considered natural when evaluating if the City tile gains power.
- Tile buildings can now be destroyed when taking lethal damage in a targeted damage step.
- Gas no longer hits tiles or lowers the hitpoints of tiles.
- Gas causes the humidity to drop by 10 percent after it spreads, but not below 39 percent.
- A ground unit that moves through snow has its movement speed slowed *by* 1, instead of *to* 1.
- A ground unit that moves *out of* a Trenches tile no longer has its movement speed slowed.
- A ground unit that moves *onto* a Trenches tile stops moving and becomes entrenched if able. If the moving unit is a Tank or would be bypassing a friendly unit, it can continue moving without being slowed.
- A ground unit that is attacked while moving out of a Trenches tile no longer benefits from the protection of the Trenches.
- A ground unit that is being attacked after performing a bypass attack from a Trenches tile does not benefit from the protection of the Trenches.

### Content
- Forest tiles now start with fewer trees.
- Added new map: **toad1v1**.
- Removed *twolakes1v1* from the map pool.

### Visuals & User Interface
- Improved animations when tiles are transformed or destroyed.
- Move guides now show how much the unit is predicted to be slowed by colorizing that many move guides blue instead of gray.
- It should now be more apparent when an order cannot be issued because the order limit has been reached.
- The mouse cursor is now always visible when hovering above a UI element.
- The camera can no longer fly too far beyond the edges of the map. (**Issue #6**)
- Fullscreen mode now forces native desktop resolution. (**Issue #19**)
- Chat text now wraps instead of extending past its container.
- Added a blinking caret to text input fields whose position is controlled by the left/right arrow keys.
- Added automatic horizontal scrolling to text input fields when the text gets longer than its container.

### Bugfixes
- Properly fixed issue where the camera would not pan when a zeppelin dropped gas.
- Fixed issue where the Quit popup was unresponsive if the chat window was open.

### Technical
- Added multithreading to prevent unresponsiveness while connecting to the server.

### Other
- Usernames are now required to be between 3 and 36 characters in length.


# Release 0.5.0 (2017-10-16)

### Content
- Made the tutorial AI produce weaker riflemen.
- Screen edge camera scrolling is now on by default.
- Camera scrolling options can be overridden in the settings.

### Bugfixes
- Fixed issue preventing 32-bit and OS X from connecting to the server. (**Issue #18**)
- Fixed errors preventing startup on OS X. (**Issue #17**)
- ~~Fixed issue where the camera would not pan when a zeppelin dropped gas.~~
- Fixed issue where figure footprint (selection circle) lingered after death.
- Fixed issue where the multiplayer menu was in a wrong state after the player left a match.

### Other
- Added these release notes.


# Release 0.4.4 (2017-10-12)

### Content
- Added a simple AI opponent for singleplayer. (**Issue #9**)
- Added a simple AI opponent for the tutorial.
- Added credits.

### Visuals & User Interface
- Changed the Ready button checkbox.
- Changed font to Munro, which has a more permissive license.

### Bugfixes
- Changed all references from Aftermath (working title) to Epicinium.

### Other
- Hardcoded font into the binary.


# Release 0.4.2 (2017-10-09)
*First beta version.*
