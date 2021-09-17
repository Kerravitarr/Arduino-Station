package Utils;

import java.util.TimerTask;

public class Timer extends java.util.Timer{
	public Timer( long delay, long period,TimerTask task){
		schedule(task, delay, period);
	}
	public Timer( long delay, TimerTask task){
		schedule(task, delay);
	}
}


/*

Timer timer = new java.util.Timer();
timer.schedule(new TimerTask() {
	@Override
	public void run() {
		if(!frame.draws.isRepaint) {
			frame.draws.isRepaint = true;
			frame.repaint();
		}
	}
}, 100,25);*/