package os2.group40.concurrency3;

import java.util.Random;

public enum LockType {
    READ,
    INSERT,
    DELETE;

    private static final Random rand = new Random();

    public static LockType randomType() {
        int val = rand.nextInt(3);
        switch (val) {
        case 0:
            return READ;
        case 1:
            return INSERT;
        case 2:
            return DELETE;
        default:
            return READ;
        }
    }
}