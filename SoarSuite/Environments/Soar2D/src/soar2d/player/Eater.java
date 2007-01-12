package soar2d.player;

import soar2d.Soar2D;
import soar2d.World;

/**
 * @author voigtjr
 *
 * The base eater class, used for human eaters.
 * Soar eaters extend this class because they might one day need to share code with
 * human eaters.
 */
public class Eater extends Player {	
	protected ToscaEater m_ToscaEater = null ;
	
	public Eater( PlayerConfig playerConfig, boolean isHuman ) {
		super(playerConfig) ;

		if (isHuman && ToscaEater.kToscaEnabled)
			m_ToscaEater = new ToscaEater(this) ;
	}
	
	/* (non-Javadoc)
	 * @see soar2d.player.Player#update(soar2d.World, java.awt.Point)
	 */
	public void update(World world, java.awt.Point location) {
		super.update(world, location);
		
		if (ToscaEater.kToscaEnabled)
		{
			m_ToscaEater.update(world, location) ;
			return ;
		}
	}
	
	/* (non-Javadoc)
	 * @see soar2d.player.Player#getMove()
	 */
	public MoveInfo getMove() {
		if (ToscaEater.kToscaEnabled)
		{
			return m_ToscaEater.getMove() ;
		}

		// if we're not graphical, the human agent can't enter input.
		// maybe we should support this in the future.
		if (Soar2D.config.graphical == false) {
			return new MoveInfo();
		}
		MoveInfo move = Soar2D.wm.getHumanMove(this);
		// the facing depends on the move
		this.setFacingInt(move.moveDirection);
		return move;
	}
	/* (non-Javadoc)
	 * @see soar2d.player.Player#shutdown()
	 */
	public void shutdown() {
		if (ToscaEater.kToscaEnabled)
		{
			m_ToscaEater.shutdown() ;
		}
		
		// nothing to do
	}
}
