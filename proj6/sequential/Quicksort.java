
public class Quicksort {

    private int hi;
    private int lo;

    private int[] arr;

    public Quicksort(int[] array) {
        this(array, 0, array.length-1);
    }

    public Quicksort(int[] array, int low, int high) {
        hi = high;
        lo = low;

        arr = array;
    }

    public void sort() {
       sort(lo, hi); 

        for (int i : arr) {
            System.out.print(i + " ");
        }
        System.out.println();
        System.out.println();
        System.out.println();
    }


    private void sort (int a, int b) {
        if (a >= b) {
            return;
        }

        int left = a;
        int right = b - 1;

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


        sort(a, left-1);
        sort(left+1, b);
    }

    public static void main(String[] args) {

        int[] a = {5, 2, 8, 3, 6, 2, 5, 8, 3, 0};

        Quicksort q = new Quicksort(a);
        
        for (int i : a) {
            System.out.print(i + " ");
        }
        System.out.println();

        q.sort();
        
        for (int i : a) {
            System.out.print(i + " ");
        }
        System.out.println();
    }
}
