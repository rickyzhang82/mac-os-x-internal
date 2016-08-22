// MyThread.java
   
import java.lang.ThreadGroup;
import java.lang.Thread;
   
class MyThread extends Thread {
   
    MyThread(ThreadGroup group, String name) {
        super(group, name);
    }
   
    public void run() {
        for (int i = 0; i < 128; i++)
            System.out.print(this.getName());
    }
}
   
class DemoApp {
    public static void main(String[] args) {
        ThreadGroup allThreads = new ThreadGroup("Threads");
        MyThread t1 = new MyThread(allThreads, "1");
        MyThread t2 = new MyThread(allThreads, "2");
        MyThread t3 = new MyThread(allThreads, "3");
        allThreads.list();
        t1.setPriority(Thread.MIN_PRIORITY);
        t2.setPriority((Thread.MAX_PRIORITY + 
                        Thread.MIN_PRIORITY) / 2);
        t3.setPriority(Thread.MAX_PRIORITY);
        t1.start();
        t2.start();
        t3.start();
    }
}
