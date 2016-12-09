public class Quicksort {

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

    public void sort() {
       sort(low, high); 
    }


    private void sort (int a, int b) {
        if (a >= b) {
            return;
        }

        int left = a;
        int right = b;

        int middle = (left + right)/2;
        int pivot = triMedian(arr[left], arr[middle], arr[b]); // get pivot

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


        sort(a, left-1);
        sort(left, b);
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
