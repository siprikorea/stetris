package stetris;

/**
 * The game. It knows nothing about how it is presented: a front end calls
 * the input methods, hands it elapsed time through {@link #tick(int)} and
 * reads the state back to draw it.
 */
public final class Play {
    /** Score for clearing one to four lines at once, times the level. */
    private static final long[] LINE_SCORE = { 0, 100, 300, 500, 800 };

    private static final int SOFT_DROP_SCORE = 1;
    private static final int HARD_DROP_SCORE = 2;

    private static final int LINES_PER_LEVEL = 10;

    private static final int FALL_INTERVAL_BASE = 800;
    private static final int FALL_INTERVAL_STEP = 70;
    private static final int FALL_INTERVAL_MIN = 100;

    private final Board board = new Board();
    private final Block currentBlock = new Block(board, 1);
    private final Block nextBlock = new Block(board, 1);
    private final Score score = new Score();
    private final Score highScore = new Score();
    private final Rng rng = new Rng();

    private State state = State.PLAYING;
    private int lines;
    private int fallTimer;

    public Play() {
        newGame(1);
    }

    /**
     * Starts a new game. The caller supplies the seed, so a given seed always
     * replays the same game. The high score is kept.
     */
    public void newGame(int seed) {
        rng.seed(seed);
        board.clear();
        score.clear();
        state = State.PLAYING;
        lines = 0;
        fallTimer = 0;

        currentBlock.reset(nextBlockType());
        nextBlock.reset(nextBlockType());
    }

    public boolean moveLeft() {
        return isPlaying() && currentBlock.moveLeft();
    }

    public boolean moveRight() {
        return isPlaying() && currentBlock.moveRight();
    }

    public boolean rotate() {
        return isPlaying() && currentBlock.rotate();
    }

    /** Moves the block down one cell, locking it when it has landed. */
    public boolean softDrop() {
        if (!isPlaying()) {
            return false;
        }

        // A manual drop restarts the fall timer, so the block does not
        // immediately fall again on the next tick
        fallTimer = 0;

        if (!currentBlock.moveDown()) {
            lockBlock();
            return false;
        }

        score.add(SOFT_DROP_SCORE);
        updateHighScore();
        return true;
    }

    /** Drops the block all the way and locks it. */
    public boolean hardDrop() {
        if (!isPlaying()) {
            return false;
        }

        int distance = currentBlock.drop();
        score.add((long) HARD_DROP_SCORE * distance);
        updateHighScore();

        lockBlock();
        return true;
    }

    /** Advances the game by the elapsed time, in milliseconds. */
    public void tick(int elapsedMillis) {
        if (!isPlaying()) {
            return;
        }

        fallTimer += elapsedMillis;

        int interval = getFallInterval();

        // Catch up if more than one interval has passed
        while (fallTimer >= interval && isPlaying()) {
            fallTimer -= interval;
            applyGravity();
            interval = getFallInterval();
        }
    }

    /** Pauses or resumes. A finished game cannot be resumed. */
    public void setPause(boolean pause) {
        if (state == State.GAMEOVER) {
            return;
        }
        state = pause ? State.PAUSED : State.PLAYING;
    }

    public void togglePause() {
        setPause(state == State.PLAYING);
    }

    public State getState() {
        return state;
    }

    public boolean isPlaying() {
        return state == State.PLAYING;
    }

    public boolean isPaused() {
        return state == State.PAUSED;
    }

    public boolean isGameOver() {
        return state == State.GAMEOVER;
    }

    public Board getBoard() {
        return board;
    }

    public Block getCurrentBlock() {
        return currentBlock;
    }

    public Block getNextBlock() {
        return nextBlock;
    }

    public Score getScore() {
        return score;
    }

    public Score getHighScore() {
        return highScore;
    }

    /** Returns the level, starting at one. */
    public int getLevel() {
        return (lines / LINES_PER_LEVEL) + 1;
    }

    /** Returns how many lines have been cleared. */
    public int getLines() {
        return lines;
    }

    /** Returns the current fall interval in milliseconds. */
    public int getFallInterval() {
        int interval = FALL_INTERVAL_BASE - (getLevel() - 1) * FALL_INTERVAL_STEP;
        return Math.max(interval, FALL_INTERVAL_MIN);
    }

    /** Falling on its own scores nothing. */
    private void applyGravity() {
        if (!currentBlock.moveDown()) {
            lockBlock();
        }
    }

    private void lockBlock() {
        setBlockToBoard();

        int cleared = clearCompleteLines();
        if (cleared > 0) {
            lines += cleared;

            if (cleared > 4) {
                cleared = 4;
            }

            // Scored after the level has taken the new lines into account
            score.add(LINE_SCORE[cleared] * getLevel());
            updateHighScore();
        }

        changeBlock();
        fallTimer = 0;
    }

    private void setBlockToBoard() {
        for (int y = 0; y < currentBlock.getYSize(); y++) {
            for (int x = 0; x < currentBlock.getXSize(); x++) {
                if (currentBlock.getCell(x, y) != 0) {
                    board.setValue(currentBlock.getXPos() + x,
                            currentBlock.getYPos() + y, currentBlock.getType());
                }
            }
        }
    }

    /** Removes every complete line, returning how many there were. */
    private int clearCompleteLines() {
        int xSize = board.getXSize();
        int ySize = board.getYSize();
        int cleared = 0;

        for (int y = ySize - 1; y >= 0; ) {
            int count = 0;
            for (int x = 0; x < xSize; x++) {
                if (board.getValue(x, y) != 0) {
                    count++;
                }
            }

            if (count != xSize) {
                // Not complete, look at the line above
                y--;
                continue;
            }

            // Pull everything above down by one, then empty the top line
            for (int move = y; move > 0; move--) {
                for (int x = 0; x < xSize; x++) {
                    board.setValue(x, move, board.getValue(x, move - 1));
                }
            }
            for (int x = 0; x < xSize; x++) {
                board.setValue(x, 0, 0);
            }

            cleared++;
        }

        return cleared;
    }

    private void changeBlock() {
        currentBlock.copyFrom(nextBlock);
        nextBlock.reset(nextBlockType());

        // The new block does not fit where it spawns, so the stack has
        // reached the top
        if (!currentBlock.canPlace()) {
            state = State.GAMEOVER;
        }
    }

    private int nextBlockType() {
        return rng.nextRange(1, Blocks.COUNT);
    }

    private void updateHighScore() {
        if (highScore.get() < score.get()) {
            highScore.set(score.get());
        }
    }
}
