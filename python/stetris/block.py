"""The falling block: a shape, a rotation and a position on the board."""

from . import blocks
from .board import Board

# Offsets tried when a rotation does not fit where the block stands
_KICKS = (0, -1, 1, -2, 2)


class Block:
    def __init__(self, board: Board, block_type: int) -> None:
        self._board = board
        self.reset(block_type)

    def reset(self, block_type: int) -> None:
        """Put the block back at its spawn position with the given type."""
        # Clamp so an out of range type cannot index past the table
        if not 1 <= block_type <= blocks.COUNT:
            block_type = 1

        self._type = block_type
        self._x_size = blocks.X_SIZE[block_type - 1]
        self._y_size = blocks.Y_SIZE[block_type - 1]
        self._x_pos = (self._board.x_size // 2) - (self._x_size // 2)
        self._y_pos = 0
        self._rotation = 0
        self._cells = [list(row) for row in blocks.SHAPE[block_type - 1][0]]

    def copy_from(self, other: "Block") -> None:
        """Take another block's shape and position, as an assignment would."""
        self._type = other._type
        self._x_size = other._x_size
        self._y_size = other._y_size
        self._x_pos = other._x_pos
        self._y_pos = other._y_pos
        self._rotation = other._rotation
        self._cells = [list(row) for row in other._cells]

    @property
    def type(self) -> int:
        return self._type

    @property
    def x_size(self) -> int:
        return self._x_size

    @property
    def y_size(self) -> int:
        return self._y_size

    @property
    def x_pos(self) -> int:
        return self._x_pos

    @property
    def y_pos(self) -> int:
        return self._y_pos

    def get(self, x: int, y: int) -> int:
        """Return a cell of the shape, or zero when out of bounds."""
        if not 0 <= x < blocks.SIZE or not 0 <= y < blocks.SIZE:
            return 0
        return self._cells[y][x]

    def rotate(self) -> bool:
        """Turn the block, nudging it sideways when it does not fit in place."""
        nxt = (self._rotation + 1) % blocks.ROTATIONS
        shape = blocks.SHAPE[self._type - 1][nxt]

        # Without the kicks a block could never be turned against a wall
        for kick in _KICKS:
            x = self._x_pos + kick
            if not self.fits(x, self._y_pos, shape):
                continue

            self._rotation = nxt
            self._x_pos = x
            self._cells = [list(row) for row in shape]
            return True

        return False

    def move_left(self) -> bool:
        if not self.fits(self._x_pos - 1, self._y_pos, self._cells):
            return False
        self._x_pos -= 1
        return True

    def move_right(self) -> bool:
        if not self.fits(self._x_pos + 1, self._y_pos, self._cells):
            return False
        self._x_pos += 1
        return True

    def move_down(self) -> bool:
        if not self.fits(self._x_pos, self._y_pos + 1, self._cells):
            return False
        self._y_pos += 1
        return True

    def drop(self) -> int:
        """Drop as far as the block goes, returning how many cells it fell."""
        distance = 0
        while self.move_down():
            distance += 1
        return distance

    def can_place(self) -> bool:
        """Return whether the block fits where it currently stands."""
        return self.fits(self._x_pos, self._y_pos, self._cells)

    def fits(self, x: int, y: int, shape) -> bool:
        """Return whether the given shape fits at the given position."""
        # Scan the whole grid, not just this block's extent - the shape
        # being tested may reach further than the current one
        for cy in range(blocks.SIZE):
            for cx in range(blocks.SIZE):
                if not shape[cy][cx]:
                    continue

                bx = x + cx
                by = y + cy

                if not 0 <= bx < self._board.x_size:
                    return False
                if not 0 <= by < self._board.y_size:
                    return False
                if self._board.get(bx, by):
                    return False

        return True
