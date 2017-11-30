package os2.group40.concurrency3;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.LockSupport;

import static os2.group40.concurrency3.LockType.*;

/**
 * A mutex which provides exclusions across three types of locking: reading,
 * insertion, and deletion.
 *
 * A read operation can occur at the same time as any other read, as it does
 * not modify the data. An insert operation must exclude other insertions,
 * but it does not exclude reads. A delete operation must exclude all other
 * operations including other deletions.
 *
 * @author Morgan Patch
 * @version 1.0
 * @see Mutex
 */
public class RIDMutex {

    private Map<Thread, LockType> locks = new HashMap<>();

    private Map<LockType, List<Thread>> queue = new HashMap<>();

    public RIDMutex() {
        queue.put(READ, new LinkedList<>());
        queue.put(INSERT, new LinkedList<>());
        queue.put(DELETE, new LinkedList<>());
    }

    /**
     * Returns whether it would be possible for a thread to request a lock,
     * if one were to request it.
     *
     * @param type The type of request to check
     * @return Whether a thread could lock the mutex
     */
    protected boolean canGetLock(LockType type) {
        return !isLocked(DELETE) && (type != INSERT || !isLocked(INSERT)) && (type != DELETE || locks.isEmpty());
    }

    /**
     * Grab a lock. If the requested type of lock can't be acquired, park the
     * thread until it is available.
     *
     * @param type The type of operation that it should be locked for
     */
    public void lock(LockType type) {
        if (!canGetLock(type)) {
            queue.get(type).add(Thread.currentThread());
            LockSupport.park(this);
            queue.get(type).remove(Thread.currentThread());
        }
        locks.put(Thread.currentThread(), type);
    }

    /**
     * Acquire the lock. If the requested tyoe of lock can't be acquired, park
     * the thread until it is available or the thread is interrupted.
     *
     * @param type The type of operation to lock the mutex for
     * @throws InterruptedException If the thread is interrupted before the lock is acquired
     */
    public void lockInterruptibly(LockType type) throws InterruptedException {
        if (Thread.interrupted()) {
            throw new InterruptedException();
        }
        if (!canGetLock(type)) {
            queue.get(type).add(Thread.currentThread());
            LockSupport.park(this);
            if (Thread.interrupted()) {
                throw new InterruptedException();
            }
            queue.get(type).remove(Thread.currentThread());
        }
        locks.put(Thread.currentThread(), type);
    }

    /**
     * Grab a lock of the given type if possible, but give up if the thread
     * would have to wait. This is guaranteed not to block.
     *
     * @param type The type of lock to acquire
     * @return Whether the lock could be successfully acquired
     */
    public boolean tryLock(LockType type) {
        if (!canGetLock(type)) {
            return false;
        }
        locks.put(Thread.currentThread(), type);
        return true;
    }

    /**
     * Grab a lock of the given type. If the requested lock is not available,
     * wait no longer than the given timeout before failing.
     *
     * @param type The type of lock to acquire
     * @param time The maximum time to wait for the lock
     * @param unit The time unit of the timeout
     *
     * @return Whether the lock could be acquired in the given time
     * @throws InterruptedException If the thread is interrupted before the lock is acquired
     */
    public boolean tryLock(LockType type, long time, TimeUnit unit) throws InterruptedException {
        if (Thread.interrupted()) {
            throw new InterruptedException();
        }
        long nanos = unit.toNanos(time);

        if (tryLock(type)) {
            return true;
        }

        long deadline = System.nanoTime() + nanos;
        boolean failed = true;
        try {
            while (true) {
                if (tryLock(type)) {
                    failed = false;
                    return true;
                }
                long timeout = deadline - System.nanoTime();
                if (timeout <= 0L) {
                    return false;
                }
                LockSupport.parkNanos(this, timeout);
                if (Thread.interrupted()) {
                    throw new InterruptedException();
                }
            }
        } finally {
            if (failed)
                queue.get(type).remove(Thread.currentThread());
        }
    }

    /**
     * Unlock the current thread, removing it from the mutex and releasing its control.
     * In doing so, schedule the next thread to lock the mutex, if available.
     */
    public void unlock() {
        Thread t = Thread.currentThread();

        if (!locks.containsKey(t)) {
            throw new IllegalMonitorStateException();
        }

        /*
         * This is really where things happen.
         *
         * We're going to prefer unparking reads over inserts, and inserts over deletes.
         *
         * To unpark a read, we just need to know that there are no active delete locks.
         * To unpark an insert, we need to know that there are no active deletes *and* no active
         * inserts. To unpark a delete, we need to know that there are no active locks of any kind.
         *
         * If we're unlocking a read, then we know that inserts and other reads have already been going
         * at the same time, so it's fine. We just need to check if our read is blocking any deletes.
         * If we're unlocking an insert, then other reads have been concurrent, but we may be blocking an
         * insert or a delete, so we need to check both. If we're unlocking a delete, then nothing else
         * has been happening at all, and we need to check whether we can unpark any type.
         */
        LockType type = locks.get(t);
        locks.remove(t);
        if (type == READ) {
            if (locks.isEmpty() && !queue.get(DELETE).isEmpty()) {
                LockSupport.unpark(queue.get(DELETE).get(0));
            }
        } else if (type == INSERT) {
             if (!isLocked(INSERT) && !isLocked(DELETE) && !queue.get(INSERT).isEmpty()) {
                LockSupport.unpark(queue.get(INSERT).get(0));
             } else if (locks.isEmpty() && !queue.get(DELETE).isEmpty()) {
                 LockSupport.unpark(queue.get(DELETE).get(0));
             }
        } else if (type == DELETE) {
            if (!isLocked(DELETE) && !queue.get(READ).isEmpty()) {
                LockSupport.unpark(queue.get(READ).get(0));
            } else if (!isLocked(DELETE) && !isLocked(INSERT) && !queue.get(INSERT).isEmpty()) {
                LockSupport.unpark(queue.get(INSERT).get(0));
            } else if (locks.isEmpty() && !queue.get(DELETE).isEmpty()) {
                LockSupport.unpark(queue.get(DELETE).get(0));
            }
        }
    }

    /**
     * Check if any threads are currently locking the mutex for a type.
     *
     * @param type The type of lock to check for
     * @return Whether the mutex is locked
     */
    public synchronized boolean isLocked(LockType type) {
        return locks.containsValue(type);
    }

    /**
     * Get the type of lock that the current thread has gotten, or {@code null} if no lock
     * is held by the current thread.
     *
     * @return This thread's lock type
     */
    public synchronized LockType currentLock() {
        return locks.get(Thread.currentThread());
    }
}