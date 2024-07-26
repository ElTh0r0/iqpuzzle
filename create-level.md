---
layout: default
---

[back](./)
# [](#create-your-own-level)Create your own level
Own puzzle levels can be created as a text file stored with **.conf** file extension. Afterwards they can be loaded via _"Game -> Choose new game -> Open own board..."_. As an example, the following parts belonging to level **pento_T.conf**:

```ini
[General]
GridSize=25
BGColor="#EEEEEE"
PossibleSolutions=106
NotAllPiecesNeeded=true
Freestyle=false

[Board]
Polygon="0,0 | 9,0 | 9,9 | 0,9 | 0,0"
Color="#FFFFFF"
BorderColor="#2E3436"
GridColor="#888A85"

##  X X X X
##  X
[Block1]
Polygon="0,0 | 4,0 | 4,1 | 1,1 | 1,2 | 0,2 | 0,0"
Color="#3465A4"
BorderColor="#000000"
StartPos="-3,-5"

[Block2]
...

[Barrier1]
Polygon="0,0 | 3,0 | 3,6 | 0,6 | 0,0"
Color="#000000"
BorderColor="#000000"
StartPos="0,3"

[Barrier2]
...
```

Explanation of the different file sections:

| General |   |
| --- | --- |
| GridSize | Decimal scale factor (default: 25). This parameter defines the size of a board field in pixels. |
| BGColor | Background color around the board as hex value (default: "#EEEEEE") |
| PossibleSolutions | Optional, if known: Number of possible solutions |
| NotAllPiecesNeeded | Optional, `true` or `false`, depending if all pieces are needed for the solution or not (default: `false`) |
| Freestyle | Optional, `true` or `false`. Dis-/enable free puzzling mode - neither counting moves nor checking if puzzle is solved (default: `false`) |

| Board |    |
| --- | --- |
| Polygon | Board definition as polygon (more information see [below](#polygon)). The board has to be rectangular! |
| Color | Background color of the board as hex value |
| BorderColor | Border color of the board as hex value |
| GridColor | Color of the board grid as hex value |

Puzzle pieces / barriers numbered from  1 to N:

| BlockN / BarrierN |   |
| --- | --- |
| Polygon | Shape of a puzzle piece/barrier as orthogonal polygon; e.g.: "0,0 \| 3,0 \| 3,1 \| 1,1 \| 1,3 \| 0,3 \| 0,0" |
| Color | Color of piece/barrier as hex value |
| BorderColor | Border color as hex value |
| StartPos | Position "x, y" at game start (see below information about the [coordinate system](#coordinate-system)). Reference point is the top left corner of the piece. |

| Comments |   |
| --- | --- |
| \#\# Comment | If needed, comments can be inserted with leading \#\# |

## [](#coordinate-system)Coordinate system
The top left corner of the board defines the origin of the used coordinate system. Points are defined as "x, y" (x-axis horizontal to the right, y-axis vertical downward). It is allowed to use negative values as start positions for the pieces.

![Graticule](https://media-cdn.ubuntu-de.org/wiki/thumbnails/b/b5/b53e9caa9d43b1dda19d79f38df96c351abd7efdix100.png)

## [](#polygon)Polygon
Board, pieces and barriers are defined as (orthogonal) polygons. The shape is defined by a list of all corners. One has to take care, that the polygon is "closed", which means the first and last corner have to be identical. The corners (coordinates) are separated by a vertical bar \| ("pipe", shortcut "AltGr + <") and the complete list is terminated by quotation marks. E.g. "0,0 \| 3,0 \| 3,1 \| 1,1 \| 1,3 \| 0,3 \| 0,0" can be seen on the picture below. 

![Example shape](https://media-cdn.ubuntu-de.org/wiki/thumbnails/f/fd/fde89b64c128973aa87d82f4caac30c514172eb3ix100.png)

[back](./)
