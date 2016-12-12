import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.atomic.AtomicInteger;

public class QuicksortTask extends Quicksort implements Runnable {
    
    ExecutorService executor;
    AtomicInteger numToSort;

    public QuicksortTask(int[] array, ExecutorService ex, AtomicInteger ai) {
        super(array);
        executor = ex;
        numToSort = ai;
    }

    public QuicksortTask(int[] array, int lo, int hi, ExecutorService ex, AtomicInteger ai) {
        super(array, lo, hi);
        executor = ex;
        numToSort = ai;
    }

    public void run() {

        if (high - low < 1000) {
            sort();
            numToSort.addAndGet(low-high-1);
        } else {
            int mid = partition(low, high);
            numToSort.decrementAndGet();
            executor.execute(new QuicksortTask(arr, low, mid-1, executor, numToSort));
            executor.execute(new QuicksortTask(arr, mid+1, high, executor, numToSort));
        }
    }
}
