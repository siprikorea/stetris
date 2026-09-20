package stetris;

/** The well the blocks fall into. Each cell holds a block type, or zero. */
public final class Board {
    /** Width of the board in cells. */
    public static final int X_SIZE = 10;
    /** Height of the board in cells. */
    public static final int Y_SIZE = 20;

    private final int[][] cells = new int[Y_SIZE][X_SIZE];

    /** Empties the board. */
    public void clear() {
        for (int y = 0; y < Y_SIZE; y++) {
            java.util.Arrays.fill(cells[y], 0);
        }
    }

    /** Returns the width in cells. */
    public int getXSize() {
        return X_SIZE;
    }

    /** Returns the height in cells. */
    public int getYSize() {
        return Y_SIZE;
    }

    /** Returns the cell value, or zero when it is empty or out of bounds. */
    public int getValue(int x, int y) {
        if (x < 0 || x >= X_SIZE || y < 0 || y >= Y_SIZE) {
            return 0;
        }
        return cells[y][x];
    }

    /** Sets the cell value, ignoring positions outside the board. */
    public void setValue(int x, int y, int value) {
        if (x < 0 || x >= X_SIZE || y < 0 || y >= Y_SIZE) {
            return;
        }
        cells[y][x] = value;
    }
}
