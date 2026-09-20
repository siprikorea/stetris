package stetris;

/** The falling block: a shape, a rotation and a position on the board. */
public final class Block {
    /** Offsets tried when a rotation does not fit where the block stands. */
    private static final int[] KICKS = { 0, -1, 1, -2, 2 };

    private final Board board;
    private int type;
    private int xSize;
    private int ySize;
    private int xPos;
    private int yPos;
    private int rotation;
    private final int[][] cells = new int[Blocks.SIZE][Blocks.SIZE];

    public Block(Board board, int type) {
        this.board = board;
        reset(type);
    }

    /** Puts the block back at its spawn position with the given type. */
    public void reset(int type) {
        // Clamp so an out of range type cannot index past the table
        if (type < 1 || type > Blocks.COUNT) {
            type = 1;
        }

        this.type = type;
        this.xSize = Blocks.X_SIZE[type - 1];
        this.ySize = Blocks.Y_SIZE[type - 1];
        this.xPos = (board.getXSize() / 2) - (xSize / 2);
        this.yPos = 0;
        this.rotation = 0;
        copyShape(cells, Blocks.SHAPE[type - 1][rotation]);
    }

    /** Copies another block's shape and position, as a value assignment would. */
    public void copyFrom(Block other) {
        this.type = other.type;
        this.xSize = other.xSize;
        this.ySize = other.ySize;
        this.xPos = other.xPos;
        this.yPos = other.yPos;
        this.rotation = other.rotation;
        copyShape(this.cells, other.cells);
    }

    private static void copyShape(int[][] dst, int[][] src) {
        for (int y = 0; y < Blocks.SIZE; y++) {
            System.arraycopy(src[y], 0, dst[y], 0, Blocks.SIZE);
        }
    }

    public int getType() {
        return type;
    }

    public int getXSize() {
        return xSize;
    }

    public int getYSize() {
        return ySize;
    }

    public int getXPos() {
        return xPos;
    }

    public int getYPos() {
        return yPos;
    }

    /** Returns a cell of the shape, or zero when it is out of bounds. */
    public int getCell(int x, int y) {
        if (x < 0 || x >= Blocks.SIZE || y < 0 || y >= Blocks.SIZE) {
            return 0;
        }
        return cells[y][x];
    }

    /** Turns the block, nudging it sideways when it does not fit in place. */
    public boolean rotate() {
        int next = (rotation + 1) % Blocks.ROTATIONS;
        int[][] shape = Blocks.SHAPE[type - 1][next];

        // Without the kicks a block could never be turned against a wall
        for (int kick : KICKS) {
            int x = xPos + kick;
            if (!fits(x, yPos, shape)) {
                continue;
            }

            rotation = next;
            xPos = x;
            copyShape(cells, shape);
            return true;
        }

        return false;
    }

    public boolean moveLeft() {
        if (!fits(xPos - 1, yPos, cells)) {
            return false;
        }
        xPos--;
        return true;
    }

    public boolean moveRight() {
        if (!fits(xPos + 1, yPos, cells)) {
            return false;
        }
        xPos++;
        return true;
    }

    public boolean moveDown() {
        if (!fits(xPos, yPos + 1, cells)) {
            return false;
        }
        yPos++;
        return true;
    }

    /** Drops the block as far as it goes, returning how many cells it fell. */
    public int drop() {
        int distance = 0;
        while (moveDown()) {
            distance++;
        }
        return distance;
    }

    /** Returns whether the block fits where it currently stands. */
    public boolean canPlace() {
        return fits(xPos, yPos, cells);
    }

    /** Returns whether the given shape fits at the given position. */
    public boolean fits(int x, int y, int[][] shape) {
        // Scan the whole grid, not just this block's extent - the shape
        // being tested may reach further than the current one
        for (int cy = 0; cy < Blocks.SIZE; cy++) {
            for (int cx = 0; cx < Blocks.SIZE; cx++) {
                if (shape[cy][cx] == 0) {
                    continue;
                }

                int bx = x + cx;
                int by = y + cy;

                if (bx < 0 || bx >= board.getXSize()) {
                    return false;
                }
                if (by < 0 || by >= board.getYSize()) {
                    return false;
                }
                if (board.getValue(bx, by) != 0) {
                    return false;
                }
            }
        }

        return true;
    }
}
