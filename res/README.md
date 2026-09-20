# Resources

| file | format | notes |
| --- | --- | --- |
| `stetris.ico` | 16, 32, 48, 64, 128, 256 | application icon |
| `stetris_icon.png` | 256 x 256, RGBA | the same icon as a plain image |
| `stetris_block.bmp` | 192 x 24, 24 bit | sprite sheet, eight 24 x 24 tiles |
| `make_icon.py` | | regenerates the two icon files |

Nothing in the project reads these today; the console UI draws with text.
They are here for a future graphical front end.

## Icon

The icon is the S block from `src/stblocks.cpp` on a rounded dark tile.
It is generated, not hand drawn, so the colours and sizes can be changed
in one place:

```sh
python3 res/make_icon.py
```

The script uses the standard library only. The 128 and 256 entries inside
the `.ico` are PNG compressed, which Windows Vista and later read and which
keeps the file at 37 KB instead of 361 KB.

## Block sheet

The tiles are laid out in one row and indexed by the cell value that
`CStBoard::GetValue()` returns, so tile 0 is the empty cell and tiles 1 to 7
are the block types in the order of `g_StBlocks` in `src/stblocks.cpp`:

```
 index   0      1      2      3      4      5      6      7
       empty    J      L      O      S      T      Z      I
        x=0    x=24   x=48   x=72   x=96  x=120  x=144  x=168
```

Blitting cell `(nX, nY)` means taking the tile at `24 * GetValue(nX, nY)`.
