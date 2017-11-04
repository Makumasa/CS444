import java.io.IOException;
import java.util.Random;
import java.util.concurrent.TimeUnit;

public class Philosopher implements Runnable {

    private static final String[] NAMES = {"Thomas Hobbes", "John Calvin", "John Locke", "Demosthenes", "David Hume"};

    static final Philosopher[] instances = new Philosopher[5];

    static {
        for (int i = 0; i < 5; i++) {
            instances[i] = new Philosopher(i);
        }
    }

    private int id;
    private String name;

    private Random random;

    Fork leftFork;
    Fork rightFork;

    boolean holdingLeft;
    boolean holdingRight;

    private Philosopher(int id) {
        this.id = id;
        this.name = NAMES[id];

        this.random = new Random();
    }

    int getId() {
        return id;
    }

    public void run() {
        if (holdingLeft) {
            leftFork.mutex.lock();
        }

        if (holdingRight) {
            rightFork.mutex.lock();
        }

        while (true) {
            try {
                think();
                getForks();
                eat();
                dropForks();
            } catch (InterruptedException e) {
                System.err.println("Caught an interrupted exception! Stack trace:");
                e.printStackTrace();
                break;
            }
        }
    }

    private void think() throws InterruptedException {
        Main.outputFile.println("[" + (System.currentTimeMillis() - Main.startTime) + "] " + this.toString() + " thinking.");
        Main.outputFile.flush();
        System.out.println(this.toString() + " is now thinking...");
        int time = random.nextInt(19000) + 1000;
        Thread.sleep(time);
        System.out.println(this.toString() + " has thought!");
        Main.outputFile.println("[" + (System.currentTimeMillis() - Main.startTime) + "] " + this.toString() + " done thinking.");
        Main.outputFile.flush();
    }

    private void getForks() throws InterruptedException {
        System.out.println(this.toString() + " is ready to grab forks.");
        while (!(leftFork.mutex.isHeldByCurrentThread() && rightFork.mutex.isHeldByCurrentThread())) {
            if (!leftFork.mutex.isHeldByCurrentThread() && leftFork.mutex.tryLock(random.nextInt(150) + 50, TimeUnit.MILLISECONDS)) {
                if (!leftFork.clean) {
                    System.out.println(this.toString() + " says " + leftFork.toString() + " is dirty; cleaning it and putting it back.");
                    leftFork.clean = true;
                    leftFork.mutex.unlock();
                } else {
                    System.out.println(this.toString() + " has picked up " + leftFork.toString() + " from the left.");
                    holdingLeft = true;
                }
            }

            if (!rightFork.mutex.isHeldByCurrentThread() && rightFork.mutex.tryLock(random.nextInt(150) + 50, TimeUnit.MILLISECONDS)) {
                if (!rightFork.clean) {
                    System.out.println(this.toString() + " says " + rightFork.toString() + " is dirty; cleaning it and putting it back.");
                    rightFork.clean = true;
                    rightFork.mutex.unlock();
                } else {
                    System.out.println(this.toString() + " has picked up " + rightFork.toString() + " from the right.");
                    holdingRight = true;
                }
            }
        }
    }

    private void eat() throws InterruptedException {
        Main.outputFile.println("[" + (System.currentTimeMillis() - Main.startTime) + "] " + this.toString() + " eating.");
        Main.outputFile.flush();
        System.out.println(this.toString() + " is eating...");
        int time = random.nextInt(8000) + 2000;
        Thread.sleep(time);
        System.out.println(this.toString() + " has eaten!");
        Main.outputFile.println("[" + (System.currentTimeMillis() - Main.startTime) + "] " + this.toString() + " done eating.");
        Main.outputFile.flush();
    }

    private void dropForks() {
        System.out.println(this.toString() + " is cleaning " + leftFork.toString() + " and placing it on the left.");
        leftFork.clean = true;
        leftFork.mutex.unlock();

        System.out.println(this.toString() + " is cleaning " + rightFork.toString() + " and placing it on the right.");
        rightFork.clean = true;
        rightFork.mutex.unlock();
    }

    @Override
    public boolean equals(Object obj) {
        return obj != null && obj instanceof Philosopher && ((Philosopher) obj).getId() == this.id;
    }

    @Override
    public int hashCode() {
        return 23093 * this.id;
    }

    @Override
    public String toString() {
        return "Philosopher '" + this.name + "' (id " + this.id + ")";
    }
}