import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

public class Quicksort {

    protected int high;
    protected int low;

    protected int[] arr;

    public Quicksort(int[] array) {
        this(array, 0, array.length-1);
    }

    public Quicksort(int[] array, int lo, int hi) {
        low  = lo;
        high = hi;

        arr = array;
    }

    public void executorSort() {
        ExecutorService executor = Executors.newFixedThreadPool(10);
        AtomicInteger numToSort = new AtomicInteger(high-low+1);

        executor.execute(new QuicksortTask(arr, low, high, executor, numToSort));

        while (numToSort.get() != 0);

        executor.shutdown();
    }

    public void executorSort(ExecutorService es) {
        
    }


    public void sort() {
       sort(low, high); 
    }

    public void parallelSort() {

        Thread[] threads = new Thread[3];

        
        // Sort bottom half
        int mid = partition(low, high);
        if (low < mid-1) {
            int lo = partition(low, mid-1);

            threads[0] = new Thread(new QuicksortThread(arr, low, lo-1));
            threads[0].start();

            threads[1] = new Thread(new QuicksortThread(arr, lo+1, mid-1));
            threads[1].start();
        }


        // Sort top half
        if (mid < high) {
            int hi = partition(mid, high);

            threads[2] = new Thread(new QuicksortThread(arr, mid+1, hi-1));
            threads[2].start();

            sort(hi+1, high);
        }


        // Wait for all threads to finish
        for (Thread t : threads) {
            try {
                t.join();
            } catch (Exception e) { }   
        }
    }


    private void sort (int a, int b) {
        if (a >= b) {
            return;
        }
        
        int left = partition(a, b);

        sort(a, left-1);
        sort(left, b);
    }

    protected int partition(int a, int b) {
        int left = a;
        int right = b-1;

        setMedian(a, b);

        int pivot = arr[b]; // get pivot

        while (left <= right) {
            while (left <= right && arr[left] < pivot) {
                ++left;
            }

            while (left <= right && arr[right] > pivot) {
                --right;
            }

            if (left <= right) {
                int temp = arr[left];
                arr[left] = arr[right];
                arr[right] = temp;
                ++left;
                --right;
            }
        }

        int temp = arr[left];
        arr[left] = arr[b];
        arr[b] = temp;

        return left;
    }

    private void setMedian(int a, int b) {

        int middle = (a + b)/2;

        if (arr[a] > arr[middle]) {
            int t = arr[a];
            arr[a] = arr[middle];
            arr[middle] = t;
        }

        if (arr[b] > arr[middle]) {
            int t = arr[b];
            arr[b] = arr[middle];
            arr[middle] = t;
        }

        if (arr[a] > arr[b]) {
            int t = arr[a];
            arr[a] = arr[b];
            arr[b] = t;
        }
    }
}
