---
layout: default
---
# [](#iqpuzzle)iQPuzzle
iQPuzzle is a diverting I.Q. challenging pentomino puzzle. Pentominos are used as puzzle pieces and more than 300 different board shapes are available, which have to be filled with them.

![alt tag](https://cn.pling.com/img/f/a/d/d/7841359a95432f690d8c2bf7a5e9723770f7.png)

# [](#createlevel)Create your own level
Manual for creating own levels can be found [here](create-level). Feel free to create a pull request [on GitHub](https://github.com/ElTh0r0/iqpuzzle) and your level might be included in the next release!

# [](#downoads)Downloads
###### Latest release v1.1.1 (06. May 2018)
* [Build for Windows](https://github.com/ElTh0r0/iqpuzzle/releases/download/v1.1.1/iQPuzzle-1.1.1-Windows.zip)
* [Build for macOS](https://github.com/ElTh0r0/iqpuzzle/releases/download/v1.1.1/iQPuzzle-1.1.1-macOS.zip)
* [Linux AppImage](https://github.com/ElTh0r0/iqpuzzle/releases/download/v1.1.1/iQPuzzle-1.1.1-x86_64.AppImage)
* [Ubuntu PPA](https://launchpad.net/~elthoro/+archive/iqpuzzle)
* [Builds for Arch, Cent OS, Debian, Fedora, openSUSE](http://software.opensuse.org/download.html?project=home%3AElThoro&package=iqpuzzle)
* [Build for Mageia Cauldron](https://madb.mageia.org/package/show/name/iqpuzzle/release/cauldron/application/0)
* [Gentoo ebuild](https://github.com/ElTh0r0/iqpuzzle/tree/packaging/ebuild)
* [Build for OS/2](http://www.ecsoft2.org/iqpuzzle)
* [Build for ReactOS](https://github.com/ElTh0r0/iqpuzzle/releases/download/v1.1.1/iQPuzzle-1.1.1-ReactOS.zip)

Note: The **macOS** release was created automatically on a build server and is untested due to missing hardware! If you are facing any trouble, [please create an issue](https://github.com/ElTh0r0/iqpuzzle/issues).

## [](#release-notes)Release notes
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
