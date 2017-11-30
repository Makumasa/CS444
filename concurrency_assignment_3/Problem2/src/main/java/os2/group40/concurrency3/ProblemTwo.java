package os2.group40.concurrency3;

import java.util.Random;

public class ProblemTwo {

    private static final int NUM_THREADS = 10;

    private static final Random rand = new Random();

    public static final long START_TIME = System.currentTimeMillis();

    private static class TestThread extends Thread {

        private static final RIDMutex mutex = new RIDMutex();

        private void print(String s) {
            StringBuilder sb = new StringBuilder();
            sb.append("[");
            sb.append(System.currentTimeMillis() - START_TIME);
            sb.append("] [Thread ");
            sb.append(this.getId());
            sb.append("]: ");
            sb.append(s);
            System.out.println(sb);
        }

        private void err(String s) {
            StringBuilder sb = new StringBuilder();
            sb.append("[");
            sb.append(System.currentTimeMillis() - START_TIME);
            sb.append("] [Thread ");
            sb.append(this.getId());
            sb.append("]: ");
            sb.append(s);
            System.err.println(sb);
        }

        @Override
        public void run() {
            while (!Thread.interrupted()) {
                try {
                    Thread.sleep(rand.nextInt(2000));
                } catch (InterruptedException e) {
                    err("Interrupted in the middle of sleeping!");
                    e.printStackTrace();
                    mutex.unlock();
                    return;
                }

                LockType type = LockType.randomType();

                print("Attempting to lock with type " + type.name());
                mutex.lock(type);
                long time = rand.nextInt(500);
                print("acquired " + type.name() + " lock; doing 'work' for " + time + " ms.");
                try {
                    Thread.sleep(time);
                } catch (InterruptedException e) {
                    err("Interrupted in the middle of doing 'work'");
                    e.printStackTrace();
                    mutex.unlock();
                    return;
                }
                print("Done 'working'; unlocking mutex");
                try {
                    mutex.unlock();
                } catch (Exception e) {
                    err("Got an exception while unlocking.");
                    e.printStackTrace();
                    return;
                }
                print("mutex unlocked; sleeping");
            }
        }
    }

    public static void main(String[] args) {
        int threadCount = NUM_THREADS;
        if (args.length == 1) {
            try {
                threadCount = Integer.parseUnsignedInt(args[0], 10);
            } catch (NumberFormatException e) {
                threadCount = NUM_THREADS;
                System.err.println("Argument was not numeric; using default thread count.");
            }
        }

        TestThread threads[] = new TestThread[threadCount];

        for (int i = 0; i < threadCount; i++) {
            threads[i] = new TestThread();
            threads[i].start();
        }

        for (TestThread thread : threads) {
            try {
                thread.join();
            } catch (InterruptedException e) {
            }
        }
    }
}
