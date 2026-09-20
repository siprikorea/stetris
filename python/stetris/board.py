"""The well the blocks fall into."""

X_SIZE = 10
Y_SIZE = 20


class Board:
    """A grid of cells, each holding a block type or zero when empty."""

    def __init__(self) -> None:
        self._cells = [[0] * X_SIZE for _ in range(Y_SIZE)]

    def clear(self) -> None:
        """Empty the board."""
        for row in self._cells:
            for x in range(X_SIZE):
                row[x] = 0

    @property
    def x_size(self) -> int:
        """Width in cells."""
        return X_SIZE

    @property
    def y_size(self) -> int:
        """Height in cells."""
        return Y_SIZE

    def get_value(self, x: int, y: int) -> int:
        """Return the cell value, or zero when empty or out of bounds."""
        if not 0 <= x < X_SIZE or not 0 <= y < Y_SIZE:
            return 0
        return self._cells[y][x]

    def set_value(self, x: int, y: int, value: int) -> None:
        """Set the cell value, ignoring positions outside the board."""
        if not 0 <= x < X_SIZE or not 0 <= y < Y_SIZE:
            return
        self._cells[y][x] = value
