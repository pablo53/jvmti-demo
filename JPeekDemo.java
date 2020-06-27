
public class JPeekDemo {

    private static JPeekDemo instance = new JPeekDemo();

    public static JPeekDemo getInstance() {
        return instance;
    }

    private JPeekDemo() { }

    private void sayHello(String name) {
        System.out.println("Hello " + name + "!\n");
    }

    public void run(String args[]) {
        sayHello(args[0]);
    }

    public static void main(String args[]) throws InterruptedException {
        JPeekDemo.getInstance().run(args);
    }

}
