# Resources

Kept from the old MFC UI for a future graphical front end. Nothing in the
project reads these files today; the console UI draws with text.

| file | format | notes |
| --- | --- | --- |
| `stetris_block.bmp` | 192 x 24, 24 bit | sprite sheet, eight 24 x 24 tiles |
| `stetris_background.bmp` | 1024 x 768, 24 bit | window background |
| `stetris.ico` | 13 icons, 48x48 down to 16x16 | application icon |

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
