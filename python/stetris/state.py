"""What the game is doing right now."""

from enum import Enum


class State(Enum):
    PLAYING = "playing"
    PAUSED = "paused"
    GAMEOVER = "gameover"
