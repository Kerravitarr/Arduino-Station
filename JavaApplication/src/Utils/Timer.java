package Utils;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.TimerTask;

public class Timer{
	
	private class  AL implements ActionListener{
		RUN task = null;
		public AL(RUN run) {task = run;}
		public void actionPerformed(ActionEvent e) {
			task.run();
			if (period > 0) start();
			else stop();
		}
		
	}
	
	javax.swing.Timer timer;
	private int period = -1;
	
	public interface RUN {public void run();}
	
	public Timer( int delay, RUN run){
		timer = new javax.swing.Timer(delay, new AL(run));
		timer.start();
	}

	public Timer(int delay, int period, RUN run) {
		this(delay, run);
		this.period = period;
	}
	
	public void stop() {
		timer.stop();
	}
	public void start() {
		timer.setDelay(period);
		timer.restart();
	}
}