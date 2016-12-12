
public class QuicksortThread extends Quicksort implements Runnable {
    
    public QuicksortThread(int[] array) {
        super(array);
    }

    public QuicksortThread(int[] array, int lo, int hi) {
        super(array, lo, hi);
    }

    public void run() {
        sort();
    }
}
