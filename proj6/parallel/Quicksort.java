public class Quicksort implements Runnable {

    private int high;
    private int low;

    private int[] arr;

    public Quicksort(int[] array) {
        this(array, 0, array.length-1);
    }

    public Quicksort(int[] array, int lo, int hi) {
        low  = lo;
        high = hi;

        arr = array;
    }

    public void run() {
        sort();
    }

    public void sort() {
       sort(low, high); 
    }

    public void parallelSort() {

        Thread[] threads = new Thread[3];

        int mid = partition(low, high);
        if (low < mid-1) {
            int lo = partition(low, mid-1);

            threads[0] = new Thread(new Quicksort(arr, low, lo-1));
            threads[0].start();

            threads[1] = new Thread(new Quicksort(arr, lo+1, mid-1));
            threads[1].start();
        }

        if (mid < high) {
            int hi = partition(mid, high);

            threads[2] = new Thread(new Quicksort(arr, mid+1, hi-1));
            threads[2].start();

            sort(hi+1, high);
        }

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

    private int partition(int a, int b) {
        int left = a;
        int right = b;

        int middle = (left + right)/2;

        if (arr[left] > arr[middle]) {
            int t = arr[left];
            arr[left] = arr[middle];
            arr[middle] = t;
        }

        if (arr[right] > arr[middle]) {
            int t = arr[right];
            arr[right] = arr[middle];
            arr[middle] = t;
        }

        if (arr[left] > arr[right]) {
            int t = arr[left];
            arr[left] = arr[right];
            arr[right] = t;
        }


        int pivot = arr[right--]; // get pivot

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

    private static int triMedian(int a, int b, int c) {
        if (a > b) {
            if (c > a) {
                return a;
            } else if (b > c) {
                return b;
            } else {
                return c;
            }
        } else {
            if (c > b) {
                return b;
            } else if (a > c) {
                return a;
            } else {
                return c;
            }
        }
    }
}