package splintersoar;

import java.io.IOException;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;

import april.config.Config;
import april.config.ConfigFile;

public class Configuration {
	public class OrcInterface {
		public int updateHz = 30;
		public int[] ports = { 1, 0 };
		public boolean[] invert = { true, false };
		public double maxThrottleAccelleration = 2.0; // percent change per second
		public double baselineMeters = 0.383;
		public double tickMeters = 0.000043225;
		public double lengthMeters = 0.64;
		public double widthMeters = 0.42;
		public boolean usePF = true;
		public double laserThreshold = 0.1;
		public long pfUpdatePeriodNanos;
		
		public double maxThrottleChangePerUpdate; // percent change per update

		public OrcInterface(Config config) {
			double pfUpdatePeriod = 0.2;
			
			if (config != null) {
				updateHz = config.getInt("orc.updateHz", updateHz);
				ports = config.getInts("orc.ports", ports);
				invert = config.getBooleans("orc.invert", invert);
				maxThrottleAccelleration = config.getDouble("orc.maxThrottleAccelleration", maxThrottleAccelleration);
				baselineMeters = config.getDouble("orc.baselineMeters", baselineMeters);
				tickMeters = config.getDouble("orc.tickMeters", tickMeters);
				lengthMeters = config.getDouble("orc.lengthMeters", lengthMeters);
				widthMeters = config.getDouble("orc.widthMeters", widthMeters);
				usePF = config.getBoolean("orc.usePF", usePF);
				laserThreshold = config.getDouble("orc.laserThreshold", laserThreshold);
				pfUpdatePeriod = config.getDouble("orc.pfUpdatePeriod", pfUpdatePeriod);
			}
			
			maxThrottleChangePerUpdate = maxThrottleAccelleration / updateHz;
			pfUpdatePeriodNanos = (long)(1000000000L * pfUpdatePeriod);
		}
	}
	
	public class ParticleFilter {
		public int numParticles = 100;
		public boolean useRandomInitial = false;
		public double xySigma = 0.05;
		public double thetaSigma = Math.PI / 4;
		public int standingPopulation = 5;
		public boolean lcmDebug = false;
		
		public ParticleFilter(Config config) {
			if (config != null) {
				numParticles = config.getInt("pf.numParticles", numParticles);
				useRandomInitial = config.getBoolean("pf.useRandomInitial", useRandomInitial);
				xySigma = config.getDouble("pf.xySigma", xySigma);
				thetaSigma = config.getDouble("pf.thetaSigma", thetaSigma);
				standingPopulation = config.getInt("pf.standingPopulation", standingPopulation);
				lcmDebug = config.getBoolean("lcmDebug", lcmDebug);
			}
		}
	}
	
	public class LaserLoc {
		public double [] laserxyt = { 0, 0, 0 }; // laser location
		public double tubeRadius = 0; 
		public int updatePeriod = 5; // seconds between status updates
		public int activityTimeout = 5;
		public double[] maxRanges;
		public String mapFile = "map.txt";
		
		public long updatePeriodNanos;
		public long activityTimeoutNanos;

		public LaserLoc(Config config) {
			if (config != null) {				
				laserxyt = config.getDoubles("lloc.laserxyt", laserxyt);
				tubeRadius = config.getDouble("lloc.tubeRadius", tubeRadius);
				updatePeriod = config.getInt("lloc.updatePeriod", updatePeriod);
				activityTimeout = config.getInt("lloc.activityTimeout", activityTimeout);
				mapFile = config.getString("lloc.mapFile", mapFile);
			}

			try {
				Config mapConfig = new ConfigFile(mapFile).getConfig();
				maxRanges = mapConfig.getDoubles("map");
			} catch (IOException e) {
			}

			updatePeriodNanos = updatePeriod * 1000000000L;
			activityTimeoutNanos = activityTimeout * 1000000000L;
		}
	}

	public class SoarInterface {
		public String productions = "../agents/follower-robot.soar";

		public SoarInterface(Config config) {
			if (config != null) {
				productions = config.getString("soar.productions", productions);
			}
		}
	}

	
	public OrcInterface orc;
	public ParticleFilter pf;
	public LaserLoc lloc;
	public SoarInterface soar;
	
	public Level loglevel = Level.INFO;
	public boolean llocDisabled = false;
	public boolean orcDisabled = false;
	public boolean rangerDisabled = false;
	public boolean soarDisabled = false;
	public boolean gamepadDisabled = false;
	public boolean tankMode = false;
	
	ArrayList<String> warnings = new ArrayList<String>();
	
	public Configuration(Config config) {
		if (config != null) {
			llocDisabled = config.getBoolean("llocDisabled", llocDisabled);
			orcDisabled = config.getBoolean("orcDisabled", orcDisabled);
			rangerDisabled = config.getBoolean("rangerDisabled", rangerDisabled);
			soarDisabled = config.getBoolean("soarDisabled", soarDisabled);
			gamepadDisabled = config.getBoolean("gamepadDisabled", gamepadDisabled);
			tankMode = config.getBoolean("tankMode", tankMode);
		}
		
		orc = new OrcInterface(config);
		pf = new ParticleFilter(config);
		lloc = new LaserLoc(config);
		soar = new SoarInterface(config);
	}
	
	public void dumpWarnings(Logger logger) {
		for (String warning : warnings) {
			logger.warning(warning);
		}
	}
	
	public boolean everythingDisabled() {
		return llocDisabled && orcDisabled && rangerDisabled && soarDisabled && gamepadDisabled;
	}
}