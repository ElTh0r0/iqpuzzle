---
layout: default
---
# [](#iqpuzzle)iQPuzzle
iQPuzzle is a diverting I.Q. challenging pentomino puzzle. Pentominos are used as puzzle pieces and more than 300 different board shapes are available, which have to be filled with them.

![Screenshot](https://user-images.githubusercontent.com/26674558/93668022-33d13180-fa8a-11ea-9279-b8f4d8c5c217.png)

## [](#downoads)Downloads
###### Latest release v1.3.1 (01. May 2024)
* [Build for Windows - Installer](https://github.com/ElTh0r0/iqpuzzle/releases/download/v1.3.1/iQPuzzle-1.3.1-Windows_Installer.exe)
* [Build for Windows - ZIP](https://github.com/ElTh0r0/iqpuzzle/releases/download/v1.3.1/iQPuzzle-1.3.1-Windows.zip)
* [Build for macOS](https://github.com/ElTh0r0/iqpuzzle/releases/download/v1.3.1/iQPuzzle-1.3.1-macOS.zip)
* [Linux AppImage](https://github.com/ElTh0r0/iqpuzzle/releases/download/v1.3.1/iQPuzzle-1.3.1-x86_64.AppImage)
* [Flatpak](https://flathub.org/apps/details/com.github.elth0r0.iqpuzzle)
* [Ubuntu PPA](https://launchpad.net/~elthoro/+archive/iqpuzzle)
* [Builds for Cent OS, Debian, Fedora, openSUSE, SLE](http://software.opensuse.org/download.html?project=home%3AElThoro&package=iqpuzzle)
* [Arch AUR](https://aur.archlinux.org/packages/iqpuzzle/)
* [Build for Mageia Cauldron](https://madb.mageia.org/package/show/name/iqpuzzle/release/cauldron/application/0)
* [Gentoo ebuild](https://github.com/ElTh0r0/iqpuzzle/tree/packaging/ebuild)
* [Build for OS/2](http://www.ecsoft2.org/iqpuzzle)
* [Build for ReactOS](https://github.com/ElTh0r0/iqpuzzle/releases/download/v1.3.1/iQPuzzle-1.3.1-ReactOS_Installer.exe)
* [FreeBSD Ports](https://www.freshports.org/games/iqpuzzle) / [DPorts](https://github.com/DragonFlyBSD/DPorts/tree/master/games/iqpuzzle)

Note: The **macOS** release was created automatically on a build server and is untested due to missing hardware! If you are facing any trouble, [please create an issue](https://github.com/ElTh0r0/iqpuzzle/issues).

## [](#translations)Translations
New translations and corrections are highly welcome! You can either fork the source code [from GitHub](https://github.com/ElTh0r0/iqpuzzle), make your changes and create a pull request or you can participate [on Transifex](https://www.transifex.com/elth0r0/iqpuzzle/).

## [](#createlevel)Create your own level
Manual for creating own levels can be found [here](create-level). Feel free to create a pull request [on GitHub](https://github.com/ElTh0r0/iqpuzzle) and your level might be included in the next release!

## [](#release-notes)Release notes
###### v1.3.1 (01. May 2024)
* Add extended board selection dialog
* Bundle light/dark icons
* Update application icon
* Add support for dark scheme on Windows with Qt 6.5
* Add Cmake support
* Linux packaging fixes: AppId, metainfo

###### v1.3.0 (21. Apr 2024)
* Release was revoked due to packaging issues. Use v1.3.1 instead!

###### v1.2.9 (09. Feb 2023)
* Add Portuguese pt and pt_BR translation (thanks to UchidoF)

###### v1.2.8 (10. Dec 2022)
* Add Chinese (zh_TW) translation (thanks to liulitchi)

###### v1.2.7 (14. Nov 2022)
* Add Greek translation (thanks to liulitchi)

###### v1.2.6 (12. Feb 2022)
* Add Chinese (zh_CN) translation (thanks to liulitchi)
* Add Italian man page and fix for translation (thanks to davi92)

###### v1.2.5 (03. Jun 2021)
* Add Norwegian translation (thanks to Allan Nordhoy)
* Update Italian translation (thanks to albanobattistella)

###### v1.2.4 (24. Apr 2021)
* Add support for Qt6
* Fix disappearing block borders

###### v1.2.3 (20. Mar 2021)
* Updated Dutch translation (thanks to Vistaus)

###### v1.2.2 (06. Mar 2021)
* Fix for oddities barrier color if system background color is used
* Packaging fixes

###### v1.2.1 (29. Nov 2020)
* Fix crash during unsolved boards list update
* Fix background color on pause screen if system background color is used
* Fixes for Qt 5.15
* Some Clazy code improvements.

###### v1.2.0 (15. Mar 2020)
* 21 new boards
* Option to use system window background color as board background.
* Fix restarting a saved game.
* Some Clang-Tidy and Clazy code improvements.

###### v1.1.5 (02. Nov 2019)
* Added Korean translation (thanks to hyuna1127)
* Add save game file extension automatically

###### v1.1.4 (19. Oct 2019)
* Fix for AppImage: Save/load relative board path
* Dropped support for Qt4

###### v1.1.3 (29. Sep 2019)
* Added Italian translation (thanks to davi92)

###### v1.1.2 (18. Aug 2019)
* Official build for ReactOS and Arch AUR.
* Minor code improvements.

###### v1.1.1 (06. May 2018)
* Updated Dutch translation (thanks to Vistaus).
* Added freestyle board.
* Fix: Keep zoom level while board restart.
* Some code improvements.

###### v1.1.0 (18. Jan 2018)
* Start random un-/solved easy/medium/hard game.
* Added freestyle mode.
* Added statistics.
* Improved window resize.
* Added many new boards.
* Start game through cmd.
* Updated French translation (thanks to kiarn).
* Embedded translations into game resource.

###### v1.0.7 (12. Aug 2017)
* Added French translation, thanks to kiarn and mothsART!

###### v1.0.6 (06. May 2017)
* Updated Dutch translation, thanks to Elbert!

###### v1.0.5 (03. May 2017)
* Moved project to GitHub (exchanged URLs).
* Compiled with Qt 5.8 for Windows

###### v1.0.4 (03. Jan 2017)
* Fixed invalid board polygons.
* Added fallback menu icons.

###### v1.0.3 (02. Jan 2017)
* Added Dutch translation (thanks to Elbert)
* Fixed: Starting board from command line.
* Fixed: Bring blocks to foreground during rescale.
* Improved board file validation.
* Change gui language without restarting the game.

###### v1.0.2 (06. Nov 2016)
* Lintian packaging fixes.

###### v1.0.1 (05. Nov 2016)
* Added resources for OS/2; thanks to Gianfilippo!
* Improved settings dialog (check for using button multiple times)

###### v1.0.0 (23. Oct 2016)
* Start random game.
* Guess user name for highscore.
* Many new boards.
* Install into /usr/bin instead of /usr/games
* Simple collision check after rotation/flip.
* Removed fixed window size.
* Bugfix: Disable save when game solved. MSVC compiler issue.

###### v0.9.0 (13. May 2015)
* Added settings dialog; mouse buttons; GUI language.
* Added Bulgarian translation (thanks to bogo1966)
* Bug fix: Highscore
* Bug fix: Wrongly solved, if piece intersects board and not all pcs needed.
* Added new boards.

###### v0.8.1 (30. Aug 2014)
* Fixed translations and XPM icon file.

###### v0.8.0 (29. Aug 2014)
* Added highscore.
* Added new boards.

###### v0.7.0 (27. Jul 2014)
* Added timer and moves counter.
* Lintian fixes.
* Added new boards.

###### v0.6.0 (12. Jul 2014)
* Added save/load game option.
* Added new boards.

###### v0.5.0 (06. Jul 2014)
* Automatic check, if puzzle is solved.
* Added new boards.

###### v0.4.0 alpha (06. Jun 2014)
* Added preview to load board dialog.
* Added new boards.

###### v0.3.0 alpha (19. May 2014)
* Added gui translation.
* Corrected block position after flip/rotate.
* Corrected board path.
* Added new board and possibility for barriers.

###### v0.2.0 alpha (18. May 2014)
* Load board via command line.
* Define color as hex.
* Added new board.

###### v0.1.0 alpha (17. May 2014)
* Load board file dialog.
* Added CBoard class.
* Minor collsion check correction.

###### v0.0.1 alpha (04. May 2014)
* Define board as polygon.
* Support for Qt 4 and Qt 5.
* Code reconstructions, rewritten collision detection.
