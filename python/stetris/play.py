"""The game itself."""

from . import blocks
from .block import Block
from .board import Board
from .rng import Rng
from .score import Score
from .state import State

# Score for clearing one to four lines at once, times the level
_LINE_SCORE = (0, 100, 300, 500, 800)

_SOFT_DROP_SCORE = 1
_HARD_DROP_SCORE = 2

_LINES_PER_LEVEL = 10

_FALL_INTERVAL_BASE = 800
_FALL_INTERVAL_STEP = 70
_FALL_INTERVAL_MIN = 100


class Play:
    """The game. It knows nothing about how it is presented."""

    def __init__(self) -> None:
        self._board = Board()
        self._current_block = Block(self._board, 1)
        self._next_block = Block(self._board, 1)
        self._score = Score()
        self._high_score = Score()
        self._rng = Rng()
        self._state = State.PLAYING
        self._lines = 0
        self._fall_timer = 0

        self.new_game(1)

    def new_game(self, seed: int) -> None:
        """Start a new game.

        The caller supplies the seed, so a given seed always replays the same
        game. The high score is kept.
        """
        self._rng.seed(seed)
        self._board.clear()
        self._score.clear()
        self._state = State.PLAYING
        self._lines = 0
        self._fall_timer = 0

        self._current_block.reset(self._next_block_type())
        self._next_block.reset(self._next_block_type())

    def move_left(self) -> bool:
        return self.is_playing and self._current_block.move_left()

    def move_right(self) -> bool:
        return self.is_playing and self._current_block.move_right()

    def rotate(self) -> bool:
        return self.is_playing and self._current_block.rotate()

    def soft_drop(self) -> bool:
        """Move the block down one cell, locking it when it has landed."""
        if not self.is_playing:
            return False

        # A manual drop restarts the fall timer, so the block does not
        # immediately fall again on the next tick
        self._fall_timer = 0

        if not self._current_block.move_down():
            self._lock_block()
            return False

        self._score.add(_SOFT_DROP_SCORE)
        self._update_high_score()
        return True

    def hard_drop(self) -> bool:
        """Drop the block all the way and lock it."""
        if not self.is_playing:
            return False

        distance = self._current_block.drop()
        self._score.add(_HARD_DROP_SCORE * distance)
        self._update_high_score()

        self._lock_block()
        return True

    def tick(self, elapsed_millis: int) -> None:
        """Advance the game by the elapsed time, in milliseconds."""
        if not self.is_playing:
            return

        self._fall_timer += elapsed_millis
        interval = self.fall_interval

        # Catch up if more than one interval has passed
        while self._fall_timer >= interval and self.is_playing:
            self._fall_timer -= interval
            self._apply_gravity()
            interval = self.fall_interval

    def set_pause(self, pause: bool) -> None:
        """Pause or resume. A finished game cannot be resumed."""
        if self._state is State.GAMEOVER:
            return
        self._state = State.PAUSED if pause else State.PLAYING

    def toggle_pause(self) -> None:
        self.set_pause(self._state is State.PLAYING)

    @property
    def state(self) -> State:
        return self._state

    @property
    def is_playing(self) -> bool:
        return self._state is State.PLAYING

    @property
    def is_paused(self) -> bool:
        return self._state is State.PAUSED

    @property
    def is_game_over(self) -> bool:
        return self._state is State.GAMEOVER

    @property
    def board(self) -> Board:
        return self._board

    @property
    def current_block(self) -> Block:
        return self._current_block

    @property
    def next_block(self) -> Block:
        return self._next_block

    @property
    def score(self) -> Score:
        return self._score

    @property
    def high_score(self) -> Score:
        return self._high_score

    @property
    def level(self) -> int:
        """The level, starting at one."""
        return (self._lines // _LINES_PER_LEVEL) + 1

    @property
    def lines(self) -> int:
        """How many lines have been cleared."""
        return self._lines

    @property
    def fall_interval(self) -> int:
        """The current fall interval in milliseconds."""
        interval = _FALL_INTERVAL_BASE - (self.level - 1) * _FALL_INTERVAL_STEP
        return max(interval, _FALL_INTERVAL_MIN)

    def _apply_gravity(self) -> None:
        # Falling on its own scores nothing
        if not self._current_block.move_down():
            self._lock_block()

    def _lock_block(self) -> None:
        self._set_block_to_board()

        cleared = self._clear_complete_lines()
        if cleared:
            self._lines += cleared
            cleared = min(cleared, 4)

            # Scored after the level has taken the new lines into account
            self._score.add(_LINE_SCORE[cleared] * self.level)
            self._update_high_score()

        self._change_block()
        self._fall_timer = 0

    def _set_block_to_board(self) -> None:
        block = self._current_block
        for y in range(block.y_size):
            for x in range(block.x_size):
                if block.get_cell(x, y):
                    self._board.set_value(block.x_pos + x, block.y_pos + y, block.type)

    def _clear_complete_lines(self) -> int:
        """Remove every complete line, returning how many there were."""
        x_size = self._board.x_size
        y_size = self._board.y_size
        cleared = 0

        y = y_size - 1
        while y >= 0:
            count = sum(1 for x in range(x_size) if self._board.get_value(x, y))

            if count != x_size:
                # Not complete, look at the line above
                y -= 1
                continue

            # Pull everything above down by one, then empty the top line
            for move in range(y, 0, -1):
                for x in range(x_size):
                    self._board.set_value(x, move, self._board.get_value(x, move - 1))
            for x in range(x_size):
                self._board.set_value(x, 0, 0)

            cleared += 1

        return cleared

    def _change_block(self) -> None:
        self._current_block.copy_from(self._next_block)
        self._next_block.reset(self._next_block_type())

        # The new block does not fit where it spawns, so the stack has
        # reached the top
        if not self._current_block.can_place():
            self._state = State.GAMEOVER

    def _next_block_type(self) -> int:
        return self._rng.next_range(1, blocks.COUNT)

    def _update_high_score(self) -> None:
        if self._high_score.get() < self._score.get():
            self._high_score.set(self._score.get())
