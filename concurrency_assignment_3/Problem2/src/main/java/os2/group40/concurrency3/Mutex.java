package os2.group40.concurrency3;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.*;

/**
 * An implementation of a simple, non reentrant mutual exclusion
 * lock. If one thread locks the mutex, it maintains control over
 * the mutex until it unlocks, excluding all other threads.
 *
 * @author Morgan Patch
 * @version 1.0
 * @see java.util.concurrent.locks.Lock
 * @see java.util.concurrent.locks.AbstractQueuedSynchronizer
 */
public class Mutex implements Lock, java.io.Serializable {

    private final Sync sync = new Sync();

    /**
     * Acquire a lock on the mutex. The calling thread will go dormant if
     * the lock is unavailable.
     */
    @Override
    public void lock() {
        sync.acquire(1);
    }

    /**
     * Acquire a lock on the mutex. The calling thread will go dormant if
     * the lock is unavailable. If the thread is interrupted, the function
     * throws an interrupted exception and returns control to the caller.
     *
     * @throws InterruptedException If the calling thread is interrupted
     */
    @Override
    public void lockInterruptibly() throws InterruptedException {
        sync.acquireInterruptibly(1);
    }

    /**
     * Attempts to acquire a lock on the mutex if it is available. The function
     * is non-blocking, and always returns immediately.
     *
     * @return Whether the lock was successfully acquired
     */
    @Override
    public boolean tryLock() {
        return sync.tryAcquire(1);
    }

    /**
     * Attempts to acquire a lock on the mutex if it is available, or becomes
     * available within the given timeout.
     *
     * @param time The maximum amount of time to wait for the lock
     * @param unit The time unit of the time argument
     * @return Whether the lock was successfully acquired
     * @throws InterruptedException If the calling thread is interrupted before the lock is acquired
     */
    @Override
    public boolean tryLock(long time, TimeUnit unit) throws InterruptedException {
        return sync.tryAcquireNanos(1, unit.toNanos(time));
    }

    /**
     * Release the mutex. May throw a runtime exception if the calling thread
     * does not already have control over the mutex.
     */
    @Override
    public void unlock() {
        sync.release(1);
    }

    /**
     * Returns a condition bound to the mutex; to wait on the condition, the
     * current thread must first acquire the lock.
     *
     * This function returns a simple condition.
     *
     * @return A new {@link Condition} for the mutex
     */
    @Override
    public Condition newCondition() {
        return sync.newCondition();
    }

    /**
     * Checks if the mutex is currently locked by a thread.
     *
     * @return The current state of the mutex
     */
    public boolean isLocked() {
        return sync.isHeldExclusively();
    }

    /**
     * Checks if this thread is the current holder of the mutex.
     *
     * @return Does the current thread hold the mutex
     */
    public boolean doesCurrentHold() {
        return sync.isHeldByCurrent();
    }

    /**
     * Checks if there are any threads queueing to gain access to the mutex
     *
     * @return Are there queued threads waiting on the mutex
     */
    public boolean hasQueuedThreads() {
        return sync.hasQueuedThreads();
    }


    private static class Sync extends AbstractQueuedSynchronizer {

        protected boolean isHeldExclusively() {
            return getState() == 1;
        }

        protected boolean isHeldByCurrent() {
            return isHeldExclusively() && getExclusiveOwnerThread().equals(Thread.currentThread());
        }

        public boolean tryAcquire(int acquires) {
            assert acquires == 1;
            if (compareAndSetState(0, 1)) {
                setExclusiveOwnerThread(Thread.currentThread());
                return true;
            }
            return false;
        }

        protected boolean tryRelease(int releases) {
            assert releases == 1;
            if (getState() == 0) {
                throw new IllegalMonitorStateException();
            }
            setExclusiveOwnerThread(null);
            setState(0);
            return true;
        }

        Condition newCondition() {
            return new ConditionObject();
        }

        private void readObject(ObjectInputStream s) throws IOException, ClassNotFoundException {
            s.defaultReadObject();
            setState(0);
        }
    }
}