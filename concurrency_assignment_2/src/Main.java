import java.io.*;

public class Main {

    static final long startTime = System.currentTimeMillis();

    static PrintWriter outputFile;

    public static void main(String[] args) {

        try {
            outputFile = new PrintWriter(new BufferedWriter(new FileWriter("timing.txt")));
        } catch (IOException e) {
            System.err.println("Error opening output file!");
            e.printStackTrace();
            System.exit(-1);
        }

        for (int i = 0; i < 5; i++) {

            Philosopher.instances[i].leftFork = Fork.instances[i];
            Philosopher.instances[i].rightFork = Fork.instances[i == 0 ? 4 : i - 1];
        }

        Philosopher.instances[0].holdingLeft = true;
        Philosopher.instances[0].holdingRight = true;

        Philosopher.instances[1].holdingLeft = true;
        Philosopher.instances[2].holdingLeft = true;
        Philosopher.instances[3].holdingLeft = true;

        for (int i = 0; i < 5; i++) {
            new Thread(Philosopher.instances[i]).start();
        }
    }
}