import java.util.concurrent.locks.ReentrantLock;

public class Fork {
    private int id;

    boolean clean;

    public final ReentrantLock mutex = new ReentrantLock();

    static final Fork[] instances = new Fork[5];

    static {
        for (int i = 0; i < 5; i++) {
            instances[i] = new Fork(i);
        }
    }

    private Fork(int id) {
        this.id = id;
        this.clean = false;
    }

    private int getId() {
        return id;
    }

    @Override
    public boolean equals(Object obj) {
        return obj != null && obj instanceof Fork && this.id == ((Fork) obj).getId();

    }

    @Override
    public int hashCode() {
        return 3184 * id;
    }

    @Override
    public String toString() {
        return "Fork " + this.id + " (" + (this.clean ? "clean" : "dirty") + ") ";
    }
}