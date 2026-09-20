package app.console;

import stetris.Block;
import stetris.Board;
import stetris.Play;

/**
 * Renders a {@link Play} to the terminal.
 *
 * <p>The logic knows nothing about this class - the app pulls the state it
 * needs and draws it.
 */
public final class ConsoleView {
    // Screen control
    private static final String ESC_HIDE_CURSOR = "\u001b[?25l";
    private static final String ESC_SHOW_CURSOR = "\u001b[?25h";
    private static final String ESC_CLEAR_SCREEN = "\u001b[2J";
    private static final String ESC_HOME = "\u001b[H";
    private static final String ESC_CLEAR_LINE = "\u001b[K";
    private static final String ESC_RESET = "\u001b[0m";

    private static final String CELL_FILLED = "[]";
    private static final String CELL_EMPTY = " .";

    /** Side panel column, one based. */
    private static final int SIDE_COL = 28;

    /** Colour per block type, index 0 is the empty cell. */
    private static final String[] COLORS = {
        "\u001b[0;90m", // empty - gray
        "\u001b[1;34m", // J - blue
        "\u001b[1;37m", // L - white
        "\u001b[1;33m", // O - yellow
        "\u001b[1;32m", // S - green
        "\u001b[1;35m", // T - magenta
        "\u001b[1;31m", // Z - red
        "\u001b[1;36m", // I - cyan
    };

    private final StringBuilder out = new StringBuilder();

    public void enterScreen() {
        System.out.print(ESC_HIDE_CURSOR + ESC_CLEAR_SCREEN + ESC_HOME);
        System.out.flush();
    }

    public void leaveScreen() {
        System.out.print(ESC_RESET + ESC_SHOW_CURSOR + "\n");
        System.out.flush();
    }

    /** Draws one frame. */
    public void render(Play play) {
        out.setLength(0);
        out.append(ESC_HOME);

        drawBoard(play);
        drawHelp();

        // Flush the whole frame at once
        System.out.print(out);
        System.out.flush();
    }

    private void putCell(int type) {
        out.append(type >= 0 && type < COLORS.length ? COLORS[type] : COLORS[0])
           .append(type != 0 ? CELL_FILLED : CELL_EMPTY)
           .append(ESC_RESET);
    }

    private void drawBoard(Play play) {
        Board board = play.getBoard();
        Block block = play.getCurrentBlock();
        int xSize = board.getXSize();
        int ySize = board.getYSize();

        // Merge the board and the falling block into one buffer
        int[][] screen = new int[ySize][xSize];
        for (int y = 0; y < ySize; y++) {
            for (int x = 0; x < xSize; x++) {
                screen[y][x] = board.getValue(x, y);
            }
        }

        // A locked block is drawn from the board, so only overlay the
        // falling one while the game is running
        if (!play.isGameOver()) {
            for (int by = 0; by < block.getYSize(); by++) {
                for (int bx = 0; bx < block.getXSize(); bx++) {
                    if (block.getCell(bx, by) == 0) {
                        continue;
                    }
                    int x = block.getXPos() + bx;
                    int y = block.getYPos() + by;
                    if (x >= 0 && x < xSize && y >= 0 && y < ySize) {
                        screen[y][x] = block.getType();
                    }
                }
            }
        }

        out.append("\u001b[1;37m       S T E T R I S").append(ESC_RESET)
           .append(ESC_CLEAR_LINE).append('\n');
        out.append("  +").append("--".repeat(xSize)).append('+')
           .append(ESC_CLEAR_LINE).append('\n');

        for (int y = 0; y < ySize; y++) {
            out.append("  |");
            for (int x = 0; x < xSize; x++) {
                putCell(screen[y][x]);
            }
            out.append('|');

            // Side panel on the same line
            out.append("\u001b[").append(SIDE_COL).append('G');
            drawSide(play, y);

            out.append(ESC_CLEAR_LINE).append('\n');
        }

        out.append("  +").append("--".repeat(xSize)).append('+')
           .append(ESC_CLEAR_LINE).append('\n');
    }

    private void drawSide(Play play, int line) {
        switch (line) {
            case 0 -> out.append("\u001b[1;37mNEXT").append(ESC_RESET);
            case 1, 2, 3, 4 -> {
                // The preview is drawn at the origin of the panel, the spawn
                // position of the next block is irrelevant here
                Block next = play.getNextBlock();
                int by = line - 1;
                for (int bx = 0; bx < 4; bx++) {
                    putCell(next.getCell(bx, by) != 0 ? next.getType() : 0);
                }
            }
            case 6 -> out.append("\u001b[1;37mSCORE").append(ESC_RESET);
            case 7 -> out.append(play.getScore().get());
            case 9 -> out.append("\u001b[1;37mHIGH SCORE").append(ESC_RESET);
            case 10 -> out.append(play.getHighScore().get());
            case 12 -> out.append("\u001b[1;37mLEVEL").append(ESC_RESET);
            case 13 -> out.append(play.getLevel());
            case 15 -> out.append("\u001b[1;37mLINES").append(ESC_RESET);
            case 16 -> out.append(play.getLines());
            case 18 -> {
                if (play.isGameOver()) {
                    out.append("\u001b[1;31mGAME OVER").append(ESC_RESET);
                } else if (play.isPaused()) {
                    out.append("\u001b[1;33mPAUSED").append(ESC_RESET);
                }
            }
            case 19 -> {
                if (play.isGameOver()) {
                    out.append("\u001b[1;31mpress R to restart").append(ESC_RESET);
                }
            }
            default -> { }
        }
    }

    private void drawHelp() {
        out.append(ESC_CLEAR_LINE).append('\n');
        out.append("  \u001b[0;90mLeft/Right: move   Up: rotate   Down: soft drop")
           .append(ESC_RESET).append(ESC_CLEAR_LINE).append('\n');
        out.append("  \u001b[0;90mSpace: hard drop   P: pause     R: restart   Q: quit")
           .append(ESC_RESET).append(ESC_CLEAR_LINE).append('\n');
    }
}
